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

#include "talk/base/socketfactory.h"

#include "p2pproxybinding.h"
//////////////////////////////////////////////////////////////////////////
//P2PProxyBindBase
//////////////////////////////////////////////////////////////////////////
P2PProxyBindBase::P2PProxyBindBase(talk_base::AsyncSocket * base_socket,
                                   P2PProxySocket *p2p_socket)
                                   :base_socket_(base_socket),
                                   p2p_socket_(p2p_socket),connected_(false),
                                   out_buffer_(kBufferSize),
                                   in_buffer_(kBufferSize)
{
  base_socket->SignalReadEvent.connect(this, 
    &P2PProxyBindBase::OnBaseSocketRead);
  base_socket->SignalWriteEvent.connect(this, 
    &P2PProxyBindBase::OnBaseSocketWrite);
  base_socket->SignalCloseEvent.connect(this, 
    &P2PProxyBindBase::OnBaseSocketClose);

  p2p_socket_->SignalReadMixData.connect(this,
    &P2PProxyBindBase::OnP2PSocketReadMixData);
  p2p_socket_->SignalReadEvent.connect(this, 
    &P2PProxyBindBase::OnP2PSocketRead);
  p2p_socket_->SignalWriteEvent.connect(this, 
    &P2PProxyBindBase::OnP2PSocketWrite);
  p2p_socket_->SignalCloseEvent.connect(this, 
    &P2PProxyBindBase::OnP2PSocketClose);
}

//////////////////////////////////////////////////////////////////////////
void P2PProxyBindBase::OnBaseSocketRead(talk_base::AsyncSocket* socket) {
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC )
    << "socket reading event";
  Read(base_socket_.get(), &out_buffer_);
  Write(p2p_socket_.get(), &out_buffer_);
}

void P2PProxyBindBase::OnBaseSocketWrite(talk_base::AsyncSocket* socket) {
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC )
    << "socket write event";
  Write(base_socket_.get(), &in_buffer_);
}

void P2PProxyBindBase::OnBaseSocketClose(talk_base::AsyncSocket* socket, 
                                         int err) 
{
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC)
    << "socket close event";
  Destroy();
}

void P2PProxyBindBase::OnP2PSocketReadMixData(const char *data, uint16 len){
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC)
    << "socket mix data read event";
  size_t size;
  int read;
  size = base_socket_->Send(data,len);
  if(size == SOCKET_ERROR){
    read = len;
    void* p = in_buffer_.GetWriteBuffer(&size);
    if(size < static_cast<size_t>(read)){
      LOG(LS_ERROR) << "Can't read all p2p data SOCKET_ERROR";
      read = size;
    }
    memcpy(p,data,read);
    in_buffer_.ConsumeWriteBuffer(talk_base::_max(read, 0));
    Write(base_socket_.get(), &in_buffer_);
  }
  else if (size < len){
    read = len - size;
    void* p = in_buffer_.GetWriteBuffer(&size);
    if(size < static_cast<size_t>(read)){
      LOG(LS_ERROR) << "Can't read all p2p data";
      read = read;
    }
    memcpy(p,data + size,read);
    in_buffer_.ConsumeWriteBuffer(talk_base::_max(read, 0));
  }
}

void P2PProxyBindBase::OnP2PSocketRead(talk_base::AsyncSocket* socket) {
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC )
    << "socket read event";
  Read(p2p_socket_.get(), &in_buffer_);
  Write(base_socket_.get(), &in_buffer_);
}

void P2PProxyBindBase::OnP2PSocketWrite(talk_base::AsyncSocket* socket) {
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC )
    << "socket write event";
  Write(p2p_socket_.get(), &out_buffer_);
}

void P2PProxyBindBase::OnP2PSocketClose(talk_base::AsyncSocket* socket, int err) {
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC )
    << "p2p socket close event";
  Destroy();
}

void P2PProxyBindBase::Read(talk_base::AsyncSocket* socket,
                            talk_base::FifoBuffer* buffer) 
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

void P2PProxyBindBase::Write(talk_base::AsyncSocket* socket,
                             talk_base::FifoBuffer* buffer) 
{
  ASSERT(socket != NULL);
  size_t size;
  int written;
  const void* p = buffer->GetReadData(&size);
  if(size == 0)
    return ;
  written = socket->Send(p, size);
  buffer->ConsumeReadData(talk_base::_max(written, 0));
}

