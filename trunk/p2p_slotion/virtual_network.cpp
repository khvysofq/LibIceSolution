/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/15      14:33
 * Filename : F:\GitHub\trunk\p2p_slotion\virtual_network.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: virtual_network
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

#include "virtual_network.h"
#include "networkbytebuffer.h"

VirtualNetwork::VirtualNetwork(AbstractICEConnection *p2p_ice_connection):
  AbstractVirtualNetwork(p2p_ice_connection)
{
  LOG(LS_INFO) << "^^^" << __FUNCTION__;

  //Send network header
  send_network_header_ = new NetworkHeader();
  send_network_header_->header_ide_ = P2P_NETWORKER_HEADER_IDE;
  send_byte_buffer_ = new talk_base::ByteBuffer(NULL,NETWORKHEADER_LENGTH);

  //Receive network header
  receive_network_header_ = new NetworkHeader();
  receive_low_buffer_  = new char[RECEIVE_BUFFER_LEN];;
  receive_current_len_ = 0;
  reading_states_ = READING_HEADER_IDE;

  //other
  socket_table_management_ = SocketTableManagement::Instance();
  temp_buffer_        = new char[RECEIVE_BUFFER_LEN];
}
void VirtualNetwork::Destory(){
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  delete send_network_header_;
  delete send_byte_buffer_;
  delete receive_network_header_;
  delete receive_low_buffer_;
  delete temp_buffer_;
}
VirtualNetwork::~VirtualNetwork(){
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  Destory();
}


void VirtualNetwork::OnReceiveDataFromLowLayer(talk_base::StreamInterface*
                                               stream)
{
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  size_t res = 0;
  int    error = 0;
  talk_base::StreamResult result = stream->ReadAll(temp_buffer_,
    RECEIVE_BUFFER_LEN,&res,&error);
  if(result != talk_base::SR_SUCCESS)
    LOG(LS_ERROR) << "\t result = " << StreamResultToString(result);

  if(res){
    LOG(LS_ERROR) << "\t receive data length is " << res;
    NetworkByteBuffer network_byte_buffer(temp_buffer_,res);

    size_t current_reading = 0;
    while(network_byte_buffer.Length() > 0 ){
      if(reading_states_ == READING_HEADER_IDE){
        uint32 network_header_ide;
        network_byte_buffer.ReadUInt32(&network_header_ide);
        if(network_header_ide == P2P_NETWORKER_HEADER_IDE){
          reading_states_ = READING_LOCAL_SOCKET;
        } else{
          LOG(LS_ERROR) <<"\t reading network header identifier error";
          continue;
        }
      }
      else if(reading_states_ == READING_LOCAL_SOCKET){
        uint32 local_socket;
        network_byte_buffer.ReadUInt32(&local_socket);
        receive_network_header_->local_socket_ = local_socket;
        reading_states_ = READING_REMOTE_SOCKET;
      }
      else if(reading_states_ == READING_REMOTE_SOCKET){
        uint32 remote_socket;
        network_byte_buffer.ReadUInt32(&remote_socket);
        receive_network_header_->local_socket_ = remote_socket;
        reading_states_ = READING_SOCKET_TYPE;
      }
      else if(reading_states_ == READING_SOCKET_TYPE){
        uint16 socket_type;
        network_byte_buffer.ReadUInt16(&socket_type);
        send_network_header_->socket_type_ = socket_type;
        reading_states_ = READING_DATA_LENGTH;
      }
      else if(reading_states_ == READING_DATA_LENGTH){
        uint16 data_len;
        network_byte_buffer.ReadUInt16(&data_len);
        if((data_len > 0) && (data_len < RECEIVE_BUFFER_LEN)){
          send_network_header_->data_len_ = data_len;
          reading_states_ = READING_DATA;
        } else{
          LOG(LS_ERROR) << "\t receive data length error";
          reading_states_ = READING_HEADER_IDE;
          continue;
        }
      }
      else if(reading_states_ == READING_DATA){
        if(network_byte_buffer.Length() >= 
          send_network_header_->data_len_ - receive_current_len_){

          network_byte_buffer.ReadBytes(&receive_low_buffer_[receive_current_len_],
              send_network_header_->data_len_ - receive_current_len_);

          SignalSendDataToUpLayer(send_network_header_->remote_socket_,
            send_network_header_->socket_type_,receive_low_buffer_,
            send_network_header_->data_len_);

          reading_states_ = READING_HEADER_IDE;
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
  LOG(LS_INFO) << "\tdata receive end";
}

void VirtualNetwork::OnReceiveDataFromUpLayer(uint32 socket,SocketType socket_type,
                                              const char *data,uint16 len)
{
  LOG(LS_INFO) << "^^^" << __FUNCTION__;
  LOG(LS_INFO) << "\t local socket is " << socket;
  LOG(LS_INFO) << "\t receive data length is " << len; 
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
  SignalSendDataToLowLayer(send_byte_buffer_->Data(),0);

  ///////////////////////////////////////////////////////////////////////////
  //BUG NOTE (GuangleiHe, 11/15/2013)
  //Maybe there has a bug.
  //because the Claer function of talk_base::ByteBuffer not release, but
  // it maybe can't write data forever.
  ///////////////////////////////////////////////////////////////////////////
  send_byte_buffer_->Clear();

  //4. send the relay data to low layer
  SignalSendDataToLowLayer(data,len);
}


void VirtualNetwork::AddInNetworkHeader(uint32 local_socket, SocketType socket_type,
                                     uint16 len){
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
  
  //2. set remote socket to local socket of the network header
  send_network_header_->local_socket_  = remote_socket;
  
  //3. set local socket to remote socket of the network header
  send_network_header_->remote_socket_ = local_socket;

  //4. setting other
  send_network_header_->data_len_      = len;
  send_network_header_->socket_type_   = socket_type;

}

void VirtualNetwork::ConvertNetworkHeaderToBuffer(){
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

void VirtualNetwork::OnMessage(talk_base::Message* msg){

}