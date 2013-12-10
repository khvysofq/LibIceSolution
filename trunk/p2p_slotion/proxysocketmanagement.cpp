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

const int DESTORY_MYSELFT       = 0;
const int SEND_CLOSE_SUCCEED    = 1;
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
ProxySocketBegin::~ProxySocketBegin(){
  out_buffer_.Close();
  in_buffer_.Close();
}

void ProxySocketBegin::Destory(){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  std::cout <<__FUNCTION__ << std::endl;

  ASSERT(p2p_socket_state_ == P2P_SOCKET_CLOSED);
  ASSERT(int_socket_state_ == INT_SOCKET_CLOSED);

  current_thread_->Post(this,DESTORY_MYSELFT);
}

bool ProxySocketBegin::IsMe(uint32 socket){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  std::cout << __FUNCTION__ << std::endl;
  return ((uint32)int_socket_.get()) == socket;
}

void ProxySocketBegin::OnP2PRead(const char *data, uint16 len){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;

  ASSERT(p2p_socket_state_ == P2P_SOCKET_PROXY_CONNECTED);
  ReadP2PDataToBuffer(data,len,&in_buffer_);
  if(int_socket_state_ == INT_SOCKET_CONNECTED)
    WriteBufferDataToSocket(int_socket_.get(),&in_buffer_);
  else LOG(LS_ERROR) << __FUNCTION__;
}

void ProxySocketBegin::OnP2PWrite(talk_base::StreamInterface *stream){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  if(p2p_socket_state_ == P2P_SOCKET_PROXY_CONNECTED)
    WriteBufferDataToP2P(&out_buffer_);
  else LOG(LS_ERROR) << __FUNCTION__;
}

void ProxySocketBegin::OnP2PClose(talk_base::StreamInterface *stream){
  std::cout << __FUNCTION__ << "\t p2p socket closed"
    << std::endl;

  p2p_socket_state_ = P2P_SOCKET_CLOSING;
  current_thread_->Post(this,SEND_CLOSE_SUCCEED);

  int_socket_->Close();
  int_socket_state_ = INT_SOCKET_CLOSED;
}

void ProxySocketBegin::OnOtherSideSocketCloseSucceed(
  talk_base::StreamInterface *stream)
{
  std::cout << __FUNCTION__ << std::endl;
  p2p_socket_state_ = P2P_SOCKET_CLOSED;
  Destory();
}

void ProxySocketBegin::OnProxySocketConnectFailure(
  talk_base::StreamInterface *stream)
{
  std::cout << __FUNCTION__ << std::endl;

  ASSERT(p2p_socket_state_ == P2P_SOCKET_CONNECTING_PROXY_SOCKET);
  ASSERT(int_socket_state_ == INT_SOCKET_CONNECTED);

  p2p_socket_state_ = P2P_SOCKET_CLOSED;
  int_socket_->Close();
  int_socket_state_ = INT_SOCKET_CLOSED;
  Destory();
}


void ProxySocketBegin::SendCloseSocketSucceed(){
  std::cout << __FUNCTION__ << std::endl;

  proxy_p2p_session_->P2PSocketCloseSucceed((uint32)int_socket_.get(),
    is_server_);
  p2p_socket_state_ = P2P_SOCKET_CLOSING;
}

//ProxySocketBegin protected function
void ProxySocketBegin::OnInternalRead(talk_base::AsyncSocket* socket){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  
  if(int_socket_state_ == INT_SOCKET_CONNECTED)
    ReadSocketDataToBuffer(int_socket_.get(),&out_buffer_);
  else LOG(LS_ERROR) << __FUNCTION__;

  if(p2p_socket_state_ == P2P_SOCKET_PROXY_CONNECTED)
    WriteBufferDataToP2P(&out_buffer_);
  else LOG(LS_ERROR) << __FUNCTION__;
}

void ProxySocketBegin::OnInternalWrite(talk_base::AsyncSocket *socket){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;

  if(int_socket_state_ == INT_SOCKET_CONNECTED)
    WriteBufferDataToSocket(int_socket_.get(),&in_buffer_);
  else LOG(LS_ERROR) << __FUNCTION__;
}

void ProxySocketBegin::OnInternalClose(talk_base::AsyncSocket* socket, int err){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  std::cout << __FUNCTION__ << std::endl;
  std::cout << __FUNCTION__ << "close the internal socket " << err 
    << std::endl;

  int_socket_->Close();
  int_socket_state_ = INT_SOCKET_CLOSED;

  if(p2p_socket_state_ != P2P_SOCKET_PROXY_CONNECTED){
    InternalSocketError(socket,err);
    return ;
  }

  CloseP2PSocket();
}

void ProxySocketBegin::InternalSocketError(talk_base::AsyncSocket* socket, int err){
  //This function is not for there, its for ClientProxySocket
  p2p_socket_state_ = P2P_SOCKET_CLOSED;
  Destory();
}

