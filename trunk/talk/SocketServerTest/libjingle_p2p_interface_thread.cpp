#include "talk/base/thread.h"
#include "talk/base/messagequeue.h"
#include "talk/base/logging.h"
#include "libjingle_p2p_interface_thread.h"
#include "libjingle_p2p_interface.h"
#include "tunnelclient.h"
#include <map>

#include "defaults.h"

enum P2PThreadMessageType
{
    P2P_CREATE_P2P_OBJECT,
    P2P_DESTROY_P2P_OBJECT,
    P2P_RECV_DATA,
    P2P_SEND_DATA_EVENT,
    P2P_ONLINE_PEERS,
    P2P_SERVER_CONNECTION_FAILURE,
    P2P_SERVER_CONNECTION_SUCCEED,
    //////////////////////////////////
    P2P_START_LOGIN,
    P2P_CONNECT_TO_PEER,
    P2P_DISCONNECTION_P2P_SERVER,
    P2P_DISCONNECTION_CURRENT_PEER,
    P2P_SEND_DATA,
    //////////////////////////////////
    P2P_SET_STUN_SERVER,
    P2P_SET_P2P_SERVER,
    //////////////////////////////////
    P2P_SET_DEBUG_LEVEL
};

struct TempStruct
{
    std::map<int,std::string> peers_map;
};

class P2PThreadMessage : public talk_base::MessageData
{
public:
    P2PThreadMessage(const void *infor = NULL, int more_infor = 0,
        TempStruct *temp_struct = NULL):
    infor_(infor),more_infor_(more_infor),
        temp_struct_(temp_struct)
    {}
    const void    *infor_;
    int     more_infor_;
    TempStruct      *temp_struct_;
};


//void *ThreadP2PTunnelClient::worker_thread_         =   NULL;
//void *ThreadP2PTunnelClient::signal_thread_         =   NULL;
//ThreadP2PTunnelClient *ThreadP2PTunnelClient::this_ =   NULL;

