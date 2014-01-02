/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/29      9:25
 * Filename : F:\GitHub\trunk\p2p_slotion\P2PConnectionImplementator.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: P2PConnectionImplementator
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

#ifndef P2P_CONNECTION_IMPLEMENTATOR_H_
#define P2P_CONNECTION_IMPLEMENTATOR_H_

#include <string>

#include "talk/base/stream.h"
#include "talk/base/sigslot.h"

#include "datamultiplexmachine.h"

class AbstractVirtualNetwork;
class SendDataBuffer;

class P2PConnectionImplementator 
  : public sigslot::has_slots<>,public talk_base::MessageHandler
{
public:
  explicit P2PConnectionImplementator(const std::string &remote_jid,
    talk_base::StreamInterface *stream,bool is_mix_data_mode = true);
  ~P2PConnectionImplementator();
  void Destory();
  void CloseStream();
  bool IsMe(const std::string &remote_jid) const {
    return remote_jid_ == remote_jid;
  };

  //
  virtual void Send(uint32,SocketType,const char*,uint16,size_t *);

  virtual void OnReceiveMultiplexData(const char *data, uint16 len); 
  sigslot::signal4<uint32 ,SocketType,const char *, uint16> SignalStreamRead;
  sigslot::signal1<talk_base::StreamInterface *> SignalIndependentStreamRead;
  sigslot::signal1<talk_base::StreamInterface *> SignalStreamWrite;
  sigslot::signal1<talk_base::StreamInterface *> SignalStreamClose;
  sigslot::signal1<talk_base::StreamInterface *> SignalConnectSucceed;
private:
  void OnStreamEvent(talk_base::StreamInterface* stream,
    int events,int error);
  void OnStreamClosed(talk_base::StreamInterface* stream);
  //application layer
  
  void OnReadStreamData(talk_base::StreamInterface *stream);


  void MixReadStreamData(talk_base::StreamInterface *stream);
  void IndependentReadStreamData(talk_base::StreamInterface *stream);

  void MixSend(uint32,SocketType,const char*,uint16,size_t *);
  void IndependentSend(uint32,SocketType,const char*,uint16,size_t *);

  virtual void OnMessage(talk_base::Message *msg);
private:
  enum{
    SIGNAL_READ,
    SIGNAL_WRITE,
    SIGNAL_CLOSE,
    SIGNAL_CONNECTSUCCEED,
  };
  enum{
    STREAM_SUCCEED,
    STREAM_BLOCK,
    STREAM_CLOSE
  }state_;
  static const int BUFFER_SIZE = 64 * 1024;
  std::string                remote_jid_;
  talk_base::StreamInterface *stream_;
  DataMultiplexMachine       *data_multiplex_machine_;
  bool                       is_connect_;
  bool                       is_mix_data_mode_;

  char                       *temp_read_buffer_;
  SendDataBuffer             *send_data_buffer_;
  talk_base::Thread          *current_thread_;
};

#endif // !P2P_CONNECTION_IMPLEMENTATOR_H_
