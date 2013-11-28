/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/15      16:01
 * Filename : F:\GitHub\trunk\p2p_slotion\virtual_network.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: virtual_network
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

#ifndef VIRTUAL_NETWORK_H_
#define VIRTUAL_NETWORK_H_
#include "talk/base/bytebuffer.h"

#include "mediator_pattern.h"

class SocketTableManagement;

class VirtualNetwork :public AbstractVirtualNetwork,
  public talk_base::MessageHandler
{
public:
  VirtualNetwork(AbstractICEConnection *p2p_ice_connection);
  ~VirtualNetwork();

public: // implementation inheritance
  // for AbstractVirtualNetwork
  virtual void OnReceiveDataFromLowLayer(talk_base::StreamInterface* );
  virtual void OnReceiveDataFromUpLayer(uint32,SocketType,const char*,uint16,
    size_t *);
  virtual void OnStreamWrite(talk_base::StreamInterface *stream);

  // for MessageHandler
  virtual void OnMessage(talk_base::Message* msg);

private:  // help function and struct
  // Destory all object
  void Destory();
  void AddInNetworkHeader(uint32 local_socket, SocketType socket_type,uint16 len);
  void ConvertNetworkHeaderToBuffer();

  // reading network header states by low layer (p2p ice layer)
  enum ReadingStates{
    READING_HEADER_IDE_1,
    READING_HEADER_IDE_2,
    READING_HEADER_IDE_3,
    READING_HEADER_IDE_4,
    READING_LOCAL_SOCKET_1,
    READING_LOCAL_SOCKET_2,
    READING_LOCAL_SOCKET_3,
    READING_LOCAL_SOCKET_4,
    READING_REMOTE_SOCKET_1,
    READING_REMOTE_SOCKET_2,
    READING_REMOTE_SOCKET_3,
    READING_REMOTE_SOCKET_4,
    READING_SOCKET_TYPE_1,
    READING_SOCKET_TYPE_2,
    READING_DATA_LENGTH_1,
    READING_DATA_LENGTH_2,
    READING_DATA
  };
  ReadingStates         reading_states_;

private:  // receive up date process
  NetworkHeader         *send_network_header_;
  // help variable that convert the network header struct to NETWORK ORDER
  talk_base::ByteBuffer *send_byte_buffer_;

private:  // receive low layer process
  NetworkHeader         *receive_network_header_;
  uint16                receive_current_len_;
  char                  *receive_low_buffer_;

private:  // other
  SocketTableManagement *socket_table_management_;
  char                  *temp_buffer_;

  //From google library, close the implicit constructor of the class
  DISALLOW_EVIL_CONSTRUCTORS(VirtualNetwork);
};
#endif