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
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

#ifndef SOCKET_ADDRESS_IPV6_H
#define SOCKET_ADDRESS_IPV6_H

#include "util/CommonHeader.h"
#include "thread/LocalMutex.h"
#include "SocketException.h"
#include "sockdefs.h"
#include "win-system/WsaStartup.h"

// FIXME: Deprecated method, only for testing of old code.
void getLocalIPAddrString(char *buffer, int buflen);

class SocketAddressIPv6
{
public:
  SocketAddressIPv6();
  SocketAddressIPv6(struct sockaddr_in6);
  SocketAddressIPv6(const TCHAR *host, unsigned short port);

  SocketAddressIPv6(const SocketAddressIPv6 &socketAddressIPv6);
  SocketAddressIPv6 &operator=(const SocketAddressIPv6 &socketAddressIPv6);

  socklen_t getAddrLen() const;
  struct sockaddr_in6 getSockAddr() const;

  // Converts socket address to it's string value (ip address as string).
  void toString(StringStorage *address) const;

  static SocketAddressIPv6 resolve(const TCHAR *host, unsigned short port) throw(SocketException);
  static addrinfo get_hints();

protected:
  WsaStartup m_wsaStartup;
  unsigned short m_port;
  struct in_addr6 m_addr;


  static LocalMutex s_resolveMutex;
 
};

#endif
