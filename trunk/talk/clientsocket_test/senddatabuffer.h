#ifndef SEND_DATA_BUFFER_H_
#define SEND_DATA_BUFFER_H_
#include "talk/base/stream.h"



class SendDataBuffer 
{
public:
  SendDataBuffer()
  SendDataBuffer(size_t buffer_length);
private:
  size_t buffer_length_;
  talk_base::FifoBuffer *fifo_buffer_;
};






#endif