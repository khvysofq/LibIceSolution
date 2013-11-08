#include "virtual_application.h"
#include "asynctcpsocketnormal.h"
#include "talk/base/thread.h"
#include "talk/base/asynctcpsocket.h"
#include "talk/base/bytebuffer.h"
#include "talk/base/dscp.h"

static const int SEND_BUFFER_LENGTH = 64 * 1024;

VirtualApplication::VirtualApplication(AbstractVirtualNetwork *virtual_network)
  :AbstarctVirtualApplication(virtual_network),
  socket_(NULL)
{
  LOG(LS_INFO) << "---" << __FUNCTION__;
  current_thread_ = talk_base::Thread::Current();
  p2p_system_command_ = new P2PSystemCommand();
  realy_to_send_ = false;
  is_server_    = false;
  new_socket_ = NULL;
  receive_momery_buffer_ = new talk_base::FifoBuffer(SEND_BUFFER_LENGTH);
  send_buffer_           = new char[SEND_BUFFER_LENGTH];
}
void VirtualApplication::Destory(){
  LOG(LS_INFO) << "---" << __FUNCTION__;
  delete receive_momery_buffer_;
  delete send_buffer_;
  delete socket_;
  delete tcp_packet_socket_;
  delete p2p_system_command_;
}
VirtualApplication::~VirtualApplication(){
  LOG(LS_INFO) << "---" << __FUNCTION__;
  Destory();
}
void VirtualApplication::OnReceiveDateFromLowLayer(int socket, 
                                                   SocketType socket_type,
                                                   const char *data, int len)
{
  LOG(LS_INFO) << "---" << __FUNCTION__;
  LOG(LS_INFO) << ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;";
  //SignalSendDataToLowLayer(socket,socket_type,data,len);
  //for(int i = 0; i < len; i++)
  //  std::cout << data[i];
  //std::cout << std::endl;
  LOG(LS_INFO) << "\t socket\t" << socket;
  LOG(LS_INFO) << "\t socket_type\t" << socket_type;
  LOG(LS_INFO) << "\t len\t\t" << len;
  LOG(LS_INFO) << "\t socket_\t\t" << socket_;
  if(NON_SOCKET == socket && (socket_ == NULL))
  {
    LOG(LS_INFO) << "\t Parse data\t" ;
    //The socket is null, it must be a create command.
    talk_base::ByteBuffer byte_buffer(data,len);
    P2PSystemCommand p2p_system_command;
    byte_buffer.ReadUInt32(&p2p_system_command.command_type_);
    byte_buffer.ReadUInt32(&p2p_system_command.local_port_);
    byte_buffer.ReadUInt32(&p2p_system_command.remote_port_);
    switch(p2p_system_command.command_type_){
    case P2PSC_CREATE_CLIENT_CONNECTION_:
      {
        LOG(LS_INFO) << "\t P2PSC_CREATE_CLIENT_CONNECTION_\t" ;
        LOG(LS_INFO) << "\t Create client socket";
        talk_base::SocketAddress server_addr("127.0.0.1",
          p2p_system_command.remote_port_);
        CreateClientSocket(server_addr);
        //CreateClientSocket(
        break;
      }
    case P2PSC_CREATE_CLIENT_CONNECTION_OK_:
      {

        LOG(LS_INFO) << "\t P2PSC_CREATE_CLIENT_CONNECTION_OK_\t" ;
        break;
      }
    }
    LOG(LS_INFO) << "\t NOTHING_AT_HERE_it is error\t" ;
  }
  else{
    if(is_server_){
      if(new_socket_){
        SendData(new_socket_,data,len);
        //int res = new_socket_->Send(data,len,
        //  talk_base::DiffServCodePoint::DSCP_CS0);
        //LOG(LS_INFO) << "\t  new_socket_\t" << res;
        //if(res <= 0 || res != len){
        //  LOG(LS_ERROR) << "\t send date to client error" << res;
      }
    } else {
      LOG(LS_INFO) << "\t  send data_________\t" ;
      if(realy_to_send_){
        SendData(tcp_packet_socket_,data,len);
        //int res = tcp_packet_socket_->Send(data,len,
        //  talk_base::DiffServCodePoint::DSCP_CS0);
        //LOG(LS_INFO) << "\t  tcp_packet_socket_\t" <<  res;
        //if(res <= 0 || res != len){
        //  LOG(LS_ERROR) << "\t send date to server error" << res;
        //}
      }
      else{
        char *msg = new char[len];
        memcpy(msg,data,len * sizeof(char));

        ReceiveDataTem receive_tem;
        receive_tem.data_length_ = len;
        receive_tem.data_ = msg;

        if(msg){
          LOG(LS_INFO) << "\t  send pending_messages_" ;
          pending_messages_.push_back(receive_tem);
          talk_base::Thread::Current()->PostDelayed(20,this);
        }
      }
    }
  }
  LOG(LS_INFO) << ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;";
}

