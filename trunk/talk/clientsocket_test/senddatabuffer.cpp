#include "senddatabuffer.h"


static const size_t DEAFULT_BUFFER_LENGTH = 64 * 1024;
SendDataBuffer::SendDataBuffer(){
  buffer_length_ = DEAFULT_BUFFER_LENGTH;
  fifo_buffer_   = new talk_base::FifoBuffer(DEAFULT_BUFFER_LENGTH);
  temp_buffer_   =  new char[buffer_length_];
}

SendDataBuffer::SendDataBuffer(size_t buffer_length)
  :buffer_length_(buffer_length)
{
  fifo_buffer_  = new talk_base::FifoBuffer(buffer_length_);
  temp_buffer_   =  new char[buffer_length_];
}

SendDataBuffer::~SendDataBuffer(){
  delete temp_buffer_;
  delete fifo_buffer_;
}

bool SendDataBuffer::SaveData(const char *data, size_t len){
  size_t save_count = 0;
  talk_base::StreamResult result =
    fifo_buffer_->WriteAll(data,len,&save_count,NULL);
  if(result != talk_base::SR_SUCCESS){
    LOG(LS_ERROR) << "write data to FIFO buffer error, the write length is " 
      << save_count;
    return false;
  }
}

bool SendDataBuffer::SendDataByStream(talk_base::StreamInterface *stream,
                                      int send_tiems)
{
  bool quit = false;
  for(int i = 0; i < send_tiems && !quit; ++i){
    //2. get FIFO buffer reading position
    size_t readable_fifo_length = 0;
    size_t send_data_length = 0;
    talk_base::StreamResult result;

    const void *buffer_point 
      = fifo_buffer_->GetReadData(&readable_fifo_length);
    LOG(LS_INFO) << "\t 2. get FIFO buffer reading position " << readable_fifo_length;

    //3. send data to remote peer
    result =  stream->WriteAll(buffer_point,readable_fifo_length,
      &send_data_length,NULL);
    if(result != talk_base::SR_SUCCESS){
      LOG(LS_ERROR) << "send data to remote peer, the send length is " 
        << send_data_length;// << "\t" << StreamResultToString(res);
    } else{
      quit = true;
    }
    LOG(LS_INFO) << "\t 3. send data to remote peer " << send_data_length;

    //4. flush data in FIFO buffer
    size_t flush_length;
    result = fifo_buffer_->Read(temp_buffer_,
      buffer_length_,&flush_length,NULL);
    if(result != talk_base::SR_SUCCESS || flush_length < send_data_length){
      LOG(LS_ERROR) << "flush data in FIFO buffer error, the write length is " 
        << flush_length;
    }
    LOG(LS_INFO) << "send times " << i;
  }
}