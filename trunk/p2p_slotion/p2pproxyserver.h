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


#ifndef P2P_PROXY_SERVER_H_
#define P2P_PROXY_SERVER_H_

#include "p2pproxybinding.h"
#include "p2pproxyserversocket.h"

class P2PProxyServer : public sigslot::has_slots<> {
 public:
  P2PProxyServer(talk_base::SocketFactory* int_factory,
    const talk_base::SocketAddress& int_addr);
  virtual ~P2PProxyServer();

 protected:
  virtual void OnAcceptEvent(talk_base::AsyncSocket* socket);
  virtual P2PProxyServerSocket* 
    WrapSocket(talk_base::AsyncSocket* socket) = 0;
  void OnBindingDestroyed(P2PProxyBindBase* binding);

  typedef std::list<P2PProxyBindBase*> BindingList;
  talk_base::scoped_ptr<talk_base::AsyncSocket> server_socket_;
  BindingList bindings_;
  DISALLOW_EVIL_CONSTRUCTORS(P2PProxyServer);
};

//////////////////////////////////////////////////////////////////////////
//RTSP Proxy Server
//////////////////////////////////////////////////////////////////////////
class P2PRTSPProxyServer : public P2PProxyServer
{
public:
  P2PRTSPProxyServer(talk_base::SocketFactory* int_factory, 
    const talk_base::SocketAddress& int_addr)
      : P2PProxyServer(int_factory, int_addr){
  }
private:
  P2PProxyServerSocket* WrapSocket(
    talk_base::AsyncSocket* socket) 
  {
    return new P2PRTSPProxyServerSocket(socket);
  }
  DISALLOW_EVIL_CONSTRUCTORS(P2PRTSPProxyServer);
};

class P2PHTTProxyServer : public P2PProxyServer
{
public:
  P2PHTTProxyServer(talk_base::SocketFactory* int_factory, 
    const talk_base::SocketAddress& int_addr)
      : P2PProxyServer(int_factory, int_addr){
  }
private:
  P2PProxyServerSocket* WrapSocket(
    talk_base::AsyncSocket* socket) 
  {
    return new P2PHTTPProxyServerSocket(socket);
  }
  DISALLOW_EVIL_CONSTRUCTORS(P2PHTTProxyServer);
};
#endif