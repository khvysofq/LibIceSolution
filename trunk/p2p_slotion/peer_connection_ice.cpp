/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/22      9:47
 * Filename : F:\GitHub\trunk\p2p_slotion\peer_connection_ice.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: peer_connection_ice
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
#include "talk/p2p/base/basicpacketsocketfactory.h"

#include "defaults.h"
#include "p2pconnectionmanagement.h"
#include "p2psourcemanagement.h"
#include "senddatabuffer.h"
#include "peer_connection_ice.h"

//////////////////////////////////////////////////////////////////////////
//Create MessageSendData structure, this structure used by thread 
//message loop, the structure only alive in this file
//////////////////////////////////////////////////////////////////////////
class MessageSendData : public talk_base::MessageData
{
public:
  MessageSendData(std::string send_data, int send_data_len = 0)
    :send_data_(send_data),send_data_len_(send_data_len){}

  std::string  send_data_;
  int send_data_len_;
};

//////////////////////////////////////////////////////////////////////////
//Create CreateNewTunnelData structure, this structure as same as prove
//////////////////////////////////////////////////////////////////////////
class CreateNewTunnelData : public talk_base::MessageData{
public:
  CreateNewTunnelData(const std::string &remote_peer_name):
    remote_peer_name_(remote_peer_name),stream_(NULL){}

  talk_base::StreamInterface *GetStream() const{return stream_;}
  void SetStream(talk_base::StreamInterface *stream){stream_ = stream;}

  const std::string GetRemotePeerName() const {return remote_peer_name_;}
private:
  talk_base::StreamInterface *stream_;
  std::string remote_peer_name_;
};

//////////////////////////////////////////////////////////////////////////
//implement for peer connection ice
//////////////////////////////////////////////////////////////////////////
PeerConnectionIce::PeerConnectionIce(talk_base::Thread *signal_thread,
                                     talk_base::Thread *worker_thread)
                                     :signal_thread_(signal_thread),
                                     worker_thread_(worker_thread)
{
  LOG_P2P(CREATE_DESTROY_INFOR | P2P_ICE_LOGIC_INFOR)
    << "Create PeerConnectIce object";
  ASSERT(signal_thread_->IsCurrent());
  
  //-----------------------Part 1: initialize P2PMediator part
  p2p_source_management_  = P2PSourceManagement::Instance();
  p2p_connection_management_ = P2PConnectionManagement::Instance();

  //-----------------------Part 2: initialize ICE part
  basic_network_manager_  =   new talk_base::BasicNetworkManager();
  basic_prot_allocator_   =   new cricket::BasicPortAllocator(
    basic_network_manager_,
    new talk_base::BasicPacketSocketFactory(worker_thread),
    KStunAddr);

  //basic_prot_allocator_   =   new cricket::BasicPortAllocator(
  //  basic_network_manager_,
  //  KStunAddr,
  //  KRelayAddr,
  //  KRelayAddr,
  //  KRelayAddr);

  //basic_prot_allocator_->set_flags(
  //  cricket::PORTALLOCATOR_USE_LARGE_SOCKET_SEND_BUFFERS);
  //initialize session manager
  session_manager_        =   new cricket::SessionManager(basic_prot_allocator_,
    worker_thread_);
  session_manager_->SignalRequestSignaling.connect(this,
    &PeerConnectionIce::OnRequestSignaling);
  session_manager_->SignalOutgoingMessage.connect(this,
    &PeerConnectionIce::OnOutgoingMessage);

  const std::string local_peer_name = p2p_source_management_->GetLocalPeerName();
  ASSERT(!local_peer_name.empty());

  buzz::Jid local_jid(local_peer_name);
  tunnel_session_client_  =   new cricket::TunnelSessionClient(
    local_jid,session_manager_);

  tunnel_session_client_->SignalIncomingTunnel.connect(this,
    &PeerConnectionIce::OnIncomingTunnel);
};

void PeerConnectionIce::Destroy(){
  ASSERT(signal_thread_->IsCurrent());
  LOG_P2P(CREATE_DESTROY_INFOR | P2P_ICE_LOGIC_INFOR)
    << "Destroy PeerConnectIce object";
  ASSERT(signal_thread_->IsCurrent());
  delete basic_network_manager_;
  delete tunnel_session_client_;
  delete session_manager_;
}

PeerConnectionIce::~PeerConnectionIce(){
  ASSERT(signal_thread_->IsCurrent());
  Destroy();
};

