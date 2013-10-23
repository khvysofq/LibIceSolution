#include "talk/base/logging.h"
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

PeerConnectionIce::PeerConnectionIce(P2PMediator *p2p_mediator,
                                     talk_base::Thread *worker_thread,
                                     talk_base::Thread *signal_thread,
                                     std::string local_peer_name,
                                     std::string remote_peer_name,
                                     int local_peer_id,
                                     int remote_peer_id)

                                     :P2PColleague(p2p_mediator),
                                     worker_thread_(worker_thread),
                                     signal_thread_(signal_thread),
                                     tunnel_session_client_(NULL),
                                     local_tunnel_(NULL),
                                     local_peer_name_(local_peer_name),
                                     remote_peer_name_(remote_peer_name),
                                     local_peer_id_(local_peer_id),
                                     remote_peer_id_(remote_peer_id),
                                     local_jid_(NULL),
                                     remote_jid_(NULL)
{
    LOG(LS_INFO) << "===" << __FUNCTION__;
    //-----------------------Part 1: initialize P2PMediator part
    p2p_mediator_->SignalReciveRemoteMessageByServer.connect(this,
        &PeerConnectionIce::OnReciveMessage);

    //-----------------------Part 2: initialize ICE part
    basic_network_manager_  =   new talk_base::BasicNetworkManager();
    http_allocator_         =   new cricket::HttpPortAllocator(
        basic_network_manager_,HTTP_USER_AGENT);

    //initialize session manager
    session_manager_        =   new cricket::SessionManager(http_allocator_,
        worker_thread_);
    session_manager_->SignalRequestSignaling.connect(this,
        &PeerConnectionIce::OnRequestSignaling);
    session_manager_->SignalOutgoingMessage.connect(this,
        &PeerConnectionIce::OnOutgoingMessage);
    
    //initialize tunnel session client
    local_jid_ = GetJid();
    tunnel_session_client_  =   new cricket::TunnelSessionClient(
        *local_jid_,session_manager_);
    tunnel_session_client_->SignalIncomingTunnel.connect(this,
        &PeerConnectionIce::OnIncomingTunnel);
};

void PeerConnectionIce::DestroyPeerConnectionIce(){
    LOG(LS_INFO) << "===" << __FUNCTION__;
    delete basic_network_manager_;
    LOG(LS_INFO) << "\t" << "delete basic_network_manager succeed";
    delete http_allocator_;
    LOG(LS_INFO) << "\t" << "delete http_allocator_ succeed";
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

buzz::Jid *PeerConnectionIce::GetJid(){
    std::string jid_string;
    if(local_peer_name_.empty())
        local_peer_name_ = GetCurrentComputerUserName();
    jid_string = local_peer_name_;
    jid_string += JID_DEFAULT_DOMAIN;
    return new buzz::Jid(jid_string);
}

void PeerConnectionIce::OnReciveMessage(std::string msg, int peer_id){
    LOG(LS_INFO) << "===" << __FUNCTION__;
    LOG(LS_VERBOSE) << "==============================================";
    LOG(LS_VERBOSE) << "\t debug information for out going message";
    LOG(LS_VERBOSE) << msg;
    LOG(LS_VERBOSE) << "==============================================";
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
                                          const buzz::XmlElement* stanza){
    LOG(LS_INFO) << "===" << __FUNCTION__;
    buzz::XmlElement* new_stanza    
        =   new buzz::XmlElement(*stanza);
    new_stanza->AddAttr(buzz::QN_FROM, tunnel_session_client_->jid().Str());
    p2p_mediator_->SignalSendMessageToRemoteByServer(new_stanza->Str(),
        remote_peer_id_);

    LOG(LS_VERBOSE) << "==============================================";
    LOG(LS_VERBOSE) << "\t debug information for out going message";
    LOG(LS_VERBOSE) << new_stanza->Str();
    LOG(LS_VERBOSE) << "==============================================";

    delete new_stanza;

}

void PeerConnectionIce::OnIncomingTunnel(cricket::TunnelSessionClient* client,
                                         buzz::Jid jid, std::string description,
                                         cricket::Session* session){
    LOG(LS_INFO) << "===" << __FUNCTION__;

    local_tunnel_ = client->AcceptTunnel(session);
    local_tunnel_->SignalEvent.connect(this,
        &PeerConnectionIce::OnStreamEvent);
}
void PeerConnectionIce::OnStreamEvent(talk_base::StreamInterface* stream, 
                                      int events,
                                      int error){
    LOG(LS_INFO) << "===" << __FUNCTION__;

    if (events & talk_base::SE_READ) {
        if (stream == local_tunnel_) {
            LOG(LS_INFO) <<"\ttalk_base::SE_READ";
        }
    }

    if (events & talk_base::SE_WRITE) {
        if (stream == local_tunnel_) {
            LOG(LS_INFO) << "\ttalk_base::SE_WRITE";
        }
    }

    if (events & talk_base::SE_CLOSE) {
        if (stream == local_tunnel_) {
            LOG(LS_INFO) << "\ttalk_base::SE_CLOSE";
        }
    }
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
            //第一，先把数据转换成XML格式。
            buzz::XmlElement* data = 
                buzz::XmlElement::ForStr(params->send_data_);
            //判断消息的格式，是一个请求，还是一个响应？
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
