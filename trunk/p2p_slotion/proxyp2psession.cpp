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

const int DESTORY_SELFT = 0;
const int CLOSE_ALL_PROXY_SOCKET = 1;

ProxyP2PSession::ProxyP2PSession(talk_base::StreamInterface *stream,
                                 const std::string &remote_peer_name,
                                 bool is_mix_data_mode)
                                 :is_mix_data_mode_(is_mix_data_mode)
{

  std::cout << __FUNCTION__ << "\t Create A New Session"
    << std::endl;
  //1. Create P2PConnectionImplementator object

  p2p_connection_implementator_ = 
    new P2PConnectionImplementator(remote_peer_name,stream,
    is_mix_data_mode_);

  is_self_close               = true;

  p2p_system_command_factory_ = P2PSystemCommandFactory::Instance();
  socket_table_management_    = SocketTableManagement::Instance();
  p2p_connection_management_  = P2PConnectionManagement::Instance();

  current_thread_             = talk_base::Thread::Current();
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
  command_send_buffer_->Close();
  delete command_send_buffer_;
  delete p2p_connection_implementator_;
}

//////////////////////////////////////////////////////////////////////////
//Implement Proxy p2p session

void ProxyP2PSession::RegisterProxySocket(ProxySocketBegin *proxy_socket_begin)
{
  
  SignalIndependentStreamRead.connect(proxy_socket_begin,
    &ProxySocketBegin::OnIndependentRead);

  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(
    proxy_socket_begin->GetSocketNumber());
  if(iter != proxy_socket_begin_map_.end()){
    LOG(LS_ERROR) << __FUNCTION__ << "\t the proxy socket begin is existed" 
      << std::endl;
    return ;
  }
  proxy_socket_begin_map_.insert(ProxySocketBeginMap::value_type(
    proxy_socket_begin->GetSocketNumber(),proxy_socket_begin));
  std::cout << __FUNCTION__ << "\t Register proxy socket " 
    << proxy_socket_begin_map_.size() << std::endl;
  std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
}

void ProxyP2PSession::DeleteProxySocketBegin(ProxySocketBegin *proxy_socket_begin){
  std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
  uint32 socket_number = proxy_socket_begin->GetSocketNumber();
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(socket_number);
  if(iter != proxy_socket_begin_map_.end()){
    proxy_socket_begin_map_.erase(iter);
    socket_table_management_->DeleteASocket(socket_number);
    delete proxy_socket_begin;
  } else{
    LOG_F(LS_ERROR) << "Delete Proxy Socket Begin error";
  }
  std::cout << "Current connect is " << proxy_socket_begin_map_.size() << std::endl;
  std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
  IsAllProxySocketClosed();
}

///////////////////////////////////////////////////////////////////////////
//TODO:(GuangleiHe) TIME: 11/20/2013
//Destory All object at the Proxy p2p session
///////////////////////////////////////////////////////////////////////////
void ProxyP2PSession::Destory(){
  p2p_connection_implementator_->CloseStream();
  p2p_connection_implementator_->Destory();
  talk_base::Thread::Current()->Post(this,DESTORY_SELFT);
}

bool ProxyP2PSession::IsMe(const std::string remote_peer_name) const {
  return p2p_connection_implementator_->IsMe(remote_peer_name);
}

bool ProxyP2PSession::RunSocketProccess(
  uint32 socket, SocketType socket_type,const char *data, uint16 len)
{  
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(socket);
  if(iter == proxy_socket_begin_map_.end()){
    LOG(LS_ERROR) << "Can't Find the object in the ProxySocketManagement " << socket;
    return false;
  }
  iter->second->OnP2PRead(data,len);
  return true;
}

void ProxyP2PSession::OnStreamClose(talk_base::StreamInterface *stream){
  CloseAllProxySokcet(stream);
  Destory();
}

void ProxyP2PSession::CloseAllProxySokcet(
  talk_base::StreamInterface *stream)
{
  // inform all proxy socket begin object that the peer is connected
  for(ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.begin();
    iter != proxy_socket_begin_map_.end();iter++){
      iter->second->OnP2PClose(stream);
  }
}

void ProxyP2PSession::OnStreamWrite(talk_base::StreamInterface *stream){
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
  if(independent_mode_state_ == P2P_SOCKET_PROXY_CONNECTED){
    //... ... translator data
    SignalIndependentStreamRead(stream);
    return ;
  }
  std::cout << __FUNCTION__ << std::endl;
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
  // inform all proxy socket begin object that the peer is connected
  std::cout << "\t inform all proxy socket begin object that the peer is connected"
    << std::endl;
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
  independent_mode_state_ = P2P_SOCKET_CONNECTING_PROXY_SOCKET;

  std::cout << __FUNCTION__ << "\tSend create RTSP client command"
    << addr.ToString() << std::endl;
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
  independent_mode_state_ = P2P_SOCKET_PROXY_CONNECTED;
  std::cout << __FUNCTION__ << "\t Replay client socket succeed"
    << std::endl;
  LOG(LS_INFO) << __FUNCTION__;
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
  if(is_mix_data_mode_){
    MixP2PSocketClose(socket,is_server);
  }
  else{
    IndependentP2PSocketClose(socket,is_server);
  }
}

