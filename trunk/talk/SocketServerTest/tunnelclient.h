#ifndef _TUNNEL_CLIENT_
#define  TUNNEL_CLIENT_

#include <string>
#include "talk/base/messagehandler.h"
#include "talk/base/scoped_ptr.h"
#include "talk/base/stream.h"
#include "talk/base/thread.h"
#include "talk/base/timeutils.h"
#include "talk/p2p/client/basicportallocator.h"
#include "talk/session/tunnel/tunnelsessionclient.h"
#include "talk/p2p/client/httpportallocator.h"
#include "peer_connection_client.h"
#include "defaults.h"


const int kBlockSize    =   4096;
class P2PTunnelClient;
class PeerConnectionObject : public talk_base::MessageHandler,
    public sigslot::has_slots<> ,
    public PeerConnectionClientObserver
{
public:
    PeerConnectionObject(talk_base::Thread *signaling_thread,
        talk_base::Thread *worker_thread);
    ~PeerConnectionObject();
    virtual void StartLogin();
    virtual void DisconnectFromServer();
    virtual void DisconnectFromCurrentPeer();
    virtual void ConnectToPeer(int peer_id);
    void SendData(const char *data,int len,int *err);
    void RegisterParentPointer(void * parent_pointer);
    bool is_initiator() const {return is_initiator_;}

    RecvDataFunc                SignalRecvData;
    OnlinePeerFunc              SignalOnlinePeer;
    StateChangeMessageFunc      SignalStateChangeMessage;
public:
    void Init();
    //void SendData(char * data, int data_len, int *send_data_len);
    //////////////////////////////////////////////////////////////
    void SetStunServerAddress(const char *stun_server_ip,int stun_server_port);
    void SetRelayServerAddress(const char *relay_server);
    void SetP2PServerAddress(const char *p2p_server_ip,int p2p_server_port);
    //////////////////////////////////////////////////////////////
    virtual void OnSignedIn();
    virtual void OnDisconnected();
    virtual void OnPeerConnected(int id, const std::string& name);
    virtual void OnPeerDisconnected(int id);
    virtual void OnMessageFromPeer(int peer_id, const std::string& message);
    virtual void OnMessageSent(int err);
    virtual void OnServerConnectionFailure();
    virtual void OnServerConnectionSucceed();


    void SendMessageToPeer(const std::string& json_object);

private:
    enum { MSG_LSIGNAL, MSG_RSIGNAL };
    // Post a message, to avoid problems with directly connecting the callbacks.
    void OnRequestSignaling();
    void OnOutgoingMessage(cricket::SessionManager* manager,
        const buzz::XmlElement* stanza);
    // Accept the tunnel when it arrives and wire up the stream.
    void OnIncomingTunnel(cricket::TunnelSessionClient* client,
        buzz::Jid jid, std::string description,
        cricket::Session* session);
    //void ReadData();
    // Spool from send_stream into the tunnel. Back up if we get flow controlled.
    //void WriteData(bool* done);
    void OnStreamEvent(talk_base::StreamInterface* stream, int events,
        int error);
    virtual void OnMessage(talk_base::Message* message);
    void DeletePeerConnection();
    buzz::Jid *GetLogcal_Jid();
    int  GetPeerIdForName(std::string peer_name);
private:
    talk_base::Thread               *signaling_thread_;
    talk_base::Thread               *worker_thread_;

    //cricket::BasicPortAllocator     *allocator_;
    cricket::HttpPortAllocator      *http_allocator_;
    talk_base::BasicNetworkManager  basicNetworkManager;
    std::vector<talk_base::SocketAddress> stun_hosts_;
    std::vector<std::string>        relay_hosts_;

    cricket::SessionManager         *session_manager_;
    cricket::TunnelSessionClient    *session_client_;
    talk_base::scoped_ptr<talk_base::StreamInterface> local_tunnel_;
    talk_base::FifoBuffer           *send_stream_;

    bool                            is_initiator_;
    bool                            is_log_p2p_server_;

    buzz::Jid                       *Local_Jid_;
    int                             peer_id_;
    PeerConnectionClient            *client_;
    std::deque<std::string*>        pending_messages_;
    talk_base::SocketAddress        p2p_server_;
    char                            *data_buffer_;
    void                            *parent_pointer_;

    std::string                     *rometa_peer_name_;
};


#endif