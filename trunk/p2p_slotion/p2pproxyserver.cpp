/*
* p2p solution
* Copyright 2013, VZ Inc.
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

#include "p2pproxyserver.h"
#include "p2pproxybinding.h"

// ProxyServer
P2PProxyServer::P2PProxyServer(
    talk_base::SocketFactory* int_factory, const talk_base::SocketAddress& int_addr)
    : server_socket_(int_factory->CreateAsyncSocket(int_addr.family(),
                                                    SOCK_STREAM)) {
  ASSERT(server_socket_.get() != NULL);
  ASSERT(int_addr.family() == AF_INET || int_addr.family() == AF_INET6);
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC | BASIC_INFOR) << "Listen port " 
    << int_addr.ToSensitiveString();
  server_socket_->Bind(int_addr);
  server_socket_->Listen(5);
  server_socket_->SignalReadEvent.connect(this, &P2PProxyServer::OnAcceptEvent);
}

P2PProxyServer::~P2PProxyServer() {
  for (BindingList::iterator it = bindings_.begin();
       it != bindings_.end(); ++it) {
    delete (*it);
  }
}

void P2PProxyServer::OnAcceptEvent(talk_base::AsyncSocket* socket) {
  ASSERT(socket != NULL && socket == server_socket_.get());
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC | BASIC_INFOR) << "New request arrived "
    << socket->GetRemoteAddress().ToString();
  talk_base::AsyncSocket* int_socket = socket->Accept(NULL);
  P2PProxyServerSocket* wrapped_socket = WrapSocket(int_socket);
  P2PProxyStartSocket  *ext_socket = new P2PProxyStartSocket();
  if (ext_socket) {
    P2PProxyBindBase *p2p_proxy_binding 
      = new P2PProxyBinding(wrapped_socket, ext_socket);
    bindings_.push_back(p2p_proxy_binding);
    p2p_proxy_binding->SignalDestroyed.connect(this,
      &P2PProxyServer::OnBindingDestroyed);
  } else {
    LOG(LS_ERROR) << "Unable to create external socket on proxy accept event";
  }
}

void P2PProxyServer::OnBindingDestroyed(P2PProxyBindBase* binding) {
  BindingList::iterator it =
      std::find(bindings_.begin(), bindings_.end(), binding);
  delete (*it);
  bindings_.erase(it);
}


//////////////////////////////////////////////////////////////////////////
//For P2P RTSP Proxy Server
//////////////////////////////////////////////////////////////////////////
