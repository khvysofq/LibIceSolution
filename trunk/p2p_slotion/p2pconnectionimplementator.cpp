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
#include "p2pconnectionimplementator.h"
#include "senddatabuffer.h"

P2PConnectionImplementator::P2PConnectionImplementator(
  const std::string &remote_jid,talk_base::StreamInterface *stream,
  bool is_mix_data_mode):remote_jid_(remote_jid),stream_(stream),
  is_mix_data_mode_(is_mix_data_mode)
{
  //LOG_F_S(LS_INFO,P2P_TUNNEL_DATA_INFOR) 
  //  << "\t Create A New P2PConnectionImplementator";
  //Initialize basic parts
  send_data_buffer_      = new SendDataBuffer();
  temp_read_buffer_      = new char[BUFFER_SIZE];
  data_multiplex_machine_ = new DataMultiplexMachine(this);
  is_connect_ = false;

  //Configuration the stream events interface
  stream_->SignalEvent.connect(this,
    &P2PConnectionImplementator::OnStreamEvent);
}

P2PConnectionImplementator::~P2PConnectionImplementator(){
  delete send_data_buffer_;
  delete temp_read_buffer_;
  delete data_multiplex_machine_;
}

void P2PConnectionImplementator::Destory(){
  send_data_buffer_->Destory();
}

void P2PConnectionImplementator::CloseStream(){
  if(stream_->GetState() == talk_base::SS_CLOSED){
    return ;
  }
  stream_->Close();
  SignalConnectSucceed.disconnect_all();
  SignalStreamClose.disconnect_all();
  SignalStreamRead.disconnect_all();
  SignalStreamWrite.disconnect_all();
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
    LOG_F(LS_INFO) << "Stream SE_READ";
    //Call send data to up layer
    OnReadStreamData(stream_);
  }

  if (events & talk_base::SE_WRITE) {
    LOG_F(LS_INFO) << "Stream SE_WRITE";

    if(is_mix_data_mode_){
      send_data_buffer_->SetNormalState();
    }

    if(!is_connect_){
      is_connect_ = true;
      SignalConnectSucceed(stream);
    } 
    else{
      if(is_mix_data_mode_){
        if(send_data_buffer_->SendDataUsedStream(stream_)){
          SignalStreamWrite(stream_);
        }
      }
      else{
        SignalStreamWrite(stream_);
      }
    }
  }

  if (events & talk_base::SE_CLOSE) {
    LOG_F(LS_INFO) << "Stream SE_CLOSE";
    //LOG_F_S(LS_INFO,P2P_TUNNEL_DATA_INFOR) << "The stream closed";
    stream_->Close();
    SignalStreamClose(stream);
  }
}

void P2PConnectionImplementator::Send(uint32 socket,SocketType socket_type,
                                      const char *data,uint16 len,
                                      size_t *written)
{
  if(is_mix_data_mode_)
    MixSend(socket,socket_type,data,len,written);
  else
    IndependentSend(socket,socket_type,data,len,written);
}

void P2PConnectionImplementator::MixSend(uint32 socket,SocketType socket_type,
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

void P2PConnectionImplementator::IndependentSend(uint32 socket,SocketType socket_type,
                                                 const char *data,uint16 len,
                                                 size_t *written)
{
  talk_base::StreamResult res = stream_->Write(data,len,written,NULL);
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
  if(is_mix_data_mode_)
    MixReadStreamData(stream);
  else
    IndependentReadStreamData(stream);
}

void P2PConnectionImplementator::MixReadStreamData(
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

void P2PConnectionImplementator::IndependentReadStreamData(
  talk_base::StreamInterface *stream)
{
  SignalIndependentStreamRead(stream);
}