/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/20      11:11
 * Filename : F:\GitHub\trunk\p2p_slotion\proxyserverfactory.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: proxyserverfactory
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
#ifndef P2P_SOLUTION_PROXY_SERVER_FACTORY_H_
#define P2P_SOLUTION_PROXY_SERVER_FACTORY_H_
///////////////////////////////////////////////////////////////////////////
//TODO:(GuangleiHe) TIME: 11/20/2013
//Change the header file as possible as inlcude in *.cpp files
//Not include in *.h files.
///////////////////////////////////////////////////////////////////////////
#include "talk/base/sigslot.h"
#include "talk/base/socketaddress.h"

#include "asyncrtspproxysocketserver.h"
#include "proxysocketmanagement.h"
#include "mediator_pattern.h"

class RTSPProxyServer;
class ProxySocketManagement;
class RTSPClientSocket;
class AsyncP2PSocket
{
public:
  AsyncP2PSocket(AbstractVirtualNetwork *virtual_network);
  virtual void Send(uint32 socket, SocketType socket_type,
    const char *data, uint16 len);
private:
  AbstractVirtualNetwork *virtual_network_;
  DISALLOW_EVIL_CONSTRUCTORS(AsyncP2PSocket);
};


class ProxyServerFactory
{
public:
  //ProxyServerFactory(){}
  ///////////////////////////////////////////////////////////////////////////
  //TODO:(GuangleiHe) TIME: 11/20/2013
  //To manage the servers that to release those server together.
  ///////////////////////////////////////////////////////////////////////////
  static RTSPProxyServer *CreateRTSPProxyServer(
    ProxySocketManagement *proxy_socket_management,
    AsyncP2PSocket *p2p_socket,
    talk_base::SocketFactory *int_factory,
    const talk_base::SocketAddress &local_rtsp_addr);

  static RTSPClientSocket *CreateRTSPClientSocket(
    ProxySocketManagement *proxy_socket_management,
    AsyncP2PSocket *p2p_socket,
    talk_base::AsyncSocket *int_socket,
    uint32 server_socket_number,
    const talk_base::SocketAddress &server_rtsp_addr);

private:
  DISALLOW_EVIL_CONSTRUCTORS(ProxyServerFactory);
};


#endif