#include "asynctcpsocketnormal.h"

#include <iostream>
#include "talk/base/thread.h"
#include "talk/base/sigslot.h"
#include "talk/base/socketaddress.h"
#include "talk/base/asynctcpsocket.h"
#include "talk/p2p/base/basicpacketsocketfactory.h"

talk_base::SocketAddress  G_local_addr("127.0.0.1",5540);
talk_base::SocketAddress  G_server_addr("192.168.1.214",554);
class SocketClient : public sigslot::has_slots<>
{
public:
  SocketClient(talk_base::Thread *main_thread):
    main_thread_(main_thread),
    realy_to_send_(false),
    receive_count_(0)
  {
    basic_packet_socket_factory_ 
      = new talk_base::BasicPacketSocketFactory(main_thread_);
  }
  ~SocketClient()
  {
    //DestoryItself();
  }
  void Initialize(const talk_base::SocketAddress &local_addr,
    const talk_base::SocketAddress &remote_addr)
  {
    talk_base::ProxyInfo proxy_info;
    tcp_socket_packet_ = basic_packet_socket_factory_->CreateClientTcpSocket(local_addr,
      remote_addr,proxy_info,"",0);
    tcp_socket_packet_->SignalReadyToSend.connect(this,
      &SocketClient::OnReadyToSend);
    tcp_socket_packet_->SignalReadPacket.connect(this,
      &SocketClient::OnReadPacket);
    //tcp_socket_packet_  = basic_packet_socket_factory_->CreateServerTcpSocket(local_addr,
    //  554,554,0);
    //tcp_socket_packet_->SignalNewConnection.connect(this,
    //  &SocketClient::OnNewConnection);
  }
  void DestoryItself()
  {
    delete tcp_socket_packet_;
  }


  void OnReadPacket(talk_base::AsyncPacketSocket* socket, const char* data, 
    size_t len, const talk_base::SocketAddress& addr)
  {
    std::cout << "receive count \t" << receive_count_ ++
      << "\t" << len << std::endl;
    //tcp_socket_packet_->Send(data,len,
    //  talk_base::DiffServCodePoint::DSCP_CS0);
  }
  void OnReadyToSend(talk_base::AsyncPacketSocket *socket)
  {
    realy_to_send_ = true;
    //SendData(socket);
  }
  void OnNewConnection(talk_base::AsyncPacketSocket* socket, 
    talk_base::AsyncPacketSocket* new_socket)
  {
    std::cout << "new connection accept ..." << std::endl;
    new_socket->SignalReadPacket.connect(this,
      &SocketClient::OnReadPacket);
    new_socket->SignalReadyToSend.connect(this,
      &SocketClient::OnReadyToSend);
  }
  void SendData(talk_base::AsyncPacketSocket *socket)
  {
  }
private:
  talk_base::Thread               *main_thread_;
  talk_base::AsyncPacketSocket    *tcp_socket_packet_;
  int                             receive_count_;
  bool                            realy_to_send_;

  talk_base::BasicPacketSocketFactory *basic_packet_socket_factory_;

};

int main(void)
{
  talk_base::Thread *main_thread 
    = talk_base::Thread::Current();
  SocketClient  socket_client(main_thread);
  std::cout << "This is a test that non-block listening ..." << std::endl;
  socket_client.Initialize(G_local_addr,G_server_addr);
  main_thread->Run();
  socket_client.DestoryItself();
  return 0;
}