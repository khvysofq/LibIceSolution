/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/19      19:14
 * Filename : F:\GitHub\trunk\p2p_slotion\asyncrtspclientsocket.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: asyncrtspclientsocket
 * File ext : h
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ASYNC_RTSP_CLIENT_SOCKET_H_
#define ASYNC_RTSP_CLIENT_SOCKET_H_

#include "talk/base/socketadapters.h"
#include "talk/base/proxyserver.h"
#include "talk/base/sigslot.h"
#include "talk/base/scoped_ptr.h"

#include "proxysocketmanagement.h"

class ProxyP2PSession;

class RTSPClientSocket : public ProxySocketBegin
{
public:
  explicit RTSPClientSocket(ProxyP2PSession *proxy_p2p_session,
    talk_base::AsyncSocket *int_socket,
    intptr_t server_socket_number,
    const talk_base::SocketAddress &server_addr);
private:
  void OnInternalConnect(talk_base::AsyncSocket* socket);
  virtual void InternalSocketError(talk_base::AsyncSocket* socket, int err);
  //virtual void OnInternalClose(talk_base::AsyncSocket* socket,
  //  int err);
private:
  intptr_t                    server_socket_number_;
  DISALLOW_EVIL_CONSTRUCTORS(RTSPClientSocket);
};



#endif