ThreadP2PTunnelClient::ThreadP2PTunnelClient()
    :p2p_tunnel_client_(NULL)
{
    LOG(INFO) << "###" << __FUNCTION__;
    worker_thread_      =   new talk_base::Thread();
    signal_thread_      =   talk_base::Thread::Current();
    this_               =   this;
    ((talk_base::Thread *)worker_thread_)->Start();

    ((talk_base::Thread *)worker_thread_)->Post(this,
        P2P_CREATE_P2P_OBJECT);
}
ThreadP2PTunnelClient::~ThreadP2PTunnelClient()
{
    LOG(INFO) << "###" << __FUNCTION__;

    ((talk_base::Thread *)worker_thread_)->Send(this,
        P2P_DESTROY_P2P_OBJECT);
    //((talk_base::Thread *)worker_thread_)->Stop();
    //delete ((talk_base::Thread *)worker_thread_);
}
///////////////////////////////////////////////////////////////
void ThreadP2PTunnelClient::OnMessage(talk_base::Message *msg)
{
    ASSERT(talk_base::Thread::Current() 
        == (talk_base::Thread *)worker_thread_);
    LOG(INFO) << "###" << __FUNCTION__;
    switch(msg->message_id)
    {
    case P2P_CREATE_P2P_OBJECT:
        {
            LOG(INFO) << "###" << __FUNCTION__ << "\tP2P_CREATE_P2P_OBJECT";
            p2p_tunnel_client_  =   new P2PTunnelClient();
            p2p_tunnel_client_->GetPeerConnectionObject()->
                RegisterParentPointer(this);
            p2p_tunnel_client_->SetCallBackRecvData(OnRecvData_w);
            p2p_tunnel_client_->SetCallBackOnlinePeerFunc(OnOnlinePeers_w);
            p2p_tunnel_client_->SetCallBackStateChangeMesasgeFunc(
                OnStateChangeMessage_w);

            break;
        }
    case P2P_DESTROY_P2P_OBJECT:
        {
            LOG(INFO) << "###" << __FUNCTION__ << "\tP2P_DESTROY_P2P_OBJECT";
            if(p2p_tunnel_client_ != NULL)
            {
                p2p_tunnel_client_->DisConnectionCurrentPeer();
                p2p_tunnel_client_->DisConnctionP2PServer();
                delete p2p_tunnel_client_;
            }
            break;
        }
        //case P2P_RECV_DATA:
        //    {
        //        LOG(INFO) << __FUNCTION__ << "\tP2P_RECV_DATA";
        //        P2PThreadMessage* params = 
        //            static_cast<P2PThreadMessage*>(msg->pdata);
        //        thread_recv_data_func_(this,(char *)params->infor_,params->more_infor_);
        //        delete params;
        //        break;
        //    }
        //case P2P_SEND_DATA_EVENT:
        //    {
        //        LOG(INFO) << __FUNCTION__ << "\tP2P_SEND_DATA_EVENT";
        //        thread_send_data_func_(this);
        //        break;
        //    }
        //case P2P_ONLINE_PEERS:
        //    { 
        //        LOG(INFO) << __FUNCTION__ << "\tP2P_ONLINE_PEERS";
        //        P2PThreadMessage* params = 
        //            static_cast<P2PThreadMessage*>(msg->pdata);
        //        thread_online_peer_func_(this,
        //            params->temp_struct_->peers_map);
        //        delete params->temp_struct_;
        //        delete params;
        //        break;
        //    }
        //case P2P_SERVER_CONNECTION_FAILURE:
        //    {
        //        LOG(INFO) << __FUNCTION__ << "\tP2P_SERVER_CONNECTION_FAILURE";
        //        thread_server_connection_failure_func_(this);
        //        break;
        //    }
        //case P2P_SERVER_CONNECTION_SUCCEED:
        //    {
        //        LOG(INFO) << __FUNCTION__ << "\tP2P_SERVER_CONNECTION_SUCCEED";
        //        thread_server_connection_succeed_func_(this);
        //        break;
        //    }
    case P2P_START_LOGIN:
        {
            LOG(INFO) << "###" << __FUNCTION__ << "\tP2P_START_LOGIN";
            if(p2p_tunnel_client_)
                p2p_tunnel_client_->StartLoginP2PServer();
            break;
        }
    case P2P_CONNECT_TO_PEER:
        {
            LOG(INFO) << "###" << __FUNCTION__ << "\tP2P_CONNECT_TO_PEER";
            P2PThreadMessage* params = 
                static_cast<P2PThreadMessage*>(msg->pdata);
            if(p2p_tunnel_client_)
                p2p_tunnel_client_->ConnectToPeer(params->more_infor_);
            delete params;
            break;
        }
    case P2P_DISCONNECTION_P2P_SERVER:
        {
            LOG(INFO) << "###" << __FUNCTION__ << "\tP2P_DISCONNECTION_P2P_SERVER";
            if(p2p_tunnel_client_)
                p2p_tunnel_client_->DisConnctionP2PServer();
            break;
        }
    case P2P_DISCONNECTION_CURRENT_PEER:
        {
            LOG(INFO) << "###" << __FUNCTION__ << "\tP2P_DISCONNECTION_CURRENT_PEER";
            if(p2p_tunnel_client_)
                p2p_tunnel_client_->DisConnectionCurrentPeer();
            break;
        }
    case P2P_SEND_DATA:
        {
            LOG(INFO) << "###" << __FUNCTION__ << "\tP2P_SEND_DATA";
            P2PThreadMessage* params = 
                static_cast<P2PThreadMessage*>(msg->pdata);
            if(p2p_tunnel_client_)
                p2p_tunnel_client_->SendData((const char *)params->infor_,
                params->more_infor_);
            delete params;
            break;
        }
    case P2P_SET_STUN_SERVER:
        {
            LOG(INFO) << "###" << __FUNCTION__ << "\tP2P_SET_STUN_SERVER";
            P2PThreadMessage* params = 
                static_cast<P2PThreadMessage*>(msg->pdata);
            if(p2p_tunnel_client_)
                p2p_tunnel_client_->SetStunServerAddress((const char *)params->infor_,
                params->more_infor_);
            delete params;
            break;
        }
    case P2P_SET_P2P_SERVER:
        {
            LOG(INFO) << "###" << __FUNCTION__ << "\tP2P_SET_P2P_SERVER";
            P2PThreadMessage* params = 
                static_cast<P2PThreadMessage*>(msg->pdata);
            if(p2p_tunnel_client_)
                p2p_tunnel_client_->SetP2PServerAddress((const char *)params->infor_,
                params->more_infor_);
            delete params;
            break;
        }
    case P2P_SET_DEBUG_LEVEL:
        {
            LOG(INFO) << "###" << __FUNCTION__ << "\tP2P_SET_DEBUG_LEVEL";
            P2PThreadMessage* params = 
                static_cast<P2PThreadMessage*>(msg->pdata);
            if(p2p_tunnel_client_)
                p2p_tunnel_client_->SetDeBugInforLevel(
                (LogInforLevel)params->more_infor_,
                (char *)params->infor_);
            delete params;
            break;
        }
    }
}
///////////////////////////////////////////////////////////////

