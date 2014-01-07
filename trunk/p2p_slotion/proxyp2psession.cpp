/*
* p2p solution
* Copyright 2013, VZ Inc.
* 
* Author   : GuangLei He
* Email    : guangleihe@gmail.com
* Created  : 2013/12/01      12:35
* Filename : F:\GitHub\trunk\p2p_slotion\proxyp2psession.cpp
* File path: F:\GitHub\trunk\p2p_slotion
* File base: proxyp2psession
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
#include "talk/base//thread.h"

#include "proxyp2psession.h"
#include "p2psystemcommand.h"
#include "proxyserverfactory.h"
#include "p2pconnectionimplementator.h"
#include "sockettablemanagement.h"
#include "p2pconnectionmanagement.h"
#include "asyncrtspclientsocket.h"

const int DESTORY_SELFT           = 0;
const int CLOSE_ALL_PROXY_SOCKET  = 1;
const int SEND_BUFFER_DATA        = 2;
const int RELEASE_ALL             = 3;

struct SendBufferMessage :public talk_base::MessageData{
  SendBufferMessage(talk_base::ByteBuffer *byter_buffer)
    :byter_buffer_(byter_buffer){}
  talk_base::ByteBuffer *byter_buffer_;
};

ProxyP2PSession::ProxyP2PSession(talk_base::StreamInterface *stream,
                                 const std::string &remote_peer_name,
                                 talk_base::Thread *signal_thread,
                                 talk_base::Thread *worker_thread,
                                 bool is_mix_data_mode)
                                 :is_mix_data_mode_(is_mix_data_mode),
                                 worker_thread_(worker_thread),
                                 signal_thread_(signal_thread)
{
  ASSERT(signal_thread_->IsCurrent());
  LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC) << "\t Create A New Session"
    << std::endl;
  //1. Create P2PConnectionImplementator object

  p2p_connection_implementator_ = 
    new P2PConnectionImplementator(remote_peer_name,stream,
    is_mix_data_mode_);

  is_self_close               = true;

  p2p_system_command_factory_ = P2PSystemCommandFactory::Instance();
  socket_table_management_    = SocketTableManagement::Instance();
  p2p_connection_management_  = P2PConnectionManagement::Instance();

  command_send_buffer_        = new talk_base::FifoBuffer(BUFFER_SIZE);

  independent_mode_state_     = P2P_SOCKET_CLOSED;

  p2p_connection_implementator_->SignalStreamWrite.connect(this,
    &ProxyP2PSession::OnStreamWrite);

  p2p_connection_implementator_->SignalStreamClose.connect(this,
    &ProxyP2PSession::OnStreamClose);

  p2p_connection_implementator_->SignalConnectSucceed.connect(this,
    &ProxyP2PSession::OnConnectSucceed);

  p2p_connection_implementator_->SignalStreamRead.connect(this,
    &ProxyP2PSession::OnStreamRead);

  p2p_connection_implementator_->SignalIndependentStreamRead.connect(
    this,&ProxyP2PSession::OnIndependentStreamRead);

}

ProxyP2PSession::~ProxyP2PSession(){
  LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC)
    << "Destroy Proxy P2p Session";
  ASSERT(signal_thread_->IsCurrent());
  command_send_buffer_->Close();
  delete command_send_buffer_;
  ASSERT(p2p_connection_implementator_ == NULL);
}

//////////////////////////////////////////////////////////////////////////
//Implement Proxy p2p session

void ProxyP2PSession::RegisterProxySocket(ProxySocketBegin *proxy_socket_begin)
{
  ASSERT(signal_thread_->IsCurrent());

  SignalIndependentStreamRead.connect(proxy_socket_begin,
    &ProxySocketBegin::OnIndependentRead);

  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) 
    << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(
    proxy_socket_begin->GetSocketNumber());
  if(iter != proxy_socket_begin_map_.end()){
    LOG(LS_ERROR) << __FUNCTION__ << "\t the proxy socket begin is existed" 
      << std::endl;
    return ;
  }
  proxy_socket_begin_map_.insert(ProxySocketBeginMap::value_type(
    proxy_socket_begin->GetSocketNumber(),proxy_socket_begin));
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "\t Register proxy socket " 
    << proxy_socket_begin_map_.size();
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) 
    << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
}

void ProxyP2PSession::DeleteProxySocketBegin(ProxySocketBegin *proxy_socket_begin){

  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) 
    << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
  ASSERT(signal_thread_->IsCurrent());
  uint32 socket_number = proxy_socket_begin->GetSocketNumber();
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(socket_number);
  if(iter != proxy_socket_begin_map_.end()){
    proxy_socket_begin_map_.erase(iter);
    socket_table_management_->DeleteASocket(socket_number);
    delete proxy_socket_begin;
  } else{
    LOG_F(LS_ERROR) << "Delete Proxy Socket Begin error";
  }
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC|BASIC_INFOR) << "Current connect is " 
    << proxy_socket_begin_map_.size();
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) 
    << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
  IsAllProxySocketClosed();
}

///////////////////////////////////////////////////////////////////////////
//TODO:(GuangleiHe) TIME: 11/20/2013
//Destory All object at the Proxy p2p session
///////////////////////////////////////////////////////////////////////////
void ProxyP2PSession::Destory(){
  ASSERT(signal_thread_->IsCurrent());
  if(p2p_connection_implementator_ != NULL)
    p2p_connection_implementator_->Destory();
  else
    talk_base::Thread::Current()->Post(this,DESTORY_SELFT);
}

bool ProxyP2PSession::IsMe(const std::string remote_peer_name) const {
  ASSERT(signal_thread_->IsCurrent());
  return p2p_connection_implementator_->IsMe(remote_peer_name);
}

bool ProxyP2PSession::RunSocketProccess(
  uint32 socket, SocketType socket_type,const char *data, uint16 len)
{  
  ASSERT(signal_thread_->IsCurrent());
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "socket proccess";
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(socket);
  if(iter == proxy_socket_begin_map_.end()){
    LOG(LS_ERROR) << "Can't Find the object in the ProxySocketManagement " << socket;
    return false;
  }
  iter->second->OnP2PRead(data,len);
  return true;
}

void ProxyP2PSession::OnStreamClose(talk_base::StreamInterface *stream){
  ASSERT(signal_thread_->IsCurrent());
  CloseAllProxySokcet(stream);
  talk_base::Thread::Current()->Post(this,DESTORY_SELFT);
  //Destory();
}

void ProxyP2PSession::CloseAllProxySokcet(
  talk_base::StreamInterface *stream)
{
  ASSERT(signal_thread_->IsCurrent());
  // inform all proxy socket begin object that the peer is connected
  for(ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.begin();
    iter != proxy_socket_begin_map_.end();iter++){
      iter->second->OnP2PClose(stream);
  }
}

void ProxyP2PSession::OnStreamWrite(talk_base::StreamInterface *stream){
  ASSERT(signal_thread_->IsCurrent());
  //at first send remain command data
  size_t length;
  const void *p = command_send_buffer_->GetReadData(&length);
  if(length != 0){
    size_t written = 0;
    p2p_connection_implementator_->Send(0,TCP_SOCKET,
      (const char *)p,length,&written);
    if(written == length){
      command_send_buffer_->ConsumeReadData(written);
    }
  }
  // inform all proxy socket begin object that the peer can write
  for(ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.begin();
    iter != proxy_socket_begin_map_.end();iter++){
      iter->second->OnP2PWrite(stream);
  }
}

void ProxyP2PSession::OnStreamRead(uint32 socket, 
                                   SocketType socket_type,
                                   const char *data, uint16 len)
{
  ASSERT(signal_thread_->IsCurrent());
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "Read stream data socket is " << socket
    << "length is " << len;
  if(socket == 0){
    //It must a system command
    if(len != P2PRTSPCOMMAND_LENGTH){
      //If the length of this data is not equal to P2PRTSPCOMMAND_LENGTH, 
      //I'm sure the command is get error. maybe it send block.
      return ;
    }
    ProceesSystemCommand(data,len);
  }
  else if(!RunSocketProccess(socket,socket_type,data,len)){
    //never reach there
    //ASSERT(0);
  }
}

void ProxyP2PSession::OnIndependentStreamRead(
  talk_base::StreamInterface *stream)
{

  LOG_P2P(P2P_PROXY_SOCKET_DATA)
    << "independent reading data";
  ASSERT(signal_thread_->IsCurrent());
  if(independent_mode_state_ == P2P_SOCKET_PROXY_CONNECTED){
    //... ... translator data
    SignalIndependentStreamRead(stream);
    return ;
  }
  ///////////////////////////////////////////////////////////////////////////
  //BUG NOTE (GuangleiHe, 12/17/2013)
  //Maybe there has a bug.
  //Because read the p2p system command maybe getting block.
  ///////////////////////////////////////////////////////////////////////////
  char data[64];
  uint16 len = P2PRTSPCOMMAND_LENGTH;
  size_t read = 0;
  stream->Read(data,len,&read,NULL);
  if(read == 0)
    return ;
  if(len != read){
    LOG(LS_ERROR) << "read p2p system command getting error";
    return;
  }
  ProceesSystemCommand(data,len);
}

void ProxyP2PSession::OnConnectSucceed(talk_base::StreamInterface *stream){
  ASSERT(signal_thread_->IsCurrent());
  // inform all proxy socket begin object that the peer is connected
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC)
    << "inform all proxy socket begin object that the peer is connected";
  for(ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.begin();
    iter != proxy_socket_begin_map_.end();iter++){
      iter->second->OnP2PPeerConnectSucceed(this);
  }
}


//////////////////////////////////////////////////////////////////////////
//P2P System command management
void ProxyP2PSession::CreateClientSocketConnection(
  uint32 socket,const talk_base::SocketAddress& addr)
{
  ASSERT(signal_thread_->IsCurrent());
  independent_mode_state_ = P2P_SOCKET_CONNECTING_PROXY_SOCKET;

  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "\tSend create RTSP client command"
    << addr.ToString();
  //Generate system command that create RTSP client
  talk_base::ByteBuffer *byte_buffer =
    p2p_system_command_factory_->CreateRTSPClientSocket(socket,addr);

  ///////////////////////////////////////////////////////////////////////////
  //BUG NOTE (GuangleiHe, 12/1/2013)
  //Maybe there has a bug.
  //Because is the stream is block, what I can do?
  ///////////////////////////////////////////////////////////////////////////
  //send the data to remote peer
  SendCommand(byte_buffer);
}

void ProxyP2PSession::ReplayClientSocketCreateSucceed(
  uint32 server_socket, uint32 client_socket,
  const talk_base::SocketAddress &addr)
{
  ASSERT(signal_thread_->IsCurrent());
  independent_mode_state_ = P2P_SOCKET_PROXY_CONNECTED;
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "\t Replay client socket succeed";
  //socket_table_management_ = SocketTableManagement::Instance();
  //generate a reply string
  if(is_mix_data_mode_)
    socket_table_management_->AddNewLocalSocket(client_socket,
    server_socket,TCP_SOCKET);

  talk_base::ByteBuffer *byte_buffer = 
    p2p_system_command_factory_->ReplyRTSPClientSocketSucceed(
    server_socket,client_socket);
  //send this string to remote peer
  SendCommand(byte_buffer);
}

void ProxyP2PSession::P2PSocketClose(uint32 socket, bool is_server){
  ASSERT(signal_thread_->IsCurrent());
  if(is_mix_data_mode_){
    MixP2PSocketClose(socket,is_server);
  }
  else{
    IndependentP2PSocketClose(socket,is_server);
  }
}

void ProxyP2PSession::MixP2PSocketClose(uint32 socket, bool is_server){
  ASSERT(signal_thread_->IsCurrent());
  uint32 server_socket, client_socket;
  is_self_close = false;
  if(is_server){
    server_socket = socket;
    client_socket = socket_table_management_->GetRemoteSocket(socket);
  } else{
    client_socket = socket;
    server_socket = socket_table_management_->GetRemoteSocket(socket);
  }

  talk_base::ByteBuffer *byte_buffer  =
    p2p_system_command_factory_->RTSPSocketClose(server_socket,client_socket,
    is_server);
  SendCommand(byte_buffer);
}

void ProxyP2PSession::IndependentP2PSocketClose(uint32 socket, 
                                                bool is_server)
{
  ASSERT(signal_thread_->IsCurrent());
  signal_thread_->Post(this,CLOSE_ALL_PROXY_SOCKET);
}

void ProxyP2PSession::P2PSocketCloseSucceed(uint32 socket, bool is_server){
  ASSERT(signal_thread_->IsCurrent());
  uint32 server_socket, client_socket;
  if(is_server){
    server_socket = socket;
    client_socket = socket_table_management_->GetRemoteSocket(socket);
  } else{
    client_socket = socket;
    server_socket = socket_table_management_->GetRemoteSocket(socket);
  }

  talk_base::ByteBuffer *byte_buffer  =
    p2p_system_command_factory_->RTSPSocketCloseSucceed(server_socket,client_socket,
    is_server);
  SendCommand(byte_buffer);
}

void ProxyP2PSession::P2PSocketConnectFailure(uint32 server_socket,
                                              uint32 client_socket)
{
  ASSERT(signal_thread_->IsCurrent());

  talk_base::ByteBuffer *byte_buffer  =
    p2p_system_command_factory_->RTSPSocketConnectFailure(
    server_socket,client_socket);

  SendCommand(byte_buffer);
}

bool ProxyP2PSession::ProceesSystemCommand(const char *data, uint16 len){
  ASSERT(signal_thread_->IsCurrent());
  
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "process System command";
  ///////////////////////////////////////////////////////////////////////////
  //BUSINESS LOGIC NOTE (GuangleiHe, 11/28/2013)
  //There didn't used P2PRTSPCommand to parse the data.
  //P2PRTSPCommand only known by P2PSystemCommandFactory
  ///////////////////////////////////////////////////////////////////////////
  //Step 1. Parse the system command.
  uint32 p2p_system_command_type;
  uint32 server_socket;
  uint32 client_socket;
  uint32 client_connection_ip;
  uint16 client_connection_port;

  if(!p2p_system_command_factory_->ParseCommand(data,len,&p2p_system_command_type,
    &server_socket,&client_socket,&client_connection_ip,&client_connection_port)){
      LOG(LS_ERROR) << "Parse the p2p system command error";
      return false;
  }

  //////////////////////////////////////////////////////////////////////////
  if(p2p_system_command_type == P2P_SYSTEM_CREATE_RTSP_CLIENT){
    LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "Create New Client Socket";
    //Step 1. Create AsyncSocket object.
    talk_base::AsyncSocket *int_socket 
      = worker_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);

    //Step 2. Create server SocketAddress
    talk_base::SocketAddress server_addr(client_connection_ip,client_connection_port);

    //Step 3. Create RTSPClient Socket
    RTSPClientSocket *rtsp_client_socket = 
      ProxyServerFactory::CreateRTSPClientSocket(this,int_socket,
      server_socket,server_addr);
  }
  else if(p2p_system_command_type == P2P_SYSTEM_CREATE_RTSP_CLIENT_SUCCEED){
    LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "Client Socket Create Succeed";
    if(is_mix_data_mode_)
      socket_table_management_->AddNewLocalSocket(server_socket,
      client_socket,TCP_SOCKET);

    independent_mode_state_ = P2P_SOCKET_PROXY_CONNECTED;
    //////////////////////////////////////////////////////////////////////////
    //there is normal business logic, some times proxsocketbegin closed.
    //so the second of socket begin map getting NULL pointer. 
    //////////////////////////////////////////////////////////////////////////
    ProxySocketBegin *res = GetProxySocketBegin(server_socket);
    if(res != NULL)
      res->OnP2PSocketConnectSucceed(this);

  }
  else if(p2p_system_command_type == P2P_SYSTEM_SERVER_SOCKET_CLOSE){
    LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "close server socket ";
    is_self_close = false;
    CloseP2PSocket(client_socket);
  }
  else if(p2p_system_command_type == P2P_SYSTEM_CLIENT_SOCKET_CLOSE){
    LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "\t close client socket";
    is_self_close = false;
    CloseP2PSocket(server_socket);
  }
  else if(p2p_system_command_type == P2P_SYSTEM_SERVER_SOCKET_CLOSE_SUCCEED){
    is_self_close = true;
    CloseP2PSocketSucceed(client_socket);
  }
  else if(p2p_system_command_type == P2P_SYSTEM_CLIENT_SOCKET_CLOSE_SUCCEED){
    is_self_close = true;
    CloseP2PSocketSucceed(server_socket);
  }
  else if(p2p_system_command_type == P2P_SYSTEM_SOCKET_CONNECT_FAILURE){
    ConnectProxySocketFailure(server_socket);
  }
  return true;
}

void ProxyP2PSession::CloseP2PSocket(uint32 socket){
  ASSERT(signal_thread_->IsCurrent());
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(socket);
  if(iter == proxy_socket_begin_map_.end()){
    LOG(LS_ERROR) << "The Close Socket is not a member of proxy socket begin map";
    //Never reach here
    ASSERT(0);
    return ;
  }
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "\t close succeed";
  proxy_socket_begin_map_[socket]->OnP2PClose(NULL);
}

void ProxyP2PSession::CloseP2PSocketSucceed(uint32 socket){
  ASSERT(signal_thread_->IsCurrent());
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(socket);
  if(iter == proxy_socket_begin_map_.end()){
    LOG(LS_ERROR) << "The Close Socket is not a member of proxy socket begin map";
    //Never reach here
    ASSERT(0);
    return ;
  }
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC)<< "Other side socket close succeed";
  proxy_socket_begin_map_[socket]->OnOtherSideSocketCloseSucceed(NULL);
}

void ProxyP2PSession::ConnectProxySocketFailure(uint32 socket){
  ASSERT(signal_thread_->IsCurrent());
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(socket);
  if(iter == proxy_socket_begin_map_.end()){
    LOG(LS_ERROR) << "The Close Socket is not a member of proxy socket begin map";
    //Never reach here
    ASSERT(0);
    return ;
  }
  proxy_socket_begin_map_[socket]->OnProxySocketConnectFailure(NULL);
}

void ProxyP2PSession::IsAllProxySocketClosed(){
  ASSERT(signal_thread_->IsCurrent());
  if(proxy_socket_begin_map_.size() == 0){
    LOG_P2P(P2P_PROXY_SOCKET_LOGIC | BASIC_INFOR) 
      << "proxy_socket_begin_map_.size() == 0";
    ///////////////////////////////////////////////////////////////////////////
    //BUSINESS LOGIC NOTE (GuangleiHe, 12/5/2013)
    //Because to close a stream (a ICE p2p connection), the ice protect has it 
    //own method to close this, if you close this stream before the ice close 
    // there will getting the bug.
    ///////////////////////////////////////////////////////////////////////////
    //if(is_self_close)
    //  p2p_connection_implementator_->CloseStream();
    // Destory();
    //signal_thread_->PostDelayed(1000 * 60,this,RELEASE_ALL);
  }
}

void ProxyP2PSession::SendCommand(talk_base::ByteBuffer *byte_buffer){
  ASSERT(signal_thread_->IsCurrent());
  signal_thread_->Post(this,SEND_BUFFER_DATA,
    new SendBufferMessage(byte_buffer));
}

void ProxyP2PSession::OnMessage(talk_base::Message *msg){
  ASSERT(signal_thread_->IsCurrent());
  switch(msg->message_id){
  case DESTORY_SELFT:
    {
      if(p2p_connection_implementator_ != NULL){
        LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC)
          << "delete p2p connection implementation";
        delete p2p_connection_implementator_;
        p2p_connection_implementator_ = NULL;
      }
      if(proxy_socket_begin_map_.size() == 0){
        LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC)
          << "delete proxy p2p session";
        p2p_connection_management_->DeleteProxyP2PSession(this);
      }
      break;
    }
  case CLOSE_ALL_PROXY_SOCKET:
    {
      // inform all proxy socket begin object that the peer is connected
      for(ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.begin();
        iter != proxy_socket_begin_map_.end();iter++){
          iter->second->OnOtherSideSocketCloseSucceed(NULL);
      }
      break;
    }
  case SEND_BUFFER_DATA:
    {        
      SendBufferMessage* params = 
        static_cast<SendBufferMessage*>(msg->pdata);

      //send this string to remote peer
      size_t written;
      ///////////////////////////////////////////////////////////////////
      //when p2p connection implementation is NULL, is can't send message
      //it is normal but it very important 
      ///////////////////////////////////////////////////////////////////
      if(p2p_connection_implementator_ == NULL){
      //delete the string
      p2p_system_command_factory_->DeleteRTSPClientCommand(
        params->byter_buffer_);
        delete params;
        return ;
      }

      p2p_connection_implementator_->Send(0,TCP_SOCKET, 
        params->byter_buffer_->Data(),
        P2PRTSPCOMMAND_LENGTH,&written);
      if(written != P2PRTSPCOMMAND_LENGTH){
        size_t res;
        command_send_buffer_->WriteAll(params->byter_buffer_->Data(),
          params->byter_buffer_->Length(),
          &res,NULL);
        ///////////////////////////////////////////////////////////////////////////
        //BUG NOTE (GuangleiHe, 12/5/2013)
        //Maybe there has a bug.
        //Because write data to the FIFO buffer maybe got block. the current operator
        //just exit the program.
        ///////////////////////////////////////////////////////////////////////////
        ASSERT(res == P2PRTSPCOMMAND_LENGTH);
      }
      break;
    }
  case RELEASE_ALL:
    {
      if(proxy_socket_begin_map_.size() == 0)
        Destory();
      break;
    }
  }
}
