#ifndef VIRTUAL_NETWORK_H_
#define VIRTUAL_NETWORK_H_

#include "mediator_pattern.h"
#include "peer_connection_ice.h"
#include "defaults.h"

struct SocketTable{
  SocketTable(){}
  SocketTable(int local_socket,int remote_socket,SocketType socket_type)
    :local_socket_(local_socket),remote_socket_(remote_socket),
    socket_type_(socket_type){}
  int local_socket_;
  int remote_socket_;
  SocketType socket_type_;
};

typedef std::vector<SocketTable*> SocketTables;
const int P2P_NETWORKER_HEADER_IDE =  0X1032FBAE;
const int RECEIVE_BUFFER_LEN       =  1024 * 32;
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
  int socket_type_;
  int data_len_;
};
const int NETWORKHEADER_LENGTH = sizeof(NetworkHeader);

class VirtualNetwork :public AbstractVirtualNetwork,
  public talk_base::MessageHandler
{
public:
  VirtualNetwork(AbstractICEConnection *p2p_ice_connection);
  void Destory();
  ~VirtualNetwork();
public:
  virtual void OnReceiveDataFromLowLayer(talk_base::StreamInterface* );
  virtual void OnReceiveDataFromUpLayer(int,SocketType,const char*,int);
  
  void OnMessage(talk_base::Message* msg);

private:
  enum ReadingStates{
    READING_HEADER_ID,
    READING_REMOTE_SOCKET,
    READING_LOCAL_SOCKET,
    READING_SOCKET_TYPE,
    READING_DATA_LENGTH,
    READING_DATA
  };
  int ReadInt(const char * data,int len);
  void AddSocketHeader(int local_socket, SocketType socket_type,int len);
  int ParserSocketHeader(talk_base::StreamInterface *stream);
  bool set_socket_table(int local_socket,int remote_socket);
  int get_local_socket(int remote_socket);
  int get_remote_socket(int local_socket);
  SocketTable *HasNonSocket();
private:
  NetworkHeader   *network_header_;
  char            parser_network_header_[NETWORKHEADER_LENGTH];
  char            *receive_buffer_;
  SocketTables     socket_tables_;

  //talk_base::ByteBuffer byte_buffer_;

private:
  int                         receive_current_len_;
  ReadingStates               reading_states_;

  char             *receive_up_buffer_;
};
#endif