/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/29      9:25
 * Filename : F:\GitHub\trunk\p2p_slotion\P2PConnectionImplementator.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: P2PConnectionImplementator
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


#include "mediator_pattern.h"
#include "P2PConnectionImplementator.h"
#include "senddatabuffer.h"

P2PConnectionImplementator::P2PConnectionImplementator(
  const std::string &remote_jid,talk_base::StreamInterface *stream)
  :remote_jid_(remote_jid),stream_(stream)
{
  //0. Initialize basic parts
  send_data_buffer_      = new SendDataBuffer();
  temp_read_buffer_      = new char[BUFFER_SIZE];
  data_multiplex_machine_ = new DataMultiplexMachine(this);
  is_connect_ = false;

  //1. Configuration the stream events interface
  stream_->SignalEvent.connect(this,
    &P2PConnectionImplementator::OnStreamEvent);

  //2. 
}

bool P2PConnectionImplementator::IsMe(const std::string &remote_jid)
{
  if(remote_jid_ == remote_jid)
    return true;
  return false;
}


//////////////////////////////////////////////////////////////
//TEST NOTE (GuangleiHe, 11/29/2013)
//There need test
//Because I don't known whether the p2p connection is complete.
//Maybe the return of error is non zero value indicate it.
//However, it needs test.
//////////////////////////////////////////////////////////////
void P2PConnectionImplementator::OnStreamEvent(
  talk_base::StreamInterface* stream,int events,int error)
{
  ASSERT(stream_ == stream);
  if(error != 0){
    LOG(LS_ERROR) << "The stream got a error";
    return ;
  }
  if (events & talk_base::SE_READ) {
    //Call send data to up layer
    OnReadStreamData(stream_);
  }

  if (events & talk_base::SE_WRITE) {
    if(!is_connect_){
      std::cout << "Connected Succeed" << std::endl;
      is_connect_ = true;
      return;
    }
    send_data_buffer_->SetNormalState();
    if(send_data_buffer_->SendDataUsedStream(stream_)){
      SignalStreamWrite(stream_);
    }
  }

  if (events & talk_base::SE_CLOSE) {
    LOG(LS_VERBOSE) << "The stream closed";
    stream_->Close();
  }
}

void P2PConnectionImplementator::Send(uint32 socket,SocketType socket_type,
                                      const char *data,uint16 len,
                                      size_t *written)
{
  size_t remain_buffer_length = send_data_buffer_->GetBufferRemainLength();

  if(remain_buffer_length == 0 ||
    send_data_buffer_->IsBlockState()){
      *written = 0;
      return ;
  }
  else{
    if(remain_buffer_length >= len)
      *written = len;
    else
      *written = remain_buffer_length;
  }
  //The call will be call OnReceiveMultiplexData Function
  data_multiplex_machine_->PacketData(socket,socket_type,data,*written);
}

void P2PConnectionImplementator::OnReceiveMultiplexData(const char *data, 
                                                        uint16 len)
{
  if(len == 0){
    send_data_buffer_->SaveData(data,NETWORKHEADER_LENGTH);
    return ;
  }
  send_data_buffer_->SaveData(data,len);
  send_data_buffer_->SendDataUsedStream(stream_);
}

void P2PConnectionImplementator::OnReadStreamData(
  talk_base::StreamInterface *stream)
{
  size_t res = 0;
  ///////////////////////////////////////////////////////////////////////////
  //BUSINESS LOGIC NOTE (GuangleiHe, 11/26/2013)
  //Below code read p2p network data by a stream interface. Because the p2p 
  //network use stun packet to make sure whether the peer is online, and 
  //Libjingle responder auto. So some read call will got zero length data.
  ///////////////////////////////////////////////////////////////////////////
  talk_base::StreamResult result = stream->Read(temp_read_buffer_,
    RECEIVE_BUFFER_LEN,&res,NULL);
  if(res)
    data_multiplex_machine_->UnpackData(temp_read_buffer_,res);
}