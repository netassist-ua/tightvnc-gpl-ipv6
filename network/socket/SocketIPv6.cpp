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

#include <stdlib.h>
#include "SocketAddressIPv6.h"
#include "SocketAddressIPv6.h"
#include "SocketIPv6.h"

#include "thread/AutoLock.h"

#include <crtdbg.h>

SocketIPv6::SocketIPv6()
: m_localAddr(NULL), m_peerAddr(NULL), m_isBound(false),
  m_wsaStartup(2, 2)
{
  DWORD v6OnlyVal = 0;
  m_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
  if( setsockopt(m_socket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&v6OnlyVal, sizeof(v6OnlyVal)) == SOCKET_ERROR) {
	throw SocketException();
  }
  m_isClosed = false;

  if (m_socket == INVALID_SOCKET) {
    throw SocketException();
  }
}

SocketIPv6::~SocketIPv6()
{
#ifdef _WIN32
  ::closesocket(m_socket);
#else
  ::close(m_socket);
#endif

  AutoLock l(&m_mutex);

  if (m_peerAddr) {
    delete m_peerAddr;
  }

  if (m_localAddr) {
    delete m_localAddr;
  }
}

void SocketIPv6::connect(const TCHAR *host, unsigned short port)
{
  SocketAddressIPv6 address(host, port);

  connect(address);
}

void SocketIPv6::connect(const SocketAddressIPv6 &addr)
{
  struct sockaddr_in6 targetSockAddr = addr.getSockAddr();

  if (::connect(m_socket, (const sockaddr *)&targetSockAddr, addr.getAddrLen()) == SOCKET_ERROR) {
    throw SocketException();
  }

  AutoLock l(&m_mutex);

  if (m_peerAddr) {
    delete m_peerAddr;
  }

  m_peerAddr = new SocketAddressIPv6(*(struct sockaddr_in6 *)&targetSockAddr);

  m_isBound = false;
}

void SocketIPv6::close()
{
  m_isClosed = true;
}

void SocketIPv6::shutdown(int how)
{
  if (::shutdown(m_socket, how) == SOCKET_ERROR) {
    throw SocketException();
  }
}

void SocketIPv6::bind(const TCHAR *bindHost, unsigned int bindPort)
{
  SocketAddressIPv6 address(bindHost, bindPort);

  bind(address);
}

void SocketIPv6::bind(const SocketAddressIPv6 &addr)
{
  struct sockaddr_in6 bindSockaddr = addr.getSockAddr();

  if (::bind(m_socket, (const sockaddr *)&bindSockaddr, addr.getAddrLen()) == SOCKET_ERROR) {
    throw SocketException();
  }

  AutoLock l(&m_mutex);

  if (m_localAddr) {
    delete m_localAddr;
  }

  m_localAddr = new SocketAddressIPv6(*(struct sockaddr_in6*)&bindSockaddr);

  m_isBound = true;
}

bool SocketIPv6::isBound()
{
  AutoLock l(&m_mutex);

  return m_isBound;
}

void SocketIPv6::listen(int backlog)
{
  if (::listen(m_socket, backlog) == SOCKET_ERROR) {
    throw SocketException();
  }
}

SocketIPv6 *SocketIPv6::accept()
{
  struct sockaddr_in6 addr;

  SOCKET result = getAcceptedSocket(&addr);

  SocketIPv6 *accepted;

  try {
    accepted = new SocketIPv6(); 
    accepted->close();
  } catch(...) {
    // Cleanup and throw further
#ifdef _WIN32
    ::closesocket(result);
#else
    ::close(result);
#endif
    throw SocketException();
  }

  // Fall out with exception, no need to check if accepted is NULL
  accepted->set(result);
  return accepted; // Valid and initialized
}

void SocketIPv6::set(SOCKET socket)
{
  AutoLock l(&m_mutex);

#ifdef _WIN32
  ::closesocket(m_socket);
#else
  ::close(m_socket);
#endif
  m_socket = socket;

  // Set local and peer addresses for new socket
  struct sockaddr_in6 addr;
  socklen_t addrlen = sizeof(struct sockaddr_in6);
  if (getsockname(socket, (struct sockaddr *)&addr, &addrlen) == 0) {
    m_localAddr = new SocketAddressIPv6(addr);
  }

  if (getpeername(socket, (struct sockaddr *)&addr, &addrlen) == 0) {
    m_peerAddr = new SocketAddressIPv6(addr);
  }
}

SOCKET SocketIPv6::getAcceptedSocket(struct sockaddr_in6 *addr)
{
  socklen_t addrlen = sizeof(struct sockaddr_in6);
  fd_set afd;

  timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 200000;
  SOCKET result = INVALID_SOCKET;

  while (true) {
    FD_ZERO(&afd);
    FD_SET(m_socket, &afd);

    // FIXME: The select() and accept() function can provoke an system
    // exception, if it allows by project settings and closesocket() has alredy
    // been called.
    int ret = select((int)m_socket + 1, &afd, NULL, NULL, &timeout);

    if (m_isClosed || ret == SOCKET_ERROR) {
      throw SocketException();
    } else if (ret == 0) {
      continue;
    } else if (ret > 0) {
      if (FD_ISSET(m_socket, &afd)) {
        result = ::accept(m_socket, (struct sockaddr*)addr, &addrlen);
        if (result == INVALID_SOCKET) {
          throw SocketException();
        }
        break;
      } // if.
    } // if select ret > 0.
  } // while waiting for incoming connection.
  return result;
}

int SocketIPv6::send(const char *data, int size, int flags)
{
  int result;
  
  result = ::send(m_socket, data, size, flags);

  if (result == -1) {
    throw IOException(_T("Failed to send data to socket."));
  }
  
  return result;
}

int SocketIPv6::recv(char *buffer, int size, int flags)
{
  int result;

  result = ::recv(m_socket, buffer, size, flags);

  // Connection has been gracefully closed.
  if (result == 0) {
    throw IOException(_T("Connection has been gracefully closed"));
  }

  // SocketIPv6 error.
  if (result == SOCKET_ERROR) {
    throw IOException(_T("Failed to recv data from socket."));
  }

  return result;
}

bool SocketIPv6::getLocalAddr(SocketAddressIPv6 *addr)
{
  AutoLock l(&m_mutex);

  if (m_localAddr == 0) {
    return false;
  }

  *addr = *m_localAddr;

  return true;
}

bool SocketIPv6::getPeerAddr(SocketAddressIPv6 *addr)
{
  AutoLock l(&m_mutex);

  if (m_peerAddr == 0) {
    return false;
  }

  *addr = *m_peerAddr;

  return true;
}

/* Auxiliary */
void SocketIPv6::setSocketOptions(int level, int name, void *value, socklen_t len)
{
  if (setsockopt(m_socket, level, name, (char*)value, len) == SOCKET_ERROR) {
    throw SocketException();
  }
}

void SocketIPv6::getSocketOptions(int level, int name, void *value, socklen_t *len)
{
  if (getsockopt(m_socket, level, name, (char*)value, len) == SOCKET_ERROR) {
    throw SocketException();
  }
}

void SocketIPv6::enableNaggleAlgorithm(bool enabled)
{
  BOOL disabled = enabled ? 0 : 1;

  setSocketOptions(IPPROTO_TCP, TCP_NODELAY, &disabled, sizeof(disabled));
}

void SocketIPv6::setExclusiveAddrUse()
{
  int val = 1;

  setSocketOptions(SOL_SOCKET, SO_EXCLUSIVEADDRUSE, &val, sizeof(val));
}
