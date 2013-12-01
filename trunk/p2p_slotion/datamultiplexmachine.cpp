/*
* p2p solution
* Copyright 2013, VZ Inc.
* 
* Author   : GuangLei He
* Email    : guangleihe@gmail.com
* Created  : 2013/11/29      10:50
* Filename : F:\GitHub\trunk\p2p_slotion\datamultiplexmachine.cpp
* File path: F:\GitHub\trunk\p2p_slotion
* File base: datamultiplexmachine
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

#include "datamultiplexmachine.h"

#include "networkbytebuffer.h"
#include "sockettablemanagement.h"
#include "p2pconnectionimplementator.h"

DataMultiplexMachine::DataMultiplexMachine(
  P2PConnectionImplementator *p2p_connection_implementator)
  :p2p_connection_implementator_(p2p_connection_implementator)
{

  send_network_header_ = new NetworkHeader();
  send_network_header_->header_ide_ = P2P_NETWORKER_HEADER_IDE;
  send_byte_buffer_ = new talk_base::ByteBuffer(NULL,NETWORKHEADER_LENGTH);

  //Receive network header
  receive_network_header_ = new NetworkHeader();
  receive_low_buffer_  = new char[RECEIVE_BUFFER_LEN];;
  receive_current_len_ = 0;
  reading_states_ = READING_HEADER_IDE_1;

  //other
  socket_table_management_ = SocketTableManagement::Instance();
}

void DataMultiplexMachine::Destory(){
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  delete send_network_header_;
  delete send_byte_buffer_;
  delete receive_network_header_;
  delete receive_low_buffer_;
}
DataMultiplexMachine::~DataMultiplexMachine(){
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  Destory();
}

void DataMultiplexMachine::PacketData(
  uint32 socket,SocketType socket_type,const char *data,uint16 len)
{
  //1. Add a header at this packet
  AddInNetworkHeader(socket,socket_type,len);

  //2. convert the network header struct to buffer
  ConvertNetworkHeaderToBuffer();

  //3. send network header to low layer
  ///////////////////////////////////////////////////////////////////////////
  //IMPORTANT NOTE (GuangleiHe, 11/15/2013)
  //Because one packet will be divide into two parts, the first is the network
  //header, second is the relay data. It is a normal business logic, but it is 
  //slowly.So, the first part, network header, will be saved by low layer (ICE 
  //layer). For this reason we set the data length is zero to indicate this is
  //a network header data.
  ///////////////////////////////////////////////////////////////////////////
  p2p_connection_implementator_->OnReceiveMultiplexData(
    send_byte_buffer_->Data(),0);

  ///////////////////////////////////////////////////////////////////////////
  //BUG NOTE (GuangleiHe, 11/15/2013)
  //Maybe there has a bug.
  //because the Claer function of talk_base::ByteBuffer not release, but
  //it maybe can't write data forever.
  ///////////////////////////////////////////////////////////////////////////
  send_byte_buffer_->Clear();

  //4. send the relay data to low layer
  p2p_connection_implementator_->OnReceiveMultiplexData(data,len);
}

void DataMultiplexMachine::UnpackData(char *data, uint16 len){
  ASSERT(len != 0);
  uint8 one_byte;
  ///////////////////////////////////////////////////////////////////////////
  //TODO:(GuangleiHe) TIME: 11/26/2013
  //This is a long code parse package algorithm. It's sure to improved by a short 
  //code algorithm, but it's hard to improve efficiency algorithm. So I have
  //no time to do this.
  ///////////////////////////////////////////////////////////////////////////
  NetworkByteBuffer network_byte_buffer(data,len);

  ///////////////////////////////////////////////////////////////////////////
  //ALGORITHM NOTE (GuangleiHe, 11/26/2013)
  //Below code is parse a network header. It parse data one byte by
  //one byte.
  ///////////////////////////////////////////////////////////////////////////
  size_t current_reading = 0;
  while(network_byte_buffer.Length() > 0 ){
    // 1. reading first unsigned int data, P2P_NETWORKER_HEADER_IDE(0X01234567)
    if(reading_states_ == READING_HEADER_IDE_1){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->header_ide_ &= 0X0;
      receive_network_header_->header_ide_ |= one_byte;
      reading_states_ = READING_HEADER_IDE_2;
    }
    else if(reading_states_ == READING_HEADER_IDE_2){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->header_ide_ <<= 8;
      receive_network_header_->header_ide_ |= one_byte;
      reading_states_ = READING_HEADER_IDE_3;
    }
    else if(reading_states_ == READING_HEADER_IDE_3){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->header_ide_ <<= 8;
      receive_network_header_->header_ide_ |= one_byte;
      reading_states_ = READING_HEADER_IDE_4;
    }
    else if(reading_states_ == READING_HEADER_IDE_4){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->header_ide_ <<= 8;
      receive_network_header_->header_ide_ |= one_byte;
      if(receive_network_header_->header_ide_ == P2P_NETWORKER_HEADER_IDE){
        LOG(LS_VERBOSE) << "\t got P2P_NETWORKER_HEADER_IDE";
        reading_states_ = READING_LOCAL_SOCKET_1;
      }
      else{
        LOG(LS_ERROR) << "\t can't P2P_NETWORKER_HEADER_IDE";
      }
    }

    //2. reading second unsigned int data, the local socket number
    else if(reading_states_ == READING_LOCAL_SOCKET_1){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->local_socket_ &= 0X0;
      receive_network_header_->local_socket_ |= one_byte;
      reading_states_ = READING_LOCAL_SOCKET_2;
    }
    else if(reading_states_ == READING_LOCAL_SOCKET_2){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->local_socket_ <<= 8;
      receive_network_header_->local_socket_ |= one_byte;
      reading_states_ = READING_LOCAL_SOCKET_3;
    }
    else if(reading_states_ == READING_LOCAL_SOCKET_3){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->local_socket_ <<= 8;
      receive_network_header_->local_socket_ |= one_byte;
      reading_states_ = READING_LOCAL_SOCKET_4;
    }
    else if(reading_states_ == READING_LOCAL_SOCKET_4){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->local_socket_ <<= 8;
      receive_network_header_->local_socket_ |= one_byte;
      LOG(LS_VERBOSE) << "\t got P2P_NETWORKER_LOCAL_SOCKET";
      reading_states_ = READING_REMOTE_SOCKET_1;
    }

    //3. reading third unsigned int data, the remote socket number
    else if(reading_states_ == READING_REMOTE_SOCKET_1){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->remote_socket_ &= 0X0;
      receive_network_header_->remote_socket_ |= one_byte;
      reading_states_ = READING_REMOTE_SOCKET_2;
    }
    else if(reading_states_ == READING_REMOTE_SOCKET_2){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->remote_socket_ <<= 8;
      receive_network_header_->remote_socket_ |= one_byte;
      reading_states_ = READING_REMOTE_SOCKET_3;
    }
    else if(reading_states_ == READING_REMOTE_SOCKET_3){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->remote_socket_ <<= 8;
      receive_network_header_->remote_socket_ |= one_byte;
      reading_states_ = READING_REMOTE_SOCKET_4;
    }
    else if(reading_states_ == READING_REMOTE_SOCKET_4){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->remote_socket_ <<= 8;
      receive_network_header_->remote_socket_ |= one_byte;
      LOG(LS_VERBOSE) << "\t got P2P_NETWORKER_REMOTE_SOCKET";
      reading_states_ = READING_SOCKET_TYPE_1;
    }

    //4. reading a unsigned short data, the socket type 
    //[TCP_SOCK = 1 or UDP_SOCK = 2]
    else if(reading_states_ == READING_SOCKET_TYPE_1){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->socket_type_ &= 0X0;
      receive_network_header_->socket_type_ |= one_byte;
      reading_states_ = READING_SOCKET_TYPE_2;
    }
    else if(reading_states_ == READING_SOCKET_TYPE_2){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->socket_type_ <<= 8;
      receive_network_header_->socket_type_ |= one_byte;
      LOG(LS_VERBOSE) << "\t got P2P_NETWORKER_SOCKTYPE";
      reading_states_ = READING_DATA_LENGTH_1;
    }

    //5. reading unsigned sort data, the network package length. 
    else if(reading_states_ == READING_DATA_LENGTH_1){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->data_len_ &= 0X0;
      receive_network_header_->data_len_ |= one_byte;
      reading_states_ = READING_DATA_LENGTH_2;
    }
    else if(reading_states_ == READING_DATA_LENGTH_2){
      network_byte_buffer.ReadUInt8(&one_byte);
      receive_network_header_->data_len_ <<= 8;
      receive_network_header_->data_len_ |= one_byte;
      LOG(LS_VERBOSE) << "\t got P2P_NETWORKER_DATA_LENGTH";
      reading_states_ = READING_DATA;
    }

    //6. reading the last data, it's relay data from remote peer.
    else if(reading_states_ == READING_DATA){
      if(network_byte_buffer.Length() >= 
        receive_network_header_->data_len_ - receive_current_len_){

          network_byte_buffer.ReadBytes(&receive_low_buffer_[receive_current_len_],
            receive_network_header_->data_len_ - receive_current_len_);

          p2p_connection_implementator_->SignalStreamRead(
            receive_network_header_->local_socket_,
            receive_network_header_->socket_type_,receive_low_buffer_,
            receive_network_header_->data_len_);

          reading_states_ = READING_HEADER_IDE_1;
          receive_current_len_ = 0;
      } else {
        int t = network_byte_buffer.Length();
        network_byte_buffer.ReadBytes(&receive_low_buffer_[receive_current_len_],
          network_byte_buffer.Length());
        receive_current_len_ += t;
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////

void DataMultiplexMachine::AddInNetworkHeader(
  uint32 local_socket, SocketType socket_type,uint16 len)
{
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  ///////////////////////////////////////////////////////////////////////////
  //BUSINESS LOGIC NOTE (GuangleiHe, 11/15/2013)
  //it is important to known that for rationality logic,
  //the local_socket of the network header is remote_socket
  //and vice versa.
  //When the remote peers network layer parse the header it will
  //get the local socket at the domain of network header's local_socket
  //and get the remote socket at the domain of network header's remote_socket
  ///////////////////////////////////////////////////////////////////////////

  //1. get remote socket
  uint32 remote_socket = socket_table_management_->GetRemoteSocket(local_socket);
  LOG(LS_INFO) << "\t" << remote_socket;
  //2. set remote socket to local socket of the network header
  send_network_header_->local_socket_  = remote_socket;

  LOG(LS_INFO) << "\t" << remote_socket;
  //3. set local socket to remote socket of the network header
  send_network_header_->remote_socket_ = local_socket;

  LOG(LS_INFO) << "\t" << local_socket;
  //4. setting other
  send_network_header_->data_len_      = len;
  LOG(LS_INFO) << "\t" << len;
  send_network_header_->socket_type_   = socket_type;
  LOG(LS_INFO) << "\t" << socket_type;
}

void DataMultiplexMachine::ConvertNetworkHeaderToBuffer(){
  LOG(LS_INFO) << "^^^" << __FUNCTION__;

  //1. Write P2P_NETWORKER_HEADER_IDE to send buffer
  send_byte_buffer_->WriteUInt32(send_network_header_->header_ide_);
  //2. Write local socket in send buffer
  send_byte_buffer_->WriteUInt32(send_network_header_->local_socket_);
  //3. Write remote socket in send buffer
  send_byte_buffer_->WriteUInt32(send_network_header_->remote_socket_);
  //4. Write socket type in send buffer
  send_byte_buffer_->WriteUInt16(send_network_header_->socket_type_);
  //5. Write data packet length in send buffer
  send_byte_buffer_->WriteUInt16(send_network_header_->data_len_);
}