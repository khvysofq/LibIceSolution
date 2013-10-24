#include <iostream>
#include "talk/base/messagehandler.h"
#include "talk/base/scoped_ptr.h"
#include "talk/base/stream.h"
#include "talk/base/thread.h"
#include "talk/base/timeutils.h"
#include "talk/p2p/base/sessionmanager.h"
#include "talk/p2p/base/transport.h"
#include "talk/session/tunnel/tunnelsessionclient.h"
#include "talk/p2p/base/basicpacketsocketfactory.h"
#include "talk/p2p/client/basicportallocator.h"

#include "tunnelclient.h"
#include "defaults.h"
#include "talk/base/helpers.h"
//#include "libjingle_p2p_interface.h"


const talk_base::SocketAddress
    kStunAddr("stun.endigovoip.com", STUN_SERVER_PORT);

static const char BASE64_NEW[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '2', '1'
};

enum TunnelMessageType{
    SEND_DATA,
    RECV_DATA,
    DELAYED_SEND,
    ICE_MESSAGE_FROM_PEER,
    ICE_DISSCONNECTION_CURRENT_PEER,
    ICE_IS_NORMAL_DISCONNECT
};

enum SendDataState { 
    SR_SEND_DATA_TO_SERVER,
    SE_OPEN = 1, 
    SE_READ = 2,
    SE_WRITE = 4, 
    SE_CLOSE = 8
};


class MessageSendData : public talk_base::MessageData
{
public:
    MessageSendData(std::string send_data, int send_data_len = 0)
        :send_data_(send_data),send_data_len_(send_data_len){}
    std::string  send_data_;
    int send_data_len_;
};

PeerConnectionObject::PeerConnectionObject(talk_base::Thread *signaling_thread,
                                           talk_base::Thread *worker_thread)
                                           :signaling_thread_(signaling_thread),
                                           worker_thread_(worker_thread),
                                           is_initiator_(false),
                                           is_log_p2p_server_(false),
                                           rometa_peer_name_(NULL)
{
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    //-------------------------Part 1: start--------------------------------//
    // Init HttpPortAllocator
    http_allocator_  = new cricket::HttpPortAllocator(
        &basicNetworkManager, "TunnelClient");
    http_allocator_->SetRelayToken("P2P Solution");
    //-------------------------Part 1: end  --------------------------------//

    //-------------------------Part 2: start--------------------------------//
    // new a SessionManager Object that manager Session and SessionClient
    session_manager_    =   new  cricket::SessionManager(http_allocator_,
        worker_thread_);
    //////////////////////////////////////////////
    Local_Jid_  =   GetLogcal_Jid();
    session_client_     =   new cricket::TunnelSessionClient(
        *Local_Jid_,session_manager_);
    /////////////////////////////////////////////////
    session_client_->SignalIncomingTunnel.connect(this,
        &PeerConnectionObject::OnIncomingTunnel);
    session_manager_->SignalRequestSignaling.connect(this,
        &PeerConnectionObject::OnRequestSignaling);
    session_manager_->SignalOutgoingMessage.connect(this,
        &PeerConnectionObject::OnOutgoingMessage);
    //-------------------------Part 2: end  --------------------------------//

    //-------------------------Part 3: start--------------------------------//
    client_     =   new PeerConnectionClient();
    client_->RegisterObserver(this);
    peer_id_    =   -1;
    //-------------------------Part 3: end  --------------------------------//

    //-------------------------Part 4: start--------------------------------//
    data_buffer_    =   new char[kBlockSize << 4];
    //-------------------------Part 5: end  --------------------------------//
}

