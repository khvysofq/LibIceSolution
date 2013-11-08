#include "asynctcpsocketnormal.h"
static const size_t kMaxPacketSize = 64 * 1024;

typedef uint16 PacketLength;
static const size_t kPacketLenSize = sizeof(PacketLength);

static const size_t kBufSize = kMaxPacketSize + kPacketLenSize;

static const int kListenBacklog = 5;

AsyncTCPSocketNormal::AsyncTCPSocketNormal(
  talk_base::AsyncSocket *socket,bool listen)
  :talk_base::AsyncTCPSocketBase(socket,listen,kBufSize){
}

AsyncTCPSocketNormal* AsyncTCPSocketNormal::Create(
  talk_base::AsyncSocket* socket,
  const talk_base::SocketAddress& bind_address,
  const talk_base::SocketAddress& remote_address){

    return new AsyncTCPSocketNormal(
      talk_base::AsyncTCPSocketBase::ConnectSocket(
      socket, bind_address, remote_address), false);
}

int AsyncTCPSocketNormal::Send(const void* pv, size_t cb, 
                               talk_base::DiffServCodePoint dscp)
{
  if (cb > kBufSize) {
    SetError(EMSGSIZE);
    return -1;
  }

  // If we are blocking on send, then silently drop this packet
  if (!IsOutBufferEmpty())
    return static_cast<int>(cb);

  //PacketLength pkt_len = HostToNetwork16(static_cast<PacketLength>(cb));
  //AppendToOutBuffer(&pkt_len, kPacketLenSize);
  AppendToOutBuffer(pv, cb);

  int res = FlushOutBuffer();
  if (res <= 0) {
    // drop packet if we made no progress
    ClearOutBuffer();
    return res;
  }

  // We claim to have sent the whole thing, even if we only sent partial
  return static_cast<int>(cb);
}
void AsyncTCPSocketNormal::ProcessInput(char* data, size_t* len){

  talk_base::SocketAddress remote_addr(GetRemoteAddress());
  SignalReadPacket(this,data, *len, remote_addr);
  *len = 0;
}

void AsyncTCPSocketNormal::HandleIncomingConnection(talk_base::AsyncSocket* socket) {
  SignalNewConnection(this, new AsyncTCPSocketNormal(socket, false));
}