void P2PProxyBindBase::Destroy() {
  //delete this;
  p2p_socket_->Close();
  base_socket_->Close();
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC ) << ">>>>> Start to close P2P Proxy Bind Base";
  talk_base::Thread::Current()->Post(this);
}

void P2PProxyBindBase::OnMessage(talk_base::Message *msg){
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC ) << "Delete P2P Proxy Bind Base";
  delete this;
}


//////////////////////////////////////////////////////////////////////////
//Basic 
//////////////////////////////////////////////////////////////////////////
// ProxyBinding
P2PProxyBinding::P2PProxyBinding(P2PProxyServerSocket* server_socket,
                                 P2PProxyStartSocket* p2p_socket)
                                 : P2PProxyBindBase(server_socket,p2p_socket),
                                 server_socket_(server_socket),
                                 p2p_proxy_start_socket_(p2p_socket)
{
  server_socket_->SignalConnectRequest.connect(this,
    &P2PProxyBinding::OnServerSocketConnectRequest);
  server_socket_->SignalConnectReadEvent.connect(this,
    &P2PProxyBinding::OnServerSocketConnectRead);
  p2p_proxy_start_socket_->SignalConnectEvent.connect(this,
    &P2PProxyBinding::OnP2PSocketConnectSucceed);
}

void P2PProxyBinding::OnServerSocketConnectRequest(P2PProxyServerSocket* socket,
                                                   const std::string &addr_ide) 
{
  ASSERT(!connected_ && p2p_socket_.get() != NULL);
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC | BASIC_INFOR)
    << "Connect the peer resource";
  p2p_proxy_start_socket_->ConnectPeer(addr_ide,socket->GetProxyType());
  // TODO: handle errors here
}

void P2PProxyBinding::OnServerSocketConnectRead(P2PProxyServerSocket* socket,
                                                const void *data, size_t len) 
{
  ASSERT(p2p_socket_.get() != NULL);
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC )
    << "socket connect read event";
  size_t size;
  int read = len;
  void* p = out_buffer_.GetWriteBuffer(&size);
  if(size < len){
    LOG(LS_ERROR) << "Can't read all p2p data";
    read = size;
  }
  memcpy(p,data,read);
  out_buffer_.ConsumeWriteBuffer(talk_base::_max(read, 0));
  Write(p2p_socket_.get(), &out_buffer_);
}


//////////////////////////////////////////////////////////////////////////
void P2PProxyBinding::OnP2PSocketConnectSucceed(talk_base::AsyncSocket* socket) {
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC | BASIC_INFOR)
    << "socket connect event";
  ASSERT(socket != NULL);
  connected_ = true;
  server_socket_->SendConnectResult(0, socket->GetRemoteAddress().ToString());
}

void P2PProxyBinding::OnP2PSocketClose(talk_base::AsyncSocket* socket, int err) {
  if(connected_ == false){
    server_socket_->SendConnectResult(err,socket->GetRemoteAddress().ToString());
  }
  Destroy();
}

//////////////////////////////////////////////////////////////////////////
//P2P Proxy Client Socket
//////////////////////////////////////////////////////////////////////////

P2PProxyClientSocketBinding::P2PProxyClientSocketBinding(
  P2PProxyEndSocket *p2p_proxy_socket,talk_base::AsyncSocket *client_socket,
  const talk_base::SocketAddress &server_addr)
  :P2PProxyBindBase(client_socket,p2p_proxy_socket),
  p2p_proxy_end_socket_(p2p_proxy_socket)
{
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC | BASIC_INFOR) << server_addr.ToString();
  client_socket->SignalConnectEvent.connect(this,
    &P2PProxyClientSocketBinding::OnClientSocketConnect);
  client_socket->Connect(server_addr);
}

void P2PProxyClientSocketBinding::OnClientSocketConnect(talk_base::AsyncSocket *socket){
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC | BASIC_INFOR)
    << "socket connect event";
  ASSERT(connected_ == false);
  connected_ = true;
  p2p_proxy_end_socket_->SocketConnectSucceed();
}

void P2PProxyClientSocketBinding::OnBaseSocketClose(talk_base::AsyncSocket* socket, 
                                                    int err) 
{
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC | BASIC_INFOR)
    << "socket close event";
  if(connected_ == false)
    p2p_proxy_end_socket_->SocketConnectFailure();
  Destroy();
}