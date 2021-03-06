/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/19      19:14
 * Filename : F:\GitHub\trunk\p2p_slotion\asyncrtspclientsocket.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: asyncrtspclientsocket
 * File ext : cpp
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

#include "talk/base/bytebuffer.h"

#include "asyncrtspclientsocket.h"
#include "proxyserverfactory.h"
#include "p2pconnectionimplementator.h"
#include "proxyp2psession.h"


//////////////////////////////////////////////////////////////////////////
RTSPClientSocket::RTSPClientSocket(ProxyP2PSession *proxy_p2p_session,
                                   talk_base::AsyncSocket *int_socket,
                                   intptr_t server_socket_number,
                                   const talk_base::SocketAddress &server_addr)
                                   :ProxySocketBegin(int_socket),
                                   server_socket_number_(server_socket_number)
{
  LOG_P2P(CREATE_DESTROY_INFOR|P2P_RTSP_LOCAL_CLIENT) 
    << "Connect to " << server_addr.ToString();
  SetProxyP2PSession(proxy_p2p_session);

  is_server_ = false;
  int_socket_->SignalConnectEvent.connect(this,
    &RTSPClientSocket::OnInternalConnect);
  p2p_socket_state_ = P2P_SOCKET_PEER_CONNECTED;
  int_socket_state_ = INT_SCOKET_START;
  int_socket_->Connect(server_addr);
  remote_peer_addr_ = server_addr;
}


void RTSPClientSocket::OnInternalConnect(
  talk_base::AsyncSocket* socket)
{
  LOG_P2P(P2P_RTSP_LOCAL_CLIENT)  << "connect server succeed ";
  ASSERT(int_socket_state_ == INT_SCOKET_START);

  //The Client connect was accept
  //Then Send the connect succeed to remote peer
  ASSERT(int_socket_.get() == socket);
  int_socket_state_ = INT_SOCKET_CONNECTED;
  p2p_socket_state_ = P2P_SOCKET_PROXY_CONNECTED;
  proxy_p2p_session_->ReplayClientSocketCreateSucceed(server_socket_number_,
    (intptr_t)socket,remote_peer_addr_);
}

void RTSPClientSocket::InternalSocketError(talk_base::AsyncSocket *socket,int err)
{
  proxy_p2p_session_->P2PSocketConnectFailure(
    server_socket_number_,(intptr_t)int_socket_.get());
  p2p_socket_state_ = P2P_SOCKET_CLOSED;
  Destory();
}
