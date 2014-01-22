/*
* p2p solution
* Copyright 2013, VZ Inc.
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

#include "p2pproxysession.h"
#include "proxyserverfactory.h"
#include "p2pconnectionimplementator.h"
#include "sockettablemanagement.h"
#include "p2pconnectionmanagement.h"
#include "asyncrtspclientsocket.h"
#include "p2pproxysocket.h"
#include "p2pproxybinding.h"

const int DESTORY_SELFT           = 0;
const int CLOSE_ALL_PROXY_SOCKET  = 1;
const int SEND_BUFFER_DATA        = 2;
const int RELEASE_ALL             = 3;
const int DELAYED_CLOSE           = 4;
const int DELAYED_CLOSE_WAIT_TIME = 60 * 1000;

//////////////////////////////////////////////////////////////////////////
//P2PCommand Data structure
//////////////////////////////////////////////////////////////////////////
P2PCommandData::P2PCommandData(const char *data, uint16 len)
  :len_(len)
{
  data_ = new char[len_];
  memcpy(data_,data,len_*sizeof(char));
}
P2PCommandData::~P2PCommandData(){
  delete data_;
}


P2PProxySession::P2PProxySession(talk_base::StreamInterface *stream,
                                 const std::string &remote_peer_name,
                                 talk_base::Thread *signal_thread,
                                 talk_base::Thread *worker_thread,
                                 std::string session_type,
                                 bool is_mix_data_mode)
                                 :is_mix_data_mode_(is_mix_data_mode),
                                 worker_thread_(worker_thread),
                                 signal_thread_(signal_thread),
                                 session_type_(session_type),
                                 state_(P2P_CONNECTING)
{
  ASSERT(signal_thread_->IsCurrent());
  LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC) << "\t Create A New Session"
    << std::endl;
  //1. Create P2PConnectionImplementator object

  p2p_connection_implementator_ = 
    new P2PConnectionImplementator(remote_peer_name,stream,
    is_mix_data_mode_);

  socket_table_management_    = SocketTableManagement::Instance();
  p2p_connection_management_  = P2PConnectionManagement::Instance();

  p2p_connection_implementator_->SignalStreamWrite.connect(this,
    &P2PProxySession::OnStreamWrite);

  p2p_connection_implementator_->SignalStreamClose.connect(this,
    &P2PProxySession::OnStreamClose);

  p2p_connection_implementator_->SignalConnectSucceed.connect(this,
    &P2PProxySession::OnP2PConnectSucceed);

  p2p_connection_implementator_->SignalStreamRead.connect(this,
    &P2PProxySession::OnStreamRead);

  p2p_connection_implementator_->SignalIndependentStreamRead.connect(
    this,&P2PProxySession::OnIndependentStreamRead);
}

P2PProxySession::~P2PProxySession(){
  LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC) << "Destroy Proxy P2p Session";
  ASSERT(signal_thread_->IsCurrent());
  ASSERT(p2p_connection_implementator_ == NULL);
}

//////////////////////////////////////////////////////////////////////////
//Implement Proxy p2p session
bool P2PProxySession::RegisterP2PProxySocket(
  P2PProxySocket *p2p_proxy_socket, bool is_server)
{
  ASSERT(signal_thread_->IsCurrent());
  if(state_ == P2P_CLOSING)
    state_ = P2P_CONNECTED;
  P2PProxySockets::iterator iter = p2p_proxy_sockets_.find(
    p2p_proxy_socket->GetSocketNumber());

  if(iter != p2p_proxy_sockets_.end()){
    LOG_F(LS_ERROR) << "\t the proxy socket begin is existed" 
      << std::endl;
    return false;
  }
  p2p_proxy_sockets_.insert(P2PProxySockets::value_type(
    p2p_proxy_socket->GetSocketNumber(),p2p_proxy_socket));
  
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC | BASIC_INFOR) 
    << "\n********* \n"
    << "Current p2p socket size is "
    << p2p_proxy_sockets_.size()
    << "\n*********";

  SignalPeerConnectSucceed.connect(p2p_proxy_socket,
    &P2PProxySocket::OnP2PConnectSucceed);
  SignalPeerConnectFailure.connect(p2p_proxy_socket,
    &P2PProxySocket::OnP2PClose);
  if(is_server && state_ == P2P_CONNECTED)
    p2p_proxy_socket->OnP2PConnectSucceed(this);
  return true;
}

void P2PProxySession::ReleaseP2PProxySocket(
  P2PProxySocket *p2p_proxy_socket)
{
  P2PProxySockets::iterator it = 
    p2p_proxy_sockets_.find(p2p_proxy_socket->GetSocketNumber());
  p2p_proxy_sockets_.erase(it);
  SignalPeerConnectFailure.disconnect(p2p_proxy_socket);
  SignalPeerConnectSucceed.disconnect(p2p_proxy_socket);
  
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC | BASIC_INFOR) 
    << "\n********* \n"
    << "Current p2p socket size is "
    << p2p_proxy_sockets_.size()
    << "\n*********";
  IsAllProxySocketClosed();
}


void P2PProxySession::RegisterNewSocketTable(uint32 local_socket, 
                                             uint32 remote_socket)
{
  ASSERT(state_ == P2P_CONNECTED);
  socket_table_management_->AddNewLocalSocket(local_socket,
    remote_socket,TCP_SOCKET);
}

void P2PProxySession::DeleteSocketTable(uint32 local_socket){
  socket_table_management_->DeleteASocket(local_socket);
}


///////////////////////////////////////////////////////////////////////////
//TODO:(GuangleiHe) TIME: 11/20/2013
//Destory All object at the Proxy p2p session
///////////////////////////////////////////////////////////////////////////
void P2PProxySession::Destory(){
  ASSERT(signal_thread_->IsCurrent());
  ASSERT(state_ == P2P_CLOSE);
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC | BASIC_INFOR) 
    << "destroy this session ";
  if(p2p_connection_implementator_ != NULL)
    p2p_connection_implementator_->Destory();
  else
    talk_base::Thread::Current()->Post(this,DESTORY_SELFT);
}

bool P2PProxySession::IsMe(const std::string remote_peer_name) const {
  ASSERT(signal_thread_->IsCurrent());
  return p2p_connection_implementator_->IsMe(remote_peer_name);
}

bool P2PProxySession::RunSocketProccess(
  uint32 socket, SocketType socket_type,const char *data, uint16 len)
{
  ASSERT(signal_thread_->IsCurrent());
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "socket proccess";
  P2PProxySockets::iterator iter = p2p_proxy_sockets_.find(socket);
  if(iter != p2p_proxy_sockets_.end()){
    iter->second->OnP2PRead(data,len);
  }
  return true;
}

void P2PProxySession::OnStreamClose(talk_base::StreamInterface *stream){
  ASSERT(signal_thread_->IsCurrent());
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "Close Stream";
  state_ = P2P_CLOSE;
  CloseAllProxySokcet(stream);
}

void P2PProxySession::CloseAllProxySokcet(
  talk_base::StreamInterface *stream)
{
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "Close All ProxySocket";
  ASSERT(signal_thread_->IsCurrent());
  SignalPeerConnectFailure(this);
  talk_base::Thread::Current()->Post(this,DESTORY_SELFT);
}

void P2PProxySession::OnStreamWrite(talk_base::StreamInterface *stream){
  ASSERT(signal_thread_->IsCurrent());
  ASSERT(state_ == P2P_CONNECTED);
  while(!command_data_buffers_.empty()){
    //send this string to remote peer
    size_t written;
    ///////////////////////////////////////////////////////////////////
    //when p2p connection implementation is NULL, is can't send message
    //it is normal but it very important 
    ///////////////////////////////////////////////////////////////////
    if(p2p_connection_implementator_ == NULL){
      return ;
    }
    P2PCommandData *p2p_command_data = command_data_buffers_.front();

    p2p_connection_implementator_->Send(0,TCP_SOCKET, 
      p2p_command_data->data_,p2p_command_data->len_,&written);
    if(written == P2PRTSPCOMMAND_LENGTH){
      command_data_buffers_.pop();
      delete p2p_command_data;
    }
    else{
      return;
    }
  }
  // inform all proxy socket begin object that the peer can write
  for(P2PProxySockets::iterator iter = p2p_proxy_sockets_.begin();
    iter != p2p_proxy_sockets_.end();iter++){
      iter->second->OnP2PWrite(this);
  }
}

void P2PProxySession::OnStreamRead(uint32 socket, 
                                   SocketType socket_type,
                                   const char *data, uint16 len)
{
  ASSERT(state_ == P2P_CONNECTED || state_ == P2P_CLOSING);
  ASSERT(signal_thread_->IsCurrent());
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "Read stream data socket is " << socket
    << "\tlength is " << len;
  if(socket == 0){
    //It must a system command
    if(len != P2PRTSPCOMMAND_LENGTH){
      //If the length of this data is not equal to P2PRTSPCOMMAND_LENGTH, 
      //I'm sure the command is get error. maybe it send block.
      return ;
    }
    ProceesSystemCommand(data,len);
  }
  else
    RunSocketProccess(socket,socket_type,data,len);
}

void P2PProxySession::OnIndependentStreamRead(
  talk_base::StreamInterface *stream)
{
  ASSERT(state_ == P2P_CONNECTED);
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "independent reading data";
  ASSERT(signal_thread_->IsCurrent());
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

void P2PProxySession::OnP2PConnectSucceed(talk_base::StreamInterface *stream){
  ASSERT(signal_thread_->IsCurrent());
  // inform all proxy socket begin object that the peer is connected
  state_ = P2P_CONNECTED;
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC)
    << "inform all proxy socket begin object that the peer is connected";
  SignalPeerConnectSucceed(this);
}

void P2PProxySession::IsAllProxySocketClosed(){
  ASSERT(signal_thread_->IsCurrent());
  if(p2p_proxy_sockets_.size() == 0){
    ///////////////////////////////////////////////////////////////////////////
    //BUSINESS LOGIC NOTE (GuangleiHe, 12/5/2013)
    //Because to close a stream (a ICE p2p connection), the ice protect has it 
    //own method to close this, if you close this stream before the ice close 
    // there will getting the bug.
    ///////////////////////////////////////////////////////////////////////////
    //if(is_self_close)
    //  p2p_connection_implementator_->CloseStream();
    if(state_ == P2P_CONNECTED){
      state_ = P2P_CLOSING;
    }
    if(session_type_ == RTSP_SERVER){
      signal_thread_->PostDelayed(DELAYED_CLOSE_WAIT_TIME,
        this,DELAYED_CLOSE);
    LOG_P2P(P2P_PROXY_SOCKET_LOGIC | BASIC_INFOR) 
      << "will destroy this session after " << DELAYED_CLOSE_WAIT_TIME/1000
      << "s";
    }
    else if(session_type_ == HTTP_SERVER){
      signal_thread_->PostDelayed(DELAYED_CLOSE_WAIT_TIME * 10,
      this,DELAYED_CLOSE);
    LOG_P2P(P2P_PROXY_SOCKET_LOGIC | BASIC_INFOR) 
      << "will destroy this session after " << DELAYED_CLOSE_WAIT_TIME/100
      << "s";
    }
    //signal_thread_->PostDelayed(1000 * 60,this,RELEASE_ALL);
  }
}

void P2PProxySession::SendSystemCommand(const char *p, size_t len){
  ASSERT(state_ == P2P_CONNECTED);
  command_data_buffers_.push(new P2PCommandData(p,len));
  signal_thread_->Post(this,SEND_BUFFER_DATA);
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC ) 
    << "command_data_buffers_.size() == " 
    << command_data_buffers_.size();
}

bool P2PProxySession::ParseCommand(const char *data, uint16 len,
                                           uint32 *p2p_system_command_type,
                                           uint32 *server_socket,
                                           uint32 *client_socket,
                                           uint32 *client_connection_ip,
                                           uint16 *client_connection_port)
{
  if(len != P2PRTSPCOMMAND_LENGTH){
    LOG(LS_ERROR) << "\t The length of data is not expected length";
    return false;
  }
  talk_base::ByteBuffer byte_buffer(data,len);

  uint32 p2p_system_command_ide;
  uint16 padding_byte;
  //
  byte_buffer.ReadUInt32(&p2p_system_command_ide);
  if(p2p_system_command_ide != P2P_SYSTEM_COMMAND_IDE){
      LOG(LS_ERROR) << "\t The P2P P2P_SYSTEM_COMMAND_IDE error " 
        << p2p_system_command_ide;
      return false;
  }

  byte_buffer.ReadUInt32(p2p_system_command_type);
  byte_buffer.ReadUInt32(server_socket);
  byte_buffer.ReadUInt32(client_socket);
  byte_buffer.ReadUInt32(client_connection_ip);
  byte_buffer.ReadUInt16(client_connection_port);
  byte_buffer.ReadUInt16(&padding_byte);
  if(padding_byte != P2P_SYSTEM_COMMAND_PADDING_BYTE)
  {
    LOG(LS_ERROR) << "the padding_byte is error";
    return false;
  }
  return true;
}

void P2PProxySession::OnMessage(talk_base::Message *msg){
  ASSERT(signal_thread_->IsCurrent());
  switch(msg->message_id){
  case DESTORY_SELFT:
    {
      if(p2p_connection_implementator_ != NULL){
        LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC|BASIC_INFOR)
          << "delete p2p connection implementation";
        delete p2p_connection_implementator_;
        p2p_connection_implementator_ = NULL;
      }
      if(p2p_proxy_sockets_.size() == 0){
        LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC|BASIC_INFOR)
          << "delete proxy p2p session";
        p2p_connection_management_->DeleteP2PProxySession(this);
      }
      break;
    }
  case CLOSE_ALL_PROXY_SOCKET:
    {
      break;
    }
  case SEND_BUFFER_DATA:
    {
      LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "Send command data";
      if(state_ == P2P_CLOSE)
        return ;
      //send this string to remote peer
      size_t written;
      ///////////////////////////////////////////////////////////////////
      //when p2p connection implementation is NULL, is can't send message
      //it is normal but it very important 
      ///////////////////////////////////////////////////////////////////
      if(p2p_connection_implementator_ == NULL || command_data_buffers_.empty()){
        return ;
      }
      P2PCommandData *p2p_command_data = command_data_buffers_.front();

      p2p_connection_implementator_->Send(0,TCP_SOCKET, 
        p2p_command_data->data_,p2p_command_data->len_,&written);
      if(written == P2PRTSPCOMMAND_LENGTH){
        command_data_buffers_.pop();
        delete p2p_command_data;
      }
      break;
    }
  case RELEASE_ALL:
    {
      if(p2p_proxy_sockets_.size() == 0)
        Destory();
      break;
    }
  case DELAYED_CLOSE:
    {
      if(state_ == P2P_CLOSING){
        state_ = P2P_CLOSE;
        Destory();
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
P2PProxyServerSession::P2PProxyServerSession(
  talk_base::StreamInterface *stream,
    const std::string &remote_peer_name,
    talk_base::Thread *signal_thread,
    talk_base::Thread *worker_thread,
    std::string server_type,
    bool is_mix_data_mode)
    :P2PProxySession(stream,remote_peer_name,signal_thread,
    worker_thread,server_type,is_mix_data_mode)
{

}


bool P2PProxyServerSession::ProceesSystemCommand(const char *data,
                                                 uint16 len)
{
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

  if(!ParseCommand(data,len,&p2p_system_command_type,&server_socket,
    &client_socket,&client_connection_ip,&client_connection_port)){
      LOG(LS_ERROR) << "Parse the p2p system command error";
      return false;
  }

  //////////////////////////////////////////////////////////////////////////
  if(p2p_system_command_type == P2P_SYSTEM_CREATE_CLIENT_SUCCEED){
    LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "Client Socket Create Succeed";
    socket_table_management_->AddNewLocalSocket(server_socket,
      client_socket,TCP_SOCKET);
    //////////////////////////////////////////////////////////////////////////
    //there is normal business logic, some times proxsocketbegin closed.
    //so the second of socket begin map getting NULL pointer. 
    //////////////////////////////////////////////////////////////////////////
    P2PProxySocket *res = GetP2PProxySocket(server_socket);
    if(res != NULL)
      res->OnP2PSocketConnectSucceed(this,client_socket);
  }
  else if(p2p_system_command_type == P2P_SYSTEM_SOCKET_CONNECT_FAILURE){
    P2PProxySocket *res = GetP2PProxySocket(server_socket);
    if(res != NULL)
      res->OnP2PSocketConnectFailure(this);
  }
  else if(p2p_system_command_type == P2P_SYSTEM_SOCKET_CLOSE){
    LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "close server socket ";
   P2PProxySocket *res = GetP2PProxySocket(server_socket);
    if(res != NULL)
      res->OnP2PClose(this);
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

P2PProxyClientSession::P2PProxyClientSession(
  talk_base::StreamInterface *stream,
    const std::string &remote_peer_name,
    talk_base::Thread *signal_thread,
    talk_base::Thread *worker_thread,
    std::string server_type,
    bool is_mix_data_mode)
    :P2PProxySession(stream,remote_peer_name,signal_thread,
    worker_thread,server_type,is_mix_data_mode)
{

}


bool P2PProxyClientSession::ProceesSystemCommand(const char *data,
                                                 uint16 len)
{
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

  if(!ParseCommand(data,len,&p2p_system_command_type,&server_socket,
    &client_socket,&client_connection_ip,&client_connection_port)){
      LOG(LS_ERROR) << "Parse the p2p system command error";
      return false;
  }

  //////////////////////////////////////////////////////////////////////////
  if(p2p_system_command_type == P2P_SYSTEM_CREATE_CLIENT_SOCKET){
    LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "Create New Client Socket";
    //Step 1. Create AsyncSocket object.
    talk_base::AsyncSocket *int_socket 
      = worker_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);
    talk_base::SocketAddress server_addr(client_connection_ip,client_connection_port);
    new P2PProxyClientSocketBinding(new P2PProxyEndSocket(this,server_socket),
      int_socket,server_addr);
  }
  else if(p2p_system_command_type == P2P_SYSTEM_SOCKET_CLOSE){
    LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "close server socket ";
    is_self_close = false;
    P2PProxySocket *res = GetP2PProxySocket(server_socket);
    if(res != NULL)
      res->OnP2PClose(this);
  }
  return true;
}