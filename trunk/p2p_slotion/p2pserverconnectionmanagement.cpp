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
//used by update
//this structure only alive in this file
//////////////////////////////////////////////////////////////////////////
class PeerInforUpdateMessage : public talk_base::MessageData{
public:
  PeerInforUpdateMessage(int peer_id, const PeerInfor &peer_infor):
    peer_infor_(peer_infor),peer_id_(peer_id){}

  int peer_id_;
  PeerInfor peer_infor_;
};

//////////////////////////////////////////////////////////////////////////
//used by online peer
//this structure only alive in this file
//////////////////////////////////////////////////////////////////////////
class OnlinePeersMessage : public talk_base::MessageData{
public:
  OnlinePeersMessage(const PeerInfors &peer_infors):
    peer_infors_(peer_infors){}

  PeerInfors peer_infors_;
};

//////////////////////////////////////////////////////////////////////////
//used by remote message
//this structure only alive in this file
//////////////////////////////////////////////////////////////////////////
class RemoteMessage : public talk_base::MessageData{
public:
  RemoteMessage(int peer_id, const std::string msg_string)
    :peer_id_(peer_id),msg_string_(msg_string){};

  int peer_id_;
  std::string msg_string_;
};

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
  :p2p_server_connection_(NULL),
  state_(SERVER_OBJECT_INITIALIZE)
{
  LOG_P2P(CREATE_DESTROY_INFOR|P2P_SERVER_MANAGER_LOGIC) 
    << "instance p2p server connection management object";
  is_server_connection_ = false;
  signal_thread_ = talk_base::Thread::Current();

  ///////////////////////////////////////////////////////////////////////////
  //TODO:(GuangleiHe) TIME: 12/19/2013
  //remove this worker thread to thread management
  ///////////////////////////////////////////////////////////////////////////
  worker_thread_ = new talk_base::Thread();
  worker_thread_->Start();
  //////////////////////////////////////////////////////////////////////////

  worker_thread_->Send(this,CREATE_P2P_SERVER_CONNECT);
  ASSERT(p2p_server_connection_ != NULL);

  p2p_source_management_ = P2PSourceManagement::Instance();

  p2p_source_management_->SignalRegisterServerResources.connect(this,
    &P2PServerConnectionManagement::OnSignalRegisterServerResources);


  SignalOnlinePeers.connect(p2p_source_management_,
    &P2PSourceManagement::OnOnlinePeers);
  SignalAPeerLogin.connect(p2p_source_management_,
    &P2PSourceManagement::OnAPeerLogin);
  SignalAPeerLogout.connect(p2p_source_management_,
    &P2PSourceManagement::OnAPeerLogout);

  //ice
}


void P2PServerConnectionManagement::OnSignalRegisterServerResources(
  const std::string &new_resources_string)
{
  LOG_P2P(P2P_SERVER_MANAGER_LOGIC) << "Register server resources signal";
  ASSERT(worker_thread_->IsCurrent());
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
  LOG_P2P(P2P_SERVER_MANAGER_LOGIC) << "server states change signal";
  ASSERT(worker_thread_->IsCurrent());
  switch(state_type){
    //
  case STATES_P2P_SERVER_LOGIN_SUCCEED:
    {
      //
      std::cout << "STATES_P2P_SERVER_LOGIN_SUCCEED" << std::endl;
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
      LOG_P2P(P2P_SERVER_MANAGER_LOGIC) 
        << "\t : STATES_P2P_REMOTE_PEER_DISCONNECTED";
      break;
    }
    //
  case ERROR_P2P_SERVER_LOGIN_SERVER_FAILURE:
    {
      LOG_P2P(P2P_SERVER_MANAGER_LOGIC)
        << "\t : ERROR_P2P_SERVER_LOGIN_SERVER_FAILURE";
      is_server_connection_ = false;
      p2p_source_management_->DeleteAllOnlinePeerResource();
      state_ = SERVER_CONNECTING;
      break;
    }
    //
  case ERROR_P2P_CAN_NOT_SEND_MESSAGE:
    {
      LOG_P2P(P2P_SERVER_MANAGER_LOGIC)
        << "\t : P2P_SERVER_MANAGER_LOGIC";
      break;
    }
  }
}

bool P2PServerConnectionManagement::SignOutP2PServer(){
  LOG_P2P(P2P_SERVER_MANAGER_LOGIC) << "signing out p2p server";
  worker_thread_->Post(this,SIGNOUT_P2P_SERVER);
  return true;
}

void P2PServerConnectionManagement::SignInP2PServer(
  const talk_base::SocketAddress &server_addr)
{
  LOG_P2P(P2P_SERVER_MANAGER_LOGIC) << "signing in p2p server";
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


  worker_thread_->Post(this,SIGNIN_P2P_SERVER);
}

bool P2PServerConnectionManagement::UpdatePeerInfor(const std::string &infor){
  LOG_P2P(P2P_SERVER_MANAGER_LOGIC) << "update peer information";
  ASSERT(worker_thread_->IsCurrent());
  if(state_ != SERVER_CONNECTING_SUCCEED){
    LOG(LS_ERROR) << "update peer information after you login p2p server";
    return false;
  }
  return p2p_server_connection_->UpdataPeerInfor(infor);
}

void P2PServerConnectionManagement::SetIceDataTunnel(
  AbstractICEConnection *ice_connection)
{
  LOG_P2P(P2P_SERVER_MANAGER_LOGIC) << "setting ice data tunnel";
  ASSERT(ice_connection != NULL);

  SignalReceiveMessageFromRemotePeer.connect(
    ice_connection,&AbstractICEConnection::OnReceiveMessageFromRemotePeer);

  ice_connection->SignalSendMessageToRemote.connect(this,
    &P2PServerConnectionManagement::OnSendMessageToRemotePeer);
}

