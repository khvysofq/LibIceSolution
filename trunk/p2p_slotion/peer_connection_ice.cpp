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

#include "p2pconnectionmanagement.h"
#include "p2psourcemanagement.h"
#include "senddatabuffer.h"
#include "peer_connection_ice.h"
#include "defaults.h"


class MessageSendData : public talk_base::MessageData
{
public:
  MessageSendData(std::string send_data, int send_data_len = 0)
    :send_data_(send_data),send_data_len_(send_data_len){}

  std::string  send_data_;
  int send_data_len_;
};

PeerConnectionIce::PeerConnectionIce(talk_base::Thread *worker_thread,
                                     talk_base::Thread *signal_thread)

                                     :worker_thread_(worker_thread),
                                     signal_thread_(signal_thread),
                                     tunnel_session_client_(NULL)
{
  LOG(LS_INFO) << "===" << __FUNCTION__;

  //-----------------------Part 1: initialize P2PMediator part
  p2p_source_management_  = P2PSourceManagement::Instance();
  p2p_connection_management_ = P2PConnectionManagement::Instance();
  //-----------------------Part 2: initialize ICE part
  basic_network_manager_  =   new talk_base::BasicNetworkManager();
  basic_prot_allocator_   =   new cricket::BasicPortAllocator(
    basic_network_manager_,
    new talk_base::BasicPacketSocketFactory(worker_thread),
    KStunAddr);
  //initialize session manager
  session_manager_        =   new cricket::SessionManager(basic_prot_allocator_,
    worker_thread_);
  session_manager_->SignalRequestSignaling.connect(this,
    &PeerConnectionIce::OnRequestSignaling);
  session_manager_->SignalOutgoingMessage.connect(this,
    &PeerConnectionIce::OnOutgoingMessage);

  const std::string local_peer_name = p2p_source_management_->GetLocalPeerName();
  if(local_peer_name.empty()){
    LOG(LS_ERROR) << "Local Peer Name is empty";
    return ;
  }
  buzz::Jid local_jid(local_peer_name);
  tunnel_session_client_  =   new cricket::TunnelSessionClient(
    local_jid,session_manager_);

  tunnel_session_client_->SignalIncomingTunnel.connect(this,
    &PeerConnectionIce::OnIncomingTunnel);
};

void PeerConnectionIce::DestroyPeerConnectionIce(){
  LOG(LS_INFO) << "===" << __FUNCTION__;
  delete basic_network_manager_;
  LOG(LS_INFO) << "\t" << "delete basic_network_manager succeed";
  delete tunnel_session_client_;
  LOG(LS_INFO) << "\t" << "delete tunnel_session_client_ succeed";
  delete session_manager_;
  LOG(LS_INFO) << "\t" << "delete session_manager_ succeed";
}

PeerConnectionIce::~PeerConnectionIce(){
  LOG(LS_INFO) << "===" << __FUNCTION__;
  DestroyPeerConnectionIce();
};

void PeerConnectionIce::ConnectionToRemotePeer(
  ProxySocketBegin *proxy_socket_begin,int remote_peer_id)
{
  LOG(LS_INFO) << "===" << __FUNCTION__;
  //initialize tunnel session client
  buzz::Jid remote_jid(
    p2p_source_management_->GetRemotePeerNameByPeerId(remote_peer_id));

  LOG(LS_INFO) << "\t remote jid is" << remote_jid.Str();
  
  talk_base::StreamInterface *stream = tunnel_session_client_->CreateTunnel(
    remote_jid,DEFAULT_DECRIBE);

  p2p_connection_management_->CreateProxyP2PSession(proxy_socket_begin,remote_jid.Str(),
    stream);
}

void PeerConnectionIce::OnReceiveMessageFromRemotePeer(const std::string msg, 
                                                       int peer_id)
{
  LOG(LS_INFO) << "===" << __FUNCTION__;
  LOG(LS_INFO) << "**********************************************";
  LOG(LS_INFO) << "\t debug information for out going message";
  LOG(LS_INFO) << msg;
  LOG(LS_INFO) << "**********************************************";
  signal_thread_->Post(this,REMOTE_PEER_MESSAGE,
    new MessageSendData(msg));
}

///////////////////////////////////////////////////////////////////////
//private ICE function
///////////////////////////////////////////////////////////////////////
void PeerConnectionIce::OnRequestSignaling(){
  LOG(LS_INFO) << "===" << __FUNCTION__;
  session_manager_->OnSignalingReady();
}

void PeerConnectionIce::OnOutgoingMessage(cricket::SessionManager* manager,
                                          const buzz::XmlElement* stanza)
{
  LOG(LS_INFO) << "===" << __FUNCTION__;
  buzz::XmlElement* new_stanza
    =   new buzz::XmlElement(*stanza);
  new_stanza->AddAttr(buzz::QN_FROM, tunnel_session_client_->jid().Str());
  ///////////////////////////////////////////////////////////////////////////
  //BUG NOTE (GuangleiHe, 11/29/2013)
  //Maybe there has a bug.
  //Because there only connect one peer.It must parse the remote id used 
  //remote jid
  ///////////////////////////////////////////////////////////////////////////
  const std::string remote_peer_name = stanza->Attr(buzz::QN_TO);;
  int remote_peer_id = 
    p2p_source_management_->GetRemotePeerIdByPeerName(remote_peer_name);
  if(remote_peer_id == 0){
    LOG(LS_ERROR) << "Can't find this peer";
    return ;
  }
  SignalSendMessageToRemote(new_stanza->Str(),
    remote_peer_id);

  LOG(LS_INFO) << "==============================================";
  LOG(LS_INFO) << "\t debug information for out going message";
  LOG(LS_INFO) << new_stanza->Str();
  LOG(LS_INFO) << tunnel_session_client_->jid().Str();
  LOG(LS_INFO) << "==============================================";

  delete new_stanza;
}

void PeerConnectionIce::OnIncomingTunnel(cricket::TunnelSessionClient* client,
                                         buzz::Jid jid, std::string description,
                                         cricket::Session* session)
{
  LOG(LS_INFO) << "===" << __FUNCTION__;
  talk_base::StreamInterface *stream = client->AcceptTunnel(session);

  p2p_connection_management_->CreateProxyP2PSession(NULL,jid.Str(),
    stream);
}

void PeerConnectionIce::OnMessage(talk_base::Message* msg){
  LOG(LS_INFO) << "===" << __FUNCTION__;
  switch(msg->message_id)
  {
  case REMOTE_PEER_MESSAGE:
    {
      LOG(LS_INFO) << "\tREMOTE_PEER_MESSAGE";
      MessageSendData* params = 
        static_cast<MessageSendData*>(msg->pdata);
      buzz::XmlElement* data = 
        buzz::XmlElement::ForStr(params->send_data_);
      bool response = data->Attr(buzz::QN_TYPE) == buzz::STR_RESULT;
      if(!response)
      {
        session_manager_->OnIncomingMessage(data);
        LOG(LS_VERBOSE) << "\tOnIncomingMessage";
      }
      else
      {
        session_manager_->OnIncomingResponse(NULL,data);
        LOG(LS_VERBOSE) << "\tOnIncomingResponse";
      }
      delete data;
      delete params;
      break;
    }
  }
}
