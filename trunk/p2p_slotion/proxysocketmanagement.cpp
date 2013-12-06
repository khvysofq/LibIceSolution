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

#include "talk/base/thread.h"


#include "proxysocketmanagement.h"
#include "proxyserverfactory.h"
#include "p2psystemcommand.h"
#include "p2pconnectionmanagement.h"
#include "proxyp2psession.h"

const int DESTORY_MYSELFT = 0;
const int SEND_CLOSE_SUCCEED = 1;
//////////////////////////////////////////////////////////////////////////
//Implement ProxySocketBegin
ProxySocketBegin::ProxySocketBegin(talk_base::AsyncSocket *int_socket)
                                   :int_socket_(int_socket),
                                   p2p_connection_implementator_(NULL),
                                   proxy_p2p_session_(NULL),
                                   out_buffer_(KBufferSize),
                                   in_buffer_(KBufferSize)
{
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  std::cout <<__FUNCTION__ << std::endl;
  client_write_date_ = false;
  if(int_socket_ == NULL){
    talk_base::Thread *thread = talk_base::Thread::Current();
    talk_base::AsyncSocket *int_socket_ 
      = thread->socketserver()->CreateAsyncSocket(SOCK_STREAM);
  }
  current_thread_          =  talk_base::Thread::Current();
  socket_table_management_ = SocketTableManagement::Instance();
  p2p_system_command_factory_ = P2PSystemCommandFactory::Instance();
  p2p_connection_management_  = P2PConnectionManagement::Instance();

  //internal_date_wait_receive_ = false;

  int_socket_->SignalReadEvent.connect(this, &ProxySocketBegin::OnInternalRead);
  int_socket_->SignalWriteEvent.connect(this, &ProxySocketBegin::OnInternalWrite);
  int_socket_->SignalCloseEvent.connect(this, &ProxySocketBegin::OnInternalClose);
}

void ProxySocketBegin::Destory(){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  std::cout <<__FUNCTION__ << std::endl;
  ASSERT(p2p_socket_state_ == SOCK_CLOSE);
  ASSERT(int_socket_state_ == SOCK_CLOSE);
  current_thread_->Post(this,DESTORY_MYSELFT);
}

bool ProxySocketBegin::IsMe(uint32 socket){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  std::cout <<__FUNCTION__ << std::endl;
  return ((uint32)int_socket_.get()) == socket;
}

void ProxySocketBegin::OnP2PRead(const char *data, uint16 len){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  std::cout <<__FUNCTION__ << std::endl;
  ASSERT(p2p_socket_state_ == SOCK_CONNECTED);
  client_write_date_ = true;
  ReadP2PDataToBuffer(data,len,&in_buffer_);
  WriteBufferDataToSocket(int_socket_.get(),&in_buffer_);
}

void ProxySocketBegin::OnP2PWrite(talk_base::StreamInterface *stream){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  std::cout <<__FUNCTION__ << std::endl;
  ASSERT(p2p_socket_state_ == SOCK_CONNECTED);
  WriteBufferDataToP2P(&out_buffer_);
}

void ProxySocketBegin::OnP2PClose(talk_base::StreamInterface *stream){
  std::cout << __FUNCTION__ << "\t p2p socket closed"
    << std::endl;
  //ASSERT(p2p_socket_state_ == SOCK_CONNECTED);
  //ASSERT(int_socket_state_ != SOCK_CLOSE);

  int_socket_->Close();
  int_socket_state_ = SOCK_CLOSE;
  current_thread_->Post(this,SEND_CLOSE_SUCCEED);
  
}

void ProxySocketBegin::OnOtherSideSocketCloseSucceed(
  talk_base::StreamInterface *stream)
{
  std::cout << __FUNCTION__ << std::endl;
  p2p_socket_state_ = SOCK_CLOSE;
  Destory();
}

void ProxySocketBegin::SendCloseSocketSucceed(){
  std::cout << __FUNCTION__ << std::endl;
  proxy_p2p_session_->P2PSocketCloseSucceed((uint32)int_socket_.get(),
    is_server_);
}

//ProxySocketBegin protected function
void ProxySocketBegin::OnInternalRead(talk_base::AsyncSocket* socket){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  std::cout <<__FUNCTION__ << std::endl;
  ReadSocketDataToBuffer(int_socket_.get(),&out_buffer_);
  
  if(p2p_socket_state_ != SOCK_CONNECTED){
    LOG(LS_ERROR) << "the p2p socket state is not socket connected";
    return ;
  }
  WriteBufferDataToP2P(&out_buffer_);
}

void ProxySocketBegin::OnInternalWrite(talk_base::AsyncSocket *socket){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  std::cout <<__FUNCTION__ << std::endl;
  if(p2p_socket_state_ != SOCK_CONNECTED 
    || int_socket_state_ != SOCK_CONNECTED){
    LOG(LS_ERROR) << "Internal write data state is error";
    return ;
  }
  WriteBufferDataToSocket(int_socket_.get(),&in_buffer_);
}

void ProxySocketBegin::OnInternalClose(talk_base::AsyncSocket* socket, int err){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  //ASSERT(int_socket_state_ != SOCK_CLOSE);
  std::cout << __FUNCTION__ << "\t int socket closed"
    << std::endl;

  std::cout << __FUNCTION__ << "close the internal socket" 
    << std::endl;
  int_socket_->Close();

  if(p2p_socket_state_ == SOCK_CLOSE)
    return ;
  //2. Send Close P2P Socket Command
  CloseP2PSocket();
  int_socket_state_ = SOCK_CLOSE;
}

void ProxySocketBegin::CloseP2PSocket(){
  ASSERT(is_server_);
  std::cout <<__FUNCTION__ << std::endl;
  proxy_p2p_session_->P2PSocketClose((uint32)int_socket_.get(),is_server_);
}