void P2PServerConnectionManagement::OnSendMessageToRemotePeer(
  const std::string &msg_string,int peer_id)
{
  LOG_P2P(P2P_SERVER_MANAGER_LOGIC) << "Send message to remote peer by p2p server";
  ASSERT(signal_thread_->IsCurrent());
  worker_thread_->Post(this,SEND_MESSAGE_FROM_REMOTE_PEER,
    new RemoteMessage(peer_id,msg_string));
}

void P2PServerConnectionManagement::OnReceiveMessageFromRemotePeer(
  const std::string msg,int peer_id)
{
  LOG_P2P(P2P_SERVER_MANAGER_LOGIC) << "receive message from remote peer by p2p server";
  ASSERT(worker_thread_->IsCurrent());
  signal_thread_->Post(this,RECEIVE_MESSAGE_FROM_REMOTE_PEER,
    new RemoteMessage(peer_id,msg));
}

void P2PServerConnectionManagement::OnOnlinePeers(const PeerInfors &peers){
  LOG_P2P(P2P_SERVER_MANAGER_LOGIC) 
    << "on line peers information get";
  ASSERT(worker_thread_->IsCurrent());
  signal_thread_->Post(this,ONLINE_PEERS,new OnlinePeersMessage(peers));
}

void P2PServerConnectionManagement::OnAPeerLogin(int peer_id,const PeerInfor &peer){
  LOG_P2P(P2P_SERVER_MANAGER_LOGIC) 
    << "a new peer register";
  ASSERT(worker_thread_->IsCurrent());
  signal_thread_->Post(this,A_PEER_LOGIN,new PeerInforUpdateMessage(peer_id,peer));
}

void P2PServerConnectionManagement::OnAPeerLogout(int peer_id,const PeerInfor &peer){
  LOG_P2P(P2P_SERVER_MANAGER_LOGIC) 
    << "a peer out of server";
  ASSERT(worker_thread_->IsCurrent());
  signal_thread_->Post(this,A_PEER_LOGOUT,new PeerInforUpdateMessage(peer_id,peer));
}


void P2PServerConnectionManagement::OnMessage(talk_base::Message *msg){
  switch(msg->message_id){
  case CREATE_P2P_SERVER_CONNECT:
    {
      ASSERT(worker_thread_->IsCurrent());
      p2p_server_connection_ = new PeerConnectionServer();

      //workerthread_
      p2p_server_connection_->SignalStatesChange.connect(this,
        &P2PServerConnectionManagement::OnServerStatesChange);

      p2p_server_connection_->SignalOnlinePeers.connect(this,
        &P2PServerConnectionManagement::OnOnlinePeers);

      p2p_server_connection_->SignalAPeerLogin.connect(this,
        &P2PServerConnectionManagement::OnAPeerLogin);

      p2p_server_connection_->SignalAPeerLogout.connect(this,
        &P2PServerConnectionManagement::OnAPeerLogout);

      //ice
      p2p_server_connection_->SignalReceiveMessageFromRemotePeer.connect(
        this,&P2PServerConnectionManagement::OnReceiveMessageFromRemotePeer);

      SignalSendMessageToRemote.connect(p2p_server_connection_,
        &AbstractP2PServerConnection::OnSendMessageToRemotePeer);
      break;
    }
  case ONLINE_PEERS:
    {
      ASSERT(signal_thread_->IsCurrent());
      OnlinePeersMessage* params = 
        static_cast<OnlinePeersMessage*>(msg->pdata);

      SignalOnlinePeers(params->peer_infors_);

      delete params;
      break;
    }
  case A_PEER_LOGIN:
    {
      ASSERT(signal_thread_->IsCurrent());
      PeerInforUpdateMessage* params = 
        static_cast<PeerInforUpdateMessage*>(msg->pdata);

      SignalAPeerLogin(params->peer_id_,params->peer_infor_);

      delete params;
      break;
    }
  case A_PEER_LOGOUT:
    {
      ASSERT(signal_thread_->IsCurrent());
      PeerInforUpdateMessage* params = 
        static_cast<PeerInforUpdateMessage*>(msg->pdata);

      SignalAPeerLogout(params->peer_id_,params->peer_infor_);

      delete params;
      break;
    }
  case RECEIVE_MESSAGE_FROM_REMOTE_PEER:
    {
      ASSERT(signal_thread_->IsCurrent());
      RemoteMessage* params = 
        static_cast<RemoteMessage*>(msg->pdata);

      SignalReceiveMessageFromRemotePeer(params->msg_string_,params->peer_id_);

      delete params;
      break;
    }
  case SEND_MESSAGE_FROM_REMOTE_PEER:
    {
      ASSERT(worker_thread_->IsCurrent());
      RemoteMessage* params = 
        static_cast<RemoteMessage*>(msg->pdata);

      SignalSendMessageToRemote(params->msg_string_,params->peer_id_);

      delete params;
      break;
    }
  case SIGNIN_P2P_SERVER:
    {
      ASSERT(worker_thread_->IsCurrent());
      p2p_server_connection_->set_server_address(server_addr_);
      p2p_server_connection_->SignInP2PServer();
      state_ = SERVER_CONNECTING;
      break;
    }
  case SIGNOUT_P2P_SERVER:
    {
      ASSERT(worker_thread_->IsCurrent());
      p2p_server_connection_->SignOutP2PServer();
      break;
    }
  }
}