#include "talk/base/logging.h"
#include "peer_connection_ice.h"
#include "defaults.h"
#include "talk/p2p/base/basicpacketsocketfactory.h"


static const int SEND_BUFFER_LENGTH = 64 * 1024;

talk_base::SocketAddress  KStunAddr("stun.endigovoip.com",3478);
class MessageSendData : public talk_base::MessageData
{
public:
  MessageSendData(std::string send_data, int send_data_len = 0)
    :send_data_(send_data),send_data_len_(send_data_len){}
  std::string  send_data_;
  int send_data_len_;
};

PeerConnectionIce::PeerConnectionIce(talk_base::Thread *worker_thread,
                                     talk_base::Thread *signal_thread,
                                     AbstractP2PServerConnection *
                                             p2p_server_connection,
                                     std::string local_peer_name)

                                     :AbstractICEConnection(p2p_server_connection),
                                     worker_thread_(worker_thread),
                                     signal_thread_(signal_thread),
                                     tunnel_session_client_(NULL),
                                     local_tunnel_(NULL),
                                     local_peer_name_(local_peer_name),
                                     remote_id_(-1),
                                     remote_jid_(NULL),
                                     local_jid_(NULL)
{
  LOG(LS_INFO) << "===" << __FUNCTION__;
  //-----------------------Part 1: initialize P2PMediator part
  receive_momery_buffer_ = new talk_base::FifoBuffer(SEND_BUFFER_LENGTH);
  send_buffer_           = new char[SEND_BUFFER_LENGTH];
  //-----------------------Part 2: initialize ICE part
  basic_network_manager_  =   new talk_base::BasicNetworkManager();
  //http_allocator_         =   new cricket::HttpPortAllocator(
  //  basic_network_manager_,HTTP_USER_AGENT);
  //stun_hosts_.push_back(KStunAddr);
  //http_allocator_->SetStunHosts(stun_hosts_);
  basic_prot_allocator_   =   new cricket::BasicPortAllocator(
    basic_network_manager_,
    new talk_base::BasicPacketSocketFactory(signal_thread_),
    KStunAddr);
  //initialize session manager
  session_manager_        =   new cricket::SessionManager(basic_prot_allocator_,
    worker_thread_);
  session_manager_->SignalRequestSignaling.connect(this,
    &PeerConnectionIce::OnRequestSignaling);
  session_manager_->SignalOutgoingMessage.connect(this,
    &PeerConnectionIce::OnOutgoingMessage);
  
  
  local_jid_ = new buzz::Jid(local_peer_name_);
  tunnel_session_client_  =   new cricket::TunnelSessionClient(
    *local_jid_,session_manager_);
  tunnel_session_client_->SignalIncomingTunnel.connect(this,
    &PeerConnectionIce::OnIncomingTunnel);
};

void PeerConnectionIce::DestroyPeerConnectionIce(){
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
void PeerConnectionIce::ConnectionToRemotePeer(int remote_peer_id, 
                                         std::string remote_peer_name)
{
  LOG(LS_INFO) << "===" << __FUNCTION__;
  ASSERT(remote_id_ == -1);
  remote_id_ = remote_peer_id;

  //TODO:(guangleiHe) there has a bug.
  //The current peer connection only connect one peer
  Add_remote_peer(remote_peer_id,remote_peer_name);
  
  //initialize tunnel session client
  remote_jid_ = new buzz::Jid(remote_peer_name);
  LOG(LS_INFO) << "\t remote jid is" << remote_jid_->Str();
  local_tunnel_ = tunnel_session_client_->CreateTunnel(
    *remote_jid_,DEFAULT_DECRIBE);
  local_tunnel_->SignalEvent.connect(this,
    &PeerConnectionIce::OnStreamEvent);
  SignalStatesChange(STATES_ICE_START_PEER_CONNECTION);
}


buzz::Jid *PeerConnectionIce::GetJid(){
  std::string jid_string;
  if(local_peer_name_.empty())
    local_peer_name_ = GetCurrentComputerUserName();
  jid_string = local_peer_name_;
  jid_string += JID_DEFAULT_DOMAIN;
  return new buzz::Jid(jid_string);
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
  if(len == -1){
    LOG(LS_INFO) << "\tSave data to receive memory buffer";
    receive_momery_buffer_->Write(data,20,NULL,NULL);
    return ;
  }
  size_t count = 0;
  talk_base::StreamResult res;
  //1. write data to FIFO buffer
  int write_fifo_count = 0;
  while(write_fifo_count < len){
    res = receive_momery_buffer_->Write(data + write_fifo_count,
      len - write_fifo_count,&count,NULL);
    if(res != talk_base::SR_SUCCESS){
      LOG(LS_ERROR) << "write data to FIFO buffer error, the write length is " 
        << count;
      break;
    }
    write_fifo_count += len;
  }
  LOG(LS_INFO) << "\t 1. write data to FIFO buffer length " << write_fifo_count;

  //2. get FIFO buffer reading position
  size_t readable_fifo_length = 0;
  size_t send_data_length = 0;
  const void *fifo_buffer 
    = receive_momery_buffer_->GetReadData(&readable_fifo_length);
  LOG(LS_INFO) << "\t 2. get FIFO buffer reading position " << readable_fifo_length;

  //3. send data to remote peer
  res = local_tunnel_->WriteAll(fifo_buffer,readable_fifo_length,
    &send_data_length,NULL);
  if(res != talk_base::SR_SUCCESS){
    LOG(LS_ERROR) << "send data to remote peer, the send length is " 
      << send_data_length << "\t" << StreamResultToString(res);
  }
  if(res == talk_base::SR_BLOCK){
    //sleep(5);
  }
  LOG(LS_INFO) << "\t 3. send data to remote peer " << send_data_length;

  //4. flush data in FIFO buffer
  size_t flush_length;
  res = receive_momery_buffer_->Read(send_buffer_,
    send_data_length,&flush_length,NULL);
  if(res != talk_base::SR_SUCCESS || flush_length < send_data_length){
    LOG(LS_ERROR) << "flush data in FIFO buffer error, the write length is " 
      << flush_length;
  }
  LOG(LS_INFO) << "\t 4. flush data in FIFO buffer " << flush_length;
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
      LOG(LS_INFO) << "\ttalk_base::SE_WRITE  ????????????????????????????????";
      SignalStatesChange(STATES_ICE_TUNNEL_SEND_DATA);
      
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
  LOG(LS_INFO) << "\tRead Data length is " << res;
}

void PeerConnectionIce::WriteData(const char *data, int len){
  LOG(LS_INFO) << "===" << __FUNCTION__;
  size_t res = 0;
  local_tunnel_->WriteAll(data,len,&res,NULL);
  LOG(LS_INFO) << "\tWrite Data length is " << res;
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
