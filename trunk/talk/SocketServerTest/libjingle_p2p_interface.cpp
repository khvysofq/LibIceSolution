#include <string>
#include <iostream>
#include <stdio.h>
#include "libjingle_p2p_interface.h"
#include "tunnelclient.h"

P2PTunnelClient::P2PTunnelClient():log_file_stream_(NULL)
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    talk_base::Thread   *current_thread = talk_base::Thread::Current();
    peer_connection_object_      =   new PeerConnectionObject(
        current_thread,current_thread);
    peer_connection_object_->RegisterParentPointer(this);
}

P2PTunnelClient::~P2PTunnelClient()
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    peer_connection_object_->DisconnectFromCurrentPeer();
    LOG(INFO) << "DisconnectFromCurrentPeer";
    peer_connection_object_->DisconnectFromServer();
    LOG(INFO) << "DisconnectFromServer";
    //if(log_file_stream_)
    //{
    //    ((talk_base::FileStream *)log_file_stream_)->Close();
    //    delete ((talk_base::FileStream *)log_file_stream_);
    //    LOG(INFO) << "log_file_stream_";
    //}
    LOG(INFO) << "peer_connection_object_";
    delete peer_connection_object_;
    //if(worker_thread_)
    //{
    //    ((talk_base::Thread *)worker_thread_)->Stop();
    //    delete ((talk_base::Thread *)worker_thread_);
    //}
}

void P2PTunnelClient::SetCallBackRecvData(RecvDataFunc recv_data_func)
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    peer_connection_object_->SignalRecvData  =   recv_data_func;
}

void P2PTunnelClient::SetCallBackOnlinePeerFunc(OnlinePeerFunc online_peer_func)
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    peer_connection_object_->SignalOnlinePeer  =   online_peer_func;
}

void P2PTunnelClient::SetCallBackStateChangeMesasgeFunc(StateChangeMessageFunc
        state_change_message_func)
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    peer_connection_object_->SignalStateChangeMessage
        =   state_change_message_func;
}
/////////////////////////////////////////////////////////////
void P2PTunnelClient::SetStunServerAddress(const char *stun_server_ip,
                                            int stun_server_port)
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    peer_connection_object_->SetStunServerAddress(stun_server_ip,stun_server_port);
}

void P2PTunnelClient::SetRelayServerAddress(const char *relay_server)
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    peer_connection_object_->SetRelayServerAddress(relay_server);
}
void P2PTunnelClient::SetP2PServerAddress(const char *p2p_server_ip,int p2p_server_port)
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    peer_connection_object_->SetP2PServerAddress(p2p_server_ip,p2p_server_port);
}

int P2PTunnelClient::SetDeBugInforLevel(LogInforLevel log_level,
                                          char * log_file_name)
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    int error = 0;
    if(!log_file_name)
        talk_base::LogMessage::LogToDebug((talk_base::LoggingSeverity)log_level);
    else
    {
        log_file_stream_ = new talk_base::FileStream();
        ((talk_base::FileStream *)log_file_stream_)->Open(
            log_file_name,"wb",&error);
        talk_base::LogMessage::LogToStream(
            ((talk_base::FileStream *)log_file_stream_),
            (talk_base::LoggingSeverity)log_level);
    }
    return error;
}
/////////////////////////////////////////////////////////////


void P2PTunnelClient::StartLoginP2PServer()
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    peer_connection_object_->StartLogin();
}


void P2PTunnelClient::ConnectToPeer(int peer_id)
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    peer_connection_object_->ConnectToPeer(peer_id);
}

void P2PTunnelClient::DisConnctionP2PServer()
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    //peer_connection_object_->local_tunnel_.get()->Close();
    peer_connection_object_->DisconnectFromServer();
}
void P2PTunnelClient::DisConnectionCurrentPeer()
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    peer_connection_object_->DisconnectFromCurrentPeer();
}

void P2PTunnelClient::SendData(const char *data, int len)
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    peer_connection_object_->SendData(data,len,NULL);
}

void P2PTunnelClient::RunMessageProcess(int ms)
{
    LOG(LS_INFO) <<"==="<<__FUNCTION__;
    talk_base::Thread::Current()->ProcessMessages(ms);
}