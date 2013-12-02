/*
* p2p solution
* Copyright 2013, VZ Inc.
* 
* Author   : GuangLei He
* Email    : guangleihe@gmail.com
* Created  : 2013/11/20      10:57
* Filename : F:\GitHub\trunk\p2p_slotion\proxysocketmanagement.cpp
* File path: F:\GitHub\trunk\p2p_slotion
* File base: proxysocketmanagement
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

#include "talk/base/thread.h"
#include "proxysocketmanagement.h"
#include "p2psystemcommand.h"
#include "p2pconnectionmanagement.h"
#include "proxyp2psession.h"


//////////////////////////////////////////////////////////////////////////
//Implement ProxySocketBegin
ProxySocketBegin::ProxySocketBegin(talk_base::AsyncSocket *int_socket)
                                   :int_socket_(int_socket),
                                   p2p_connection_implementator_(NULL),
                                   proxy_p2p_session_(NULL),
                                   out_buffer_(KBufferSize),
                                   in_buffer_(KBufferSize)
{
  socket_table_management_ = SocketTableManagement::Instance();
  p2p_system_command_factory_ = P2PSystemCommandFactory::Instance();
  p2p_connection_management_  = P2PConnectionManagement::Instance();

  //internal_date_wait_receive_ = false;

  int_socket_->SignalReadEvent.connect(this, &ProxySocketBegin::OnInternalRead);
  int_socket_->SignalWriteEvent.connect(this, &ProxySocketBegin::OnInternalWrite);
  int_socket_->SignalCloseEvent.connect(this, &ProxySocketBegin::OnInternalClose);
}

bool ProxySocketBegin::IsMe(uint32 socket){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  return ((uint32)int_socket_.get()) == socket;
}

void ProxySocketBegin::OnP2PRead(const char *data, uint16 len){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;

  ReadP2PDataToBuffer(data,len,&in_buffer_);
  WriteBufferDataToSocket(int_socket_.get(),&in_buffer_);
}

void ProxySocketBegin::OnP2PWrite(talk_base::StreamInterface *stream){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  ASSERT(p2p_socket_state_ == SOCK_CONNECTED);
  WriteBufferDataToP2P(&out_buffer_);
}

void ProxySocketBegin::OnP2PClose(talk_base::StreamInterface *stream){
  int_socket_->Close();
}

//ProxySocketBegin protected function
void ProxySocketBegin::OnInternalRead(talk_base::AsyncSocket* socket){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  ReadSocketDataToBuffer(int_socket_.get(),&out_buffer_);
  WriteBufferDataToP2P(&out_buffer_);
}

void ProxySocketBegin::OnInternalWrite(talk_base::AsyncSocket *socket){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  WriteBufferDataToSocket(int_socket_.get(),&in_buffer_);
}

void ProxySocketBegin::OnInternalClose(talk_base::AsyncSocket* socket, int err){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  CloseP2PSocket();
}

void ProxySocketBegin::CloseP2PSocket(){
  if(is_server_)
    proxy_p2p_session_->P2PServerSocketClose((uint32)int_socket_.get());
  else
    proxy_p2p_session_->P2PClientSocketClose((uint32)int_socket_.get());
}

//////////////////////////////////////////////////////////////////////////
void ProxySocketBegin::ReadSocketDataToBuffer(talk_base::AsyncSocket *socket, 
                                              talk_base::FifoBuffer *buffer)
{
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
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

void ProxySocketBegin::ReadP2PDataToBuffer(const char *data, uint16 len,
                                           talk_base::FifoBuffer *buffer)
{
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  ASSERT(data != NULL);
  buffer->Write(data,len,NULL,NULL);
}

void ProxySocketBegin::WriteBufferDataToSocket(talk_base::AsyncSocket *socket,
                                               talk_base::FifoBuffer *buffer)
{
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  ASSERT(socket != NULL);
  
  if(int_socket_state_ != SOCK_CONNECTED)
    return ;

  size_t size;
  int written;
  const void* p = buffer->GetReadData(&size);
  written = socket->Send(p, size);
  buffer->ConsumeReadData(talk_base::_max(written, 0));
}

void ProxySocketBegin::WriteBufferDataToP2P(talk_base::FifoBuffer *buffer){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  ASSERT(buffer != NULL);

  if(p2p_socket_state_ != SOCK_CONNECTED){
    return ;
  }

  size_t size;
  size_t written = 0;
  const void* p = buffer->GetReadData(&size);
  p2p_connection_implementator_->Send((uint32)int_socket_.get(),
    TCP_SOCKET,(const char *)p,size,&written);
  buffer->ConsumeReadData(written);
}

bool ProxySocketBegin::StartConnect(const talk_base::SocketAddress& addr){
  //remote_peer_addr_ = addr;
  ASSERT(int_socket_state_ == SOCK_CONNECTED
    && p2p_socket_state_ == SOCK_CLOSE);
  p2p_socket_state_ = SOCK_CONNECT_PEER;

  remote_peer_addr_ = addr;

  p2p_connection_management_->Connect(this,addr,&proxy_p2p_session_);
  if(proxy_p2p_session_){
    p2p_connection_implementator_ 
      = proxy_p2p_session_->GetP2PConnectionImplementator();
    p2p_socket_state_ = SOCK_PEER_CONNECT_SUCCEED;
    return true;
    //Needs states change
  }
  return false;
}

bool ProxySocketBegin::StartConnectBySourceIde(const std::string &source){
  //remote_peer_addr_ = addr;
  ASSERT(int_socket_state_ == SOCK_CONNECTED
    && p2p_socket_state_ == SOCK_CLOSE);
  p2p_socket_state_ = SOCK_CONNECT_PEER;


  p2p_connection_management_->ConnectBySourceIde(this,source,
    &proxy_p2p_session_,&remote_peer_addr_);
  if(proxy_p2p_session_){
    p2p_connection_implementator_ 
      = proxy_p2p_session_->GetP2PConnectionImplementator();
    p2p_socket_state_ = SOCK_PEER_CONNECT_SUCCEED;
    return true;
    //Needs states change
  }
  return false;
}

void ProxySocketBegin::OnP2PPeerConnectSucceed(ProxyP2PSession 
                                           *proxy_p2p_session)
{
  if(!is_server_)
    return ;
  ASSERT(int_socket_state_ == SOCK_CONNECTED
    && p2p_socket_state_ == SOCK_CONNECT_PEER);
  //initialize p2p data tunnel
  ASSERT(p2p_connection_implementator_ == NULL);
  ASSERT(proxy_p2p_session_ == NULL);

  proxy_p2p_session_ = proxy_p2p_session;
  p2p_connection_implementator_ 
    = proxy_p2p_session->GetP2PConnectionImplementator();

  p2p_socket_state_ = SOCK_PEER_CONNECT_SUCCEED;
  proxy_p2p_session_->CreateClientSocketConnection((uint32)int_socket_.get(),
    remote_peer_addr_);
}

void ProxySocketBegin::OnP2PSocketConnectSucceed(ProxyP2PSession *proxy_p2p_session){
  ASSERT(int_socket_state_ == SOCK_CONNECTED
    && p2p_socket_state_ == SOCK_PEER_CONNECT_SUCCEED);
  p2p_socket_state_ = SOCK_CONNECTED;
  OnP2PWrite(NULL);
}