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


//////////////////////////////////////////////////////////////////////////
//Implement ProxySocketBegin
ProxySocketBegin::ProxySocketBegin(AsyncP2PSocket *p2p_socket,
                                   talk_base::AsyncSocket *int_socket)
                                   :int_socket_(int_socket),
                                   p2p_socket_(p2p_socket),
                                   out_buffer_(KBufferSize),
                                   in_buffer_(KBufferSize)
{
  socket_table_management_ = SocketTableManagement::Instance();
  p2p_system_command_factory_ = P2PSystemCommandFactory::Instance();
  internal_date_wait_receive_ = false;
  p2p_socket_->SignalWriteEvent.connect(this,
    &ProxySocketBegin::OnP2PWrite);

  int_socket_->SignalReadEvent.connect(this, &ProxySocketBegin::OnInternalRead);
  int_socket_->SignalWriteEvent.connect(this, &ProxySocketBegin::OnInternalWrite);
  int_socket_->SignalCloseEvent.connect(this, &ProxySocketBegin::OnInternalClose);
}

bool ProxySocketBegin::IsMe(uint32 socket){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  return ((uint32)int_socket_.get()) == socket;
}

void ProxySocketBegin::OnP2PReceiveData(const char *data, uint16 len){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;

  ReadP2PDataToBuffer(data,len,&in_buffer_);
  WriteBufferDataToSocket(int_socket_.get(),&in_buffer_);
}

void ProxySocketBegin::OnP2PWrite(AsyncP2PSocket *p2p_socket){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  WriteBufferDataToP2P(&out_buffer_);
  if(internal_date_wait_receive_){
    ReadSocketDataToBuffer(int_socket_.get(),&out_buffer_);
    WriteBufferDataToP2P(&out_buffer_);
  }
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
    internal_date_wait_receive_ = false;
    void* p = buffer->GetWriteBuffer(&size);
    read = socket->Recv(p, size);

    if(strncmp(RTSP_HEADER,(const char *)p,RTSP_HEADER_LENGTH) == 0){
      ParseRTSP((char *)p,(size_t *)&read);

      for(int i = 0; i < read; i++){
        std::cout <<((char*)p)[i];
      }
      std::cout << std::endl;
    }
    buffer->ConsumeWriteBuffer(talk_base::_max(read, 0));

  }
  else {
    internal_date_wait_receive_ = true;
  }
}

void ProxySocketBegin::ParseRTSP(char *data, size_t *len){
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
  written = socket->Send(p, size);
  buffer->ConsumeReadData(talk_base::_max(written, 0));
}

void ProxySocketBegin::WriteBufferDataToP2P(talk_base::FifoBuffer *buffer){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  ASSERT(buffer != NULL);
  size_t size;
  size_t written = 0;
  const void* p = buffer->GetReadData(&size);
  p2p_socket_->Send((uint32)int_socket_.get(),TCP_SOCKET,(const char *)p,size,&written);
  buffer->ConsumeReadData(written);
}

//////////////////////////////////////////////////////////////////////////
//Implement ProxySocketManagement

ProxySocketManagement::ProxySocketManagement(){
  p2p_system_command_factory_ = P2PSystemCommandFactory::Instance();
  current_thread_             = talk_base::Thread::Current();
}

void ProxySocketManagement::RegisterProxySocket(
  uint32 local_socket, ProxySocketBegin *proxy_socket_begin)
{
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  proxy_socket_begin_map_.insert(ProxySocketBeginMap::value_type(local_socket,
    proxy_socket_begin));
}



///////////////////////////////////////////////////////////////////////////
//TODO:(GuangleiHe) TIME: 11/20/2013
//DestoryAll object at the proxy socket management.
///////////////////////////////////////////////////////////////////////////
void ProxySocketManagement::DestoryAll(){
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
}

const ProxySocketBegin *ProxySocketManagement::GetProxySocketBegin(
  uint32 local_socket)
{
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(local_socket);
  if(iter == proxy_socket_begin_map_.end()){
    LOG(LS_ERROR) << "Can't find the object in the ProxySocketManagement";
    return NULL;
  }
  return iter->second;
}

bool ProxySocketManagement::RunSocketProccess(
  uint32 socket, SocketType socket_type,const char *data, uint16 len)
{  
  LOG(LS_INFO) << "&&&" << __FUNCTION__;
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(socket);
  if(iter == proxy_socket_begin_map_.end()){
    LOG(LS_ERROR) << "Can't Find the object in the ProxySocketManagement";
    return false;
  }
  iter->second->OnP2PReceiveData(data,len);
  return true;
}

void ProxySocketManagement::OnReceiveP2PData(uint32 socket, 
                                             SocketType socket_type,
                                             const char *data, uint16 len)
{
  LOG(LS_INFO) << "---" << __FUNCTION__;
  LOG(LS_INFO) << "-----------------------------";
  LOG(LS_INFO) << "\t socket = " << socket;
  LOG(LS_INFO) << "\t socket type = " << socket_type;
  LOG(LS_INFO) << "\t data length = " << len;
  LOG(LS_INFO) << "-----------------------------";
  if(!RunSocketProccess(socket,socket_type,data,len))
  {
    //Not found the socket, it must be a system command that create a client socket.
    LOG(LS_INFO) << "Create New Client Socket";
    ///////////////////////////////////////////////////////////////////////////
    //BUSINESS LOGIC NOTE (GuangleiHe, 11/28/2013)
    //There didn't used P2PRTSPCommand to parse the data.
    //P2PRTSPCommand only known by P2PSystemCommandFactory
    ///////////////////////////////////////////////////////////////////////////
    //Step 1. Parse the system command.
    P2PRTSPCommand p2p_rtsp_command;
    p2p_system_command_factory_->ParseCommand(&p2p_rtsp_command,data,len);
    if(p2p_rtsp_command.p2p_system_command_type_ != P2P_SYSTEM_CREATE_RTSP_CLIENT){
      LOG(LS_ERROR) << "Parse p2p system command error";
      return ;
    }
    //Step 2. Create AsyncSocket object.
    talk_base::AsyncSocket *int_socket 
      = current_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);

    //Step 3. Create server SocketAddress
    talk_base::SocketAddress server_addr(p2p_rtsp_command.client_connection_ip_,
      p2p_rtsp_command.client_connection_port_);

    //Step 4. Create RTSPClient Socket
    ProxyServerFactory::CreateRTSPClientSocket(this,
      NULL,int_socket,p2p_rtsp_command.server_socket_,server_addr);
  }
}