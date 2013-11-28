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

//////////////////////////////////////////////////////////////////////////
AsyncRTSPProxyServerSocket::AsyncRTSPProxyServerSocket(
  talk_base::AsyncSocket* socket)
  :AsyncProxyServerSocket(socket,KBufferSize)
{
  BufferInput(false);
  //Turn off data process in AsyncProxyServerSocket
  talk_base::SocketAddress server_addr("127.0.0.1",554);
  SignalConnectRequest(this,server_addr);
}

void AsyncRTSPProxyServerSocket::ProcessInput(char* data, size_t* len) {
  LOG(LS_INFO) << __FUNCTION__ << "\t" << len;
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
  std::cout << serverip << std::endl;

  serverip_length += 1;
  //3. delete the server ip in the data
  for(size_t i = break_char_pos + 1; i < *len; i++){
    data[i - serverip_length] = data[i];
    data[i] = 0;
  }
  *len -= serverip_length;
  SignalReadEvent(this);
  BufferInput(false);
}

void AsyncRTSPProxyServerSocket::SendConnectResult(
  int result,const talk_base::SocketAddress& addr){

}

//////////////////////////////////////////////////////////////////////////
//Implement RTSPProxyServer
RTSPProxyServer::RTSPProxyServer(ProxySocketManagement *proxy_socket_management,
                                 AsyncP2PSocket *p2p_socket,
                                 talk_base::SocketFactory *int_factory, 
                                 const talk_base::SocketAddress &int_addr)
                                 :server_socket_(int_factory->CreateAsyncSocket(
                                 int_addr.family(),SOCK_STREAM)),
                                 p2p_socket_(p2p_socket),
                                 proxy_socket_management_(proxy_socket_management)
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
    = new RTSPServerSocketStart(p2p_socket_.get(),async_proxy_server_socket);

  //Step 4. Register RTSPServerSocketStart in ProxySocketManagement
  proxy_socket_management_->RegisterProxySocket((uint32)async_proxy_server_socket,
    rtsp_server_socket_start);
}

talk_base::AsyncProxyServerSocket* RTSPProxyServer::WrapSocket(
  talk_base::AsyncSocket* socket)
{
    return  new AsyncRTSPProxyServerSocket(socket);
}

//////////////////////////////////////////////////////////////////////////
RTSPServerSocketStart::RTSPServerSocketStart(AsyncP2PSocket * p2p_socket,
                       talk_base::AsyncProxyServerSocket *int_socket)
                       :ProxySocketBegin(p2p_socket,int_socket),
                       rtsp_socket_(int_socket)
{
  LOG(LS_INFO) << "4. " << __FUNCTION__;
  state_ = RP_CLIENT_CONNCTED;
  rtsp_socket_->SignalConnectRequest.connect(this,
    &RTSPServerSocketStart::OnConnectRequest);
  

  //rtsp_socket_->BufferInput(true);

  has_wait_data = false;
  talk_base::SocketAddress server_addr("127.0.0.1",554);
  OnConnectRequest(int_socket,server_addr);
}

void RTSPServerSocketStart::OnConnectRequest(talk_base::AsyncProxyServerSocket* socket,
                                  const talk_base::SocketAddress& addr)
{
  //Generate a p2p system command that let remote peer to create a
  //RTSP Client socket connection

  //But at first step is to create a new socket table map
  ASSERT(socket == int_socket_.get());

  LOG(LS_INFO) << "5. " << __FUNCTION__;
  socket_table_management_->AddNewLocalSocket((uint32)socket,
    (uint32)socket,TCP_SOCKET);

  //Generate system command that create RTSP client
  talk_base::ByteBuffer *create_rtsp_client_command =
    p2p_system_command_factory_->CreateRTSPClientSocket((uint32)socket,addr);

  //send the data to remote peer
  p2p_socket_->Send((uint32)socket,TCP_SOCKET,
    create_rtsp_client_command->Data(),P2PRTSPCOMMAND_LENGTH,NULL);

  //delete this command
  p2p_system_command_factory_->DeleteRTSPClientCommand(create_rtsp_client_command);

  state_ = RP_SEND_RTSP_CLIENT_CREATE_COMMAND;
}

void RTSPServerSocketStart::OnP2PReceiveData(const char *data, uint16 len){
  if(state_ == RP_SEND_RTSP_CLIENT_CREATE_COMMAND){
    LOG(LS_VERBOSE) << " state_ == RP_SEND_RTSP_CLIENT_CREATE_COMMAND";
    P2PRTSPCommand rtsp_command;
    p2p_system_command_factory_->ParseCommand(&rtsp_command,data,len);
    socket_table_management_->UpdateRemoteSocketTable(rtsp_command.server_socket_,
      rtsp_command.client_socket_);
    state_ = RP_SCUCCEED;
    if(has_wait_data){
      ReadSocketDataToBuffer(int_socket_.get(),&out_buffer_);
      WriteBufferDataToP2P(&out_buffer_);
    }
    return ;
  }
  ASSERT(state_ == RP_SCUCCEED);
  ReadP2PDataToBuffer(data,len,&in_buffer_);
  WriteBufferDataToSocket(int_socket_.get(),&in_buffer_);
}

void RTSPServerSocketStart::OnInternalRead(talk_base::AsyncSocket* socket){
  if(state_ == RP_SCUCCEED){
    ReadSocketDataToBuffer(int_socket_.get(),&out_buffer_);
    WriteBufferDataToP2P(&out_buffer_);
  }
  else{
    LOG(LS_VERBOSE) << "state_ != RP_SCUCCEED";
    has_wait_data = true;
  }
}