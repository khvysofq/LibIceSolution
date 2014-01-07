#include <iostream>
#include "talk/base/socketaddress.h"
#include "talk/base/logging.h"
#include "talk/base/thread.h"

const talk_base::SocketAddress KLocalAddr("127.0.0.1",9632);

class SocketThreadTest : public sigslot::has_slots<>
{
public:
  SocketThreadTest(talk_base::Thread *signal_thread)
    :signal_thread_(signal_thread)
  {
    socket_ = signal_thread_->socketserver()->CreateAsyncSocket(SOCK_STREAM);
    std::cout << "Bind local port " << KLocalAddr.port() << std::endl;
    if(socket_->Bind(KLocalAddr)){
      LOG(LS_ERROR) << "Can't bind port " << KLocalAddr.port();
      return;
    }
    std::cout << "Listen this local port" << std::endl;;
    socket_->Listen(5);
    socket_->SignalReadEvent.connect(this, &SocketThreadTest::OnAcceptEvent);
  }

private:
  void OnAcceptEvent(talk_base::AsyncSocket *socket){
    //new socket at here.
    ASSERT(socket_ == socket);
    socket->Accept(NULL);
    std::cout << "new socket at here" << std::endl;
  }

  talk_base::Thread       *signal_thread_;
  talk_base::AsyncSocket  *socket_;
};

int main(void){
  talk_base::Thread *signal_thread = talk_base::Thread::Current();
  SocketThreadTest socket_thread_test(signal_thread);

  signal_thread->Run();
  return 0;
}

