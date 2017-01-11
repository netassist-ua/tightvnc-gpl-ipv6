// Copyright (C) 2009,2010,2011,2012 GlavSoft LLC.
// All rights reserved.
//
//-------------------------------------------------------------------------
// This file is part of the TightVNC software.  Please visit our Web site:
//
//                       http://www.tightvnc.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

#include <string.h>
#include <stdlib.h>
#include "HostPath.h"

const size_t HostPath::m_VNC_HOST_MAX_CHARS = 255;
const size_t HostPath::m_VNC_PORT_MAX_CHARS = 5;

HostPath::HostPath()
  : m_path(0),
    m_vncHost(0),
    m_vncPort(0),
    m_defaultPort(5900)
{
}

HostPath::HostPath(const char *path, int defaultPort)
  : m_path(0),
    m_vncHost(0),
    m_vncPort(0),
    m_defaultPort(defaultPort)
{
  set(path);
}

HostPath::~HostPath()
{
  clear();
}

bool
HostPath::set(const char *path)
{
  // Forget previous path if one was set earlier.
  clear();

  // Compute the maximal length of a valid path. Note that there can be no
  // more than five delimiter characters: "user@sshhost:port/vnchost::port".
  const size_t MAX_PATH_LEN =
     (m_VNC_HOST_MAX_CHARS + m_VNC_PORT_MAX_CHARS + 5);

  // Check the path length and save a copy for this object.
  if (path == 0) {
    return false;
  }
  size_t pathLen = strlen(path);
  if (pathLen < 1 || pathLen > MAX_PATH_LEN) {
    return false;
  }
  m_path = new char[pathLen + 1];
  memcpy(m_path, path, pathLen);
  m_path[pathLen] = '\0';

  // Perform initial parsing and checking of the path.
  size_t tokens[3];
  parsePath(tokens);
  if ( tokens[0] + tokens[1] == 0 ) {
    clear();
    return false;
  }
  const char *tokenStart = m_path;

  // Handle VNC host name.
  const char* hostStart = tokenStart + tokens[0];
  size_t hostLen = tokens[1];
  if (tokens[1] == 0) {
    hostStart = "localhost";
    hostLen = 9; // strlen("localhost")
  } else {
    if (hostLen > m_VNC_HOST_MAX_CHARS) {
      clear();
      return false;
    }
  }
  m_vncHost = new char[hostLen + 1];
  memcpy(m_vncHost, hostStart, hostLen);
  m_vncHost[hostLen] = '\0';
  tokenStart += tokens[1] + tokens[0];

  // Handle VNC display or port number.
  if (tokens[2] == 0) {
    m_vncPort = m_defaultPort;
  } else {
	if(tokens[0] != 0){
		//Include IPv6 address delimiter...
		tokenStart += 1;
	}
    size_t portLen = tokens[2] - 1;
    if (portLen < 1 || *tokenStart != ':') {
      clear();
      return false;
    }
    const char* portStart = tokenStart + 1;
    bool twoColons = (*portStart == ':');
    if (twoColons) {
      portStart++;
      portLen--;
    }
    if (portLen < 1 ||
        portLen > m_VNC_PORT_MAX_CHARS ||
        strspn(portStart, "0123456789") != portLen) {
      clear();
      return false;
    }
    m_vncPort = atoi(portStart);
    if (!twoColons && m_vncPort >= 0 && m_vncPort <= 99) {
      m_vncPort += m_defaultPort;
    }
  }

  // Perform strict validation of m_vncHost (and m_sshHost if present).
  if (!validateHostNames()) {
    clear();
    return false;
  }

  return true;
}

void
HostPath::clear()
{
  if (m_path != 0) {
    delete[] m_path;
    m_path = 0;
  }
  if (m_vncHost != 0) {
    delete[] m_vncHost;
    m_vncHost = 0;
  }
  m_vncPort = 0;
}

void
HostPath::parsePath(size_t results[]) const
{
  bool v6_address = false;
  char * v6delim_end = NULL;
  memset(results, 0, 3 * sizeof(size_t));

  if (m_path == 0)
    return;

  const char* vncHostStart = m_path;
  results[ 0 ] = 0; //by default hostname starts from the beginning
  if( vncHostStart[ 0 ] == '[' ) { // we have an IPv6 address delimiter
		results [ 0 ] = 1;
		v6delim_end = (char *) strchr(vncHostStart, ']');
		if( v6delim_end == NULL )
			return;
		results[ 1 ] = v6delim_end - vncHostStart - 1;
		v6_address = true;
  }

  const char *colonPtr = strchr(v6_address ? v6delim_end : vncHostStart, ':');
	  if (colonPtr != 0) {
		if(!v6_address)
			results[1] = colonPtr - vncHostStart;
		results[2] = strlen(colonPtr);
	  } else {
		if(!v6_address)
			results[1] = strlen(vncHostStart);
	  }
}

bool
HostPath::validateHostNames() const
{
  const char* acceptChars =
    ".-_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01234567890:";

  if (m_vncHost == 0)
    return false;

  size_t vncHostLen = strlen(m_vncHost);
  if (vncHostLen < 1 || vncHostLen > m_VNC_HOST_MAX_CHARS ||
      strspn(m_vncHost, acceptChars) != vncHostLen) {
    return false;
  }

  return true;
}