void ThreadP2PTunnelClient::StartLoginP2PServer()
{
    LOG(INFO) << "###" << __FUNCTION__;
    ((talk_base::Thread *)worker_thread_)->Post(this,P2P_START_LOGIN);
}
void ThreadP2PTunnelClient::ConnectToPeer(int peer_id)
{
    LOG(INFO) << "###" << __FUNCTION__;
    ((talk_base::Thread *)worker_thread_)->Post(this,P2P_CONNECT_TO_PEER,
        new P2PThreadMessage(NULL,peer_id));
}
void ThreadP2PTunnelClient::DisConnctionP2PServer()
{
    LOG(INFO) << "###" << __FUNCTION__;
    ((talk_base::Thread *)worker_thread_)->Post(this,
        P2P_DISCONNECTION_P2P_SERVER);
}
void ThreadP2PTunnelClient::DisConnectionCurrentPeer()
{
    LOG(INFO) << "###" << __FUNCTION__;
    ((talk_base::Thread *)worker_thread_)->Post(this,
        P2P_DISCONNECTION_CURRENT_PEER);
}
void ThreadP2PTunnelClient::SendData(const char *data, int len)
{
    LOG(INFO) << "###" << __FUNCTION__;
    ((talk_base::Thread *)worker_thread_)->Post(this,P2P_SEND_DATA,
        new P2PThreadMessage(data,len));
}

///////////////////////////////////////////////////////////////////////
void ThreadP2PTunnelClient::SetStunServerAddress(const char *stun_server_ip,
                                                 int stun_server_port)
{
    LOG(INFO) << "###" << __FUNCTION__;
    ((talk_base::Thread *)worker_thread_)->Post(this,P2P_SET_STUN_SERVER,
        new P2PThreadMessage(stun_server_ip,stun_server_port));
}
void ThreadP2PTunnelClient::SetP2PServerAddress(const char *p2p_server_ip,
                                                int p2p_server_port)
{
    LOG(INFO) << "###" << __FUNCTION__;
    ((talk_base::Thread *)worker_thread_)->Post(this,P2P_SET_P2P_SERVER,
        new P2PThreadMessage(p2p_server_ip,p2p_server_port));
}

void ThreadP2PTunnelClient::SetDeBugInforLevel(LogInforLevel log_level,
                                               char * log_file_name )
{
    LOG(INFO) << "###" << __FUNCTION__;
    ((talk_base::Thread *)worker_thread_)->Post(this,P2P_SET_DEBUG_LEVEL,
        new P2PThreadMessage(log_file_name,(int)log_level));
}

void ThreadP2PTunnelClient::RunMainThreadMessageProcess(int cms)
{
    LOG(INFO) << "###" << __FUNCTION__;
    ((talk_base::Thread *)signal_thread_)->ProcessMessages(cms);
}
//////////////////////////////////////////////////////////////////////
void ThreadP2PTunnelClient::SetCallBackThreadRecvData(
    ThreadRecvDataFunc recv_data_func)
{
    LOG(INFO) << "###" << __FUNCTION__;
    thread_recv_data_func_      =   recv_data_func;
}

void ThreadP2PTunnelClient::SetCallBackThreadOnlinePeerFunc(
    ThreadOnlinePeerFunc online_peer_func)
{
    LOG(INFO) << "###" << __FUNCTION__;
    thread_online_peer_func_    =   online_peer_func;
}
void ThreadP2PTunnelClient::SetCallBackThreadStateChangeMessageFunc(
    StateChangeMessageFunc state_change_message_func)
{
    LOG(INFO) << "###" << __FUNCTION__;
    state_change_message_func_  =
        state_change_message_func;
}


void ThreadP2PTunnelClient::OnRecvData_w(char *data, int len,
                                         void *thread_p2p_tunnel_client)
{
    LOG(INFO) << __FUNCTION__;
    //((talk_base::Thread *)signal_thread_)->Post(this_,
    //    P2P_RECV_DATA,new P2PThreadMessage(data,len));
    ((ThreadP2PTunnelClient *)thread_p2p_tunnel_client)->
        thread_recv_data_func_(data,len,thread_p2p_tunnel_client);

}
void ThreadP2PTunnelClient::OnOnlinePeers_w(std::map<int, std::string> peers,
                                            void *thread_p2p_tunnel_client)
{
    LOG(INFO) << __FUNCTION__;
    //TempStruct  *temp_struct = new TempStruct;
    //temp_struct->peers_map  =   peers;
    //P2PThreadMessage    *temp_p2p_message_ = 
    //    new P2PThreadMessage(NULL,0,temp_struct);
    //((talk_base::Thread *)signal_thread_)->Post(this_,
    //    P2P_ONLINE_PEERS,temp_p2p_message_);
    //thread_online_peer_func_(this_,peers);
    ((ThreadP2PTunnelClient *)thread_p2p_tunnel_client)->
        thread_online_peer_func_(peers,thread_p2p_tunnel_client);
}

void ThreadP2PTunnelClient::OnStateChangeMessage_w(StateMessageType state_message_type,
                                                   void *thread_p2p_tunnel_client)
{
    LOG(INFO) << __FUNCTION__;
    ((ThreadP2PTunnelClient *)thread_p2p_tunnel_client)->
        state_change_message_func_(state_message_type,thread_p2p_tunnel_client);
}