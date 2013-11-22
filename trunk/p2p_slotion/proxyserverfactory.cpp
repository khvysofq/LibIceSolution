/*
* p2p solution
* Copyright 2013, VZ Inc.
* 
* Author   : GuangLei He
* Email    : guangleihe@gmail.com
* Created  : 2013/11/20      11:11
* Filename : F:\GitHub\trunk\p2p_slotion\proxyserverfactory.cpp
* File path: F:\GitHub\trunk\p2p_slotion
* File base: proxyserverfactory
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
#include "proxyserverfactory.h"
#include "asyncrtspproxysocketserver.h"
#include "asyncrtspclientsocket.h"
//////////////////////////////////////////////////////////////////////////
AsyncP2PSocket::AsyncP2PSocket(AbstractVirtualNetwork *virtual_network)
  :virtual_network_(virtual_network),has_data_(0)
{
}

void AsyncP2PSocket::Send(uint32 socket, SocketType socket_type,
                          const char *data, uint16 len, size_t *written)
{
  LOG(LS_INFO) << "6. " << __FUNCTION__;
  if(len == 0)
    return ;
  virtual_network_->OnReceiveDataFromUpLayer(socket,socket_type,data,
    len,written,&has_data_);
}

size_t AsyncP2PSocket::GetAvalibeSendData(){
  return MAX_SAVE_DATA_LEN - has_data_;
}

//////////////////////////////////////////////////////////////////////////
RTSPProxyServer *ProxyServerFactory::CreateRTSPProxyServer(
  ProxySocketManagement *proxy_socket_management,
  AsyncP2PSocket *p2p_socket,
  talk_base::SocketFactory *int_factory,
  const talk_base::SocketAddress &local_rtsp_addr)
{
  LOG(LS_INFO) << "1. " << __FUNCTION__;
  return new RTSPProxyServer(proxy_socket_management,
    p2p_socket,int_factory,local_rtsp_addr);
}

RTSPClientSocket* ProxyServerFactory::CreateRTSPClientSocket(
  ProxySocketManagement *proxy_socket_management,
  AsyncP2PSocket *p2p_socket,
  talk_base::AsyncSocket *int_socket,
  uint32 server_socket_number,
  const talk_base::SocketAddress &server_rtsp_addr)
{
  RTSPClientSocket *rtsp_client_socket = 
    new RTSPClientSocket(p2p_socket,int_socket,
    server_socket_number,server_rtsp_addr);

  proxy_socket_management->RegisterProxySocket((uint32)int_socket,
    rtsp_client_socket);

  return rtsp_client_socket;
}