PeerConnectionObject::~PeerConnectionObject()
{
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    delete session_manager_;
    LOG(LS_INFO) <<">>>"<<__FUNCTION__ << "\tdelete session_manager_";
    delete client_;
    LOG(LS_INFO) <<">>>"<<__FUNCTION__ << "\tdelete client_";
    delete Local_Jid_;
    LOG(LS_INFO) <<">>>"<<__FUNCTION__ << "\tdelete Local_Jid_";
    delete http_allocator_;
    LOG(LS_INFO) <<">>>"<<__FUNCTION__ << "\thttp_allocator_";
    delete []data_buffer_;
    LOG(LS_INFO) <<">>>"<<__FUNCTION__ << "\tdata_buffer_";
    delete send_stream_;
    LOG(LS_INFO) <<">>>"<<__FUNCTION__ << "\tsend_stream_";
}
buzz::Jid *PeerConnectionObject::GetLogcal_Jid()
{
    std::string LocalJid    =   GetPeerName();
    LocalJid += "@Test.com";
    return new buzz::Jid(LocalJid);
}
void PeerConnectionObject::SetStunServerAddress(const char *stun_server_ip,int stun_server_port)
{
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    talk_base::SocketAddress stun_server;
    stun_server.SetIP(stun_server_ip);
    stun_server.SetPort(stun_server_port);
    stun_hosts_.push_back(stun_server);
    http_allocator_->SetStunHosts(stun_hosts_);
}
void PeerConnectionObject::SetRelayServerAddress(const char *relay_server)
{
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    relay_hosts_.push_back(relay_server);
    http_allocator_->SetRelayHosts(relay_hosts_);
}
void PeerConnectionObject::SetP2PServerAddress(const char *p2p_server_ip,int p2p_server_port)
{
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    p2p_server_.SetIP(p2p_server_ip);
    p2p_server_.SetPort(p2p_server_port);
    client_->set_server_ip(p2p_server_);
}
void PeerConnectionObject::Init()
{
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
}

void PeerConnectionObject::OnRequestSignaling() 
{
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    session_manager_->OnSignalingReady();
}

void PeerConnectionObject::OnOutgoingMessage(cricket::SessionManager* manager,
                                             const buzz::XmlElement* stanza) 
{
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;

    //创建一个新的XmlElement，加入本地的Jid
    buzz::XmlElement* new_stanza    
        =   new buzz::XmlElement(*stanza);
    new_stanza->AddAttr(buzz::QN_FROM, session_client_->jid().Str());
    //这里面需要把数据传送出去，传送到另一个结点中去。
    LOG(LS_INFO) << "-----------------------------------------";
    LOG(LS_INFO) << new_stanza->Str();
    LOG(LS_INFO) << "-----------------------------------------";
    SendMessageToPeer(new_stanza->Str());
    delete new_stanza;
}

void PeerConnectionObject::OnMessageFromPeer(int peer_id, const std::string& message) 
{
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    LOG(INFO) << "****************************************************";
    LOG(INFO) << message;
    LOG(INFO) << "****************************************************";
    ASSERT(peer_id_ == peer_id || peer_id_ == -1);
    ASSERT(!message.empty());
    peer_id_ = peer_id;
    worker_thread_->Post(this,ICE_MESSAGE_FROM_PEER,
        new MessageSendData(message));
}

// Accept the tunnel when it arrives and wire up the stream.
void PeerConnectionObject::OnIncomingTunnel(cricket::TunnelSessionClient* client,
                                            buzz::Jid jid, std::string description,
                                            cricket::Session* session) 
{
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    local_tunnel_.reset(session_client_->AcceptTunnel(session));
    local_tunnel_->SignalEvent.connect(this,
        &PeerConnectionObject::OnStreamEvent);
}


void PeerConnectionObject::OnStreamEvent(talk_base::StreamInterface* stream, int events,
                                         int error) 
{
    LOG(LS_INFO) <<">>>"<<__FUNCTION__ << "\t error = " << error;
    if (events & talk_base::SE_READ) {
        if (stream == local_tunnel_.get()) {
            LOG(LS_INFO) <<">>>"<<__FUNCTION__ <<"\ttalk_base::SE_READ";
            size_t read;
            while ((local_tunnel_->Read(data_buffer_, kBlockSize, &read, NULL)) ==
                talk_base::SR_SUCCESS) {
                    if(read)
                        SignalRecvData(data_buffer_,read,parent_pointer_);
            }
        }
    }
    if (events & talk_base::SE_WRITE) {
        if (stream == local_tunnel_.get()) {
            LOG(LS_INFO) <<">>>"<<__FUNCTION__ << "\ttalk_base::SE_WRITE";
            SignalStateChangeMessage(STATE_ICE_SEND_DATA_EVENT,parent_pointer_);
        }
    }
    if (events & talk_base::SE_CLOSE) {
        if (stream == local_tunnel_.get()) {
            local_tunnel_->Close();
            LOG(LS_INFO) << "$$$$$$$$$$$$$$$$$$\t"<< error;
            if(error != 0)
            {
                signaling_thread_->Post(this,ICE_IS_NORMAL_DISCONNECT);
            }
            else
                SignalStateChangeMessage(STATE_ICE_PEER_DISCONNECTED,
                parent_pointer_);
            LOG(LS_INFO) <<">>>"<<__FUNCTION__ << "\ttalk_base::SE_CLOSE";
        }
    }
}

