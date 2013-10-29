//#include <Windows.h>
#include <crtdbg.h>
#include <iostream>
#include "p2p_user_client.h"

int main(void)
{
#ifdef _DEBUG
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    _CrtSetBreakAlloc(816);
#endif 
  talk_base::Thread *main_thread 
    = talk_base::Thread::Current();

  P2PUserClient p2p_user_client(main_thread,main_thread);
  p2p_user_client.Initiatlor();
  p2p_user_client.StartRun();

  main_thread->ProcessMessages(10000);
  
  p2p_user_client.Destory();
  return 0;
}