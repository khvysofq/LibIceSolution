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

#include "asyncp2ppackagesocket.h"
#include "p2pconnectionmanagement.h"


//Our function
AsyncP2PPackageSocket::AsyncP2PPackageSocket()
  :connect_state_(CS_CLOSED),
  partner_socket_(NULL),
  partner_socket_type_(TCP_SOCKET)
{
  p2p_connection_management_ = P2PConnectionManagement::Instance();
  //SignalSendDataToLowLayer.connect(virtual_network_,
  //  &AbstractVirtualNetwork::OnReceiveDataFromUpLayer);
}

//for AsyncSocket
talk_base::AsyncSocket* AsyncP2PPackageSocket::Accept(
  talk_base::SocketAddress* paddr){
    LOG(LS_INFO) << "p2p socket have no accept function";
    return NULL;
}

bool AsyncP2PPackageSocket::SetPartnerSocket(
  talk_base::AsyncSocket *socket, SocketType socket_type)
{
  if(!socket){
    LOG(LS_ERROR) << "partner socket is null";
    return false;
  }
  partner_socket_ = socket;
  partner_socket_type_ = socket_type;
  return true;
}

//for Socket Class

// Returns the address to which the socket is bound.  If the socket is not
// bound, then the any-address is returned.
talk_base::SocketAddress AsyncP2PPackageSocket::GetLocalAddress(){
  return local_addr_;
}

// Returns the address to which the socket is connected.  If the socket is
// not connected, then the any-address is returned.
talk_base::SocketAddress AsyncP2PPackageSocket::GetRemoteAddress(){
  return remote_addr_;
}

//
//Upon successful completion, bind() shall return 0; 
//otherwise, -1 shall be returned and errno set to indicate the error.
int AsyncP2PPackageSocket::Bind(const talk_base::SocketAddress& addr){
  //In p2p socket that bind function return 0, indicate the bind succeed.
  LOG(LS_ERROR) << "p2p socket have no bind function, the local port depend \
    with p2p ICE";
  return 0;
}

//The connect addr is the relay connect addr.
int AsyncP2PPackageSocket::Connect(const talk_base::SocketAddress& addr){
  if(connect_state_ != CS_CLOSED){
    SetError(EALREADY);
    return SOCKET_ERROR;
  }
  remote_addr_ = addr;
  connect_state_ = CS_CONNECTING;



  return 0;
}

int AsyncP2PPackageSocket::Send(const void *pv, size_t cb){
  return 0;
}

int AsyncP2PPackageSocket::SendTo(const void *pv, 
                                  size_t cb, 
                                  const talk_base::SocketAddress& addr)
{
  return 0;
}

int AsyncP2PPackageSocket::Recv(void *pv, size_t cb){
  return 0;
}

int AsyncP2PPackageSocket::RecvFrom(void *pv, 
                                    size_t cb, 
                                    talk_base::SocketAddress *paddr)
{
  return 0;
}
int AsyncP2PPackageSocket::Listen(int backlog){
  return 0;
}
//virtual talk_base::Socket *Accept(talk_base::SocketAddress *paddr);
int AsyncP2PPackageSocket::Close(){
  return 0;
}
int AsyncP2PPackageSocket::GetError() const{
  return 0;
}
void AsyncP2PPackageSocket::SetError(int error){
  error_ = error;
}

// Fills in the given uint16 with the current estimate of the MTU along the
// path to the address to which this socket is connected. NOTE: This method
// can block for up to 10 seconds on Windows.
int AsyncP2PPackageSocket::EstimateMTU(uint16* mtu){
  return 0;
}

int AsyncP2PPackageSocket::GetOption(Option opt, int* value){
  return 0;
}
int AsyncP2PPackageSocket::SetOption(Option opt, int value){
  return 0;
}