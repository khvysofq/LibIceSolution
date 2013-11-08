#ifndef VIRTUAL_APPLICATION__H_
#define VIRTUAL_APPLICATION__H_
#include "talk/base/asyncpacketsocket.h"
#include "mediator_pattern.h"

const int P2PSC_CREATE_CLIENT_CONNECTION_     = 0XFF;
const int P2PSC_CREATE_CLIENT_CONNECTION_OK_  = 0XFF01;

struct P2PSystemCommand{
  uint32 command_type_;
  uint32 local_port_;
  uint32 remote_port_;
};

struct ReceiveDataTem{
  int data_length_;
  char *data_;
};


const talk_base::SocketAddress KLocalServerAddr("127.0.0.1",554);
const talk_base::SocketAddress KBindAddr("127.0.0.1",5540);
//typedef std::vector<talk_base::AsyncPacketSocket> PacketSockets;
class VirtualApplication : public AbstarctVirtualApplication,
  public talk_base::MessageHandler
{
public:
  VirtualApplication(AbstractVirtualNetwork *virtual_network);
  void Destory();
  ~VirtualApplication();
public:
  virtual bool ListenATcpPort(int port);
public:
  void OnReceiveDateFromLowLayer(int socket, SocketType socket_type,
    const char *data, int len);
  /////tcp socket call back function
  void OnNewConnection(talk_base::AsyncPacketSocket* socket, 
    talk_base::AsyncPacketSocket* new_socket);
  void OnReadyToSend(talk_base::AsyncPacketSocket *socket);
  void OnReadPacket(talk_base::AsyncPacketSocket* socket, const char* data, 
    size_t len, const talk_base::SocketAddress& addr);
  void OnConnect(talk_base::AsyncPacketSocket* socket);
  void OnMessage(talk_base::Message *msg);
private:
  void CreateSystemCommand(int command_type,int local_port,int remote_prot);
  void CreateClientSocket(talk_base::SocketAddress &server_addr);
private:
  talk_base::Thread            *current_thread_;
  //PacketSockets       packet_sockets_;
  talk_base::AsyncSocket       *socket_;
  talk_base::AsyncPacketSocket *tcp_packet_socket_;
  talk_base::AsyncPacketSocket *new_socket_;
  P2PSystemCommand             *p2p_system_command_;
private:
  bool                          realy_to_send_;
  bool                          is_server_;
  std::deque<ReceiveDataTem>       pending_messages_;
};

#endif