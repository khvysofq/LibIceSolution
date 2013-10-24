#ifndef  LIBJIGNLE_P2P_INTERFACE_H_
#define  LIBJIGNLE_P2P_INTERFACE_H_



#include "defaults.h"
#include <map>
class PeerConnectionObject;


class P2PTunnelClient
{
public:
    P2PTunnelClient();
    void StartLoginP2PServer();
    void ConnectToPeer(int peer_id);
    void DisConnctionP2PServer();
    void DisConnectionCurrentPeer();
    void SendData(const char *data, int len);
    //when you used the RunMessageProcess function
    //the FOREVER(-1) means naver back.
    void RunMessageProcess(int ms);
    PeerConnectionObject    *GetPeerConnectionObject() const
    { 
        return peer_connection_object_;
    }

    // this function return the error information when you set the log to file.
    // return 0 is none error.
    int SetDeBugInforLevel(LogInforLevel log_level = LogInforLevel::LOG_INFO,
        char * log_file_name = NULL);
public:
    ~P2PTunnelClient();
    ////////////////////////////////////////////////////////////////////
    void SetStunServerAddress(const char *stun_server_ip,int stun_server_port);
    void SetRelayServerAddress(const char *relay_server);
    void SetP2PServerAddress(const char *p2p_server_ip,int p2p_server_port);
    ////////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////
    //This is call back function set
    // the five function must be call when you used this interface
    void SetCallBackRecvData(RecvDataFunc recv_data_func);
    void SetCallBackOnlinePeerFunc(OnlinePeerFunc online_peer_func);
    void SetCallBackStateChangeMesasgeFunc(StateChangeMessageFunc
        state_change_message_func);
    //////////////////////////////////////////////////////////////////

private:
    PeerConnectionObject            *peer_connection_object_;
    void                            *log_file_stream_;
};

#endif // DEBUG