#ifndef PEER_CONNECTION_ICE_H_
#define PEER_CONNECTION_ICE_H_

#include "talk/base/thread.h"
#include "talk/p2p/base/sessionmanager.h"
#include "talk/p2p/client/httpportallocator.h"
#include "talk/session/tunnel/tunnelsessionclient.h"
#include "mediator_pattern.h"

class PeerConnectionIce
    :public P2PColleague,
    public sigslot::has_slots<>,
    public talk_base::MessageHandler
{
public:     //user interface
    PeerConnectionIce(P2PMediator *p2p_mediator,
        talk_base::Thread *worker_thread,
        talk_base::Thread *signal_thread,
        std::string local_peer_name = "",
        std::string remote_peer_name = "",
        int local_peer_id = 0,
        int remote_peer_id = 0);

    //set function
    void set_local_peer_name(std::string local_peer_name){
        local_peer_name_ = local_peer_name;
    }
    void set_remote_peer_name(std::string remote_peer_name){
        remote_peer_name_ = remote_peer_name;
    }
    void set_local_peer_id(int local_peer_id) {
        local_peer_id_ = local_peer_id;
    }
    void set_remote_peer_id(int remote_peer_id){
        remote_peer_id_ = remote_peer_id;
    }
    std::string get_local_peer_name() const { return local_peer_name_; }
    std::string get_remote_peer_name() const { return local_peer_name_; }
    int get_local_peer_id() const {return local_peer_id_; }
    int get_remote_peer_id() const {return remote_peer_id_; }


public:
    ~PeerConnectionIce();
    void DestroyPeerConnectionIce();
private:    //p2p server function and some help function
    //receive messages that from remote peer by p2p server
    virtual void OnReciveMessage(std::string, int);
    buzz::Jid *GetJid();

private:  //libjingle system function
    // implements the MessageHandler interface
    void OnMessage(talk_base::Message* msg);

private:    //ICE part function
    enum {REMOTE_PEER_MESSAGE};
    //interior message that start college transport information
    void OnRequestSignaling();

    //interior message that should send to remote peer by p2p server
    void OnOutgoingMessage(cricket::SessionManager* manager,
        const buzz::XmlElement* stanza);

    // Accept the tunnel when it arrives and wire up the stream.
    void OnIncomingTunnel(cricket::TunnelSessionClient* client,
        buzz::Jid jid, std::string description,
        cricket::Session* session);
    //data event
    void OnStreamEvent(talk_base::StreamInterface* stream, int events,
        int error);

private:    //ICE part member
    talk_base::Thread               *worker_thread_;
    talk_base::Thread               *signal_thread_;
    cricket::HttpPortAllocator      *http_allocator_;
    talk_base::BasicNetworkManager  *basic_network_manager_;
    cricket::SessionManager         *session_manager_;
    cricket::TunnelSessionClient    *tunnel_session_client_;
    talk_base::StreamInterface      *local_tunnel_;
private:    //p2p server member
    std::string                     local_peer_name_;
    std::string                     remote_peer_name_;
    buzz::Jid                       *local_jid_;
    buzz::Jid                       *remote_jid_;

    int                             local_peer_id_;
    int                             remote_peer_id_;
};





#endif