/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/18      18:44
 * Filename : F:\GitHub\trunk\p2p_slotion\asyncrtspserversocket.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: asyncrtspserversocket
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

#ifndef ASYNC_RTSP_PROXY_SOCKET_SERVER_H_
#define ASYNC_RTSP_PROXY_SOCKET_SERVER_H_

#include <map>

#include "talk/base/socketadapters.h"
#include "talk/base/proxyserver.h"
#include "talk/base/sigslot.h"
#include "talk/base/scoped_ptr.h"

#include "mediator_pattern.h"
#include "sockettablemanagement.h"
#include "p2psystemcommand.h"
#include "proxysocketmanagement.h"

class AsyncP2PSocket;
class ProxySocketManagement;
class ProxySocketBegin;
//////////////////////////////////////////////////////////////////////////
//Implements a RTSP Proxy Socket Server that as the name said.
//This class used when the socket already connection.
///////////////////////////////////////////////////////////////////////////
//TODO:(GuangleiHe) TIME: 11/19/2013
//This class recently only for test RTSP server, if you want getting great
//you must be Implements ICE part of the p2p solution
///////////////////////////////////////////////////////////////////////////
class AsyncRTSPProxyServerSocket : public talk_base::AsyncProxyServerSocket
{
public:
  explicit AsyncRTSPProxyServerSocket(talk_base::AsyncSocket* socket);

private:
  virtual void ProcessInput(char* data, size_t* len);
  virtual void SendConnectResult(int result, const talk_base::SocketAddress& addr);
  
  static const int KBufferSize = 64 * 1024;
  
  DISALLOW_EVIL_CONSTRUCTORS(AsyncRTSPProxyServerSocket);
};

//////////////////////////////////////////////////////////////////////////
class RTSPProxyServer : public sigslot::has_slots<>
{
public:
  RTSPProxyServer(ProxySocketManagement *proxy_socket_management,
    AsyncP2PSocket *p2p_socket,
    talk_base::SocketFactory *int_factory, 
    const talk_base::SocketAddress &int_addr);

protected:
  void OnAcceptEvent(talk_base::AsyncSocket* socket);
  virtual talk_base::AsyncProxyServerSocket* WrapSocket(talk_base::AsyncSocket* socket);

private:
  ///////////////////////////////////////////////////////////////////////////
  //BUG NOTE (GuangleiHe, 11/20/2013)
  //Maybe there has a bug.
  //Because the smart pointer maybe can't used as a normal parameter
  ///////////////////////////////////////////////////////////////////////////
  talk_base::scoped_ptr<talk_base::AsyncSocket> server_socket_;
  talk_base::scoped_ptr<AsyncP2PSocket> p2p_socket_;
  talk_base::scoped_ptr<ProxySocketManagement> proxy_socket_management_;

  DISALLOW_EVIL_CONSTRUCTORS(RTSPProxyServer);
};

//////////////////////////////////////////////////////////////////////////
class RTSPServerSocketStart : public ProxySocketBegin
{
public:
  RTSPServerSocketStart(AsyncP2PSocket * p2p_socket,
    talk_base::AsyncProxyServerSocket *int_socket);
private:
  void OnConnectRequest(talk_base::AsyncProxyServerSocket* socket,
    const talk_base::SocketAddress& addr);
  virtual void OnP2PReceiveData(const char *data, uint16 len);
  virtual void OnInternalRead(talk_base::AsyncSocket* socket);
  //Some help function
private:
  enum RTSPProxyServerState{
    RP_CLIENT_CONNCTED,RP_SEND_RTSP_CLIENT_CREATE_COMMAND,RP_SCUCCEED
  } state_;
  talk_base::AsyncProxyServerSocket *rtsp_socket_;
  bool has_wait_data;
  DISALLOW_EVIL_CONSTRUCTORS(RTSPServerSocketStart);
};


#endif