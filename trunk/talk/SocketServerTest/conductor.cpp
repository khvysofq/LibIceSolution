#include "conductor.h"

#include <utility>

#include "talk/base/common.h"
#include "talk/base/json.h"
#include "talk/base/logging.h"
#include "talk\p2p\base\p2ptransport.h"
#include "defaults.h"
#include "peer_connection_client.h"

// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

int     thread_cmd_choice   =   NO_CMD_INPUT;
int     is_communication     =   NOT_COMMUNICATION;

const int NOT_INIT          =   0;
const int IS_INIT           =   1;


Conductor::Conductor(talk_base::Thread* signaling_thread,
                     talk_base::Thread* worker_thread)
{
    client_     =   new PeerConnectionClient();
    client_->RegisterObserver(this);
    peer_id_    =   -1;

    signaling_thread_		=	signaling_thread;
    worker_thread_          =   worker_thread;
    is_peer_connect         =   false;
    network_namager_	=	
        new talk_base::BasicNetworkManager();

    allocator_			    =
        new cricket::BasicPortAllocator(network_namager_,
        new talk_base::BasicPacketSocketFactory(worker_thread),
        kStunAddr);
    allocator_->set_flags(cricket::PORTALLOCATOR_ENABLE_BUNDLE);

    p2p_transport_          =   new cricket::P2PTransport(signaling_thread_,
        worker_thread_,"Test_Content_Name",allocator_);

}

Conductor::~Conductor() {
}


bool Conductor::connection_active() const {
    return true;
}


void Conductor::Close() {


    client_->SignOut();
    DeletePeerConnection();
}


bool Conductor::InitializePeerConnection() {

    if(is_peer_connect)
        return false;

    channel_            = p2p_transport_->CreateChannel(1);

    p2p_transport_->SignalCandidatesAllocationDone.connect(this,
        &Conductor::OnSignalCandidatesAllocationDone);
    p2p_transport_->SignalRequestSignaling.connect(this,
        &Conductor::OnSignalRequestSignaling);    
    p2p_transport_->SignalCandidatesReady.connect(this,
        &Conductor::OnSignalCandidatesReady);
    p2p_transport_->SignalRoleConflict.connect(this,
        &Conductor::OnSignalRoleConflict);
    p2p_transport_->SignalReadableState.connect(this,
        &Conductor::OnSignalReadableState);
    p2p_transport_->SignalWritableState.connect(this,
        &Conductor::OnSignalWritableState);
    p2p_transport_->SignalRouteChange.connect(this,
        &Conductor::OnSignalRouteChange);
    p2p_transport_->SignalConnecting.connect(this,
        &Conductor::OnSignalConnecting);

    channel_->SignalReadPacket.connect(
        this, &Conductor::OnReadPacket);
    channel_->SignalReadyToSend.connect(
        this,&Conductor::OnReadyToSend);

    transport_description_  =   
        transport_description_factory_.CreateOffer(transport_options_,NULL);

    p2p_transport_->SetLocalTransportDescription(*transport_description_,
        cricket::ContentAction::CA_OFFER);


    is_peer_connect =   true;
    return true;
}

void Conductor::DeletePeerConnection() {
    peer_id_ = -1;
}

void Conductor::OnSignedIn() {
    LOG(INFO) << __FUNCTION__;
}

void Conductor::OnDisconnected() {
    LOG(INFO) << __FUNCTION__;

    DeletePeerConnection();
}

void Conductor::OnPeerConnected(int id, const std::string& name) {
    LOG(INFO) << __FUNCTION__;
    LOG(INFO) << "New Peer connection from server";
    LOG(INFO) <<"===================================";
    client_->ShowServerConnectionPeer();
    LOG(INFO) <<"===================================";
}

void Conductor::OnPeerDisconnected(int id) {
    LOG(INFO) << __FUNCTION__;
    if (id == peer_id_) {
        LOG(INFO) << "Our peer disconnected";
    } else {
        LOG(INFO) << "a Peer disconnected from server";
        LOG(INFO) <<"===================================";
        client_->ShowServerConnectionPeer();
        LOG(INFO) <<"===================================";
    }
}

void Conductor::OnMessageFromPeer(int peer_id, const std::string& message) {
    ASSERT(peer_id_ == peer_id || peer_id_ == -1);
    ASSERT(!message.empty());
    is_communication    =   IS_COMMUNICATION;
    LOG(LS_INFO)<<"-------------Peer_id:"<<peer_id<<"Say-------------";
    LOG(LS_INFO)<<message;
    LOG(LS_INFO)<<"--------------------------------------------------";

    if(InitializePeerConnection())
    {
        peer_id_    =   peer_id;
        p2p_transport_->SetRole(cricket::TransportRole::ROLE_CONTROLLED);
        p2p_transport_->SetTiebreaker(2222);
        p2p_transport_->ConnectChannels();
    }
    romete_transport_description_ = 
        JsonStringToTransportDescription(message);
    p2p_transport_->SetRemoteTransportDescription(*romete_transport_description_,
        cricket::ContentAction::CA_ANSWER);
    p2p_transport_->OnRemoteCandidates(romete_transport_description_->candidates);
}