void VirtualApplication::SendData(talk_base::AsyncPacketSocket *socket,
                                  const char *data,int len)
{
  size_t count = 0;
  talk_base::StreamResult res;
  //1. write data to FIFO buffer
  int write_fifo_count = 0;
  while(write_fifo_count < len){
    res = receive_momery_buffer_->Write(data + write_fifo_count,
      len - write_fifo_count,&count,NULL);
    if(res != talk_base::SR_SUCCESS){
      LOG(LS_ERROR) << "write data to FIFO buffer error, the write length is " 
        << count;
      break;
    }
    write_fifo_count += len;
  }
  LOG(LS_INFO) << "\t 1. write data to FIFO buffer length " << write_fifo_count;

  //2. get FIFO buffer reading position
  size_t readable_fifo_length = 0;
  size_t send_data_length = 0;
  const void *fifo_buffer 
    = receive_momery_buffer_->GetReadData(&readable_fifo_length);
  LOG(LS_INFO) << "\t 2. get FIFO buffer reading position " << readable_fifo_length;

  //3. send data to remote peer

  send_data_length = socket->Send(fifo_buffer,readable_fifo_length,
    talk_base::DiffServCodePoint::DSCP_CS0);
  if(send_data_length <= 0){
    LOG(LS_ERROR) << "send data to remote peer, the write length is " 
      << readable_fifo_length;
  }
  LOG(LS_INFO) << "\t 3. send data to remote peer " << send_data_length;

  //4. flush data in FIFO buffer
  size_t flush_length;
  res = receive_momery_buffer_->Read(send_buffer_,
    send_data_length,&flush_length,NULL);
  if(res != talk_base::SR_SUCCESS || flush_length < send_data_length){
    LOG(LS_ERROR) << "flush data in FIFO buffer error, the write length is " 
      << flush_length;
  }
  LOG(LS_INFO) << "\t 4. flush data in FIFO buffer " << flush_length;
}

bool VirtualApplication::ListenATcpPort(int port){
  LOG(LS_INFO) << "---" << __FUNCTION__;
  is_server_  = true;
  //talk_base::AsyncSocket  *async_socket 
  //  = current_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);
  //talk_base::AsyncPacketSocket *tcp_socket 
  //  = new talk_base::AsyncTCPSocket(async_socket,true);
  socket_ = current_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);
  if(socket_->Bind(KLocalServerAddr) < 0){
    LOG(LS_ERROR) << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$";
    LOG(LS_ERROR) << "Bind 554 port error............";
    LOG(LS_ERROR) << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$";
  }
  tcp_packet_socket_ = new AsyncTCPSocketNormal(socket_,true);
  tcp_packet_socket_->SignalNewConnection.connect(this,
    &VirtualApplication::OnNewConnection);
  return true;
}

void VirtualApplication::CreateClientSocket(talk_base::SocketAddress &server_addr){

  LOG(LS_INFO) << "---" << __FUNCTION__;
  is_server_  = false;
  socket_ = current_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);
  //socket_->Bind(KLocalServerAddr);
  tcp_packet_socket_ = AsyncTCPSocketNormal::Create(socket_,
    KBindAddr,KLocalServerAddr);
  tcp_packet_socket_->SignalReadPacket.connect(this,
    &VirtualApplication::OnReadPacket);
  tcp_packet_socket_->SignalReadyToSend.connect(this,
    &VirtualApplication::OnReadyToSend);
  tcp_packet_socket_->SignalConnect.connect(this,
    &VirtualApplication::OnConnect);

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

void VirtualApplication::OnConnect(talk_base::AsyncPacketSocket* socket){
  LOG(LS_INFO) << "---" << __FUNCTION__;
  LOG(LS_INFO) << ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;";
  if(socket != tcp_packet_socket_){
    LOG(LS_ERROR) << "the socket is change";
  }
  else 
    LOG(LS_INFO) << "the socket is same";
  LOG(LS_INFO) << ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;";
}
void VirtualApplication::OnNewConnection(
  talk_base::AsyncPacketSocket* socket, 
  talk_base::AsyncPacketSocket* new_socket)
{
  LOG(LS_INFO) << "---" << __FUNCTION__;
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
  new_socket_ = new_socket;
}

void VirtualApplication::OnReadPacket(talk_base::AsyncPacketSocket* socket, 
                                      const char* data, size_t len, 
                                      const talk_base::SocketAddress& addr)
{
  LOG(LS_INFO) << "---" << __FUNCTION__;
  LOG(LS_INFO) << "++++++++++++++++++++++++++++++++++++++++";
  LOG(LS_INFO) << "\t receive data length is " << len;
  SignalSendDataToLowLayer((int)socket,TCP_SOCKET,data,len);
  LOG(LS_INFO) << "++++++++++++++++++++++++++++++++++++++++";
}

void VirtualApplication::OnReadyToSend(talk_base::AsyncPacketSocket *socket){
  LOG(LS_INFO) << "---" << __FUNCTION__;
  realy_to_send_ = true;
}

void VirtualApplication::CreateSystemCommand(int command_type,
                                             int local_port,
                                             int remote_port)
{
  LOG(LS_INFO) << "---" << __FUNCTION__;
  p2p_system_command_->command_type_ = command_type;
  p2p_system_command_->local_port_ = local_port;
  p2p_system_command_->remote_port_ = remote_port;
}

void VirtualApplication::OnMessage(talk_base::Message *msg){
  LOG(LS_INFO) << "---" << __FUNCTION__;
  if(!pending_messages_.empty()){
    LOG(LS_INFO) << "\t pending_message is not empty" ;
    if(realy_to_send_){
      ReceiveDataTem receive_tem = pending_messages_.front();
      pending_messages_.pop_front();
      LOG(LS_INFO) << "\t pending_message is realy to send" ;
      tcp_packet_socket_->Send(receive_tem.data_,receive_tem.data_length_,
        talk_base::DiffServCodePoint::DSCP_CS0);
      delete receive_tem.data_;
    } else {
      talk_base::Thread::Current()->PostDelayed(20,this);
    }
  }
}