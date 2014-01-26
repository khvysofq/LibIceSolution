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

#ifndef P2P_PROXY_BINDING_H_
#define P2P_PROXY_BINDING_H_

#include "talk/base/proxyserver.h"
#include "talk/base/scoped_ptr.h"
#include "talk/base/thread.h"

#include "p2pproxyserversocket.h"
#include "p2pconnectionmanagement.h"
#include "proxyp2psession.h"
#include "p2pproxysocket.h"

class SocketFactory;

// ProxyServer is a base class that allows for easy construction of proxy
// servers. With its helper class ProxyBinding, it contains all the necessary
// logic for receiving and bridging connections. The specific client-server
// proxy protocol is implemented by an instance of the AsyncProxyServerSocket
// class; children of ProxyServer implement WrapSocket appropriately to return
// the correct protocol handler.

class P2PProxyBindBase : public sigslot::has_slots<>,
  public talk_base::MessageHandler
{
public:
  P2PProxyBindBase(talk_base::AsyncSocket* base_socket, 
    P2PProxySocket* p2p_socket);
  sigslot::signal1<P2PProxyBindBase*> SignalDestroyed;

  static void Read(talk_base::AsyncSocket* socket, 
    talk_base::FifoBuffer* buffer);
  static void Write(talk_base::AsyncSocket* socket, 
    talk_base::FifoBuffer* buffer);
  virtual void Destroy();
private:
  virtual void OnBaseSocketRead(talk_base::AsyncSocket* socket);
  virtual void OnBaseSocketWrite(talk_base::AsyncSocket* socket);
  virtual void OnBaseSocketClose(talk_base::AsyncSocket* socket, int err);
  
  virtual void OnP2PSocketReadMixData(const char *data, uint16 len);
  virtual void OnP2PSocketRead(talk_base::AsyncSocket* socket);
  virtual void OnP2PSocketWrite(talk_base::AsyncSocket* socket);
  virtual void OnP2PSocketClose(talk_base::AsyncSocket* socket, int err);

  virtual void OnMessage(talk_base::Message *msg);
protected:
  static const int kBufferSize = 4096;
  talk_base::scoped_ptr<talk_base::AsyncSocket> base_socket_;
  talk_base::scoped_ptr<P2PProxySocket>         p2p_socket_;
  bool connected_;
  talk_base::FifoBuffer out_buffer_;
  talk_base::FifoBuffer in_buffer_;
  talk_base::Thread     *current_thread_;
  DISALLOW_EVIL_CONSTRUCTORS(P2PProxyBindBase);
};

class P2PProxyBinding : public P2PProxyBindBase {
 public:
  P2PProxyBinding(P2PProxyServerSocket* in_socket, 
    P2PProxyStartSocket* out_socket);

 protected:
  void OnServerSocketConnectRequest(P2PProxyServerSocket* socket,
    const std::string &addr_ide);
  void OnServerSocketConnectRead(P2PProxyServerSocket* socket,
    const void *data, size_t len);

  void OnP2PSocketConnectSucceed(talk_base::AsyncSocket* socket);
  virtual void OnP2PSocketClose(talk_base::AsyncSocket* socket, int err);
  //virtual void Destroy();

  P2PProxyServerSocket        *server_socket_;
  P2PProxyStartSocket         *p2p_proxy_start_socket_;
  DISALLOW_EVIL_CONSTRUCTORS(P2PProxyBinding);
};

//////////////////////////////////////////////////////////////////////////
//P2P Proxy Client Socket
//////////////////////////////////////////////////////////////////////////
class P2PProxyClientSocketBinding : public P2PProxyBindBase{
public:
  P2PProxyClientSocketBinding(P2PProxyEndSocket *p2p_proxy_socket,
    talk_base::AsyncSocket *client_socket,
    const talk_base::SocketAddress &server_addr);
  void OnClientSocketConnect(talk_base::AsyncSocket* socket);
  virtual void OnBaseSocketClose(talk_base::AsyncSocket* socket, int err);
private:
  P2PProxyEndSocket *p2p_proxy_end_socket_;
};

#endif