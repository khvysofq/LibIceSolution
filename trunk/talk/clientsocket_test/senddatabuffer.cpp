#include "senddatabuffer.h"

static const size_t DEAFULT_BUFFER_LENGTH = 64 * 1024;
SendDataBuffer::SendDataBuffer(){
  buffer_length_ = DEAFULT_BUFFER_LENGTH;
  fifo_buffer_   = new talk_base::FifoBuffer(DEAFULT_BUFFER_LENGTH);
}

SendDataBuffer::SendDataBuffer(size_t buffer_length)
  :buffer_length_(buffer_length)
{

}