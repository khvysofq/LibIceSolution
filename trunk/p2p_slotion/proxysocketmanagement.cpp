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
#include "proxysocketmanagement.h"
#include "proxyserverfactory.h"

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

  int_socket_->SignalReadEvent.connect(this, &ProxySocketBegin::OnInternalRead);
  int_socket_->SignalWriteEvent.connect(this, &ProxySocketBegin::OnInternalWrite);
  int_socket_->SignalCloseEvent.connect(this, &ProxySocketBegin::OnInternalClose);
}

bool ProxySocketBegin::IsMe(uint32 socket){
  return ((uint32)int_socket_.get()) == socket;
}

void ProxySocketBegin::OnP2PReceiveData(const char *data, uint16 len){
  ReadP2PDataToBuffer(data,len,&in_buffer_);
  WriteBufferDataToSocket(int_socket_.get(),&in_buffer_);
}

//ProxySocketBegin protected function
void ProxySocketBegin::OnInternalRead(talk_base::AsyncSocket* socket){
  ReadSocketDataToBuffer(int_socket_.get(),&out_buffer_);
  WriteBufferDataToP2P(&out_buffer_);
}

void ProxySocketBegin::OnInternalWrite(talk_base::AsyncSocket *socket){
  WriteBufferDataToP2P(&in_buffer_);
}

void ProxySocketBegin::OnInternalClose(talk_base::AsyncSocket* socket, int err){

}

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
  else {
    LOG(LS_ERROR) << "p2p network buffer is not empty";
  }
}

void ProxySocketBegin::ReadP2PDataToBuffer(const char *data, uint16 len,
                                           talk_base::FifoBuffer *buffer)
{
  ASSERT(data != NULL);
  buffer->WriteAll(data,len,NULL,NULL);
}

void ProxySocketBegin::WriteBufferDataToSocket(talk_base::AsyncSocket *socket,
                                               talk_base::FifoBuffer *buffer)
{
  ASSERT(socket != NULL);
  size_t size;
  int written;
  const void* p = buffer->GetReadData(&size);
  written = socket->Send(p, size);
  buffer->ConsumeReadData(talk_base::_max(written, 0));
}

void ProxySocketBegin::WriteBufferDataToP2P(talk_base::FifoBuffer *buffer){
  ASSERT(buffer != NULL);
  size_t size;
  size_t written = 0;
  const void* p = buffer->GetReadData(&size);
  p2p_socket_->Send((uint32)int_socket_.get(),TCP_SOCKET,(const char *)p,size,&written);
  buffer->ConsumeReadData(written);
}

//////////////////////////////////////////////////////////////////////////
//Implement ProxySocketManagement

void ProxySocketManagement::RegisterProxySocket(
  uint32 local_socket, ProxySocketBegin *proxy_socket_begin)
{
  proxy_socket_begin_map_.insert(ProxySocketBeginMap::value_type(local_socket,
    proxy_socket_begin));
}

///////////////////////////////////////////////////////////////////////////
//TODO:(GuangleiHe) TIME: 11/20/2013
//DestoryAll object at the proxy socket management.
///////////////////////////////////////////////////////////////////////////
void ProxySocketManagement::DestoryAll(){

}

const ProxySocketBegin *ProxySocketManagement::GetProxySocketBegin(
  uint32 local_socket)
{
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
  ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(socket);
  if(iter == proxy_socket_begin_map_.end()){
    return false;
  }
  iter->second->OnP2PReceiveData(data,len);
  return true;
}