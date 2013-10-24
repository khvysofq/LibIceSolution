#ifndef LIBJINGLE_P2P_INTERFACE_THREAD_H_
#define LIBJINGLE_P2P_INTERFACE_THREAD_H_
#include "talk/base/messagehandler.h"
#include "defaults.h"
class   P2PTunnelClient;

//struct  talk_base::Message;
class ThreadP2PTunnelClient : public talk_base::MessageHandler
{
public:
    ThreadP2PTunnelClient();
    ~ThreadP2PTunnelClient();
    void OnMessage(talk_base::Message* msg);

    void StartLoginP2PServer();
    void ConnectToPeer(int peer_id);
    void DisConnctionP2PServer();
    void DisConnectionCurrentPeer();
    void SendData(const char *data, int len);
    void RunMainThreadMessageProcess(int cms);

    ////////////////////////////////////////////////////////////////////
    void SetStunServerAddress(const char *stun_server_ip,int stun_server_port);
    void SetP2PServerAddress(const char *p2p_server_ip,int p2p_server_port);
    ////////////////////////////////////////////////////////////////////

    void SetDeBugInforLevel(LogInforLevel log_level = LogInforLevel::LOG_INFO,
        char * log_file_name                        = NULL);
public:
    //////////////////////////////////////////////////////////////////
    //This is call back function set
    // the five function must be call when you used this interface
    void SetCallBackThreadRecvData(ThreadRecvDataFunc recv_data_func);
    void SetCallBackThreadOnlinePeerFunc(ThreadOnlinePeerFunc online_peer_func);
    void SetCallBackThreadStateChangeMessageFunc(StateChangeMessageFunc
        state_change_message_func);

private:
    // thoese functioon are thread conver function
    static void OnRecvData_w(
        char *data, int len,void *thread_p2p_tunnel_client);
    static void OnOnlinePeers_w(std::map<int, std::string> peers,
        void *thread_p2p_tunnel_client);
    static void OnStateChangeMessage_w(StateMessageType state_message_type,
        void *thread_p2p_tunnel_client);

private:
    P2PTunnelClient         *p2p_tunnel_client_;
    void                    *worker_thread_;
    void                    *signal_thread_;
    ThreadP2PTunnelClient   * this_;

    ThreadRecvDataFunc          thread_recv_data_func_;
    ThreadOnlinePeerFunc        thread_online_peer_func_;
    StateChangeMessageFunc      state_change_message_func_; 
};

#endif