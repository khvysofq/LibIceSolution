#include <iostream>
#include "talk/base/socketaddress.h"
#include "talk/base/logging.h"


const uint32 BASIC_MESSAGE      = 1 << 0;
const uint32 IMPORTANT_MESSAGE  = 1 << 1;
const uint32 TEST_MESSAGE       = 1 << 2;

const uint32 CURRENT_MESSAGE    = IMPORTANT_MESSAGE;

#define LOG_MY(X) (!((X)&CURRENT_MESSAGE))?(void)0:LOG_F(LS_INFO)

int main(void){

  LOG_MY(IMPORTANT_MESSAGE) << "Hello world 1";

  LOG_MY(BASIC_MESSAGE) << "Hello world 2";

  LOG_MY(TEST_MESSAGE) << "Hello world 3";

  LOG_MY(TEST_MESSAGE | BASIC_MESSAGE) << "Hello world 4";

  LOG_MY(TEST_MESSAGE | IMPORTANT_MESSAGE) << "Hello world 5";
  return 0;
}

