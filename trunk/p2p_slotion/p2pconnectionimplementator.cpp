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

#include "talk/base/thread.h"

#include "mediator_pattern.h"
#include "p2pconnectionimplementator.h"
#include "senddatabuffer.h"

P2PConnectionImplementator::P2PConnectionImplementator(
  const std::string &remote_jid,talk_base::StreamInterface *stream,
  bool is_mix_data_mode):remote_jid_(remote_jid),stream_(stream),
  is_mix_data_mode_(is_mix_data_mode)
{
  LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC)
    << "Create P2P Connection Implementation";

  state_ = STREAM_CLOSE;
  current_thread_ = talk_base::Thread::Current();

  if(is_mix_data_mode_){
    send_data_buffer_       = new SendDataBuffer(KBufferSize);
    temp_read_buffer_       = new char[BUFFER_SIZE + 16];
    data_multiplex_machine_ = new DataMultiplexMachine(this);
  }
  else{
    send_data_buffer_       = NULL;
    temp_read_buffer_       = NULL;
    data_multiplex_machine_ = NULL;
  }

  //Configuration the stream events interface
  stream_->SignalEvent.connect(this,
    &P2PConnectionImplementator::OnStreamEvent);
}

P2PConnectionImplementator::~P2PConnectionImplementator(){
  ASSERT(current_thread_->IsCurrent());
  LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC)
    << "delete P2P Connection Implementation";
  ASSERT(send_data_buffer_ == NULL);
  ASSERT(temp_read_buffer_ == NULL);
  ASSERT(data_multiplex_machine_ == NULL);
  ASSERT(state_ == STREAM_CLOSE);
}

void P2PConnectionImplementator::Destory(){
  ASSERT(current_thread_->IsCurrent());
  LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC)
    << "Destroy P2P Connection Implementation";
  //At first close the stream
  if(state_ != STREAM_CLOSE ){
    state_ = STREAM_CLOSE;
    stream_->Close();
  }
  OnStreamClosed(stream_);
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
  ASSERT(current_thread_->IsCurrent());
  ASSERT(stream_ == stream);
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "Stream EVENT";
  if(state_ == STREAM_CLOSE && error == 0 
    && !(events & talk_base::SE_CLOSE)){
      state_ = STREAM_SUCCEED;
      //send_data_buffer_->SetNormalState(); // There is not nature
      current_thread_->Post(this,SIGNAL_CONNECTSUCCEED);
      //return;
  }

  if (events & talk_base::SE_READ) {
    LOG_P2P(P2P_PROXY_SOCKET_DATA) << "Stream SE_READ";
    //Call send data to up layer
    current_thread_->Post(this,SIGNAL_READ);
  }

  if (events & talk_base::SE_WRITE) {
    LOG_P2P(P2P_PROXY_SOCKET_DATA) << "Stream SE_WRITE";

    if(is_mix_data_mode_){
      send_data_buffer_->SetNormalState();
      if(send_data_buffer_->SendDataUsedStream(stream_)){
        current_thread_->Post(this,SIGNAL_WRITE);
      }
    }
    else{
      current_thread_->Post(this,SIGNAL_WRITE);
    }
  }

  if (events & talk_base::SE_CLOSE) {
    LOG_P2P(P2P_PROXY_SOCKET_DATA) << "Stream SE_CLOSE";
    Destory();
  }
}

void P2PConnectionImplementator::OnStreamClosed(talk_base::StreamInterface* stream){
  ASSERT(current_thread_->IsCurrent());
  LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC)
    << "OnStreamClosed P2P Connection Implementation";
  //delete other
  //stream_->SignalEvent.disconnect_all();
  if(is_mix_data_mode_){
    delete send_data_buffer_;
    send_data_buffer_ = NULL;
    delete temp_read_buffer_;
    temp_read_buffer_ = NULL;
    delete data_multiplex_machine_;
    data_multiplex_machine_ = NULL;
  }
  current_thread_->Post(this,SIGNAL_CLOSE);
}

void P2PConnectionImplementator::Send(uint32 socket,SocketType socket_type,
                                      const char *data,uint16 len,
                                      size_t *written)
{
  ASSERT(current_thread_->IsCurrent());
  if(state_ == STREAM_CLOSE)
    return ;
  if(is_mix_data_mode_)
    MixSend(socket,socket_type,data,len,written);
  else
    IndependentSend(socket,socket_type,data,len,written);
}

