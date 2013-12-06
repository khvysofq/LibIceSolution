/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/29      10:49
 * Filename : F:\GitHub\trunk\p2p_slotion\datamultiplexmachine.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: datamultiplexmachine
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

#ifndef DATA_MULTIPLEX_MACHINE_H_
#define DATA_MULTIPLEX_MACHINE_H_

#include "talk/base/basictypes.h"
#include "talk/base/sigslot.h"
#include "talk/base/bytebuffer.h"

#include "defaults.h"

class SocketTableManagement;
class P2PConnectionImplementator;
///////////////////////////////////////////////////////////////////////////
//BUSINESS LOGIC NOTE (GuangleiHe, 12/1/2013)
//There needs the patent class for DataMultiplexMachine and 
//P2PConnectionImplementator
///////////////////////////////////////////////////////////////////////////
//class MultiplexTunnelInterface{
//public:
//  virtual void OnReceiveMultiplexData(const char *data, uint16 len) = 0;
//  sigslot::signal4<uint32 ,SocketType,const char *, uint16> SignalSendDataToUpLayer;
//};

class DataMultiplexMachine{
public:
  DataMultiplexMachine(P2PConnectionImplementator *p2p_connection_implementator);
  ~DataMultiplexMachine();
  void PacketData(uint32 socket,SocketType socket_type,
    const char *data,uint16 len);
  void UnpackData(char *data,uint16 len);
  void Destory();
private:
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

  P2PConnectionImplementator *p2p_connection_implementator_;
};


#endif // !DATA_MULTIPLEX_MACHINE_H_
