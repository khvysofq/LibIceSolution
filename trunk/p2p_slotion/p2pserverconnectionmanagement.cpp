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


    p2p_source_management_ = P2PSourceManagement::Instance();

    p2p_server_connection_->SignalStatesChange.connect(this,
      &P2PServerConnectionManagement::OnServerStatesChange);
    p2p_server_connection_->SignalOnlinePeers.connect(p2p_source_management_,
      &P2PSourceManagement::OnOnlinePeers);
    p2p_server_connection_->SignalAPeerLogin.connect(p2p_source_management_,
      &P2PSourceManagement::OnAPeerLogin);
    p2p_server_connection_->SignalAPeerLogout.connect(p2p_source_management_,
      &P2PSourceManagement::OnAPeerLogout);

    ////This is test set.
    ////Add Server Resource
    //p2p_source_management_->AddNewServerResource("RTSP_SERVER","192.168.1.1",554,"RTSP");
    //p2p_source_management_->AddNewServerResource("HTTP_SERVER","192.168.1.1",80,"HTTP");
    ////Set Local peer name.
    //std::string local_peer_name = GetCurrentComputerUserName();
    //local_peer_name += JID_DEFAULT_DOMAIN;
}

//bool P2PServerConnectionManagement::Initialize(
//  AbstractP2PServerConnection *p2p_server_connection)
//{
//  if(state_ != SERVER_OBJECT_INITIALIZE){
//    LOG(LS_ERROR) << "The Initialize function only called before Instance";
//    return false;
//  }
//  delete p2p_server_connection_;
//  p2p_server_connection_ = p2p_server_connection;
//  return true;
//}

//void P2PServerConnectionManagement::OnSendMessageToRemotePeer(
//  int peer_id, const std::string &msg)
//{
//  if(state_ != SERVER_CONNECTING_SUCCEED){
//    LOG(LS_ERROR) << "Server not login succeed";
//  }
//  p2p_server_connection_->OnSendMessageToRemotePeer(msg,peer_id);
//}
//
//void P2PServerConnectionManagement::OnReceiveMessageFromRemotePeer(
//  const std::string msg,int peer_id)
//{
//  SignalReceiveMessageFromRemotePeer(peer_id,msg);
//}

void P2PServerConnectionManagement::OnServerStatesChange(
  StatesChangeType state_type)
{
  switch(state_type){
  case STATES_P2P_SERVER_LOGIN_SUCCEED:
    {
      std::cout << "\tSTATES_P2P_SERVER_LOGIN_SUCCEED" << std::endl;
      if(state_ != SERVER_CONNECTING){
        LOG(LS_ERROR) << "Has Error at here";
        return ;
      }
      /////////////////////////////////////////////////////////
      //BUG NOTE (GuangleiHe, 11/28/2013)
      //Maybe there has a bug.
      //Because if the peer is a client peer that no 
      //server resource. What I can do?
      /////////////////////////////////////////////////////////
      p2p_server_connection_->UpdataPeerInfor(
        p2p_source_management_->GetServerResourceString());
      state_ = LOCAL_UPDATA_INFO;
      break;
    }
  case STATES_P2P_PEER_CONNECTION:
    {
      std::cout << "\tSTATES_P2P_PEER_CONNECTION" << std::endl;
      break;
    }
  case STATES_P2P_REMOTE_PEER_DISCONNECTED:
    {
      std::cout << "\tSTATES_P2P_REMOTE_PEER_DISCONNECTED" << std::endl;
      break;
    }
  case STATES_P2P_ONLINE_PEER_CONNECTION:
    {
      std::cout << "\tSTATES_P2P_ONLINE_PEER_CONNECTION" << std::endl;
      break;
    }
  case STATES_P2P_PEER_SIGNING_OUT:
    {
      std::cout << "\tSTATES_P2P_PEER_SIGNING_OUT" << std::endl;
      break;
    }
  case ERROR_P2P_SERVER_TIME_OUT:
    {
      std::cout << "\tERROR_P2P_SERVER_TIME_OUT" << std::endl;
      break;
    }
  case ERROR_P2P_SERVER_LOGIN_SERVER_FAILURE:
    {
      std::cout << "\tERROR_P2P_SERVER_LOGIN_SERVER_FAILURE" << std::endl;
      break;
    }
  case ERROR_P2P_PEER_NO_THIS_PEER:
    {
      std::cout << "\tERROR_P2P_PEER_NO_THIS_PEER" << std::endl;
      break;
    }
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

//void P2PServerConnectionManagement::SetLocalName(const std::string &local_name){
//  if(state_ != SERVER_OBJECT_INITIALIZE){
//    LOG(LS_ERROR) << "SetLocalName is error";
//  }
//  p2p_source_management_->SetLocalPeerName(local_name);
//  p2p_server_connection_->set_local_peer_name(local_name);
//  state_ = LOCAL_PEER_NAME_SET;
//}
//
//const std::string P2PServerConnectionManagement::GetLocalName() const {
//  return p2p_server_connection_->get_local_name();
//}

