#ifndef PEER_CONNECTION_ICE_H_
#define PEER_CONNECTION_ICE_H_

#include "talk/base/thread.h"
#include "talk/p2p/base/sessionmanager.h"
#include "talk/p2p/client/httpportallocator.h"
#include "talk/session/tunnel/tunnelsessionclient.h"
#include "mediator_pattern.h"

class PeerConnectionIce
  :public AbstractICEConnection,
  public talk_base::MessageHandler
{
public:     //user interface
  PeerConnectionIce(talk_base::Thread *worker_thread,
    talk_base::Thread *signal_thread,
    AbstractP2PServerConnection *p2p_server_connection,
    std::string local_peer_name = "");

public:
  ~PeerConnectionIce();
  void DestroyPeerConnectionIce();
  void ConnectionToRemotePeer(int remote_peer_id, std::string remote_peer_name);

  
  //receive messages that from remote peer by p2p server
  virtual void OnReceiveMessageFromRemotePeer(std::string, int);
  virtual void OnReceiveDataFromUpLayer(char *, int);
private:    //p2p server function and some help function
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
  buzz::Jid                       *local_jid_;
};





#endif