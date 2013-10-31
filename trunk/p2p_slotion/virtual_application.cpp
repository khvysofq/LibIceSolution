#include "virtual_application.h"
#include "talk/base/thread.h"
#include "talk/base/asynctcpsocket.h"

VirtualApplication::VirtualApplication(AbstractVirtualNetwork *virtual_network)
  :AbstarctVirtualApplication(virtual_network)
{
  current_thread_ = talk_base::Thread::Current();
}

void VirtualApplication::OnReceiveDateFromLowLayer(int socket, 
                                                   SocketType socket_type,
                                                   char *data, int len)
{
  if(NON_SOCKET == socket)
  {
    //The socket is null, it must be a create command.
    
  }
}
bool VirtualApplication::ListenATcpPort(int port){
  talk_base::AsyncSocket  *async_socket 
    = current_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);
  talk_base::AsyncPacketSocket *tcp_socket 
    = new talk_base::AsyncTCPSocket(async_socket,true);
}