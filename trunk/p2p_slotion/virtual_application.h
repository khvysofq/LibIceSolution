#ifndef VIRTUAL_APPLICATION__H_
#define VIRTUAL_APPLICATION__H_
#include "talk/base/asyncpacketsocket.h"
#include "talk/p2p/base/basicpacketsocketfactory.h"
#include "mediator_pattern.h"
#include "senddatabuffer.h"
#include "asyncrtspproxysocketserver.h"
#include "p2psystemcommand.h"
#include "proxyserverfactory.h"

const int P2PSC_CREATE_CLIENT_CONNECTION_     = 0XFF;
const int P2PSC_CREATE_CLIENT_CONNECTION_OK_  = 0XFF01;

struct ReceiveDataTem{
  int data_length_;
  char *data_;
};


const talk_base::SocketAddress KLocalServerAddr("127.0.0.1",554);
const talk_base::SocketAddress KBindAddr("127.0.0.1",5540);
//////////////////////////////////////////////////////////////////////////

class VirtualApplication : public AbstarctVirtualApplication,
  public talk_base::MessageHandler
{
public:

  VirtualApplication(AbstractVirtualNetwork *virtual_network);
  void Destory();
  ~VirtualApplication();

public:
  virtual bool CreateRTSPServer(const talk_base::SocketAddress 
    &rtsp_server);
public:
  void OnReceiveDateFromLowLayer(uint32 socket, SocketType socket_type,
    const char *data, uint16 len);
  void OnMessage(talk_base::Message *msg);

private:
  P2PSystemCommandFactory       *p2p_system_command_factory_; //static object
  SocketTableManagement         *socket_table_management_;    //static object
  talk_base::Thread             *current_thread_;

  bool                          is_server_;
  RTSPProxyServer               *rtsp_proxy_server_;
  ProxySocketManagement         *proxy_socket_management_;
  AsyncP2PSocket                *p2p_socket_;
};

#endif