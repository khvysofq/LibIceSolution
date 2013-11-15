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

#include "mediator_pattern.h"
#include "peer_connection_ice.h"
#include "defaults.h"
#include "sockettablemanagement.h"

class VirtualNetwork :public AbstractVirtualNetwork,
  public talk_base::MessageHandler
{
public:
  VirtualNetwork(AbstractICEConnection *p2p_ice_connection);
  ~VirtualNetwork();

public: // implementation inheritance
  // for AbstractVirtualNetwork
  virtual void OnReceiveDataFromLowLayer(talk_base::StreamInterface* );
  virtual void OnReceiveDataFromUpLayer(uint32,SocketType,const char*,uint16);

  // for MessageHandler
  void OnMessage(talk_base::Message* msg);

private:  // help function and struct

  // reading network header states by low layer (p2p ice layer)
  enum ReadingStates{
    READING_HEADER_IDE,
    READING_LOCAL_SOCKET,
    READING_REMOTE_SOCKET,
    READING_SOCKET_TYPE,
    READING_DATA_LENGTH,
    READING_DATA
  };

  // Destory all object
  void Destory();

  void AddInNetworkHeader(uint32 local_socket, SocketType socket_type,uint16 len);
  void ConvertNetworkHeaderToBuffer();

private:  // receive up date process
  NetworkHeader         *send_network_header_;
  // help variable that convert the network header struct to NETWORK ORDER
  talk_base::ByteBuffer *send_byte_buffer_;

private:  // receive low layer process
  NetworkHeader         *receive_network_header_;
  uint16                receive_current_len_;
  ReadingStates         reading_states_;
  char                  *receive_low_buffer_;

private:  // other
  SocketTableManagement *socket_table_management_;
  char                  *temp_buffer_;
};
#endif