void Conductor::OnMessageSent(int err) {
    // Process the next pending message if any.
}

void Conductor::OnServerConnectionFailure() {
    LOG(INFO) << __FUNCTION__;
}


void Conductor::StartLogin(const std::string& server, int port) {
    if (client_->is_connected())
        return;
    //save the server ip address and port
    server_ = server;
    client_->Connect(server, port, GetPeerName());
}

void Conductor::DisconnectFromServer() {
    if (client_->is_connected())
        client_->SignOut();
}


void Conductor::ConnectToPeer(int peer_id) {
    ASSERT(peer_id_ == -1);
    ASSERT(peer_id != -1);

    if(InitializePeerConnection())
    {
        peer_id_    =   peer_id;

        p2p_transport_->SetRole(cricket::TransportRole::ROLE_CONTROLLING);
        p2p_transport_->SetTiebreaker(1111);
        p2p_transport_->ConnectChannels();
    }
    else
    {
        client_->SignOut();
        exit(0);
    }
}

void Conductor::DisconnectFromCurrentPeer() {
    LOG(INFO) << __FUNCTION__;
    //    if (peer_connection_.get()) {
    client_->SendHangUp(peer_id_);
    DeletePeerConnection();
    //    }
}
void Conductor::SendMessageToPeer(const std::string& json_object) {
    std::string* msg = new std::string(json_object);


    LOG(INFO) << "SEND_MESSAGE_TO_PEER";
    if (msg) {
        // For convenience, we always run the message through the queue.
        // This way we can be sure that messages are sent to the server
        // in the same order they were signaled without much hassle.
        pending_messages_.push_back(msg);
    }
    if (!pending_messages_.empty() && !client_->IsSendingMessage()) {
        msg = pending_messages_.front();
        pending_messages_.pop_front();
        //
        if (!client_->SendToPeer(peer_id_, *msg) && peer_id_ != -1) {
            LOG(LS_ERROR) << "SendToPeer failed";
            DisconnectFromServer();
        }
        delete msg;
    }
}


//得到输入消息
void Conductor::OnReadPacket(cricket::TransportChannel* channel, const char* data,
                             size_t len, int flags) 
{
    LOG(INFO) << __FUNCTION__<<"======================================";

    LOG(INFO) << __FUNCTION__<<"---------------- Get DATA ----------------";
    char display_date[256];
    memcpy(display_date,data,len);
    display_date[len]   =   '\0';
    std::cout<<display_date<<std::endl;
    LOG(INFO) << __FUNCTION__<<"----------------          ----------------";
    current_send_channel_   =   channel;
    std::cout<< (int )(current_send_channel_) << std::endl;
}

void Conductor::SendData_w(cricket::TransportChannel* channel) 
{
    LOG(INFO)<<"==============================="<<__FUNCTION__;
    while(true)
    {
        std::cout<< (int )(current_send_channel_) << "\t\t"
            << (int )channel << std::endl;
        std::cin>>send_data_;
        worker_thread_->Post(this,MY_SEND_MESSAGE_,
            new CandidateData(channel,send_data_.c_str(),send_data_.length()));
    }
}

void Conductor::RecvData_W(cricket::TransportChannel* channel, const char* data,
                           size_t len, int flags)
{
    //    LOG(INFO)<<"==============================="<<__FUNCTION__;
}

void Conductor::OnMessage(talk_base::Message* msg) 
{
    LOG(INFO) << __FUNCTION__<<"======================================";
    //CandidateData* params = static_cast<CandidateData*>(msg->pdata);
    switch(msg->message_id)
    {
    case MY_SEND_MESSAGE_:
        {
            CandidateData* params = static_cast<CandidateData*>(msg->pdata);
            params->channel_->SendPacket(params->data_,params->len_);
            delete params;
            break;
        }
    case MY_RECIVE_MESSAGE_:
        //        RecvData_W(params->channel_,params->data_,params->len_,params->flags_);
        break;
    case MY_READY_TO_SEND:
        {
            CandidateData* params = static_cast<CandidateData*>(msg->pdata);
            SendData_w(params->channel_);
            delete params;
            break;
        }
        //delete msg;
        break;
    }
}


void Conductor::OnReadyToSend(cricket::TransportChannel* channel) 
{
    LOG(INFO)<<"///////////////////////////////////////"<<__FUNCTION__;
    signaling_thread_->Post(this,MY_READY_TO_SEND,
        new CandidateData(channel));
}

std::string Conductor::TransportDescriptionToJsonString(cricket::TransportDescription 
                                                        *transport_description)
{
    std::ostringstream ost;
    ost << "["
        << "{"
        << "\"transport_type\":" << "\"" << transport_description->transport_type << "\","
        << "\"ice_ufrag\":" << "\"" << transport_description->ice_ufrag << "\","
        << "\"ice_pwd\":" << "\"" << transport_description->ice_pwd << "\",";
    if(transport_description->ice_mode == cricket::IceMode::ICEMODE_FULL)
        ost << "\"ice_mode\":" << 0;
    else 
        ost << "\"ice_mode\":" << 1;
    ost << "}";
    int length = transport_description->candidates.size();
    if(length != 0)
    {
        ost << ",";
        for(int i = 0; i < length; i++)
        {
            ost << transport_description->candidates[i].ToJsonString();
            if(i != length - 1)
                ost << ",";
        }
    }
    ost<< "]";
    return ost.str();
}

