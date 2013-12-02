/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/18      18:46
 * Filename : F:\GitHub\trunk\p2p_slotion\asyncrtspproxysocketserver.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: asyncrtspproxysocketserver
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
#include "asyncrtspproxysocketserver.h"
#include "proxyserverfactory.h"
#include "proxysocketmanagement.h"
#include "p2pconnectionimplementator.h"

//////////////////////////////////////////////////////////////////////////
AsyncRTSPProxyServerSocket::AsyncRTSPProxyServerSocket(
  talk_base::AsyncSocket* socket)
  :AsyncProxyServerSocket(socket,KBufferSize)
{
  BufferInput(false);
}

void AsyncRTSPProxyServerSocket::ProcessInput(char* data, size_t* len) {
  //LOG(LS_INFO) << __FUNCTION__ << "\t" << len;
}

void AsyncRTSPProxyServerSocket::SendConnectResult(
  int result,const talk_base::SocketAddress& addr){

}

//////////////////////////////////////////////////////////////////////////
//Implement RTSPProxyServer
RTSPProxyServer::RTSPProxyServer(talk_base::SocketFactory *int_factory, 
                                 const talk_base::SocketAddress &int_addr)
                                 :server_socket_(int_factory->CreateAsyncSocket(
                                 int_addr.family(),SOCK_STREAM))
{
  ASSERT(server_socket_.get() != NULL);
  ASSERT(int_addr.family() == AF_INET || int_addr.family() == AF_INET6);

  LOG(LS_INFO) << "2. " << __FUNCTION__;
  server_socket_->Bind(int_addr);
  server_socket_->Listen(5);
  server_socket_->SignalReadEvent.connect(this, &RTSPProxyServer::OnAcceptEvent);
}

void RTSPProxyServer::OnAcceptEvent(talk_base::AsyncSocket* socket){
  LOG(LS_INFO) << "3. " << __FUNCTION__;
  ASSERT(socket != NULL && socket == server_socket_.get());
  //Step 1. Accept this connection socket
  talk_base::AsyncSocket *accept_socket = socket->Accept(NULL);

  //Step 2. Wrap this socket by AsyncProxyServerSocket, add some new control to it.
  //NOTE: the WrapSocket used the new operator to create a 
  //talk_base::AsyncProxyServerSocket object. so you must be delete it 
  //When you release this object
  talk_base::AsyncProxyServerSocket* async_proxy_server_socket =
    WrapSocket(accept_socket);

  //Step 3. create RTSPServerSocketStart object
  RTSPServerSocketStart *rtsp_server_socket_start
    = new RTSPServerSocketStart(async_proxy_server_socket);

  ////Step 4. Register RTSPServerSocketStart in ProxySocketManagement
  //proxy_socket_management_->RegisterProxySocket((uint32)async_proxy_server_socket,
  //  rtsp_server_socket_start);
}

talk_base::AsyncProxyServerSocket* RTSPProxyServer::WrapSocket(
  talk_base::AsyncSocket* socket)
{
    return  new AsyncRTSPProxyServerSocket(socket);
}

//////////////////////////////////////////////////////////////////////////
RTSPServerSocketStart::RTSPServerSocketStart(
  talk_base::AsyncProxyServerSocket *int_socket)
                       :ProxySocketBegin(int_socket),
                       rtsp_socket_(int_socket)
{
  LOG(LS_INFO) << "4. " << __FUNCTION__;
  is_server_        = true;
  int_socket_state_ = SOCK_CONNECTED;
  p2p_socket_state_ = SOCK_CLOSE;
  
  rtsp_socket_->SignalConnectRequest.connect(this,
    &RTSPServerSocketStart::OnConnectRequest);
  
  //talk_base::SocketAddress server_addr("127.0.0.1",554);
  //OnConnectRequest(int_socket,server_addr);
}

void RTSPServerSocketStart::OnConnectRequest(talk_base::AsyncProxyServerSocket* socket,
                                  const talk_base::SocketAddress& addr)
{

}