//////////////////////////////////////////////////////////////////////////
void ProxySocketBegin::ReadSocketDataToBuffer(talk_base::AsyncSocket *socket, 
                                              talk_base::FifoBuffer *buffer)
{
  std::cout <<__FUNCTION__ << std::endl;
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  // Only read if the buffer is empty.
  ASSERT(socket != NULL);
  size_t size;
  int read;
  if (buffer->GetBuffered(&size) && size == 0) {
    void* p = buffer->GetWriteBuffer(&size);
    read = socket->Recv(p, size);

    if(!is_server_ && client_write_date_ != true){
      LOG(LS_ERROR) << "Can't Send data " <<read;
      return ;
    }

    buffer->ConsumeWriteBuffer(talk_base::_max(read, 0));
  }
}

void ProxySocketBegin::ReadP2PDataToBuffer(const char *data, uint16 len,
                                           talk_base::FifoBuffer *buffer)
{
  std::cout <<__FUNCTION__ << std::endl;
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  ASSERT(data != NULL);
  ASSERT(p2p_socket_state_ == SOCK_CONNECTED);
  buffer->Write(data,len,NULL,NULL);
}

void ProxySocketBegin::WriteBufferDataToSocket(talk_base::AsyncSocket *socket,
                                               talk_base::FifoBuffer *buffer)
{
  std::cout <<__FUNCTION__ << std::endl;
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  ASSERT(socket != NULL);
  
  if(int_socket_state_ != SOCK_CONNECTED){
    LOG(LS_ERROR) << "the int_socket_ is not connected";
    return ;
  }

  size_t size;
  int written;
  const void* p = buffer->GetReadData(&size);
  if(!size){
    LOG(LS_ERROR) << "P2P Data is Zero length";
    return ;
  }
  written = socket->Send(p, size);
  std::cout << "==========================" << written << std::endl;
  buffer->ConsumeReadData(talk_base::_max(written, 0));
}

void ProxySocketBegin::WriteBufferDataToP2P(talk_base::FifoBuffer *buffer){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  std::cout <<__FUNCTION__ << std::endl;
  ASSERT(buffer != NULL);
  //std::cout <<__FUNCTION__ << std::endl;


  size_t size;
  size_t written = 0;
  const void* p = buffer->GetReadData(&size);

  if(size != 0 && p2p_socket_state_ == SOCK_CONNECTED){
    p2p_connection_implementator_->Send((uint32)int_socket_.get(),
      TCP_SOCKET,(const char *)p,size,&written);
  } else{
    LOG(LS_ERROR) << "The p2p socket state is not Socket Connected";
    written = size;
  }
  buffer->ConsumeReadData(written);
}


bool ProxySocketBegin::StartConnectBySourceIde(const std::string &source){
  //remote_peer_addr_ = addr;
  std::cout <<__FUNCTION__ << std::endl;
  ASSERT(p2p_socket_state_ == SOCK_CLOSE && int_socket_state_ == SOCK_CONNECTED);
  p2p_socket_state_ = SOCK_CONNECT_PEER;

  std::cout << __FUNCTION__ << "\t Start Connect By Source"
    << std::endl;

  bool is_existed;
  ProxyP2PSession *proxy_p2p_session = 
    p2p_connection_management_->ConnectBySourceIde(source,&remote_peer_addr_,
    &is_existed);

  if(proxy_p2p_session){
    std::cout << __FUNCTION__ << "inform peer connect succeed" << std::endl;

    proxy_p2p_session_ = proxy_p2p_session;
    p2p_connection_implementator_ 
      = proxy_p2p_session_->GetP2PConnectionImplementator();

    proxy_p2p_session_->RegisterProxySocket(this);

    if(is_existed)
      OnP2PPeerConnectSucceed(proxy_p2p_session_);
    return true;
  }
  current_thread_->Post(this,DESTORY_MYSELFT);
  return false;
}

void ProxySocketBegin::OnP2PPeerConnectSucceed(ProxyP2PSession 
                                           *proxy_p2p_session)
{
  std::cout << __FUNCTION__ << "\t Inform the peer connect succeed"
    << std::endl;
  if(!is_server_)
    return ;
  ASSERT(p2p_socket_state_ == SOCK_CONNECT_PEER);
  //initialize p2p data tunnel

  p2p_socket_state_ = SOCK_PEER_CONNECT_SUCCEED;

  
  proxy_p2p_session_->CreateClientSocketConnection((uint32)int_socket_.get(),
    remote_peer_addr_);
}

void ProxySocketBegin::OnP2PSocketConnectSucceed(ProxyP2PSession *proxy_p2p_session){
  std::cout <<__FUNCTION__ << std::endl;
  ASSERT(p2p_socket_state_ == SOCK_PEER_CONNECT_SUCCEED);
  p2p_socket_state_ = SOCK_CONNECTED;
  OnP2PWrite(NULL);
}

void ProxySocketBegin::OnMessage(talk_base::Message *msg){
  switch(msg->message_id){
  case DESTORY_MYSELFT:
    {
      std::cout << __FUNCTION__ << "DESTORY MYSELEFT" << std::endl;
      out_buffer_.Close();
      in_buffer_.Close();
      if(proxy_p2p_session_)
        proxy_p2p_session_->DeleteProxySocketBegin(this);
      else{
        delete this;
      }
      break;
    }
  case SEND_CLOSE_SUCCEED:
    {
      std::cout << __FUNCTION__ << "DESTORY MYSELEFT" << std::endl;
      SendCloseSocketSucceed();
      p2p_socket_state_ = SOCK_CLOSE;
      Destory();
    }
  }
}