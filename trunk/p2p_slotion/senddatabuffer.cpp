/*
* p2p solution
* Copyright 2013, VZ Inc.
* 
* Author   : GuangLei He
* Email    : guangleihe@gmail.com
* Created  : 2013/11/21      11:03
* Filename : F:\GitHub\trunk\p2p_slotion\senddatabuffer.cpp
* File path: F:\GitHub\trunk\p2p_slotion
* File base: senddatabuffer
* File ext : cpp
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*  3. The name of the author may not be used to endorse or promote products
*     derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include "senddatabuffer.h"

#include "talk/base/basictypes.h"

SendDataBuffer::SendDataBuffer(size_t buffer_length)
  :buffer_length_(buffer_length + REMAIN_MIX_SIZE){
    fifo_buffer_  = new talk_base::FifoBuffer(buffer_length_);
    state_ = BLOCK_STATE;
}

SendDataBuffer::~SendDataBuffer(){
  ASSERT(fifo_buffer_ != NULL);
  delete fifo_buffer_;
}

void SendDataBuffer::Destory(){
  fifo_buffer_->Close();
}

bool SendDataBuffer::SaveData(const char *data, size_t len){
  ASSERT(len != 0 && data != NULL);

  if(state_ == BLOCK_STATE){
    LOG(LS_ERROR) << "Can't save data, because the stream is block.";
    return false;
  }

  size_t size;
  if (fifo_buffer_->GetBuffered(&size)) {
    void* p = fifo_buffer_->GetWriteBuffer(&size);
    if(size < len){
      LOG(LS_ERROR) << "The available size is small than length ";
      return false;
    }
    memcpy(p,data,len);
    fifo_buffer_->ConsumeWriteBuffer(len);
  }
  else {
    LOG(LS_ERROR) << "The written buffer data length is error";
  }
  return true;
}

size_t SendDataBuffer::GetBufferRemainLength(){

  if(state_ == BLOCK_STATE)
    return 0;
  size_t res = 0;
  fifo_buffer_->GetWriteRemaining(&res);
  //if(res - REMAIN_MIX_SIZE <= 0)
  //  return 0;
  return res;
}


bool SendDataBuffer::SendDataUsedStream(talk_base::StreamInterface *stream){
  ASSERT(stream != NULL);

  if(state_ == BLOCK_STATE)
    return false;
  size_t size;
  size_t written = 0;
  const void *p = fifo_buffer_->GetReadData(&size);
  if(!size)
    return true;
  talk_base::StreamResult res = stream->Write(p,size,&written,NULL);
  if(res == talk_base::SR_BLOCK){
    LOG(LS_WARNING) << "The p2p socket block";
    state_ = BLOCK_STATE;
    return false;
  }
  fifo_buffer_->ConsumeReadData(written);
  return true;
}

void SendDataBuffer::SetNormalState(){
  ASSERT(state_ == BLOCK_STATE);
  state_ = NORMAL_STATE;
}

void SendDataBuffer::SetBlockState(){
  ASSERT(state_ == NORMAL_STATE);
  state_ = BLOCK_STATE;
}

bool SendDataBuffer::IsBlockState() const{
  return state_ == BLOCK_STATE;
}