/*
* p2p solution
* Copyright 2013, VZ Inc.
* 
* Author   : GuangLei He
* Email    : guangleihe@gmail.com
* Created  : 2013/11/28      11:52
* Filename : F:\GitHub\trunk\p2p_slotion\p2pserverconnectionmanagement.cpp
* File path: F:\GitHub\trunk\p2p_slotion
* File base: p2pserverconnectionmanagement
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

#include "p2pserverconnectionmanagement.h"
#include "peer_connection_server.h"
#include "p2psourcemanagement.h"

//////////////////////////////////////////////////////////////////////////
//Singleton Pattern 
P2PServerConnectionManagement 
  *P2PServerConnectionManagement::p2p_server_connection_management_ = NULL;

P2PServerConnectionManagement *P2PServerConnectionManagement::Instance(){
  if(!p2p_server_connection_management_)
    p2p_server_connection_management_ = new P2PServerConnectionManagement();
  return p2p_server_connection_management_;
}

//////////////////////////////////////////////////////////////////////////
P2PServerConnectionManagement::P2PServerConnectionManagement()
  :p2p_server_connection_(new PeerConnectionServer()),
  state_(SERVER_OBJECT_INITIALIZE)
{
  is_server_connection_ = false;
  current_thread_ = talk_base::Thread::Current();

  p2p_source_management_ = P2PSourceManagement::Instance();

  p2p_source_management_->SignalRegisterServerResources.connect(this,
    &P2PServerConnectionManagement::OnSignalRegisterServerResources);

  p2p_server_connection_->SignalStatesChange.connect(this,
    &P2PServerConnectionManagement::OnServerStatesChange);
  p2p_server_connection_->SignalOnlinePeers.connect(p2p_source_management_,
    &P2PSourceManagement::OnOnlinePeers);
  p2p_server_connection_->SignalAPeerLogin.connect(p2p_source_management_,
    &P2PSourceManagement::OnAPeerLogin);
  p2p_server_connection_->SignalAPeerLogout.connect(p2p_source_management_,
    &P2PSourceManagement::OnAPeerLogout);

}


void P2PServerConnectionManagement::OnSignalRegisterServerResources(
  const std::string &new_resources_string)
{
  /////////////////////////////////////////////////////////
  //BUG NOTE (GuangleiHe, 11/28/2013)
  //Maybe there has a bug.
  //Because if the peer is a client peer that has no 
  //server resource. What I can do?
  /////////////////////////////////////////////////////////
  p2p_server_connection_->UpdataPeerInfor(new_resources_string);
}

void P2PServerConnectionManagement::OnServerStatesChange(
  StatesChangeType state_type)
{
  switch(state_type){
    //
  case STATES_P2P_SERVER_LOGIN_SUCCEED:
    {
      //
      std::cout << "\tSTATES_P2P_SERVER_LOGIN_SUCCEED" << std::endl;
      if(state_ != SERVER_CONNECTING){
        LOG(LS_ERROR) << "Has Error at here";
        return ;
      }
      is_server_connection_ = true;

      p2p_server_connection_->UpdataPeerInfor(
        p2p_source_management_->GetServerResourceString());
      state_ = SERVER_CONNECTING_SUCCEED;
      break;
    }
    //
  case STATES_P2P_REMOTE_PEER_DISCONNECTED:
    {
      std::cout << "\tSTATES_P2P_REMOTE_PEER_DISCONNECTED" << std::endl;
      break;
    }
    //
  case ERROR_P2P_SERVER_LOGIN_SERVER_FAILURE:
    {
      std::cout << "\tERROR_P2P_SERVER_LOGIN_SERVER_FAILURE" << std::endl;
      std::cout << "\tWe will reconnect after 1 second." << std::endl;
      is_server_connection_ = false;
      p2p_source_management_->DeleteAllOnlinePeerResource();
      state_ = SERVER_CONNECTING;
      break;
    }
    //
  case ERROR_P2P_CAN_NOT_SEND_MESSAGE:
    {
      std::cout << "\tERROR_CAN_NOT_SEND_MESSAGE" << std::endl;
      break;
    }
  }
}

bool P2PServerConnectionManagement::SignOutP2PServer(){
  return p2p_server_connection_->SignOutP2PServer();
}

void P2PServerConnectionManagement::SignInP2PServer(
  const talk_base::SocketAddress &server_addr)
{
  if(server_addr.IsNil()){
    LOG(LS_ERROR) << "The p2p server address is not complete";
    return ;
  }
  if(p2p_source_management_->IsSetLocalPeerName()){
    LOG(LS_ERROR) << "You must to set local peer name after login server";
    return ;
  }
  ASSERT(state_ == SERVER_OBJECT_INITIALIZE);
  server_addr_ = server_addr;
  p2p_server_connection_->set_server_address(server_addr);
  p2p_server_connection_->SignInP2PServer();
  state_ = SERVER_CONNECTING;
}

bool P2PServerConnectionManagement::UpdatePeerInfor(const std::string &infor){
  if(state_ != SERVER_CONNECTING_SUCCEED){
    LOG(LS_ERROR) << "update peer infor after you login p2p server";
    return false;
  }
  return p2p_server_connection_->UpdataPeerInfor(infor);
}

void P2PServerConnectionManagement::SetIceDataTunnel(
  AbstractICEConnection *ice_connection)
{
  ASSERT(ice_connection != NULL);

  p2p_server_connection_->SignalReceiveMessageFromRemotePeer.connect(
    ice_connection,&AbstractICEConnection::OnReceiveMessageFromRemotePeer);
  ice_connection->SignalSendMessageToRemote.connect(p2p_server_connection_,
    &AbstractP2PServerConnection::OnSendMessageToRemotePeer);
}