void PeerConnectionObject::SendData(const char *data,int len,int *err)
{
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    size_t size_write;
    int error = 0;
    talk_base::StreamResult res;
    res = local_tunnel_->WriteAll(data,len,&size_write,&error);
    LOG(LS_INFO) << "|||||||TEST SEND: size_write = " << size_write <<
        "\terror code = " << error << "\tres = " << (int)res;
}

void PeerConnectionObject::OnMessage(talk_base::Message* message) {
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    switch(message->message_id)
    {
    case TunnelMessageType::SEND_DATA:
        {
            LOG(LS_INFO) <<">>>"<<__FUNCTION__ << "\tTunnelMessageType::SEND_DATA";
            MessageSendData* params = static_cast<MessageSendData*>(message->pdata);
            size_t total = 0;
            size_t res = 0;
            char *buffer = new char[params->send_data_len_];
            send_stream_->Read(buffer,params->send_data_len_,NULL,NULL);
            while((int)total < params->send_data_len_)
            {
                local_tunnel_->Write(&buffer[total],params->send_data_len_ - total,
                    &res,NULL);
                total += res;
                LOG(LS_INFO) <<">>>"<<__FUNCTION__ 
                    << "\tTunnelMessageType::SEND_DATA. write date length \t" <<res;
            }
            delete buffer;
            delete params;
            //signaling_thread_->PostDelayed(300,this,TunnelMessageType::DELAYED_SEND);
            break;
        }
    case TunnelMessageType::DELAYED_SEND:
        {
            break;
        }
    case TunnelMessageType::RECV_DATA:
        {
            size_t read;
            while ((local_tunnel_->Read(data_buffer_, kBlockSize, &read, NULL)) ==
                talk_base::SR_SUCCESS) {
                    if(read)
                        SignalRecvData(data_buffer_,read,parent_pointer_);
            }
            break;
        }
    case TunnelMessageType::ICE_MESSAGE_FROM_PEER:
        {
            LOG(LS_INFO) <<">>>"<<__FUNCTION__ 
                << "\tTunnelMessageType::ICE_MESSAGE_FROM_PEER";
            MessageSendData* params = static_cast<MessageSendData*>(message->pdata);
            //第一，先把数据转换成XML格式。
            buzz::XmlElement* data = 
                buzz::XmlElement::ForStr(params->send_data_);
            //判断消息的格式，是一个请求，还是一个响应？
            bool response = data->Attr(buzz::QN_TYPE) == buzz::STR_RESULT;
            if(!response)
            {
                session_manager_->OnIncomingMessage(data);
                LOG(LS_INFO) <<">>>"<<__FUNCTION__ << "\tOnIncomingMessage";
            }
            else
            {                
                session_manager_->OnIncomingResponse(NULL,data);
                LOG(LS_INFO) <<">>>"<<__FUNCTION__ << "\tOnIncomingResponse";
            }
            delete data;
            delete params;
            break;
        }
    case TunnelMessageType::ICE_IS_NORMAL_DISCONNECT:
        {
            if(GetPeerIdForName(*rometa_peer_name_))
            {
                //ConnectToPeer(peer_id_);
                SignalStateChangeMessage(ERROR_ICE_PEER_DISCONNECTED,parent_pointer_);
            }
        }
    }
}

int  PeerConnectionObject::GetPeerIdForName(std::string peer_name)
{
    for(std::map<int, std::string>::const_iterator iter =
        client_->peers().begin();
        iter != client_->peers().end(); ++iter)
    {
        if(iter->second == *rometa_peer_name_)
            peer_id_ = iter->first;
        return peer_id_;
    }
    return 0;
}
void PeerConnectionObject::DeletePeerConnection() {
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    peer_id_ = -1;
}


void PeerConnectionObject::OnSignedIn() {
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
}

void PeerConnectionObject::OnDisconnected() {
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    DeletePeerConnection();
}