void ProxySocketBegin::CloseP2PSocket(){
  //ASSERT(is_server_);
  std::cout <<__FUNCTION__ << std::endl;
  proxy_p2p_session_->P2PSocketClose((uint32)int_socket_.get(),is_server_);
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
  size_t size;
  int written;
  const void* p = buffer->GetReadData(&size);
  if(!size){
    LOG(LS_ERROR) << "P2P Data is Zero length";
    return ;
  }
  written = socket->Send(p, size);
  buffer->ConsumeReadData(talk_base::_max(written, 0));
}

void ProxySocketBegin::WriteBufferDataToP2P(talk_base::FifoBuffer *buffer){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  ASSERT(buffer != NULL);
  size_t size;
  size_t written = 0;
  const void* p = buffer->GetReadData(&size);
  p2p_connection_implementator_->Send((uint32)int_socket_.get(),
    TCP_SOCKET,(const char *)p,size,&written);
  buffer->ConsumeReadData(written);
}

bool ProxySocketBegin::StartConnectBySourceIde(const std::string &source){
  //remote_peer_addr_ = addr;
  std::cout <<__FUNCTION__ << std::endl;
  ASSERT(p2p_socket_state_ == P2P_SOCKET_START 
    && int_socket_state_ == INT_SOCKET_CONNECTED);

  p2p_socket_state_ = P2P_SOCKET_CONNECTING_PEER;

  std::cout << __FUNCTION__ << "\t Start Connect By Source"
    << std::endl;

  bool is_existed;
  ProxyP2PSession *proxy_p2p_session = 
    p2p_connection_management_->ConnectBySourceIde(source,&remote_peer_addr_,
    &is_existed);

  //If the Proxy p2p session created
  if(proxy_p2p_session){
    std::cout << __FUNCTION__ << "inform peer connect succeed" << std::endl;
    SetProxyP2PSession(proxy_p2p_session);
    if(is_existed)
      OnP2PPeerConnectSucceed(proxy_p2p_session_);
    return true;
  }
  //else
  p2p_socket_state_ = P2P_SOCKET_CLOSED;
  int_socket_->Close();
  int_socket_state_ = INT_SOCKET_CLOSED;
  Destory();
  return false;
}

void ProxySocketBegin::OnP2PPeerConnectSucceed(ProxyP2PSession 
                                               *proxy_p2p_session)
{
  std::cout << __FUNCTION__ << "\t Inform the peer connect succeed"
    << std::endl;
  ///////////////////////////////////////////////////////////////////////////
  //BUSINESS LOGIC NOTE (GuangleiHe, 12/9/2013)
  //the blow code show that this is a client, it do not need to create 
  //client socket
  ///////////////////////////////////////////////////////////////////////////
  if(!is_server_)
    return ;

  ASSERT(p2p_socket_state_ == P2P_SOCKET_CONNECTING_PEER);

  p2p_socket_state_ = P2P_SOCKET_PEER_CONNECTED;

  SetProxyP2PSession(proxy_p2p_session);
  proxy_p2p_session_->CreateClientSocketConnection((uint32)int_socket_.get(),
    remote_peer_addr_);

  p2p_socket_state_ = P2P_SOCKET_CONNECTING_PROXY_SOCKET;
}

void ProxySocketBegin::OnP2PSocketConnectSucceed(ProxyP2PSession *proxy_p2p_session){
  std::cout <<__FUNCTION__ << std::endl;
  ASSERT(p2p_socket_state_ == P2P_SOCKET_CONNECTING_PROXY_SOCKET);
  p2p_socket_state_ = P2P_SOCKET_PROXY_CONNECTED;
  OnP2PWrite(NULL);
}

void ProxySocketBegin::SetProxyP2PSession(ProxyP2PSession *proxy_p2p_session){
  if(proxy_p2p_session_ != NULL)
    return ;
  proxy_p2p_session_ = proxy_p2p_session;
  p2p_connection_implementator_ 
    = proxy_p2p_session_->GetP2PConnectionImplementator();
  proxy_p2p_session_->RegisterProxySocket(this);
}

void ProxySocketBegin::OnMessage(talk_base::Message *msg){
  switch(msg->message_id){
  case DESTORY_MYSELFT:
    {
      std::cout << __FUNCTION__ << "DESTORY MYSELEFT" << std::endl;
      if(proxy_p2p_session_)
        proxy_p2p_session_->DeleteProxySocketBegin(this);
      else{
        std::cout << "delete this" << std::endl;
        delete this;
      }
      break;
    }
  case SEND_CLOSE_SUCCEED:
    {
      std::cout << __FUNCTION__ << "SEND_CLOSE_SUCCEED" << std::endl;
      SendCloseSocketSucceed();
      p2p_socket_state_ = P2P_SOCKET_CLOSED;
      Destory();
    }
  }
}