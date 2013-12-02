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


ProxyP2PSession::ProxyP2PSession(P2PConnectionImplementator 
                                 *p2p_connection_implementator)
                                 :p2p_connection_implementator_(
                                 p2p_connection_implementator)
{

  p2p_system_command_factory_ = P2PSystemCommandFactory::Instance();
  socket_table_management_    = SocketTableManagement::Instance();

  current_thread_             = talk_base::Thread::Current();

  p2p_connection_implementator_->SignalStreamRead.connect(this,
    &ProxyP2PSession::OnStreamRead);
  p2p_connection_implementator_->SignalStreamClose.connect(this,
    &ProxyP2PSession::OnStreamClose);
  p2p_connection_implementator_->SignalStreamWrite.connect(this,
    &ProxyP2PSession::OnStreamWrite);
  p2p_connection_implementator_->SignalConnectSucceed.connect(this,
    &ProxyP2PSession::OnConnectSucceed);
}

//////////////////////////////////////////////////////////////////////////
//Implement Proxy p2p session

void ProxyP2PSession::RegisterProxySocket(ProxySocketBegin *proxy_socket_begin)
{
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  
  proxy_socket_begin_map_.insert(ProxySocketBeginMap::value_type(
    proxy_socket_begin->GetSocketNumber(),proxy_socket_begin));

}

///////////////////////////////////////////////////////////////////////////
//TODO:(GuangleiHe) TIME: 11/20/2013
//DestoryAll object at the Proxy p2p session
///////////////////////////////////////////////////////////////////////////
void ProxyP2PSession::DestoryAll(){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
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
    LOG(LS_ERROR) << "Can't Find the object in the ProxySocketManagement";
    return false;
  }
  iter->second->OnP2PRead(data,len);
  return true;
}

void ProxyP2PSession::OnStreamClose(talk_base::StreamInterface *stream){
  // inform all proxy socket begin object that the peer is connected
  for(ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.begin();
    iter != proxy_socket_begin_map_.end();iter++){
      iter->second->OnP2PClose(stream);
  }
}

void ProxyP2PSession::OnStreamWrite(talk_base::StreamInterface *stream){
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
  LOG(LS_INFO) << "---" << __FUNCTION__;
  LOG(LS_INFO) << "-----------------------------";
  LOG(LS_INFO) << "\t socket = " << socket;
  LOG(LS_INFO) << "\t socket type = " << socket_type;
  LOG(LS_INFO) << "\t data length = " << len;
  LOG(LS_INFO) << "-----------------------------";
  if(socket == 0){
    //It must a system command
    ProceesSystemCommand(data,len);
  }
  else if(!RunSocketProccess(socket,socket_type,data,len)){
    //never reach there
    ASSERT(0);
  }
}

void ProxyP2PSession::OnConnectSucceed(talk_base::StreamInterface *stream){
  // inform all proxy socket begin object that the peer is connected
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

  LOG(LS_INFO) << __FUNCTION__ << addr.ToString();
  //Generate system command that create RTSP client
  talk_base::ByteBuffer *create_rtsp_client_command =
    p2p_system_command_factory_->CreateRTSPClientSocket(socket,addr);

  ///////////////////////////////////////////////////////////////////////////
  //BUG NOTE (GuangleiHe, 12/1/2013)
  //Maybe there has a bug.
  //Because is the stream is block, what I can do?
  ///////////////////////////////////////////////////////////////////////////
  //send the data to remote peer
  size_t written;
  p2p_connection_implementator_->Send(0,TCP_SOCKET,
    create_rtsp_client_command->Data(),P2PRTSPCOMMAND_LENGTH,&written);
  if(written != P2PRTSPCOMMAND_LENGTH){
    LOG(LS_ERROR) << "The stream is block, you can't send the system command";
  }
  ASSERT(written == P2PRTSPCOMMAND_LENGTH);
  //delete this command
  p2p_system_command_factory_->DeleteRTSPClientCommand(create_rtsp_client_command);
}

