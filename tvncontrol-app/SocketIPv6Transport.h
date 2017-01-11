// Copyright (C) 2010,2011,2012 GlavSoft LLC.
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

#ifndef _SOCKET_IPV6_TRANSPORT_H_
#define _SOCKET_IPV6_TRANSPORT_H_

#include "Transport.h"

#include "network/socket/SocketIPv6.h"

/**
 * Transport that uses IPv6 socket.
 */
class SocketIPv6Transport : public Transport
{
public:
  /**
   * Creates transport and takes ownership over existing socket.
   */
  SocketIPv6Transport(SocketIPv6 *socket);
  /**
   * Deletes transport and frees resources.
   */
  virtual ~SocketIPv6Transport();

  /**
   * Returns transport's IO Stream.
   */
  virtual Channel *getIOStream();

  /**
   * Accepts new connection.
   * @return transport for accepted connection.
   * @throws SocketException on fail.
   */
  virtual Transport *accept() throw(SocketException);

  /**
   * Destroys transport(closes socket).
   */
  virtual void close() throw(SocketException);

private:
  /**
   * Real transport.
   */
  SocketIPv6 *m_socket;
  /**
   * Stream.
   */
  Channel *m_stream;
};

#endif