void PeerConnectionObject::OnPeerConnected(int id, const std::string& name) {
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    SignalOnlinePeer(client_->peers(),parent_pointer_);
}

void PeerConnectionObject::OnPeerDisconnected(int id) {
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    if (id == peer_id_) {
        LOG(INFO) << "Our connection peer disconnected";
        DisconnectFromCurrentPeer();
    } else {
        SignalOnlinePeer(client_->peers(),parent_pointer_);
    }
}


void PeerConnectionObject::OnMessageSent(int err) {
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    //if(err == WSAETIMEDOUT)
    //{
    //    LOG(LS_WARNING) << ">>> Log p2p server failure : Time out error\
    //                       we will try again after 2000 ms";
    //    signaling_thread_->PostDelayed(2000,client_,0);
    //}
}
void PeerConnectionObject::OnServerConnectionFailure() {
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    LOG(LS_WARNING) << ">>> Log p2p server failure\
                       we will try again after 2000 ms";
    signaling_thread_->PostDelayed(2000,client_,0);
    SignalStateChangeMessage(ERROR_P2P_LOGING_SERVER_FAILURE,parent_pointer_);
}

void PeerConnectionObject::OnServerConnectionSucceed()
{
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    SignalStateChangeMessage(STATE_P2P_LOGING_SERVER_SUCCEED,parent_pointer_);
}


void PeerConnectionObject::StartLogin() {
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    if (client_->is_connected())
        return;
    client_->Connect(p2p_server_.ipaddr().ToString(),
        p2p_server_.port(), Local_Jid_->Str());
}

void PeerConnectionObject::DisconnectFromServer() {
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    if (talk_base::StreamState::SS_CLOSED != local_tunnel_->GetState())
    {
        DisconnectFromCurrentPeer();
    }
    client_->SignOut();
    SignalStateChangeMessage(STATE_P2P_SERVER_DISCONNECTED,parent_pointer_);

}


void PeerConnectionObject::ConnectToPeer(int peer_id) {
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    //ASSERT(peer_id_ == -1);
    //ASSERT(peer_id != -1);

    if(client_->peers().empty())
    {
        LOG(LS_ERROR) << "No peers at p2p server";
        SignalStateChangeMessage(ERROR_P2P_SERVER_NO_THIS_PEER,
            parent_pointer_);
    }
    else
    {
        std::map<int, std::string>::const_iterator iter =
            client_->peers().find(peer_id);
        if(iter == client_->peers().end())
        {
            LOG(LS_INFO) << "No this peer at p2p server";
            SignalStateChangeMessage(ERROR_P2P_SERVER_NO_THIS_PEER,
                parent_pointer_);
        }
        else
        {
            rometa_peer_name_   = new std::string(iter->second);
            is_initiator_ = true;
            peer_id_    =   peer_id;
            Init();    ///Testsfer
            buzz::Jid RemoteJid(*rometa_peer_name_);
            local_tunnel_.reset(session_client_->CreateTunnel(RemoteJid,"Test"));
            local_tunnel_->SignalEvent.connect(this,
                &PeerConnectionObject::OnStreamEvent);
            SignalStateChangeMessage(STATE_P2P_START_PEER_CONNECTION,
                parent_pointer_);
        }
    }
}

void PeerConnectionObject::DisconnectFromCurrentPeer() {
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    if (peer_id_    !=   -1)
    {
        if (talk_base::StreamState::SS_CLOSED != local_tunnel_->GetState())
        {
            local_tunnel_->Close();
            SignalStateChangeMessage(STATE_ICE_PEER_DISCONNECTED,parent_pointer_);
            LOG(INFO) << __FUNCTION__ << " local_tunnel_->Close();";
        }
        else
            SignalStateChangeMessage(ERROR_ICE_PEER_DISCONNECTED,parent_pointer_);
        //client_->SendHangUp(peer_id_);
    }
}
void PeerConnectionObject::SendMessageToPeer(const std::string& json_object) {
    LOG(LS_INFO) <<">>>"<<__FUNCTION__;
    LOG(LS_INFO) << "0~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    LOG(LS_INFO) << json_object;
    LOG(LS_INFO) << "0~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    client_->SendMeesageToPeer(peer_id_,json_object);
}

void PeerConnectionObject::RegisterParentPointer(
    void * parent_pointer)
{
    parent_pointer_ = parent_pointer;
}