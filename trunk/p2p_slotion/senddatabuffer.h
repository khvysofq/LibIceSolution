#ifndef SEND_DATA_BUFFER_H_
#define SEND_DATA_BUFFER_H_
#include "talk/base/stream.h"
#include "talk/base/asyncpacketsocket.h"



class SendDataBuffer 
{
public:
  SendDataBuffer();
  SendDataBuffer(size_t buffer_length);
  ~SendDataBuffer();
  bool SaveData(const char *data, size_t len);
  size_t SendDataByStream(talk_base::StreamInterface *stream,int send_tiems,
    bool *block);
  bool SendDataBySocket(talk_base::AsyncPacketSocket *socket, int send_times);
  size_t GetBufferRemainLength();
  bool SendDataUsedStream(talk_base::StreamInterface *stream);
  void SetNormalState();
  void SetBlockState();
  bool IsBlockState();
private:
  enum {NORMAL_STATE,BLOCK_STATE} state_;
  size_t                buffer_length_;
  talk_base::FifoBuffer *fifo_buffer_;
  char                  *temp_buffer_;
};

#endif