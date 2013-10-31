#ifndef VIRTUAL_APPLICATION__H_
#define VIRTUAL_APPLICATION__H_
#include "talk/base/asyncpacketsocket.h"
#include "mediator_pattern.h"
typedef std::vector<talk_base::AsyncPacketSocket> PacketSockets;
class VirtualApplication : public AbstarctVirtualApplication{
public:
  VirtualApplication(AbstractVirtualNetwork *virtual_network);
public:
  void OnReceiveDateFromLowLayer(int socket, SocketType socket_type,
    char *data, int len);
  virtual bool ListenATcpPort(int port);
private:
  talk_base::Thread   *current_thread_;
  PacketSockets       packet_sockets_;
};





#endif