// Copyright (C) 2008,2009,2010,2011,2012 GlavSoft LLC.
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
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.f
//-------------------------------------------------------------------------
//

#include <stdlib.h>
#include <string.h>
#include <vector>

#include "sockdefs.h"
#include "SocketAddressIPv6.h"
#include "SocketAddressIPv6.h"
#include "SocketException.h"
#include "util/AnsiStringStorage.h"

#include "inet_pton.cpp"
#include "inet_ntop.cpp"

#include "thread/AutoLock.h"

LocalMutex SocketAddressIPv6::s_resolveMutex;



void getLocalIPAddrString(char *buffer, int buflen)
{
  char namebuf[256];

  if (gethostname(namebuf, 256) != 0) {
    strncpy(buffer, "Host name unavailable", buflen);
    return;
  };

  addrinfo addr_hints = SocketAddressIPv6::get_hints();

  addrinfo * addr_info = NULL;
  addrinfo * addr_ptr = NULL;
  
  addr_hints.ai_family = AF_UNSPEC;

  if(getaddrinfo(namebuf, NULL, &addr_hints, &addr_info) != 0){
		strncpy(buffer, "Cannot get local IP address", buflen);
  }
  
  if (!addr_info) {
    strncpy(buffer, "IP address unavailable", buflen);
    return;
  };

  *buffer = '\0';

  //ipv6 string address buffer
  char v6text[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];

  //link local prefix
  unsigned char v6_link_local_prefix[] = { 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  v6text[ 0 ]  = '\0';
  for( addr_ptr = addr_info; addr_ptr != NULL; addr_ptr = addr_ptr->ai_next ){
	  switch( addr_ptr->ai_family ){
	  case AF_INET6:
		  //avoid link local
		  if( memcmp(&v6_link_local_prefix, &((sockaddr_in6 *)addr_ptr->ai_addr)->sin6_addr.u.Byte, 8) == 0)
			  continue;

		  inet_ntop(AF_INET6, ((sockaddr_in6 *)addr_ptr->ai_addr)->sin6_addr.u.Byte, v6text, sizeof(v6text));
		  break;
	  case AF_INET:
		  inet_ntop(AF_INET, &((sockaddr_in *)addr_ptr->ai_addr)->sin_addr.S_un.S_addr, v6text, sizeof(v6text));
		  break;
	  default:
			continue;
	  }
	  strncat(buffer, v6text, (buflen-1)-strlen(buffer));
	  buffer[strlen(buffer)] = '\0';
	  if (addr_ptr->ai_next != NULL)
		strncat(buffer, ", ", (buflen-1)-strlen(buffer));
	  }
}

addrinfo SocketAddressIPv6::get_hints(){
	addrinfo addr_hints;
	ZeroMemory(&addr_hints, sizeof(addrinfo));
	addr_hints.ai_family = PF_INET6;
    addr_hints.ai_socktype = SOCK_STREAM;
    addr_hints.ai_protocol = IPPROTO_TCP;
	addr_hints.ai_flags = AI_ALL|AI_V4MAPPED;
	return addr_hints;
}

SocketAddressIPv6::SocketAddressIPv6()
: m_wsaStartup(2, 2)
{
  IN6_SET_ADDR_UNSPECIFIED(&m_addr);
  m_port = 0;
};

SocketAddressIPv6::SocketAddressIPv6(struct sockaddr_in6 addr)
: m_wsaStartup(2, 2)
{
  IN6_SET_ADDR_UNSPECIFIED(&m_addr);
  m_port = 0;

  if (addr.sin6_family != AF_INET6) {
    throw SocketException(_T("The specified m_addr is not AF_INET6 family m_addr!"));
  }

  m_addr = addr.sin6_addr;
  m_port = ntohs(addr.sin6_port);
};

SocketAddressIPv6::SocketAddressIPv6(const TCHAR *host, unsigned short port)
: m_wsaStartup(2, 2)
{
  SocketAddressIPv6 sa = SocketAddressIPv6::resolve(host, port);

  this->m_addr = sa.m_addr;
  this->m_port = sa.m_port;
};

SocketAddressIPv6::SocketAddressIPv6(const SocketAddressIPv6 &socketAddressIPv6)
: m_wsaStartup(2, 2) {
  m_addr = socketAddressIPv6.m_addr;
  m_port = socketAddressIPv6.m_port;
}

SocketAddressIPv6& SocketAddressIPv6::operator=(const SocketAddressIPv6 &socketAddressIPv6) {
  if (this != &socketAddressIPv6) {
    this->m_addr = socketAddressIPv6.m_addr;
    this->m_port = socketAddressIPv6.m_port;
  }
  return *this;
}

socklen_t SocketAddressIPv6::getAddrLen() const
{
  return sizeof(struct sockaddr_in6);
};

struct sockaddr_in6 SocketAddressIPv6::getSockAddr() const
{
  struct sockaddr_in6 saddr;
  ZeroMemory(&saddr, sizeof(saddr));
  saddr.sin6_family = AF_INET6;
  memcpy(&(saddr.sin6_addr), &m_addr, sizeof(in6_addr));
  saddr.sin6_port = htons(m_port);

#ifndef _WIN32
  saddr.sin6_len = sizeof(struct sockaddr_in6);
#endif

  return saddr;
}

void SocketAddressIPv6::toString(StringStorage *address) const
{
  char v6text[128];
  inet_pton(AF_INET6, (const char *)m_addr.u.Byte, &v6text);
  address->appendString((const TCHAR *)v6text);
}

SocketAddressIPv6 SocketAddressIPv6::resolve(const TCHAR *host, unsigned short m_port)
{
  SocketAddressIPv6 resolvedAddress;

  StringStorage hostStorage(host);

  {
    AutoLock l(&s_resolveMutex);

    AnsiStringStorage hostAnsi(&hostStorage);
	addrinfo addr_hints = SocketAddressIPv6::get_hints();
	addrinfo * addr_info = NULL;

   if(getaddrinfo(hostAnsi.getString(), NULL, &addr_hints, &addr_info) != 0 || addr_info == NULL){
		 throw SocketException();
   }

   memcpy(&(resolvedAddress.m_addr), &((struct sockaddr_in6 *) addr_info->ai_addr)->sin6_addr, sizeof(in6_addr));
  }

  resolvedAddress.m_port = m_port;

  return resolvedAddress;
}
