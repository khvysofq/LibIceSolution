#ifndef P2P_SOLUTION_P2P_USER_CLIENT_H_
#define P2P_SOLUTION_P2P_USER_CLIENT_H_

#include "talk/base/sigslot.h"
#include "talk/base/messagehandler.h"
#include "talk/base/thread.h"
#include "talk/base/stream.h"

#include "defaults.h"
#include "mediator_pattern.h"

const talk_base::SocketAddress ServerAddr("192.168.1.116",8888);
const int RECEIVE_BUFFER_LENGTH   = 1024 * 8;

class P2PUserClient :public sigslot::has_slots<>,
  public talk_base::MessageHandler
{
public:
  P2PUserClient(talk_base::Thread *worker_thread,
    talk_base::Thread *signal_thread);
  ~P2PUserClient();

  void Initiatlor();
  void StartRun();
  void Destory();

  void ConnectionToPeer(int peer_id);
  //p2p ICE part
  void OnStatesChange(StatesChangeType states_type);
  void OnReceiveDataFromLoweLayer(talk_base::StreamInterface* stream);
  sigslot::signal2<const char *, int> SignalSendDataToLowLayer;

  //p2p server part
  void OnOnlinePeers(const Peers peers);
  
  // implements the MessageHandler interface
  void OnMessage(talk_base::Message* msg);

  // implements the Virtual network receive data interface
  void OnReceiveDataFromVirtualNetwokr(int socket,SocketType socket_type,
    const char *data, int len);
private:
  void SendRandomData();
private:
  AbstractVirtualNetwork      *p2p_virtual_network_;
  AbstractICEConnection       *p2p_ICE_connection_;
  AbstractP2PServerConnection *p2p_server_connection_;
  AbstarctVirtualApplication  *p2p_virtual_application_;

  talk_base::Thread           *worker_thread_;
  talk_base::Thread           *signal_thread_;
  bool                        initiator_;
  char                        *receive_buffer_;
};

#endif