#include "asynctcpsocketnormal.h"

#include <iostream>
#include "talk/base/thread.h"
#include "talk/base/sigslot.h"
#include "talk/base/socketaddress.h"
#include "talk/base/asynctcpsocket.h"
#include "talk/p2p/base/basicpacketsocketfactory.h"

static const int    MAX_SEND_BUFFER  = 64 * 1024;
static const int    BASIC_PACKET_SIZE = 32;
static const int    MAX_PACKET_SEND   = 10240;
class SocketClient : public sigslot::has_slots<>
{
public:
  SocketClient(talk_base::Thread *main_thread,
    talk_base::Thread *worker_thread)
    :main_thread_(main_thread),
    worker_thread_(worker_thread),
    realy_to_send_(false),
    receive_count_(0)
  {
    std::cout << __FUNCTION__ << std::endl;
    basic_packet_socket_factory_ 
      = new talk_base::BasicPacketSocketFactory(main_thread_);
    send_buffer_  = new char [MAX_SEND_BUFFER];
    packet_size_  = 1;
  }
  ~SocketClient()
  {
    std::cout << __FUNCTION__ << std::endl;
    //DestoryItself();unittestpppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppppp
  }
  void Initialize(const talk_base::SocketAddress &local_addr,
    const talk_base::SocketAddress &remote_addr)
  {
    std::cout << __FUNCTION__ << std::endl;
    talk_base::ProxyInfo proxy_info;
    tcp_socket_packet_ = basic_packet_socket_factory_->CreateClientTcpSocket(local_addr,
      remote_addr,proxy_info,"",0);
    
    //tcp_socket_packet_->SetOption(talk_base::Socket::OPT_NODELAY,0);
    //int   opt = -1;
    //int res = tcp_socket_packet_->GetOption(talk_base::Socket::OPT_NODELAY,&opt);
    //std::cout << "SOCKET NODELAY IS " << opt 
    //  << "\t" << res << std::endl;

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
    std::cout << __FUNCTION__ << std::endl;
    delete tcp_socket_packet_;
  }


  void OnReadPacket(talk_base::AsyncPacketSocket* socket, const char* data, 
    size_t len, const talk_base::SocketAddress& addr)
  {
    
    //receive_count_++;
    //if(receive_count_ > MAX_PACKET_SEND){
    //  receive_count_ = 0;
    //  packet_size_ ++;
    //  std::cout << "packet size " << packet_size_ * BASIC_PACKET_SIZE << std::endl;
    //}
    //tcp_socket_packet_->SetOption(talk_base::Socket::OPT_NODELAY,1);
    //int res = socket->Send(send_buffer_,60 * BASIC_PACKET_SIZE,
    //  talk_base::DiffServCodePoint::DSCP_DEFAULT);
    //if(res != len)
    //  std::cout << "ERROR send data error !" << std::endl;
  }
  void OnReadyToSend(talk_base::AsyncPacketSocket *socket)
  {
    realy_to_send_ = true;
    std::cout << "packet size " << packet_size_ * BASIC_PACKET_SIZE << std::endl;
    while(true){
    socket->Send(send_buffer_,9096,
      talk_base::DiffServCodePoint::DSCP_DEFAULT);
    }
  }
  void OnNewConnection(talk_base::AsyncPacketSocket* socket, 
    talk_base::AsyncPacketSocket* new_socket)
  {
    std::cout << __FUNCTION__ << std::endl;
    std::cout << "new connection accept ..." << std::endl;
    new_socket->SignalReadPacket.connect(this,
      &SocketClient::OnReadPacket);
    new_socket->SignalReadyToSend.connect(this,
      &SocketClient::OnReadyToSend);
  }
  void SendData(talk_base::AsyncPacketSocket *socket)
  {
    std::cout << __FUNCTION__ << std::endl;
  }
private:
  talk_base::Thread               *main_thread_;
  talk_base::Thread               *worker_thread_;
  talk_base::AsyncPacketSocket    *tcp_socket_packet_;
  int                             receive_count_;
  int                             packet_size_;
  bool                            realy_to_send_;
  char                            *send_buffer_;
  talk_base::BasicPacketSocketFactory *basic_packet_socket_factory_;

};

int main(void)
{

  
  talk_base::SocketAddress  local_addr("127.0.0.1",0);
  talk_base::SocketAddress  server_addr("192.168.1.214",554);

  talk_base::Thread *main_thread 
    = talk_base::Thread::Current();
  talk_base::Thread *worker_thread = new talk_base::Thread();
  worker_thread->Start();
  SocketClient  socket_client(main_thread,worker_thread);
  ////////////////////////////////////////////////////
  char  local_ip[32];
  char  server_ip[32];
  int   server_port;
  std::cout << "I'm a TCP socket client \n you must setup three steps\n";
  std::cout << "First, input the local ip address" << std::endl;
  std::cin >> local_ip;
  local_addr.SetIP(local_ip);
  std::cout << "Second, input the remote server IP" << std::endl;
  std::cin >> server_ip;
  server_addr.SetIP(server_ip);
  std::cout << "Third, input the remote server port" << std::endl;
  std::cin >> server_port;
  server_addr.SetPort(server_port);
  ////////////////////////////////////////////////////
  socket_client.Initialize(local_addr,server_addr);
  main_thread->Run();
  socket_client.DestoryItself();
  worker_thread->Stop();
  return 0;
}