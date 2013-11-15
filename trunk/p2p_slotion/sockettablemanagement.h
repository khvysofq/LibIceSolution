#ifndef SOCKET_TABLE_MANAGEMENT_H_
#define SOCKET_TABLE_MANAGEMENT_H_
#include "talk/base/basictypes.h"
#include "defaults.h"
#include <map>

//SocketTable
struct SocketTable{
  SocketTable(){}
  SocketTable(int local_socket,int remote_socket,SocketType socket_type)
    :local_socket_(local_socket),remote_socket_(remote_socket),
    socket_type_(socket_type){}
  int local_socket_;
  int remote_socket_;
  SocketType socket_type_;
};

class SocketTableManagement
{
public:
  SocketTableManagement();
  static SocketTableManagement *Instance();


  bool   AddNewLocalSocket(uint32 local_socket, uint32 remote_socket,
    SocketType socket_type);
  bool   DeleteASocket(uint32 local_socket);
  void   DeleteAllSockets();
  
  uint32 GetLocalSocket(uint32 remote_socket);
  uint32 GetRemoteSocket(uint32 local_socket);
  //
  typedef std::map<uint32,SocketTable *> SocketTableMap;
private:
  static SocketTableManagement *socket_table_management_;
  SocketTableMap               remote_socket_map_;
  SocketTableMap               local_socket_map_;
};








#endif