void P2PConnectionImplementator::MixSend(uint32 socket,SocketType socket_type,
                                         const char *data,uint16 len,
                                         size_t *written)
{
  ASSERT(current_thread_->IsCurrent());
  size_t remain_buffer_length = send_data_buffer_->GetBufferRemainLength();
  if(remain_buffer_length == 0 || send_data_buffer_->IsBlockState()){
    *written = 0;
    return ;
  }
  else{
    if(remain_buffer_length >= len)
      *written = len;
    else
      *written = remain_buffer_length;
  }
  
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "Send PacketData";
  //The call will be call OnReceiveMultiplexData Function
  data_multiplex_machine_->PacketData(socket,socket_type,data,*written);
}

void P2PConnectionImplementator::IndependentSend(uint32 socket,SocketType socket_type,
                                                 const char *data,uint16 len,
                                                 size_t *written)
{
  ASSERT(current_thread_->IsCurrent());
  talk_base::StreamResult res = stream_->Write(data,len,written,NULL);
  if(res == talk_base::SR_BLOCK){
    LOG(LS_ERROR) << "The stream is block" << stream_;
  }
}

void P2PConnectionImplementator::OnReceiveMultiplexData(const char *data, 
                                                        uint16 len)
{
  ASSERT(current_thread_->IsCurrent());
  if(state_ == STREAM_CLOSE)
    return ;
  if(len == 0){
    send_data_buffer_->SaveData(data,NETWORKHEADER_LENGTH);
    return ;
  }
  LOG_P2P(P2P_PROXY_SOCKET_DATA) << "Send Multiplex data";
  send_data_buffer_->SaveData(data,len);
  send_data_buffer_->SendDataUsedStream(stream_);
}

void P2PConnectionImplementator::OnReadStreamData(
  talk_base::StreamInterface *stream)
{
  LOG_P2P(P2P_PROXY_SOCKET_DATA|P2P_PROXY_SOCKET_LOGIC) 
    << "state_ is " << (state_ == STREAM_CLOSE);
  ASSERT(current_thread_->IsCurrent());
  if(state_ == STREAM_CLOSE)
    return ;
  if(is_mix_data_mode_)
    MixReadStreamData(stream);
  else
    IndependentReadStreamData(stream);
}

void P2PConnectionImplementator::MixReadStreamData(
  talk_base::StreamInterface *stream)
{
  ASSERT(current_thread_->IsCurrent());
  size_t res = 0;
  ///////////////////////////////////////////////////////////////////////////
  //BUSINESS LOGIC NOTE (GuangleiHe, 11/26/2013)
  //Below code read p2p network data by a stream interface. Because the p2p 
  //network use stun packet to make sure whether the peer is online, and 
  //Libjingle responder auto. So some read call will got zero length data.
  ///////////////////////////////////////////////////////////////////////////
  talk_base::StreamResult result = stream->Read(temp_read_buffer_,
    BUFFER_SIZE + 16,&res,NULL);
  
  LOG_P2P(P2P_PROXY_SOCKET_DATA|P2P_PROXY_SOCKET_LOGIC) 
    << "read data length is  " << res;
  if(res)
    data_multiplex_machine_->UnpackData(temp_read_buffer_,res);
}

void P2PConnectionImplementator::IndependentReadStreamData(
  talk_base::StreamInterface *stream)
{
  ASSERT(current_thread_->IsCurrent());
  SignalIndependentStreamRead(stream);
}

void P2PConnectionImplementator::OnMessage(talk_base::Message *msg){
  ASSERT(current_thread_->IsCurrent());
  switch(msg->message_id){
  case SIGNAL_READ:
    {
      LOG_P2P(P2P_PROXY_SOCKET_DATA|P2P_PROXY_SOCKET_LOGIC) 
        << "stream read event";
      OnReadStreamData(stream_);
      break;
    }
  case SIGNAL_WRITE:
    {
      LOG_P2P(P2P_PROXY_SOCKET_DATA|P2P_PROXY_SOCKET_LOGIC) 
        << "stream write event";
      SignalStreamWrite(stream_);
      break;
    }
  case SIGNAL_CLOSE:
    {
      LOG_P2P(CREATE_DESTROY_INFOR|P2P_PROXY_SOCKET_LOGIC)
        << "OnMessage P2P Connection Implementation";
      SignalStreamClose(stream_);
      SignalStreamRead.disconnect_all();
      SignalIndependentStreamRead.disconnect_all();
      SignalStreamWrite.disconnect_all();
      SignalStreamClose.disconnect_all();
      SignalConnectSucceed.disconnect_all();
      break;
    }
  case SIGNAL_CONNECTSUCCEED:
    {
      LOG_P2P(P2P_PROXY_SOCKET_DATA|P2P_PROXY_SOCKET_LOGIC) 
        << "stream connect succeed";
      SignalConnectSucceed(stream_);
      break;
    }
  }
}