/*
* p2p solution
* Copyright 2013, VZ Inc.
* 
* Author   : GuangLei He
* Email    : guangleihe@gmail.com
* Created  : 2013/11/27      9:15
* Filename : F:\GitHub\trunk\p2p_slotion\asyncp2ppackagesocket.cpp
* File path: F:\GitHub\trunk\p2p_slotion
* File base: asyncp2ppackagesocket
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

#include "talk/base/logging.h"
#include "talk/base/socket.h"

#include "p2pproxysocket.h"
#include "p2pconnectionmanagement.h"
#include "defaults.h"

//Our function
P2PProxySocket::P2PProxySocket()
  :connect_state_(CS_CLOSED),
  p2p_proxy_session_(NULL),
  other_side_socket_(0),
  p2p_connection_implementator_(NULL)
{
  p2p_connection_management_ = P2PConnectionManagement::Instance();
  //SignalSendDataToLowLayer.connect(virtual_network_,
  //  &AbstractVirtualNetwork::OnReceiveDataFromUpLayer);
}

P2PProxySocket::P2PProxySocket(P2PProxySession *p2p_proxy_session,
                               uint32 other_side_socket)
  :p2p_proxy_session_(p2p_proxy_session),connect_state_(CS_SOCKET_CONNECTING),
  other_side_socket_(other_side_socket),
  p2p_connection_implementator_(NULL)
{
  p2p_connection_management_ = P2PConnectionManagement::Instance();
  if(!p2p_proxy_session_->RegisterP2PProxySocket(this,false)){
    connect_state_ = CS_CLOSED;
    SetError(EALREADY);
    SignalCloseEvent(this,SOCKET_ERROR);
  }
  p2p_connection_implementator_ = 
    p2p_proxy_session_->GetP2PConnectionImplementator();
  ASSERT(p2p_connection_implementator_ != NULL);
  ASSERT(p2p_proxy_session_ != NULL);
}

//for AsyncSocket
talk_base::AsyncSocket* P2PProxySocket::Accept(
  talk_base::SocketAddress* paddr){
    //LOG(LS_INFO) << "p2p socket have no accept function";
    return NULL;
}

//for Socket Class

// Returns the address to which the socket is bound.  If the socket is not
// bound, then the any-address is returned.
talk_base::SocketAddress P2PProxySocket::GetLocalAddress() const {
  return local_addr_;
}

// Returns the address to which the socket is connected.  If the socket is
// not connected, then the any-address is returned.
talk_base::SocketAddress P2PProxySocket::GetRemoteAddress() const {
  return remote_addr_;
}

//
//Upon successful completion, bind() shall return 0; 
//otherwise, -1 shall be returned and errno set to indicate the error.
int P2PProxySocket::Bind(const talk_base::SocketAddress& addr){
  //In p2p socket that bind function return 0, indicate the bind succeed.
  LOG(LS_ERROR) << "p2p socket have no bind function, the local port depend \
    with p2p ICE";
  return 0;
}

//The connect addr is the relay connect addr.
int P2PProxySocket::Connect(const talk_base::SocketAddress& addr){
  SetError(EALREADY);
  return SOCKET_ERROR;
}


int P2PProxySocket::Send(const void *pv, size_t cb){
  if(cb == 0)
    return SOCKET_ERROR;
  if(connect_state_ != CS_SOCKET_CONNECTED){
    SetError(ENOTCONN);
    return SOCKET_ERROR;
  }
  size_t written = 0;
  if(p2p_connection_implementator_)
    p2p_connection_implementator_->Send(GetSocketNumber(),
    TCP_SOCKET,(const char *)pv,cb,&written);
  return written;
}

int P2PProxySocket::SendTo(const void *pv,size_t cb,
                           const talk_base::SocketAddress& addr)
{
  return Send(pv,cb);
}

int P2PProxySocket::Recv(void *pv, size_t cb){
  return SOCKET_ERROR;
}

int P2PProxySocket::RecvFrom(void *pv,size_t cb,
                             talk_base::SocketAddress *paddr)
{
  return SOCKET_ERROR;
}
int P2PProxySocket::Listen(int backlog){
  return 0;
}
//virtual talk_base::Socket *Accept(talk_base::SocketAddress *paddr);
int P2PProxySocket::Close(){
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "P2P Socket Close";
  if(connect_state_ == CS_CLOSED)
    return 0;

  if(connect_state_ == CS_SOCKET_CONNECTED)
    CreateP2PSystemCommand(P2P_SYSTEM_SOCKET_CLOSE,
      other_side_socket_,GetSocketNumber(),remote_addr_);
  connect_state_ = CS_CLOSED;
  if(p2p_proxy_session_){
    p2p_proxy_session_->ReleaseP2PProxySocket(this);
    p2p_proxy_session_->DeleteSocketTable(GetSocketNumber());
    p2p_proxy_session_ = NULL;
    p2p_connection_implementator_ = NULL;
  }
  return 0;
}
int P2PProxySocket::GetError() const{
  return error_;
}
void P2PProxySocket::SetError(int error){
  error_ = error;
}

talk_base::Socket::ConnState P2PProxySocket::GetState() const {
  return talk_base::Socket::ConnState::CS_CLOSED;
}

// Fills in the given uint16 with the current estimate of the MTU along the
// path to the address to which this socket is connected. NOTE: This method
// can block for up to 10 seconds on Windows.
int P2PProxySocket::EstimateMTU(uint16* mtu){
  return 0;
}

int P2PProxySocket::GetOption(Option opt, int* value){
  return 0;
}
int P2PProxySocket::SetOption(Option opt, int value){
  return 0;
}


//////////////////////////////////////////////////////////////////////////
//Other help function
//////////////////////////////////////////////////////////////////////////

void P2PProxySocket::CreateP2PSystemCommand(uint32 command_type,
    uint32 server_socket,uint32 client_socket,
    const talk_base::SocketAddress &addr)
{
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "Send system command";
  talk_base::ByteBuffer byte_buffer(NULL,P2PRTSPCOMMAND_LENGTH);
  byte_buffer.WriteUInt32(P2P_SYSTEM_COMMAND_IDE);
  byte_buffer.WriteUInt32(command_type);
  byte_buffer.WriteUInt32(server_socket);
  byte_buffer.WriteUInt32(client_socket);
  byte_buffer.WriteUInt32(addr.ip());
  byte_buffer.WriteUInt16(addr.port());
  byte_buffer.WriteUInt16(P2P_SYSTEM_COMMAND_PADDING_BYTE);

  p2p_proxy_session_->SendSystemCommand(byte_buffer.Data(), 
    byte_buffer.Length());
}

void P2PProxySocket::OnP2PRead(const char *data, uint16 len){
  if(connect_state_ != CS_SOCKET_CONNECTED)
    return ;
  SignalReadMixData(data,len);
}

void P2PProxySocket::OnP2PWrite(P2PProxySession *p2p_proxy_session){
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "stream write event socket";
  if(connect_state_ != CS_SOCKET_CONNECTED)
    return ;
  SignalWriteEvent(this);
}

void P2PProxySocket::OnP2PClose(P2PProxySession *p2p_proxy_session){
  ASSERT(p2p_proxy_session_ == p2p_proxy_session);
  if(connect_state_ == CS_P2P_CONNECTED
    || connect_state_ == CS_SOCKET_CONNECTED){
    ASSERT(p2p_proxy_session_ != NULL
      && p2p_connection_implementator_ != NULL);
    connect_state_ = CS_CLOSED;
    p2p_proxy_session_->ReleaseP2PProxySocket(this);
    p2p_proxy_session_->DeleteSocketTable(GetSocketNumber());
    p2p_proxy_session_ = NULL;
    p2p_connection_implementator_ = NULL;
  }
  SetError(EALREADY);
  SignalCloseEvent(this,SOCKET_ERROR);
}


//////////////////////////////////////////////////////////////////////////
//P2P ProxyStart Socket implementation
//////////////////////////////////////////////////////////////////////////

P2PProxyStartSocket::P2PProxyStartSocket()
  :P2PProxySocket()
{

}

int P2PProxyStartSocket::ConnectPeer(const std::string &addr_ide,
                                const std::string &proxy_type)
{
  ASSERT(connect_state_ == CS_CLOSED);
  connect_state_ = CS_P2P_CONNECTING;

  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) 
    << "Connect the peer by source identifier " << addr_ide;
  p2p_proxy_session_ = p2p_connection_management_->ConnectBySourceIde(
    addr_ide,&remote_addr_,proxy_type);

  if(p2p_proxy_session_ == NULL){
    SetError(EALREADY);
    SignalCloseEvent(this,SOCKET_ERROR);
    return SOCKET_ERROR;
  }
  if(!p2p_proxy_session_->RegisterP2PProxySocket(this,true)){
    connect_state_ = CS_CLOSED;
    SetError(EALREADY);
    SignalCloseEvent(this,SOCKET_ERROR);
    return SOCKET_ERROR;
  }
  return 0;
}

void P2PProxyStartSocket::OnP2PConnectSucceed(
  P2PProxySession *p2p_proxy_session)
{
  //////////////////////////////////////////////////////////////////////////
  //Because the client socket connect is connected, and the system has send
  //the p2p connected succeed command to me, so ,we can do nothing for this.
  //////////////////////////////////////////////////////////////////////////
  ASSERT(connect_state_ == CS_P2P_CONNECTING);
  ASSERT(p2p_proxy_session_ == p2p_proxy_session);
  connect_state_ = CS_P2P_CONNECTED;

  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "Connect peer succeed";
  p2p_connection_implementator_ 
    = p2p_proxy_session_->GetP2PConnectionImplementator();
  ASSERT(p2p_connection_implementator_ != NULL);
  Connect(remote_addr_);
}

void P2PProxyStartSocket::OnP2PSocketConnectSucceed(
  P2PProxySession *p2p_proxy_session,uint32 other_side_socket){
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "Other side server connect succeed";
  ASSERT(p2p_proxy_session_ == p2p_proxy_session);
  ASSERT(connect_state_ == CS_SOCKET_CONNECTING);

  connect_state_ = CS_SOCKET_CONNECTED;
  other_side_socket_ = other_side_socket;
  SignalConnectEvent(this);
  SignalWriteEvent(this);
}

void P2PProxyStartSocket::OnP2PSocketConnectFailure(
  P2PProxySession *p2p_proxy_session)
{
  ASSERT(p2p_proxy_session_ == p2p_proxy_session);
  ASSERT(connect_state_ == CS_SOCKET_CONNECTING);
  connect_state_ = CS_CLOSED;
  p2p_proxy_session_->ReleaseP2PProxySocket(this);
  p2p_proxy_session_ = NULL;
  p2p_connection_implementator_ = NULL;
  SetError(EALREADY);
  SignalCloseEvent(this,SOCKET_ERROR);
}

int P2PProxyStartSocket::Connect(const talk_base::SocketAddress& addr){
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "Start to connect other side server";
  if(connect_state_ != CS_P2P_CONNECTED){
    SetError(EALREADY);
    return SOCKET_ERROR;
  }
  connect_state_ = CS_SOCKET_CONNECTING;
  CreateP2PSystemCommand(P2P_SYSTEM_CREATE_CLIENT_SOCKET,GetSocketNumber(),
    0,remote_addr_);
  return 0;
}

//////////////////////////////////////////////////////////////////////////
//P2P Proxy End Socket
//////////////////////////////////////////////////////////////////////////
P2PProxyEndSocket::P2PProxyEndSocket(P2PProxyClientSession *p2p_proxy_session,
                  uint32 other_side_socket)
  :P2PProxySocket(p2p_proxy_session,other_side_socket)
{

}

void P2PProxyEndSocket::SocketConnectSucceed(){
  ASSERT(connect_state_ == CS_SOCKET_CONNECTING);
  connect_state_ = CS_SOCKET_CONNECTED;
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "Create Client socket succeed";
  p2p_proxy_session_->RegisterNewSocketTable(GetSocketNumber(),
    other_side_socket_);
  CreateP2PSystemCommand(P2P_SYSTEM_CREATE_CLIENT_SUCCEED,
    other_side_socket_,GetSocketNumber(),remote_addr_);
  //SignalReadEvent(this);
}

void P2PProxyEndSocket::SocketConnectFailure(){
  ASSERT(connect_state_ == CS_SOCKET_CONNECTING);
  connect_state_ = CS_CLOSED;
  LOG_P2P(P2P_PROXY_SOCKET_LOGIC) << "Create Client socket failure";
  CreateP2PSystemCommand(P2P_SYSTEM_CREATE_CLIENT_FAILURE,
    other_side_socket_,0,remote_addr_);
}
