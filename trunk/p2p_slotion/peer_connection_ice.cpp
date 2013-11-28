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
                                     tunnel_session_client_(NULL),
                                     local_tunnel_(NULL),
                                     remote_id_(-1),
                                     remote_jid_(NULL),
                                     local_jid_(NULL)
{
  LOG(LS_INFO) << "===" << __FUNCTION__;

  //-----------------------Part 1: initialize P2PMediator part
  receive_momery_buffer_ = new talk_base::FifoBuffer(SEND_BUFFER_LENGTH);
  send_buffer_           = new char[SEND_BUFFER_LENGTH];
  send_data_buffer_      = new SendDataBuffer();
  p2p_source_management_  = P2PSourceManagement::Instance();

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
  local_jid_ = new buzz::Jid(local_peer_name);
  tunnel_session_client_  =   new cricket::TunnelSessionClient(
    *local_jid_,session_manager_);

  tunnel_session_client_->SignalIncomingTunnel.connect(this,
    &PeerConnectionIce::OnIncomingTunnel);
};

void PeerConnectionIce::DestroyPeerConnectionIce(){
  delete send_data_buffer_;
  delete send_buffer_;
  receive_momery_buffer_->Close();
  if(remote_jid_)
    delete remote_jid_;
  LOG(LS_INFO) << "===" << __FUNCTION__;
  delete basic_network_manager_;
  LOG(LS_INFO) << "\t" << "delete basic_network_manager succeed";
  delete http_allocator_;
  LOG(LS_INFO) << "\t" << "delete http_allocator_ succeed";
  if(local_jid_)
  delete local_jid_;
  LOG(LS_INFO) << "\t" << "delete local_jid_ succeed";

  if(local_tunnel_)
  {
    delete local_tunnel_;
    local_tunnel_ = NULL;
    LOG(LS_INFO) << "\t" << "delete local_tunnel_ succeed";
  }
  delete tunnel_session_client_;
  LOG(LS_INFO) << "\t" << "delete tunnel_session_client_ succeed";
  delete session_manager_;
  LOG(LS_INFO) << "\t" << "delete session_manager_ succeed";
}

PeerConnectionIce::~PeerConnectionIce(){
  LOG(LS_INFO) << "===" << __FUNCTION__;
  DestroyPeerConnectionIce();
};
void PeerConnectionIce::ConnectionToRemotePeer(int remote_peer_id)
{
  LOG(LS_INFO) << "===" << __FUNCTION__;
  ASSERT(remote_id_ == -1);
  remote_id_ = remote_peer_id;

  //TODO:(guangleiHe) there has a bug.
  //The current peer connection only connect one peer
  //Add_remote_peer(remote_peer_id,remote_peer_name);
  
  //initialize tunnel session client
  remote_jid_ = new buzz::Jid(
    p2p_source_management_->GetRemotePeerNameByPeerId(remote_peer_id));

  LOG(LS_INFO) << "\t remote jid is" << remote_jid_->Str();
  
  local_tunnel_ = tunnel_session_client_->CreateTunnel(
    *remote_jid_,DEFAULT_DECRIBE);
  local_tunnel_->SignalEvent.connect(this,
    &PeerConnectionIce::OnStreamEvent);
  SignalStatesChange(STATES_ICE_START_PEER_CONNECTION);
}


void PeerConnectionIce::OnReceiveMessageFromRemotePeer(const std::string msg, 
                                                       int peer_id)
{
  LOG(LS_INFO) << "===" << __FUNCTION__;
  LOG(LS_INFO) << "**********************************************";
  LOG(LS_INFO) << "\t debug information for out going message";
  LOG(LS_INFO) << msg;
  LOG(LS_INFO) << "**********************************************";

  if(remote_id_ == -1)
    remote_id_ = peer_id;
  signal_thread_->Post(this,REMOTE_PEER_MESSAGE,
    new MessageSendData(msg));
}

void PeerConnectionIce::OnReceiveDataFromUpLayer(const char *data, int len)
{
  LOG(LS_INFO) << "===" << __FUNCTION__;
  //local_tunnel_->WriteAll(data,len,NULL,NULL);
  if(len == 0){
    send_data_buffer_->SaveData(data,NETWORKHEADER_LENGTH);
    return ;
  }
  send_data_buffer_->SaveData(data,len);
  send_data_buffer_->SendDataUsedStream(local_tunnel_);
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
  
  SignalSendMessageToRemote(new_stanza->Str(),
    remote_id_);

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

  local_tunnel_ = client->AcceptTunnel(session);
  local_tunnel_->SignalEvent.connect(this,
    &PeerConnectionIce::OnStreamEvent);
}
void PeerConnectionIce::OnStreamEvent(talk_base::StreamInterface* stream, 
                                      int events,
                                      int error)
{
  LOG(LS_INFO) << "===" << __FUNCTION__;
  //LOG(LS_INFO) << "\t Error code is >>>>>>>>>>>>>>>\t" << error;

  if (events & talk_base::SE_READ) {
    if (stream == local_tunnel_) {
      LOG(LS_INFO) <<"\ttalk_base::SE_READ";
      if(error == 0){
        //ReadData(stream);
        SignalSendDataToUpLayer(stream);
      }
    }
  }

  if (events & talk_base::SE_WRITE) {
    if (stream == local_tunnel_) {
      //std::cout << "\ttalk_base::SE_WRITE  ????????????????????????"
      //  << std::endl;
      send_data_buffer_->SetNormalState();
      if(send_data_buffer_->SendDataUsedStream(local_tunnel_)){
        SignalStatesChange(STATES_ICE_TUNNEL_SEND_DATA);
        SignalStreamWrite(stream);
      }
    }
  }

  if (events & talk_base::SE_CLOSE) {
    if (stream == local_tunnel_) {
      local_tunnel_->Close();
      LOG(LS_INFO) << "\ttalk_base::SE_CLOSE";
      SignalStatesChange(STATES_ICE_TUNNEL_CLOSED);
    }
  }
}

void PeerConnectionIce::ReadData(talk_base::StreamInterface *stream){
  LOG(LS_INFO) << "===" << __FUNCTION__;
  size_t res = 0;
  stream->Read(send_buffer_,SEND_BUFFER_LENGTH,&res,NULL);
  std::cout << "\tRead Data length is " << res << std::endl;
}

void PeerConnectionIce::WriteData(const char *data, int len){
  LOG(LS_INFO) << "===" << __FUNCTION__;
  size_t res = 0;
  local_tunnel_->Write(data,len,&res,NULL);
  LOG(LS_INFO) << "\t Write Data length is " << res;
  if(res != len)
    LOG(LS_ERROR) << "\t error write data not complete";
}

bool PeerConnectionIce::IsBlock() const{
  return send_data_buffer_->IsBlockState();
}

size_t PeerConnectionIce::GetRemainBufferLength() const {
  return send_data_buffer_->GetBufferRemainLength();
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