void ProxyP2PSession::ReplayClientSocketCreateSucceed(
  uint32 server_socket, uint32 client_socket,
  const talk_base::SocketAddress &addr)
{
  LOG(LS_INFO) << __FUNCTION__;
  //socket_table_management_ = SocketTableManagement::Instance();
  //generate a reply string
  socket_table_management_->AddNewLocalSocket(client_socket,
    server_socket,TCP_SOCKET);

  talk_base::ByteBuffer *reply_string = 
    p2p_system_command_factory_->ReplyRTSPClientSocketSucceed(
    server_socket,client_socket);
  //send this string to remote peer

  size_t written;
  p2p_connection_implementator_->Send(0,TCP_SOCKET, reply_string->Data(),
    P2PRTSPCOMMAND_LENGTH,&written);
  if(written != P2PRTSPCOMMAND_LENGTH){
    LOG(LS_ERROR) << "The stream is block, you can't \
      send the system command";
  }
  ASSERT(written == P2PRTSPCOMMAND_LENGTH);
  //delete the string
  p2p_system_command_factory_->DeleteRTSPClientCommand(reply_string);
}

void ProxyP2PSession::P2PServerSocketClose(uint32 server_socket){
  LOG(LS_INFO) << __FUNCTION__;
  //socket_table_management_ = SocketTableManagement::Instance();
  //generate a reply string
  uint32 client_socket = 
    socket_table_management_->GetRemoteSocket(server_socket);

  talk_base::ByteBuffer *reply_string = 
    p2p_system_command_factory_->RTSPServerSocketClose(
    server_socket,client_socket);
  //send this string to remote peer

  size_t written;
  p2p_connection_implementator_->Send(0,TCP_SOCKET, reply_string->Data(),
    P2PRTSPCOMMAND_LENGTH,&written);
  if(written != P2PRTSPCOMMAND_LENGTH){
    LOG(LS_ERROR) << "The stream is block, you can't \
                     send the system command";
  }
  ASSERT(written == P2PRTSPCOMMAND_LENGTH);
  //delete the string
  p2p_system_command_factory_->DeleteRTSPClientCommand(reply_string);
}

void ProxyP2PSession::P2PClientSocketClose(uint32 client_socket){

  LOG(LS_INFO) << __FUNCTION__;
  //socket_table_management_ = SocketTableManagement::Instance();
  //generate a reply string
  uint32 server_socket = 
    socket_table_management_->GetRemoteSocket(client_socket);

  talk_base::ByteBuffer *reply_string = 
    p2p_system_command_factory_->RTSPServerSocketClose(
    server_socket,client_socket);
  //send this string to remote peer

  size_t written;
  p2p_connection_implementator_->Send(0,TCP_SOCKET, reply_string->Data(),
    P2PRTSPCOMMAND_LENGTH,&written);
  if(written != P2PRTSPCOMMAND_LENGTH){
    LOG(LS_ERROR) << "The stream is block, you can't \
                     send the system command";
  }
  ASSERT(written == P2PRTSPCOMMAND_LENGTH);
  //delete the string
  p2p_system_command_factory_->DeleteRTSPClientCommand(reply_string);
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
    //Step 1. Create AsyncSocket object.
    talk_base::AsyncSocket *int_socket 
      = current_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);

    //Step 2. Create server SocketAddress
    talk_base::SocketAddress server_addr(client_connection_ip,client_connection_port);

    //Step 3. Create RTSPClient Socket
    ProxyServerFactory::CreateRTSPClientSocket(this,int_socket,
      server_socket,server_addr);
    return true;
  }
  else if(p2p_system_command_type == P2P_SYSTEM_CREATE_RTSP_CLIENT_SUCCEED){
    LOG(LS_INFO) << "Client Socket Create Succeed";
    socket_table_management_->AddNewLocalSocket(server_socket,
      client_socket,TCP_SOCKET);
    GetProxySocketBegin(server_socket)->OnP2PSocketConnectSucceed(this);
    return true;
  }
  else if(p2p_system_command_type == P2P_SYSTEM_SERVER_SOCKET_CLOSE){
    CloseP2PSocket(client_socket);
  }
  else if(p2p_system_command_type == P2P_SYSTEM_CLIENT_SOCKET_CLOSE){
    CloseP2PSocket(server_socket);
  }
  //never reach here.
  ASSERT(0);
  return true;
}

void ProxyP2PSession::CloseP2PSocket(uint32 socket){
  proxy_socket_begin_map_[socket]->OnP2PClose(NULL);
}