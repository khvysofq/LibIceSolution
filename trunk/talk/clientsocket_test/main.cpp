#include "asynctcpsocketnormal.h"

#include <iostream>
#include "talk/base/thread.h"
#include "talk/base/sigslot.h"
#include "talk/base/socketaddress.h"
#include "talk/base/asynctcpsocket.h"

talk_base::SocketAddress  G_remote_addr("127.0.0.1",554);
talk_base::SocketAddress  G_server_addr("127.0.0.1",5005);
class SocketClient : public sigslot::has_slots<>
{
public:
  SocketClient(talk_base::Thread *main_thread):
    main_thread_(main_thread),
    remote_addr_(&G_remote_addr),
    local_addr_(&G_server_addr),
    socket_(NULL),
    receive_count_(0)
  {

  }
  ~SocketClient()
  {
    //DestoryItself();
  }
  void Initialize(const talk_base::SocketAddress &remote_addr)
  {
    remote_addr_ = new talk_base::SocketAddress(remote_addr);
    if(local_addr_)
      socket_ = main_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);
    else
      socket_ = main_thread_->socketserver()->CreateAsyncSocket(
      local_addr_->family(),SOCK_STREAM);
    tcp_socket_packet_ = AsyncTCPSocketNormal::Create(socket_,G_server_addr,
      G_remote_addr);
    tcp_socket_packet_->SignalReadPacket.connect(this,
      &SocketClient::OnReadPacket);
    tcp_socket_packet_->SignalReadyToSend.connect(this,
      &SocketClient::OnReadyToSend);
    //tcp_socket_packet_->SignalNewConnection.connect(this,
    //  &SocketClient::OnNewConnection);
  }
  void DestoryItself()
  {
    delete remote_addr_;
    delete local_addr_;
    delete socket_;
    delete tcp_socket_packet_;
  }
  void set_local_addr(const talk_base::SocketAddress &local_addr)
  {
    local_addr_ = new talk_base::SocketAddress(local_addr);
  }
  void OnReadPacket(talk_base::AsyncPacketSocket* socket, const char* data, 
    size_t len, const talk_base::SocketAddress& addr)
  {
    std::cout << "receive count \t" << receive_count_ ++
      << "\t" << len << std::endl;
    tcp_socket_packet_->Send(data,len,
      talk_base::DiffServCodePoint::DSCP_CS0);
  }
  void OnReadyToSend(talk_base::AsyncPacketSocket *socket)
  {
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
    //SendData(new_socket);
  }
  void SendData(talk_base::AsyncPacketSocket *socket)
  {
    std::cout << "please input some message that to send " << std::endl;
    std::cin >> send_string_;
    //socket->Send(send_string_.c_str(),send_string_.length());
    send_string_.clear();
  }
private:
  talk_base::Thread         *main_thread_;
  talk_base::SocketAddress  *remote_addr_;
  talk_base::SocketAddress  *local_addr_;
  talk_base::AsyncSocket    *socket_;
  talk_base::AsyncPacketSocket   *tcp_socket_packet_;
  std::string               send_string_;
  int                       receive_count_;
};

int main(void)
{
  talk_base::Thread *main_thread 
    = talk_base::Thread::Current();

  SocketClient  socket_client(main_thread);
  socket_client.set_local_addr(G_remote_addr);
  std::cout << "This is a test that non-block listening ..." << std::endl;
  socket_client.Initialize(G_server_addr);
  main_thread->Run();
  socket_client.DestoryItself();
  return 0;
}