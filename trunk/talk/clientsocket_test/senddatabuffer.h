#ifndef SEND_DATA_BUFFER_H_
#define SEND_DATA_BUFFER_H_
#include "talk/base/stream.h"



class SendDataBuffer 
{
public:
  SendDataBuffer();
  SendDataBuffer(size_t buffer_length);
  ~SendDataBuffer();
  bool SaveData(const char *data, size_t len);
  bool SendDataByStream(talk_base::StreamInterface *stream,int send_tiems);
private:
  size_t                buffer_length_;
  talk_base::FifoBuffer *fifo_buffer_;
  char                  *temp_buffer_;
};






#endif