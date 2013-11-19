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
#include "asyncrtspproxysocketserver.h"

//////////////////////////////////////////////////////////////////////////
AsyncRTSPProxyServerSocket::AsyncRTSPProxyServerSocket(
  talk_base::AsyncSocket* socket)
  :AsyncProxyServerSocket(socket,KBufferSize)
{
  //Turn off data process in AsyncProxyServerSocket
  BufferInput(true);
}

void AsyncRTSPProxyServerSocket::ProcessInput(char* data, size_t* len) {

}

void AsyncRTSPProxyServerSocket::SendConnectResult(
  int result,const talk_base::SocketAddress& addr){

}

//////////////////////////////////////////////////////////////////////////
RTSPProxyServer::RTSPProxyServer(AbstractVirtualNetwork *virtual_network,
                                 talk_base::SocketFactory *int_factory, 
                                 const talk_base::SocketAddress &int_addr)
                                 :server_socket_(int_factory->CreateAsyncSocket(
                                 int_addr.family(),SOCK_STREAM)),
                                 virtual_network_(virtual_network)
{
  ASSERT(server_socket_.get() != NULL);
  ASSERT(int_addr.family() == AF_INET || int_addr.family() == AF_INET6);

  server_socket_->Bind(int_addr);
  server_socket_->Listen(5);
  server_socket_->SignalReadEvent.connect(this, &RTSPProxyServer::OnAcceptEvent);
}

void RTSPProxyServer::OnAcceptEvent(talk_base::AsyncSocket* socket){
  ASSERT(socket != NULL && socket == server_socket_.get());
  talk_base::AsyncSocket *new_socket_client_connection
    = socket->Accept(NULL);

  proxy_start_map_.insert(ProxyStartMap::value_type(
    (uint32)new_socket_client_connection,
    new ProxyStart(virtual_network_,WrapSocket(new_socket_client_connection)))
    );
}

talk_base::AsyncProxyServerSocket* RTSPProxyServer::WrapSocket(
  talk_base::AsyncSocket* socket)
{
    return  new AsyncRTSPProxyServerSocket(socket);
}


//////////////////////////////////////////////////////////////////////////
ProxyStart::ProxyStart(AbstractVirtualNetwork *virtual_network,
                       talk_base::AsyncProxyServerSocket *int_socket)
                       :int_socket_(int_socket),virtual_network_(virtual_network),
                       out_buffer_(KBufferSize),in_buffer_(KBufferSize)
{
  socket_table_management_ = SocketTableManagement::Instance();
  p2p_system_command_factory_ = P2PSystemCommandFactory::Instance();

  int_socket_->SignalConnectRequest.connect(this,
    &ProxyStart::OnConnectRequest);
  int_socket_->SignalReadEvent.connect(this, &ProxyStart::OnInternalRead);
  int_socket_->SignalWriteEvent.connect(this, &ProxyStart::OnInternalWrite);
  int_socket_->SignalCloseEvent.connect(this, &ProxyStart::OnInternalClose);
}

void ProxyStart::OnConnectRequest(talk_base::AsyncProxyServerSocket* socket,
                                  const talk_base::SocketAddress& addr)
{
  //Generate a p2p system command that let remote peer to create a
  //RTSP Client socket connection

  //But at first step is to create a new socket table map
  socket_table_management_->AddNewLocalSocket((uint32)socket,
    (uint32)socket,TCP_SOCKET);

  //Generate system command that create RTSP client
  const char *create_rtsp_client_command =
    p2p_system_command_factory_->CreateRTSPClientSocket((uint32)socket,addr);

  //send the data to remote peer
  virtual_network_->OnReceiveDataFromUpLayer((uint32)socket,TCP_SOCKET,
    create_rtsp_client_command,RTSP_CLIENT_SOCKET_COMMAND);

  //delete this command
  p2p_system_command_factory_->DeleteRTSPClientCommand(create_rtsp_client_command);

}

void ProxyStart::OnInternalRead(talk_base::AsyncSocket* socket){
  ReadSocketDataToBuffer(int_socket_.get(),&out_buffer_);
  WriteBufferDataToP2P(&out_buffer_);
}

void ProxyStart::OnInternalWrite(talk_base::AsyncSocket *socket){
  WriteBufferDataToP2P(&in_buffer_);
}

void ProxyStart::OnInternalClose(talk_base::AsyncSocket* socket, int err){

}

void ProxyStart::OnP2PReceiveData(const char *data, uint16 len){
  ReadP2PDataToBuffer(data,len,&in_buffer_);
  WriteBufferDataToSocket(int_socket_.get(),&in_buffer_);
}

void ProxyStart::ReadSocketDataToBuffer(talk_base::AsyncSocket *socket, 
                                        talk_base::FifoBuffer *buffer)
{
  // Only read if the buffer is empty.
  ASSERT(socket != NULL);
  size_t size;
  int read;
  if (buffer->GetBuffered(&size) && size == 0) {
    void* p = buffer->GetWriteBuffer(&size);
    read = socket->Recv(p, size);
    buffer->ConsumeWriteBuffer(talk_base::_max(read, 0));
  }
}

void ProxyStart::ReadP2PDataToBuffer(const char *data, uint16 len,
                                     talk_base::FifoBuffer *buffer)
{
  ASSERT(data != NULL);
  buffer->WriteAll(data,len,NULL,NULL);
}

void ProxyStart::WriteBufferDataToSocket(talk_base::AsyncSocket *socket,
                                         talk_base::FifoBuffer *buffer)
{
  ASSERT(socket != NULL);
  size_t size;
  int written;
  const void* p = buffer->GetReadData(&size);
  written = socket->Send(p, size);
  buffer->ConsumeReadData(talk_base::_max(written, 0));
}

void ProxyStart::WriteBufferDataToP2P(talk_base::FifoBuffer *buffer){
  ASSERT(buffer != NULL);
  size_t size;
  const void* p = buffer->GetReadData(&size);
  virtual_network_->OnReceiveDataFromUpLayer(
    (uint32)int_socket_.get(),TCP_SOCKET,(const char *)p,size);
  buffer->ConsumeReadData(size);
}