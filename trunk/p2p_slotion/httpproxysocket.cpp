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
#include <iostream>

#include "httpproxysocket.h"
#include "asyncrtspproxysocketserver.h"
#include "talk/base/thread.h"

const char HTTP_GET_HEADER[] = "GET /";
const char HTTP_POST_HEADER[] = "POST /";
const size_t HTTP_POST_HEADER_LENGTH = sizeof(HTTP_POST_HEADER) - 1;
const size_t HTTP_GET_HEADER_LENGTH = sizeof(HTTP_GET_HEADER) - 1;
const char HTTP_A            = '\r';
const char HTTP_N            = '\n';

//////////////////////////////////////////////////////////////////////////
//Implement RTSPProxyServer
HTTPProxyServer::HTTPProxyServer(talk_base::SocketFactory *int_factory, 
                                 const talk_base::SocketAddress &int_addr)
                                 :server_socket_(int_factory->CreateAsyncSocket(
                                 int_addr.family(),SOCK_STREAM))
{
  ASSERT(server_socket_.get() != NULL);
  ASSERT(int_addr.family() == AF_INET || int_addr.family() == AF_INET6);

  signal_thread_ = talk_base::Thread::Current();
  
  LOG_P2P(BASIC_INFOR|P2P_RTSP_LOCAL_SERVER)
    << "Listen" << int_addr.ToString();
  if(server_socket_->Bind(int_addr)){
    LOG(LS_ERROR) << "Can't bind port " << int_addr.port();
    return;
  }
  server_socket_->Listen(5);
  server_socket_->SignalReadEvent.connect(this, 
    &HTTPProxyServer::OnAcceptEvent);
}

void HTTPProxyServer::OnAcceptEvent(talk_base::AsyncSocket* socket){
  ASSERT(signal_thread_->IsCurrent());
  ASSERT(socket != NULL && socket == server_socket_.get());
  //Step 1. Accept this connection socket
  talk_base::AsyncSocket *accept_socket = socket->Accept(NULL);

  //Step 2. Wrap this socket by AsyncProxyServerSocket, add some new control to it.
  //NOTE: the WrapSocket used the new operator to create a 
  //talk_base::AsyncProxyServerSocket object. so you must be delete it 
  //When you release this object
  talk_base::AsyncProxyServerSocket* async_proxy_server_socket =
    WrapSocket(accept_socket);

  LOG_P2P(BASIC_INFOR|P2P_RTSP_LOCAL_SERVER) << "\tAccept a socket " 
    << (intptr_t)async_proxy_server_socket <<" socket  " << (intptr_t)socket;
  //Step 3. create RTSPServerSocketStart object
  HTTPProxyServerSocket *http_proxy_server_socket
    = new HTTPProxyServerSocket(async_proxy_server_socket);

  ////Step 4. Register RTSPServerSocketStart in ProxySocketManagement
  //proxy_socket_management_->RegisterProxySocket((uint32)async_proxy_server_socket,
  //  rtsp_server_socket_start);
}

talk_base::AsyncProxyServerSocket* HTTPProxyServer::WrapSocket(
  talk_base::AsyncSocket* socket)
{
  ASSERT(signal_thread_->IsCurrent());
    return  new AsyncRTSPProxyServerSocket(socket);
}




//////////////////////////////////////////////////////////////////////////
HTTPProxyServerSocket::HTTPProxyServerSocket(
  talk_base::AsyncProxyServerSocket *int_socket)
                       :ProxySocketBegin(int_socket),
                       rtsp_socket_(int_socket)
{
  LOG_P2P(BASIC_INFOR|P2P_RTSP_LOCAL_SERVER) 
    << "\tCreate RTSPServerSocketStart Object ";
  is_server_        = true;
  int_socket_state_ = INT_SOCKET_CONNECTED;
  p2p_socket_state_ = P2P_SOCKET_START;
  
  rtsp_socket_->SignalConnectRequest.connect(this,
    &HTTPProxyServerSocket::OnConnectRequest);
}

