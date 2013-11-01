#include "virtual_network.h"

VirtualNetwork::VirtualNetwork(AbstractICEConnection *p2p_ice_connection):
  AbstractVirtualNetwork(p2p_ice_connection)
{
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  network_header_ = new NetworkHeader();
  network_header_->header_ide_ = P2P_NETWORKER_HEADER_IDE;
  receive_buffer_ = new char[RECEIVE_BUFFER_LEN];
  receive_data_len_ = 0;
  receive_current_len_ = 0;
}
void VirtualNetwork::Destory(){
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  delete network_header_;
  delete receive_buffer_;
  for(size_t i = 0; i < socket_tables_.size(); i++){
    delete socket_tables_[i];
  }
}
VirtualNetwork::~VirtualNetwork(){
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  Destory();
}

void VirtualNetwork::OnReceiveDataFromLowLayer(talk_base::StreamInterface*
                                               stream)
{
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  //size_t res = 0;
  //int error = 0;
  //stream->ReadAll(receive_buffer_,RECEIVE_BUFFER_LEN,&res,&error);
  //LOG(LS_INFO) << "\t receive data \t =" << res;
  //LOG(LS_INFO) << "\t receive error code \t =" << error;
  if(!receive_data_len_){
    receive_data_len_ = ParserSocketHeader(stream);
    LOG(LS_INFO) << "\t data length is \t" << receive_data_len_; 
  }
  else{
    size_t res = 0;
    stream_->Read(&receive_buffer_[receive_current_len_],
      receive_data_len_ - receive_current_len_,&res,NULL);
    receive_current_len_ += res;
    if(receive_current_len_ == receive_data_len_)
    {
      LOG(LS_INFO) << "\t receive net worker header \t" << receive_current_len_;
      //!!!!There is inverse
      int local_socket = get_local_socket(
        ((NetworkHeader *)parser_network_header_)->local_socket_);
      SocketType socket_type =  
        ((NetworkHeader *)parser_network_header_)->socket_type_;
      SignalSendDataToUpLayer(local_socket,socket_type,receive_buffer_,
        receive_data_len_);
      receive_data_len_ = receive_current_len_ = 0;
    }
  }
}
SocketTable *VirtualNetwork::HasNonSocket(){
  
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  for(size_t i = 0; i < socket_tables_.size(); i++){
    if(socket_tables_[i]->remote_socket_ == NON_SOCKET
      || socket_tables_[i]->local_socket_ == NON_SOCKET)
      return socket_tables_[i];
  }
  return NULL;
}
void VirtualNetwork::OnReceiveDataFromUpLayer(int socket,SocketType socket_type,
                                              const char *data,int len)
{
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  SocketTable *socket_table = HasNonSocket();
  if(socket_table){
    socket_table->local_socket_ = socket;
  }

  int remote_socket = get_remote_socket(socket);
  if(remote_socket){
    network_header_->remote_socket_= remote_socket;
  }
  else{
    network_header_->remote_socket_= NON_SOCKET;
    set_socket_table(socket,NON_SOCKET);
  }
  AddSocketHeader(socket,socket_type,len);
  LOG(LS_INFO) << "\t network_header_->data_len_\t=" 
    << network_header_->data_len_;
  LOG(LS_INFO) << "\t network_header_->header_ide\t=" 
    << network_header_->header_ide_;
  LOG(LS_INFO) << "\t network_header_->local_socket_\t=" 
    << network_header_->local_socket_;
  LOG(LS_INFO) << "\t network_header_->remote_socket_\t=" 
    << network_header_->remote_socket_;
  LOG(LS_INFO) << "\t network_header_->socket_type_\t=" 
    << network_header_->socket_type_;
  SignalSendDataToLowLayer((char *)network_header_,
    NETWORKHEADER_LENGTH);
  SignalSendDataToLowLayer(data,len);
}

bool VirtualNetwork::set_socket_table(int local_socket,int remote_socket){
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  //if(socket_table_[local_socket] )
  socket_tables_.push_back(new SocketTable(local_socket,remote_socket,TCP_SOCKET));
  return true;
}
int VirtualNetwork::get_local_socket(int remote_socket){
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  for(size_t i = 0; i < socket_tables_.size(); i++){
    if(socket_tables_[i]->remote_socket_ == remote_socket)
      return socket_tables_[i]->local_socket_;
  }
  return NON_SOCKET;
}
int VirtualNetwork::get_remote_socket(int local_socket){
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  for(size_t i = 0; i < socket_tables_.size(); i++){
    if(socket_tables_[i]->local_socket_ == local_socket)
      return socket_tables_[i]->remote_socket_;
  }
  return NON_SOCKET;
}

void VirtualNetwork::AddSocketHeader(int local_socket,SocketType socket_type,int len){
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  network_header_->local_socket_ = local_socket;
  network_header_->data_len_     = len;
  network_header_->socket_type_  = socket_type;

}
int VirtualNetwork::ParserSocketHeader(talk_base::StreamInterface*
                                       stream)
{
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  size_t res = 0;
  stream->ReadAll(receive_buffer_,NETWORKHEADER_LENGTH,&res,NULL);
  if(res){
    LOG(LS_INFO) << "\t((NetworkHeader *)parser_network_header_)->data_len_\t="
      << ((NetworkHeader *)parser_network_header_)->data_len_;
    LOG(LS_INFO) << "\t((NetworkHeader *)parser_network_header_)->header_ide_\t="
      << ((NetworkHeader *)parser_network_header_)->header_ide_;
    LOG(LS_INFO) << "\t((NetworkHeader *)parser_network_header_)->local_socket_\t="
      << ((NetworkHeader *)parser_network_header_)->local_socket_;
    LOG(LS_INFO) << "\t((NetworkHeader *)parser_network_header_)->remote_socket_\t="
      << ((NetworkHeader *)parser_network_header_)->remote_socket_;
    LOG(LS_INFO) << "\t((NetworkHeader *)parser_network_header_)->socket_type_\t="
      << ((NetworkHeader *)parser_network_header_)->socket_type_;
    return ((NetworkHeader *)parser_network_header_)->data_len_;
  }
  return 0;
}

void VirtualNetwork::OnMessage(talk_base::Message* msg){
    size_t res = 0;
    size_t pos = 0;
    while(pos < receive_data_len_)
    {
      stream_->ReadAll(&receive_buffer_[pos],receive_data_len_ - pos,&res,NULL);
      pos += res;
    }
    if(pos)
    {
      LOG(LS_INFO) << "\t receive net worker header \t" << res;
      //!!!!There is inverse
      int local_socket = get_local_socket(
        ((NetworkHeader *)parser_network_header_)->local_socket_);
      SocketType socket_type =  
        ((NetworkHeader *)parser_network_header_)->socket_type_;
      SignalSendDataToUpLayer(local_socket,socket_type,receive_buffer_,
        receive_data_len_);
      receive_data_len_ = 0;
    }
}