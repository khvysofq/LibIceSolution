#ifndef VIRTUAL_NETWORK_H_
#define VIRTUAL_NETWORK_H_

#include "mediator_pattern.h"
#include "peer_connection_ice.h"
const int P2P_NETWORKER_HEADER_IDE = 0X1032FBAED;
struct SOCKETHeader{
  int header_ide_;
  int socket_number_;
  int data_len_;
};

struct NetworkHeader{
  int header_ide_;
  int socket_number_;
  int socket_type_;
  int data_len_;
};

class VirtualNetwork :public AbstractVirtualNetwork{
public:
  VirtualNetwork(AbstractICEConnection *p2p_ice_connection);
  void Destory();
  ~VirtualNetwork();
private:
  virtual void OnReceiveDataFromLowLayer(talk_base::StreamInterface* );
  virtual void OnReceiveDataFromUpLayer(int,char*,int);
private:
  NetworkHeader   *network_header_;
};




#endif