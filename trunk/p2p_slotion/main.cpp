//#include <Windows.h>
#include <iostream>
#include "talk/base/logging.h"
#include "talk/base/stream.h"
#include "p2p_user_client.h"

#include "defaults.h"

int choose = 0;

class Worker :public talk_base::MessageHandler{
public:
  void OnMessage(talk_base::Message* msg){
    std::cout << "please choose a peer id" << std::endl;
    std::cin >> choose;
    //talk_base::Thread::Current()->Stop();
  }
};

int main(void)
{
//#ifdef _DEBUG
//    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
//    _CrtSetBreakAlloc(816);
  //#endif 
  //talk_base::FileStream *log_file_stream_ = new talk_base::FileStream();
  //((talk_base::FileStream *)log_file_stream_)->Open(
  //  "C:/log.txt","wb",NULL);
  //talk_base::LogMessage::LogToStream(log_file_stream_,
  //  talk_base::LoggingSeverity::LS_INFO);
  //talk_base::LogMessage::LogToDebug(talk_base::LoggingSeverity::LS_ERROR);

  log_filter = P2P_ICE_DATA_INFOR | P2P_TUNNEL_DATA_INFOR;

  talk_base::Thread *main_thread 
    = talk_base::Thread::Current();
  talk_base::Thread *command_thread = new talk_base::Thread();
  command_thread->Start();
  Worker    worker;
  command_thread->Post(&worker);
  

  //talk_base::Thread *worker_thread = new talk_base::Thread();
  //worker_thread->Start();

  P2PUserClient p2p_user_client(main_thread,main_thread);
  p2p_user_client.Initiatlor();
  p2p_user_client.StartRun();

  while(true){
    if(false == p2p_user_client.is_peer_connect_){
      if(choose){
        p2p_user_client.ConnectionToPeer(choose);
        break;
      }
    } else{
      break;
    }

    main_thread->ProcessMessages(1000);
    
  }

  main_thread->Run();

  p2p_user_client.Destory();
  return 0;
}