void RTSPServerSocketStart::ReadSocketDataToBuffer(talk_base::AsyncSocket *socket, 
                                              talk_base::FifoBuffer *buffer)
{
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  // Only read if the buffer is empty.
  ASSERT(socket != NULL);
  size_t size;
  int read;
  if (buffer->GetBuffered(&size) && size == 0) {
//    internal_date_wait_receive_ = false;
    void* p = buffer->GetWriteBuffer(&size);
    read = socket->Recv(p, size);

    if(strncmp(RTSP_HEADER,(const char *)p,RTSP_HEADER_LENGTH) == 0){

      ParseRTSPGetSourceName((char *)p,(size_t *)&read);

      //for(int i = 0; i < read; i++){
      //  std::cout <<((char*)p)[i];
      //}
      //std::cout << std::endl;
    }
    buffer->ConsumeWriteBuffer(talk_base::_max(read, 0));
  }
  //else {
  //  internal_date_wait_receive_ = true;
  //}
}

void RTSPServerSocketStart::ParseRTSP(char *data, size_t *len){
  size_t header_len = RTSP_HEADER_LENGTH;
  size_t backlash_pos = 0;
  size_t break_char_pos = 0;
  char serverip[64];
  size_t serverip_length = 0;

  memset(serverip,0,64);

  //1. Find backlash position and break char position
  for(size_t i = header_len; i < *len; i++){
    if(data[i] == RTSP_BACKLASH)
      //add 1 because the / is no a member of server ip
        backlash_pos = i + 1; 
    if(data[i] == RTSP_BREAK_CHAR){
      break_char_pos = i;
      break;
    }
  }

  //2. get the server ip and port
  serverip_length = break_char_pos - backlash_pos;
  strncpy(serverip,data + backlash_pos, serverip_length);

  //
  ConnectTheAddr(serverip);

  serverip_length += 1;
  //3. delete the server ip in the data
  for(size_t i = break_char_pos + 1; i < *len; i++){
    data[i - serverip_length] = data[i];
    data[i] = 0;
  }
  *len -= serverip_length;
}

bool RTSPServerSocketStart::ConnectTheAddr(const std::string &server_ip){
  if(p2p_socket_state_ != SOCK_CLOSE)
    return false;
  //1. Find server ip string, stop at ':'
  int ip_pos = server_ip.find(':');
  int port = ::atoi(server_ip.substr(ip_pos + 1,std::string::npos).c_str());
  talk_base::SocketAddress addr(server_ip.substr(0,ip_pos),port);

  //std::cout << addr.ToString() << std::endl;
  return StartConnect(addr);
}

void RTSPServerSocketStart::ParseRTSPGetSourceName(char *data, size_t *len){
  size_t header_len = RTSP_HEADER_LENGTH;
  size_t backlash_pos = 0;
  size_t break_char_pos = 0;
  char source_ide[64];
  size_t serouce_ide_len = 0;

  memset(source_ide,0,64);

  //1. Find backlash position and break char position
  for(size_t i = header_len; i < *len; i++){
    if(data[i] == RTSP_BACKLASH)
      //add 1 because the / is no a member of server ip
        backlash_pos = i + 1; 
    if(data[i] == RTSP_BREAK_CHAR){
      break_char_pos = i;
      break;
    }
  }

  //2. get the server ip and port
  serouce_ide_len = break_char_pos - backlash_pos;
  strncpy(source_ide,data + backlash_pos, serouce_ide_len);

  //
  ConnectTheSource(source_ide);

  serouce_ide_len += 1;
  //3. delete the server ip in the data
  for(size_t i = break_char_pos + 1; i < *len; i++){
    data[i - serouce_ide_len] = data[i];
    data[i] = 0;
  }
  *len -= serouce_ide_len;
}

bool RTSPServerSocketStart::ConnectTheSource(const std::string &source_ide){
  if(p2p_socket_state_ != SOCK_CLOSE)
    return false;
  //std::cout << addr.ToString() << std::endl;
  return StartConnectBySourceIde(source_ide);
}