cricket::TransportDescription * Conductor::JsonStringToTransportDescription(
    const std::string &JsonString)
{
    cricket::TransportDescription   *temp_trsnportdescription =
        new cricket::TransportDescription();


    //直接把得到的candidate加入自己的连接当中

    ///////////////////////////////////////////////////////
    Json::Reader reader;
    Json::Value json_object;
    //std::cout<<test_candidate.ToJsonString()<< std::endl;
    if (!reader.parse(JsonString, json_object,false))
        return NULL;

    int length = json_object.size();
    //第一步，将TransportDescription的前面部分得到。
    temp_trsnportdescription->transport_type    
        =   json_object[0]["transport_type"].asString();
    temp_trsnportdescription->ice_ufrag    
        =   json_object[0]["ice_ufrag"].asString();
    temp_trsnportdescription->ice_pwd    
        =   json_object[0]["ice_pwd"].asString();
    int t = json_object[0]["ice_mode"].asInt();
    if(t == 0)
        temp_trsnportdescription->ice_mode = cricket::IceMode::ICEMODE_FULL;
    else
        temp_trsnportdescription->ice_mode = cricket::IceMode::ICEMODE_LITE;
    //第二步：将Candidate加入到temp_trsnportdescription中
    for(int i = 1; i < length; i++)
    {
        std::string id         = json_object[i]["id_"].asString();
        int component          = json_object[i]["component_"].asInt();
        std::string protocol   = json_object[i]["protocol_"].asString();
        talk_base::SocketAddress address;
        address.SetIP(json_object[i]["address_ipaddr"].asString());
        address.SetPort(json_object[i]["address_port"].asInt());

        uint32 priority        = json_object[i]["priority_"].asInt();
        std::string username   = json_object[i]["username_"].asString();
        std::string password   = json_object[i]["password_"].asString();
        std::string type       = json_object[i]["type_"].asString();
        std::string network_name   =   json_object[i]["network_name_"].asString();
        uint32 generation      = json_object[i]["generation_"].asInt();
        std::string foundation = json_object[i]["foundation"].asString();
        talk_base::SocketAddress related_address;
        related_address.SetIP(json_object[i]["related_address_ipaddr"].asString());
        related_address.SetPort(json_object[i]["related_address_port"].asInt());

        cricket::Candidate  test_candidate(id,component,protocol,
            address,priority,username,password,type,network_name,
            generation,foundation);
        temp_trsnportdescription->candidates.push_back(test_candidate);
    }
    return temp_trsnportdescription;
}

//p2p_transport------------------------------------------
//
void Conductor::OnSignalCandidatesAllocationDone(cricket::Transport* channel) 
{
    LOG(INFO)<<"==============================="<<__FUNCTION__;

    transport_description_->candidates = local_candidates_;
    std::cout<<TransportDescriptionToJsonString(transport_description_)<<std::endl;
    SendMessageToPeer(TransportDescriptionToJsonString(transport_description_));
}
//
void Conductor::OnSignalRequestSignaling(cricket::Transport* channel) 
{
    LOG(INFO)<<"==============================="<<__FUNCTION__;
    p2p_transport_->OnSignalingReady();
}
//
void Conductor::OnSignalCandidatesReady(cricket::Transport* ch,
                                        const std::vector<cricket::Candidate> &candidates) 
{
    LOG(INFO)<<"==============================="<<__FUNCTION__;
    for(std::vector<cricket::Candidate>::const_iterator iter =   candidates.begin();
        iter != candidates.end(); iter++)
    {
        local_candidates_.push_back(*iter);
        //        SendMessageToPeer(iter->ToJsonString());
        std::cout<<iter->ToString()<<std::endl;
    }
    LOG(INFO) <<"---------------------------------------------------";
}

//
void Conductor::OnSignalRoleConflict() 
{
    LOG(INFO)<<"==============================="<<__FUNCTION__;
}
//
void Conductor::OnSignalReadableState(cricket::Transport* trasport)
{
    LOG(INFO)<<"/////////////////////////////////"<<__FUNCTION__;
}
//
void Conductor::OnSignalWritableState(cricket::Transport* trasport)
{
    LOG(INFO)<<"==============================="<<__FUNCTION__;
}
//
void Conductor::OnSignalRouteChange(cricket::Transport* transport,
                                    int component,  // component
                                    const cricket::Candidate& candidate)
{
    LOG(INFO)<<"==============================="<<__FUNCTION__;
}
//
void Conductor::OnSignalConnecting(cricket::Transport* trasport)
{
    LOG(INFO)<<"==============================="<<__FUNCTION__;
}

//p2p_transport------------------------------------------