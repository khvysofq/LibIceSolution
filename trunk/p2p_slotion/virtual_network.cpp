#include "virtual_network.h"

VirtualNetwork::VirtualNetwork(AbstractICEConnection *p2p_ice_connection):
  AbstractVirtualNetwork(p2p_ice_connection)
{
  network_header_ = new NetworkHeader();
  network_header_->header_ide_ = P2P_NETWORKER_HEADER_IDE;
  receive_buffer_ = new char[RECEIVE_BUFFER_LEN];
}
void VirtualNetwork::Destory(){
  delete network_header_;
  delete receive_buffer_;
}
VirtualNetwork::~VirtualNetwork(){
  Destory();
}

void VirtualNetwork::OnReceiveDataFromLowLayer(talk_base::StreamInterface*
                                               stream)
{
  int len = ParserSocketHeader(stream);
  size_t res = 0;
  int pos = 0;
  while(pos < len){
    stream->Read(&receive_buffer_[pos],len - pos,&res,NULL);
    pos += res;
  }

  int local_socket = get_local_socket(
    ((NetworkHeader *)parser_network_header_)->remote_socket_);
  SocketType socket_type =  
    ((NetworkHeader *)parser_network_header_)->socket_type_;

  SignalSendDataToUpLayer(local_socket,socket_type,receive_buffer_,len);

}

void VirtualNetwork::OnReceiveDataFromUpLayer(int socket,SocketType socket_type,char *data, 
                                              int len)
{
  AddSocketHeader(socket,socket_type,len);
  SignalSendDataToLowLayer(data,len);
}

bool VirtualNetwork::set_socket_table(int local_socket,int remote_socket){
  //if(socket_table_[local_socket] )
  socket_table_[local_socket] = remote_socket;
  return true;
}
int VirtualNetwork::get_local_socket(int remote_socket){
  for(SocketTable::iterator iter = socket_table_.begin();
    iter != socket_table_.end(); ++iter){
      if(iter->second == remote_socket)
        return iter->first;
  }
  return NON_SOCKET;
}
int VirtualNetwork::get_remote_socket(int local_socket){
  SocketTable::iterator iter = socket_table_.find(local_socket);
  if(iter == socket_table_.end())
    return NON_SOCKET;
  return iter->second;
}

void VirtualNetwork::AddSocketHeader(int local_socket,SocketType socket_type,int len){
  int remote_socket = get_remote_socket(local_socket);
  if(remote_socket){
    network_header_->remote_socket_= remote_socket;
  }
  else{
    network_header_->remote_socket_= NON_SOCKET;
    set_socket_table(local_socket,NON_SOCKET);
  }
  network_header_->local_socket_ = local_socket;
  network_header_->data_len_     = len;
  network_header_->socket_type_  = socket_type;

  SignalSendDataToLowLayer((char *)network_header_,
    NETWORKHEADER_LENGTH);
}
int VirtualNetwork::ParserSocketHeader(talk_base::StreamInterface*
                                       stream)
{
  size_t res = 0;
  size_t pos = 0;
  while(pos < NETWORKHEADER_LENGTH){
    stream->Read(&parser_network_header_[pos],NETWORKHEADER_LENGTH - pos,
      &res,NULL);
    pos += res;
  }
  return ((NetworkHeader *)parser_network_header_)->data_len_;
}
