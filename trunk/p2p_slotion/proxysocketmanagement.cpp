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
  LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC) 
    << "Create Proxy Socket Begin Object";
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
  LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC) 
    << "Destroy Proxy Socket Begin Object";
  out_buffer_.Close();
  in_buffer_.Close();
}

void ProxySocketBegin::Destory(){
  LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC) 
    << "Destroy Proxy Socket Begin Object";

  ASSERT(p2p_socket_state_ == P2P_SOCKET_CLOSED);
  ASSERT(int_socket_state_ == INT_SOCKET_CLOSED);

  current_thread_->Post(this,DESTORY_MYSELFT);
}

bool ProxySocketBegin::IsMe(intptr_t socket){
  return ((intptr_t)int_socket_.get()) == socket;
}

void ProxySocketBegin::OnP2PRead(const char *data, uint16 len){
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "Read P2P Data";

  ASSERT(p2p_socket_state_ == P2P_SOCKET_PROXY_CONNECTED);
  ReadP2PDataToBuffer(data,len,&in_buffer_);
  if(int_socket_state_ == INT_SOCKET_CONNECTED)
    WriteBufferDataToSocket(int_socket_.get(),&in_buffer_);
  else LOG(LS_ERROR) << __FUNCTION__;
}

void ProxySocketBegin::OnIndependentRead(talk_base::StreamInterface *stream)
{
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "Read Independent P2P Data";
  if(p2p_socket_state_ == P2P_SOCKET_PROXY_CONNECTED)
    ReadP2PStreamDatatoBuffer(stream,&in_buffer_);
  if(int_socket_state_ == INT_SOCKET_CONNECTED)
    WriteBufferDataToSocket(int_socket_.get(),&in_buffer_);
  else LOG(LS_ERROR) << __FUNCTION__;
}

void ProxySocketBegin::OnP2PWrite(talk_base::StreamInterface *stream){
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "P2P Socket Can write";
  if(p2p_socket_state_ == P2P_SOCKET_PROXY_CONNECTED)
    WriteBufferDataToP2P(&out_buffer_);
  else LOG(LS_ERROR) << __FUNCTION__;
}

void ProxySocketBegin::OnP2PClose(talk_base::StreamInterface *stream){
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "p2p socket closed"
    << std::endl;

  p2p_socket_state_ = P2P_SOCKET_CLOSING;
  current_thread_->Post(this,SEND_CLOSE_SUCCEED);

  int_socket_->Close();
  int_socket_state_ = INT_SOCKET_CLOSED;
}

void ProxySocketBegin::OnOtherSideSocketCloseSucceed(
  talk_base::StreamInterface *stream)
{
  LOG_P2P(P2P_PROXY_SOCKET_DATA|P2P_PROXY_SOCKET_LOGIC) 
    << "Other side socket close succeed";
  p2p_socket_state_ = P2P_SOCKET_CLOSED;
  Destory();
}

void ProxySocketBegin::OnProxySocketConnectFailure(
  talk_base::StreamInterface *stream)
{
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "P2P Socket connect failure";
  ASSERT(p2p_socket_state_ == P2P_SOCKET_CONNECTING_PROXY_SOCKET);
  ASSERT(int_socket_state_ == INT_SOCKET_CONNECTED);

  p2p_socket_state_ = P2P_SOCKET_CLOSED;
  int_socket_->Close();
  int_socket_state_ = INT_SOCKET_CLOSED;
  Destory();
}


void ProxySocketBegin::SendCloseSocketSucceed(){
  LOG_P2P(P2P_PROXY_SOCKET_DATA|P2P_PROXY_SOCKET_LOGIC) 
    << "Send Close Socket Succeed";
  proxy_p2p_session_->P2PSocketCloseSucceed((intptr_t)int_socket_.get(),
    is_server_);
  p2p_socket_state_ = P2P_SOCKET_CLOSING;
}

//ProxySocketBegin protected function
void ProxySocketBegin::OnInternalRead(talk_base::AsyncSocket* socket){
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "Internal socket read";
  
  if(int_socket_state_ == INT_SOCKET_CONNECTED)
    ReadSocketDataToBuffer(int_socket_.get(),&out_buffer_);
  else LOG(LS_ERROR) << __FUNCTION__;

  if(p2p_socket_state_ == P2P_SOCKET_PROXY_CONNECTED)
    WriteBufferDataToP2P(&out_buffer_);
  else LOG(LS_ERROR) << __FUNCTION__;
}

void ProxySocketBegin::OnInternalWrite(talk_base::AsyncSocket *socket){
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "Internal socket write";

  if(int_socket_state_ == INT_SOCKET_CONNECTED)
    WriteBufferDataToSocket(int_socket_.get(),&in_buffer_);
  else LOG(LS_ERROR) << __FUNCTION__;
}

