#ifndef VIRTUAL_NETWORK_H_
#define VIRTUAL_NETWORK_H_

#include "mediator_pattern.h"
#include "peer_connection_ice.h"
#include "defaults.h"

typedef std::map<int,int> SocketTable;
const int P2P_NETWORKER_HEADER_IDE =  0X1032FBAED;
const int RECEIVE_BUFFER_LEN       =  1024 * 8;
struct SOCKETHeader{
  int header_ide_;
  int remote_socket_;
  int local_socket_;
  int data_len_;
};

struct NetworkHeader{
  int header_ide_;
  int remote_socket_;
  int local_socket_;
  unsigned short socket_type_;
  unsigned short data_len_;
};
const int NETWORKHEADER_LENGTH = sizeof(NetworkHeader);

class VirtualNetwork :public AbstractVirtualNetwork{
public:
  VirtualNetwork(AbstractICEConnection *p2p_ice_connection);
  void Destory();
  ~VirtualNetwork();
private:
  virtual void OnReceiveDataFromLowLayer(talk_base::StreamInterface* );
  virtual void OnReceiveDataFromUpLayer(int,SocketType,char*,int);

private:
  void AddSocketHeader(int local_socket, SocketType socket_type,int len);
  int ParserSocketHeader(talk_base::StreamInterface *stream);
  bool set_socket_table(int local_socket,int remote_socket);
  int get_local_socket(int remote_socket);
  int get_remote_socket(int local_socket);
private:
  NetworkHeader   *network_header_;
  char            parser_network_header_[NETWORKHEADER_LENGTH];
  char            *receive_buffer_;
  SocketTable     socket_table_;
};




#endif