#include "virtual_application.h"
#include "talk/base/thread.h"
#include "talk/base/asynctcpsocket.h"
#include "talk/base/bytebuffer.h"

VirtualApplication::VirtualApplication(AbstractVirtualNetwork *virtual_network)
  :AbstarctVirtualApplication(virtual_network),
  socket_(NULL)
{
  current_thread_ = talk_base::Thread::Current();
  p2p_system_command_ = new P2PSystemCommand();
  realy_to_send_ = false;
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
  //for(int i = 0; i < len; i++)
  //  std::cout << data[i];
  //std::cout << std::endl;
  LOG(LS_INFO) << "\t socket\t" << socket;
  LOG(LS_INFO) << "\t socket_type\t" << socket_type;
  LOG(LS_INFO) << "\t len\t\t" << len;
  if(NON_SOCKET == socket && (socket_ == NULL))
  {
    //The socket is null, it must be a create command.
    talk_base::ByteBuffer byte_buffer(data,len);
    P2PSystemCommand p2p_system_command;
    byte_buffer.ReadUInt32(&p2p_system_command.command_type_);
    byte_buffer.ReadUInt32(&p2p_system_command.local_port_);
    byte_buffer.ReadUInt32(&p2p_system_command.remote_port_);
    switch(p2p_system_command.command_type_){
    case P2PSC_CREATE_CLIENT_CONNECTION_:
      {
        LOG(LS_INFO) << "\t Create client socket";
        talk_base::SocketAddress server_addr("127.0.0.1",
          p2p_system_command.remote_port_);
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
    if(realy_to_send_)
      tcp_packet_socket_->Send(data,len);
    else{

    }
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
  tcp_packet_socket_ = talk_base::AsyncTCPSocket::Create(socket_,
    KBindAddr,KLocalServerAddr);
  tcp_packet_socket_->SignalReadPacket.connect(this,
    &VirtualApplication::OnReadPacket);
  tcp_packet_socket_->SignalReadyToSend.connect(this,
    &VirtualApplication::OnReadyToSend);
  //CreateSystemCommand(P2PSC_CREATE_CLIENT_CONNECTION_OK_,server_addr.port(),
  //  server_addr.port());

  ////reading data to network byte order
  //talk_base::ByteBuffer byte_buffer;
  //byte_buffer.WriteUInt32(p2p_system_command_->command_type_);
  //byte_buffer.WriteUInt32(p2p_system_command_->local_port_);
  //byte_buffer.WriteUInt32(p2p_system_command_->remote_port_);
  ////send create command to remote peer.
  //SignalSendDataToLowLayer((int)tcp_packet_socket_,TCP_SOCKET,
  //  byte_buffer.Data(),byte_buffer.Length());

}

void VirtualApplication::OnNewConnection(
  talk_base::AsyncPacketSocket* socket, 
  talk_base::AsyncPacketSocket* new_socket)
{
  LOG(LS_INFO) << "MMM" << __FUNCTION__;
  LOG(LS_INFO)<< "\tnew connection";
  //send create a client socket to remote peer's
  CreateSystemCommand(P2PSC_CREATE_CLIENT_CONNECTION_,
    KLocalServerAddr.port(),KLocalServerAddr.port());

  //reading data to network byte order
  talk_base::ByteBuffer byte_buffer;
  byte_buffer.WriteUInt32(p2p_system_command_->command_type_);
  byte_buffer.WriteUInt32(p2p_system_command_->local_port_);
  byte_buffer.WriteUInt32(p2p_system_command_->remote_port_);
  SignalSendDataToLowLayer((int)new_socket,TCP_SOCKET,
    byte_buffer.Data(),byte_buffer.Length());
  
  new_socket->SignalReadPacket.connect(this,
    &VirtualApplication::OnReadPacket);
  new_socket->SignalReadyToSend.connect(this,
    &VirtualApplication::OnReadyToSend);
}

void VirtualApplication::OnReadPacket(talk_base::AsyncPacketSocket* socket, 
                                      const char* data, size_t len, 
                                      const talk_base::SocketAddress& addr)
{
  LOG(LS_INFO) << "MMM" << __FUNCTION__;
  //for(int i = 0; i < len; i ++)
  //  std::cout << data[i];
  //std::cout << std::endl;
  SignalSendDataToLowLayer((int)socket,TCP_SOCKET,data,len);
}

void VirtualApplication::OnReadyToSend(talk_base::AsyncPacketSocket *socket){
  LOG(LS_INFO) << "MMM" << __FUNCTION__;
  realy_to_send_ = true;
}

void VirtualApplication::CreateSystemCommand(int command_type,
                                             int local_port,
                                             int remote_port)
{
  p2p_system_command_->command_type_ = command_type;
  p2p_system_command_->local_port_ = local_port;
  p2p_system_command_->remote_port_ = remote_port;
}