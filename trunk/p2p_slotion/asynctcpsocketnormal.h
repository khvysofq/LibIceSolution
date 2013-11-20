#ifndef ASYNC_TCP_SOCKET_NORMAL_H_
#define ASYNC_TCP_SOCKET_NORMAL_H_
#include "talk/base/asynctcpsocket.h"

class AsyncTCPSocketNormal : public talk_base::AsyncTCPSocketBase
{
public:
  AsyncTCPSocketNormal(talk_base::AsyncSocket *socket,bool listen);
  virtual ~AsyncTCPSocketNormal(){}
  static AsyncTCPSocketNormal* Create(
    talk_base::AsyncSocket* socket,
    const talk_base::SocketAddress& bind_address,
    const talk_base::SocketAddress& remote_address);

  virtual int Send(const void* pv, size_t cb, 
    talk_base::DiffServCodePoint dscp);

  virtual void ProcessInput(char* data, size_t* len);
  virtual void HandleIncomingConnection(talk_base::AsyncSocket* socket);

 private:
  DISALLOW_EVIL_CONSTRUCTORS(AsyncTCPSocketNormal);
};

#endif