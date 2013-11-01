#include "virtual_application.h"
#include "talk/base/thread.h"
#include "talk/base/asynctcpsocket.h"

VirtualApplication::VirtualApplication(AbstractVirtualNetwork *virtual_network)
  :AbstarctVirtualApplication(virtual_network)
{
  current_thread_ = talk_base::Thread::Current();
  p2p_system_command_ = new P2PSystemCommand();
}
void VirtualApplication::Destory(){
  delete socket_;
  delete tcp_packet_socket_;
  delete p2p_system_command_;
}
VirtualApplication::~VirtualApplication(){
  Destory();
}
void VirtualApplication::OnReceiveDateFromLowLayer(int socket, 
                                                   SocketType socket_type,
                                                   const char *data, int len)
{
  if(NON_SOCKET == socket)
  {
    //The socket is null, it must be a create command.
    P2PSystemCommand *p2p_system_command = (P2PSystemCommand *)data;
    switch(p2p_system_command->command_type_){
    case P2PSC_CREATE_CLIENT_CONNECTION_:
      {
        socket_ 
          = current_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);
        talk_base::SocketAddress server_addr("127.0.0.1",
          p2p_system_command->remote_port_);
        CreateClientSocket(server_addr);
        //CreateClientSocket(
        break;
      }
    case P2PSC_CREATE_CLIENT_CONNECTION_OK_:
      {

        break;
      }
    }
  }
  else{
    tcp_packet_socket_->Send(data,len);
  }
}
bool VirtualApplication::ListenATcpPort(int port){
  //talk_base::AsyncSocket  *async_socket 
  //  = current_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);
  //talk_base::AsyncPacketSocket *tcp_socket 
  //  = new talk_base::AsyncTCPSocket(async_socket,true);
  socket_ = current_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);
  socket_->Bind(KLocalServerAddr);
  tcp_packet_socket_ = new talk_base::AsyncTCPSocket(socket_,true);
  tcp_packet_socket_->SignalNewConnection.connect(this,
    &VirtualApplication::OnNewConnection);
  return true;
}

void VirtualApplication::CreateClientSocket(talk_base::SocketAddress &server_addr){

  socket_ = current_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);
  //socket_->Bind(KLocalServerAddr);
  socket_->Connect(server_addr);
  tcp_packet_socket_ = new talk_base::AsyncTCPSocket(socket_,true);

  tcp_packet_socket_->SignalReadPacket.connect(this,
    &VirtualApplication::OnReadPacket);
  tcp_packet_socket_->SignalReadyToSend.connect(this,
    &VirtualApplication::OnReadyToSend);
  CreateSystemCommand(P2PSC_CREATE_CLIENT_CONNECTION_OK_,server_addr.port(),
    server_addr.port());
  SignalSendDataToLowLayer((int)tcp_packet_socket_,TCP_SOCKET,(char *)p2p_system_command_,
    sizeof(P2PSystemCommand));

}

void VirtualApplication::OnNewConnection(
  talk_base::AsyncPacketSocket* socket, 
  talk_base::AsyncPacketSocket* new_socket)
{
  //send create a client socket to remote peer's
  CreateSystemCommand(P2PSC_CREATE_CLIENT_CONNECTION_,
    KLocalServerAddr.port(),KLocalServerAddr.port());
  SignalSendDataToLowLayer((int)new_socket,TCP_SOCKET,(char *)p2p_system_command_,
    sizeof(P2PSystemCommand));
  
  new_socket->SignalReadPacket.connect(this,
    &VirtualApplication::OnReadPacket);
  new_socket->SignalReadyToSend.connect(this,
    &VirtualApplication::OnReadyToSend);
  
}

void VirtualApplication::OnReadPacket(talk_base::AsyncPacketSocket* socket, 
                                      const char* data, size_t len, 
                                      const talk_base::SocketAddress& addr)
{
  SignalSendDataToLowLayer((int)socket,TCP_SOCKET,data,len);
}

void VirtualApplication::OnReadyToSend(talk_base::AsyncPacketSocket *socket){

}

void VirtualApplication::CreateSystemCommand(int command_type,
                                             int local_port,
                                             int remote_port)
{
  p2p_system_command_->command_type_ = command_type;
  p2p_system_command_->local_port_ = local_port;
  p2p_system_command_->remote_port_ = remote_port;
}