void ProxyP2PSession::MixP2PSocketClose(uint32 socket, bool is_server){
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

void ProxyP2PSession::IndependentP2PSocketClose(uint32 socket, bool is_server){
  current_thread_->Post(this,CLOSE_ALL_PROXY_SOCKET);
  std::cout << __FUNCTION__ << std::endl;
}

void ProxyP2PSession::P2PSocketCloseSucceed(uint32 socket, bool is_server){
  std::cout << __FUNCTION__ << std::endl;
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
  std::cout << __FUNCTION__ << std::endl;

  talk_base::ByteBuffer *byte_buffer  =
    p2p_system_command_factory_->RTSPSocketConnectFailure(
    server_socket,client_socket);

  SendCommand(byte_buffer);
}

bool ProxyP2PSession::ProceesSystemCommand(const char *data, uint16 len){

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
    LOG(LS_INFO) << "Create New Client Socket";
    std::cout << __FUNCTION__ << "\t Start Create New Client Socket" << std::endl;
    //Step 1. Create AsyncSocket object.
    talk_base::AsyncSocket *int_socket 
      = current_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);

    //Step 2. Create server SocketAddress
    talk_base::SocketAddress server_addr(client_connection_ip,client_connection_port);

    //Step 3. Create RTSPClient Socket
    RTSPClientSocket *rtsp_client_socket = 
      ProxyServerFactory::CreateRTSPClientSocket(this,int_socket,
      server_socket,server_addr);
  }
  else if(p2p_system_command_type == P2P_SYSTEM_CREATE_RTSP_CLIENT_SUCCEED){
    LOG(LS_INFO) << "Client Socket Create Succeed";
    std::cout << __FUNCTION__ << "\tClient Socket Create Succeed"
      << std::endl;
    if(is_mix_data_mode_)
      socket_table_management_->AddNewLocalSocket(server_socket,
      client_socket,TCP_SOCKET);

    independent_mode_state_ = P2P_SOCKET_PROXY_CONNECTED;

    GetProxySocketBegin(server_socket)->OnP2PSocketConnectSucceed(this);

  }
  else if(p2p_system_command_type == P2P_SYSTEM_SERVER_SOCKET_CLOSE){
    std::cout << __FUNCTION__ << "\t close server socket \n"
      << std::endl;
    is_self_close = false;
    CloseP2PSocket(client_socket);
  }
  else if(p2p_system_command_type == P2P_SYSTEM_CLIENT_SOCKET_CLOSE){
    std::cout << __FUNCTION__ << "\t close client socket \n"
      << std::endl;
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
  std::cout << __FUNCTION__ << std::endl;
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(socket);
  if(iter == proxy_socket_begin_map_.end()){
    LOG(LS_ERROR) << "The Close Socket is not a member of proxy socket begin map";
    //Never reach here
    ASSERT(0);
    return ;
  }
  std::cout << __FUNCTION__ << "\t close succeed" << std::endl;;
  proxy_socket_begin_map_[socket]->OnP2PClose(NULL);
}

void ProxyP2PSession::CloseP2PSocketSucceed(uint32 socket){
  std::cout << __FUNCTION__ << std::endl;
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(socket);
  if(iter == proxy_socket_begin_map_.end()){
    LOG(LS_ERROR) << "The Close Socket is not a member of proxy socket begin map";
    //Never reach here
    ASSERT(0);
    return ;
  }
  std::cout << __FUNCTION__ << "\t Other side socket close succeed";
  proxy_socket_begin_map_[socket]->OnOtherSideSocketCloseSucceed(NULL);
}

void ProxyP2PSession::ConnectProxySocketFailure(uint32 socket){
  std::cout << __FUNCTION__ << std::endl;
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
  std::cout << __FUNCTION__ << std::endl;
  if(proxy_socket_begin_map_.size() == 0){
    std::cout << __FUNCTION__ << "proxy_socket_begin_map_.size() == 0" << std::endl;
    ///////////////////////////////////////////////////////////////////////////
    //BUSINESS LOGIC NOTE (GuangleiHe, 12/5/2013)
    //Because to close a stream (a ICE p2p connection), the ice protect has it 
    //own method to close this, if you close this stream before the ice close 
    // there will getting the bug.
    ///////////////////////////////////////////////////////////////////////////
    //if(is_self_close)
    //  p2p_connection_implementator_->CloseStream();
    //Destory();
  }
}

void ProxyP2PSession::SendCommand(talk_base::ByteBuffer *byte_buffer){
  //send this string to remote peer
  size_t written;
  p2p_connection_implementator_->Send(0,TCP_SOCKET, byte_buffer->Data(),
    P2PRTSPCOMMAND_LENGTH,&written);
  if(written != P2PRTSPCOMMAND_LENGTH){
    size_t res;
    command_send_buffer_->WriteAll(byte_buffer->Data(),byte_buffer->Length(),
      &res,NULL);
    ///////////////////////////////////////////////////////////////////////////
    //BUG NOTE (GuangleiHe, 12/5/2013)
    //Maybe there has a bug.
    //Because write data to the FIFO buffer maybe got block. the current operator
    //just exit the program.
    ///////////////////////////////////////////////////////////////////////////
    ASSERT(res == P2PRTSPCOMMAND_LENGTH);
  }
  //delete the string
  p2p_system_command_factory_->DeleteRTSPClientCommand(byte_buffer);
}


void ProxyP2PSession::OnMessage(talk_base::Message *msg){
  switch(msg->message_id){
  case DESTORY_SELFT:
    {
      delete p2p_connection_implementator_;
      p2p_connection_management_->DeleteProxyP2PSession(this);
      break;
    }
  case CLOSE_ALL_PROXY_SOCKET:
    {
      OnStreamClose(NULL);
      break;
    }
  }
}
