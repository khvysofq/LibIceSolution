/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/26      14:55
 * Filename : F:\GitHub\trunk\p2p_slotion\senddatabuffer.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: senddatabuffer
 * File ext : h
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
  
  void Destory();
  //Save data to FIFO buffer
  bool SaveData(const char *data, size_t len);

  //Return the available write buffer length
  size_t GetBufferRemainLength();

  bool SendDataUsedStream(talk_base::StreamInterface *stream);
  void SetNormalState();
  void SetBlockState();
  bool IsBlockState() const;
private:
  
  //The state of this class.
  enum {
    NORMAL_STATE,
    BLOCK_STATE
  } state_;

  size_t                buffer_length_;
  talk_base::FifoBuffer *fifo_buffer_;
};

#endif