void ProxySocketBegin::OnInternalClose(talk_base::AsyncSocket* socket, int err){
  LOG_P2P(P2P_PROXY_SOCKET_DATA|P2P_PROXY_SOCKET_LOGIC) 
    << "close the internal socket " << err ;

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
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "P2P Socket close";
  proxy_p2p_session_->P2PSocketClose((intptr_t)int_socket_.get(),is_server_);
}

//////////////////////////////////////////////////////////////////////////
void ProxySocketBegin::ReadSocketDataToBuffer(talk_base::AsyncSocket *socket, 
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

void ProxySocketBegin::ReadP2PDataToBuffer(const char *data, uint16 len,
                                           talk_base::FifoBuffer *buffer)
{
  ASSERT(data != NULL);
  buffer->Write(data,len,NULL,NULL);
}

void ProxySocketBegin::ReadP2PStreamDatatoBuffer(
  talk_base::StreamInterface *stream,talk_base::FifoBuffer *buffer)
{
  size_t size;
  size_t read = 0;
  if (buffer->GetBuffered(&size) && size == 0) {
    void* p = buffer->GetWriteBuffer(&size);
    stream->Read(p, size,&read,NULL);
    buffer->ConsumeWriteBuffer(read);
  }
}

void ProxySocketBegin::WriteBufferDataToSocket(talk_base::AsyncSocket *socket,
                                               talk_base::FifoBuffer *buffer)
{
  ASSERT(socket != NULL);
  size_t size;
  int written;
  const void* p = buffer->GetReadData(&size);
  if(!size){
    return ;
  }
  written = socket->Send(p, size);
  buffer->ConsumeReadData(talk_base::_max(written, 0));
}

void ProxySocketBegin::WriteBufferDataToP2P(talk_base::FifoBuffer *buffer){
  ASSERT(buffer != NULL);
  size_t size;
  size_t written = 0;
  const void* p = buffer->GetReadData(&size);
  p2p_connection_implementator_->Send((intptr_t)int_socket_.get(),
    TCP_SOCKET,(const char *)p,size,&written);
  buffer->ConsumeReadData(written);
}

bool ProxySocketBegin::StartConnectBySourceIde(const std::string &source){
  //remote_peer_addr_ = addr;
  ASSERT(p2p_socket_state_ == P2P_SOCKET_START 
    && int_socket_state_ == INT_SOCKET_CONNECTED);

  p2p_socket_state_ = P2P_SOCKET_CONNECTING_PEER;

  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "\t Start Connect By Source"
    << std::endl;

  bool is_existed;
  ProxyP2PSession *proxy_p2p_session = 
    p2p_connection_management_->ConnectBySourceIde(source,&remote_peer_addr_,
    &is_existed);

  //If the Proxy p2p session created
  if(proxy_p2p_session){
    
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "inform peer connect succeed";
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
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "P2P Peer Connect Succeed";
    
  ///////////////////////////////////////////////////////////////////////////
  //BUSINESS LOGIC NOTE (GuangleiHe, 12/9/2013)
  //the blow code show that this is a client, it do not need to create 
  //client socket
  ///////////////////////////////////////////////////////////////////////////
  //if(p2p_socket_state_ == P2P_SOCKET_CONNECTING_PROXY_SOCKET)
  //  return ;
  if(!is_server_)
    return ;

  ASSERT(p2p_socket_state_ == P2P_SOCKET_CONNECTING_PEER);

  p2p_socket_state_ = P2P_SOCKET_PEER_CONNECTED;

  SetProxyP2PSession(proxy_p2p_session);
  proxy_p2p_session_->CreateClientSocketConnection((intptr_t)int_socket_.get(),
    remote_peer_addr_);

  p2p_socket_state_ = P2P_SOCKET_CONNECTING_PROXY_SOCKET;
}

void ProxySocketBegin::OnP2PSocketConnectSucceed(ProxyP2PSession *proxy_p2p_session){
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "P2P socket Connect Succeed";
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
      
      LOG_P2P(P2P_PROXY_SOCKET_LOGIC|BASIC_INFOR) << "DESTORY MYSELEFT";
      if(proxy_p2p_session_)
        proxy_p2p_session_->DeleteProxySocketBegin(this);
      else{
        LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "delete this";
        delete this;
      }
      break;
    }
  case SEND_CLOSE_SUCCEED:
    {
      LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "SEND_CLOSE_SUCCEED";
      SendCloseSocketSucceed();
      p2p_socket_state_ = P2P_SOCKET_CLOSED;
      Destory();
    }
  }
}