talk_base::StreamInterface *PeerConnectionIce::ConnectionToRemotePeer(
  const std::string remote_peer_name)
{
  ASSERT(signal_thread_->IsCurrent());
  LOG_P2P(P2P_ICE_LOGIC_INFOR)
    << "Connection to remote peer " << remote_peer_name;
  CreateNewTunnelData *new_tunnel_data = new CreateNewTunnelData(remote_peer_name);
  signal_thread_->Send(this,CREATE_NEW_TUNNEL,new_tunnel_data);
  talk_base::StreamInterface *stream = new_tunnel_data->GetStream();
  ASSERT(stream != NULL);
  delete new_tunnel_data;
  return stream;
}

void PeerConnectionIce::OnReceiveMessageFromRemotePeer(const std::string msg, 
                                                       int peer_id)
{
  LOG_P2P(P2P_ICE_DATA_INFOR) << "receive data from server "
    << "\tThe peer id is " << peer_id
    << "\tThe data is " << msg;
  ASSERT(signal_thread_->IsCurrent());
  signal_thread_->Post(this,REMOTE_PEER_MESSAGE,new MessageSendData(msg));
}

///////////////////////////////////////////////////////////////////////
//private ICE function
///////////////////////////////////////////////////////////////////////
void PeerConnectionIce::OnRequestSignaling(){
  ASSERT(signal_thread_->IsCurrent());
  LOG_P2P(P2P_ICE_LOGIC_INFOR) << "Collection local candidate";
  session_manager_->OnSignalingReady();
}

void PeerConnectionIce::OnOutgoingMessage(cricket::SessionManager* manager,
                                          const buzz::XmlElement* stanza)
{
  LOG_P2P(P2P_ICE_DATA_INFOR) << "Send p2p ice data by server";
  ASSERT(signal_thread_->IsCurrent());
  buzz::XmlElement* new_stanza
    =   new buzz::XmlElement(*stanza);
  new_stanza->AddAttr(buzz::QN_FROM, tunnel_session_client_->jid().Str());

  //get the remote peer name by JID, we need the peer name to get peer id
  //and then pass the peer id it to server, make it known what we want to
  //connect
  const std::string remote_peer_name = stanza->Attr(buzz::QN_TO);
  int remote_peer_id = 
    p2p_source_management_->GetRemotePeerIdByPeerName(remote_peer_name);
  if(remote_peer_id == 0){
    LOG(LS_ERROR) << "Can't find this peer";
    return ;
  }

  //Send data to remote peer by server.
  SignalSendMessageToRemote(new_stanza->Str(),remote_peer_id);

  delete new_stanza;
}

void PeerConnectionIce::OnIncomingTunnel(cricket::TunnelSessionClient* client,
                                         buzz::Jid jid, std::string description,
                                         cricket::Session* session)
{
  LOG_P2P(P2P_ICE_LOGIC_INFOR) << "get p2p request";
  ASSERT(signal_thread_->IsCurrent());
  talk_base::StreamInterface *stream = client->AcceptTunnel(session);
  
  p2p_connection_management_->CreateP2PProxySession(jid.Str(),stream);
}

void PeerConnectionIce::OnMessage(talk_base::Message* msg){
  switch(msg->message_id)
  {
  case REMOTE_PEER_MESSAGE:
    {
      ASSERT(signal_thread_->IsCurrent());

      MessageSendData* params = 
        static_cast<MessageSendData*>(msg->pdata);

      buzz::XmlElement* data = 
        buzz::XmlElement::ForStr(params->send_data_);

      bool response = data->Attr(buzz::QN_TYPE) == buzz::STR_RESULT;
      if(!response)
      {
        session_manager_->OnIncomingMessage(data);
      }
      else
      {
        session_manager_->OnIncomingResponse(NULL,data);
      }

      delete data;
      delete params;
      break;
    }
  case CREATE_NEW_TUNNEL:
    {
      ASSERT(signal_thread_->IsCurrent());
      CreateNewTunnelData* params = 
        static_cast<CreateNewTunnelData*>(msg->pdata);

      std::string remote_peer_name = params->GetRemotePeerName();
      buzz::Jid remote_jid(remote_peer_name);
      ASSERT(!remote_jid.IsEmpty());
      std::string decribe;
      talk_base::CreateRandomString(16,RANDOM_BASE64,&decribe);
      talk_base::StreamInterface *stream = tunnel_session_client_->CreateTunnel(
        remote_jid,decribe);
      params->SetStream(stream);
      break;
    }
  }
}
