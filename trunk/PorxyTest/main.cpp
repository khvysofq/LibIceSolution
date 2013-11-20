#include "talk/base/thread.h"
#include "talk/base/proxyserver.h"

int main(void){
  //1. Create some basic component that to create other parts
  talk_base::Thread *main_thread = talk_base::Thread::Current();
  talk_base::SocketAddress int_addr("127.0.0.1",554);
  talk_base::SocketAddress ext_ip("192.168.1.225",554);

  //2. Create socket factory;
  talk_base::SocketFactory *int_factory = main_thread->socketserver();
  talk_base::SocketFactory *ext_factory = main_thread->socketserver();

  talk_base::SocksProxyServer proxy_server(int_factory,int_addr,ext_factory,ext_ip);

  main_thread->Run();
  return 0;
}