void HTTPProxyServerSocket::OnConnectRequest(talk_base::AsyncProxyServerSocket* socket,
                                  const talk_base::SocketAddress& addr)
{

}

void HTTPProxyServerSocket::ReadSocketDataToBuffer(talk_base::AsyncSocket *socket, 
                                              talk_base::FifoBuffer *buffer)
{
  LOG_P2P(P2P_RTSP_LOCAL_SERVER) << "Reading local socket data";
  // Only read if the buffer is empty.
  ASSERT(socket != NULL);
  size_t size;
  int read;
  if (buffer->GetBuffered(&size) && size == 0) {
    void* p = buffer->GetWriteBuffer(&size);
    read = socket->Recv(p, size);

    for(int i = 0; i < read; i++)
      std::cout << ((char *)p)[i];
    std::cout << std::endl;

    if(strncmp(HTTP_GET_HEADER,(const char *)p,
      HTTP_GET_HEADER_LENGTH) == 0){
      ParseHTTPGetSourceName((char *)p,HTTP_GET_HEADER_LENGTH,
        (size_t *)&read);
    }
    else if(strncmp(HTTP_POST_HEADER,(const char *)p,
      HTTP_POST_HEADER_LENGTH) == 0){
      ParseHTTPGetSourceName((char *)p,HTTP_POST_HEADER_LENGTH,
        (size_t *)&read);
    }
    for(int i = 0; i < read; i++)
      std::cout << ((char *)p)[i];
    std::cout << std::endl;

    buffer->ConsumeWriteBuffer(talk_base::_max(read, 0));
  }
}


void HTTPProxyServerSocket::ParseHTTPGetSourceName(char *data, 
                                                   size_t header_length,
                                                   size_t *len)
{
  LOG_P2P(P2P_RTSP_LOCAL_SERVER) << "parsing HTTP source";
  size_t header_len = header_length;
  size_t backlash_pos = header_len;
  size_t break_char_pos = 0;
  char source_ide[64];
  size_t serouce_ide_len = 0;

  memset(source_ide,0,64);

  //1. Find backlash position and break char position
  for(size_t i = header_len; i < *len; i++){
    if(data[i] == RTSP_BACKLASH)
    {
      //add 1 because the / is no a member of server ip
      break_char_pos = i + 1;
      break;
    }
    if(data[i] == ' ')
    {
      //add 1 because the / is no a member of server ip
      break_char_pos = i; 
      break;
    }
  }

  //2. get the server ip and port
  serouce_ide_len = break_char_pos - backlash_pos;
  strncpy(source_ide,data + backlash_pos, serouce_ide_len);

  //
  if(p2p_socket_state_ == P2P_SOCKET_START){

    if(source_ide[serouce_ide_len - 1] == RTSP_BACKLASH)
      source_ide[serouce_ide_len - 1] = 0;

    StartConnectBySourceIde(source_ide,HTTP_SERVER);
  }

  //serouce_ide_len += 1;
  //3. delete the server ip in the data
  for(size_t i = break_char_pos; i < *len; i++){
    data[i - serouce_ide_len] = data[i];
    data[i] = 0;
  }
  *len -= serouce_ide_len;

}

void HTTPProxyServerSocket::OnInternalClose(talk_base::AsyncSocket* socket, int err){
  LOG_P2P(P2P_PROXY_SOCKET_DATA|P2P_PROXY_SOCKET_LOGIC) 
    << "close the internal socket " << err ;

  int_socket_->Close();

  if(p2p_socket_state_ != P2P_SOCKET_PROXY_CONNECTED){
    int_socket_state_ = INT_SOCKET_CLOSED;
    InternalSocketError(socket,err);
    return ;
  }
  if(int_socket_state_ == INT_SOCKET_CONNECTED){
    int_socket_state_ = INT_SOCKET_CLOSED;
  }
  else{
    int_socket_state_ = INT_SOCKET_CLOSED;
    CloseP2PSocket();
  }
}