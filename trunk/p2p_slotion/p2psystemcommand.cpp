/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/19      17:07
 * Filename : F:\GitHub\trunk\p2p_slotion\p2psystemcommand.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: p2psystemcommand
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

#include "p2psystemcommand.h"
#include "virtual_network.h"

P2PSystemCommandFactory 
  *P2PSystemCommandFactory::p2p_system_command_factory_ = NULL;

P2PSystemCommandFactory *P2PSystemCommandFactory::Instance(){
  if(!p2p_system_command_factory_)
    p2p_system_command_factory_ = new P2PSystemCommandFactory();
  return p2p_system_command_factory_;
}

P2PSystemCommandFactory::P2PSystemCommandFactory(){}

bool P2PSystemCommandFactory::IsP2PSystemCommand(const char *data, int len){
  return true;
}

talk_base::ByteBuffer * P2PSystemCommandFactory::CreateRTSPClientSocket(
  uint32 socket,const talk_base::SocketAddress &addr)
{
  LOG(LS_INFO) << __FUNCTION__;
  //CreateRTSPClientSocketCommand rtsp_client_socket_command;
  
  talk_base::ByteBuffer *byte_buffer = new talk_base::ByteBuffer();

  byte_buffer->WriteUInt32(P2P_SYSTEM_COMMAND_IDE);
  LOG(LS_INFO) << "\t p2p_system_command_ide_ " << P2P_SYSTEM_COMMAND_IDE;
  byte_buffer->WriteUInt32(P2P_SYSTEM_CREATE_RTSP_CLIENT);
  LOG(LS_INFO) << "\t p2p_system_command_type_ " << P2P_SYSTEM_CREATE_RTSP_CLIENT;
  byte_buffer->WriteUInt32(socket);
  LOG(LS_INFO) << "\t server_socket_" << socket;
  byte_buffer->WriteUInt32(socket);
  LOG(LS_INFO) << "\t client_socket_ " << socket;
  byte_buffer->WriteUInt32(addr.ip());
  LOG(LS_INFO) << "\t client_connection_ip_ " << addr.ip();
  byte_buffer->WriteUInt16(addr.port());
  LOG(LS_INFO) << "\t client_connection_port_ " << addr.port();
  byte_buffer->WriteUInt16(P2P_SYSTEM_COMMAND_PADDING_BYTE);
  LOG(LS_INFO) << "\t padding_byte_ " << P2P_SYSTEM_COMMAND_PADDING_BYTE;

  //Maybe there is a bug.
  return byte_buffer;
}

talk_base::ByteBuffer *P2PSystemCommandFactory::ReplyRTSPClientSocketSucceed(
  uint32 server_socket, uint32 client_socket)
{

  talk_base::ByteBuffer *byte_buffer = new talk_base::ByteBuffer();

  byte_buffer->WriteUInt32(P2P_SYSTEM_COMMAND_IDE);
  byte_buffer->WriteUInt32(P2P_SYSTEM_CREATE_RTSP_CLIENT_SUCCEED);
  byte_buffer->WriteUInt32(server_socket);
  byte_buffer->WriteUInt32(client_socket);
  byte_buffer->WriteUInt32(0);
  byte_buffer->WriteUInt16(0);
  byte_buffer->WriteUInt16(P2P_SYSTEM_COMMAND_PADDING_BYTE);

  //Maybe there is a bug.
  return byte_buffer;

}

void P2PSystemCommandFactory::DeleteRTSPClientCommand(talk_base::ByteBuffer *data){
  ASSERT(data != NULL);
  delete data;
}

bool P2PSystemCommandFactory::ParseCommand(
  P2PRTSPCommand *client_socket_command,const char *data,
  uint16 len)
{
  if(len != P2PRTSPCOMMAND_LENGTH){
    LOG(LS_ERROR) << "\t The length of data is not expected length";
    return false;
  }
  talk_base::ByteBuffer byte_buffer(data,len);

  //
  byte_buffer.ReadUInt32(&client_socket_command->p2p_system_command_ide_);
  if(client_socket_command->p2p_system_command_ide_ 
    != P2P_SYSTEM_COMMAND_IDE){
      LOG(LS_ERROR) << "\t The P2P P2P_SYSTEM_COMMAND_IDE error " 
        << client_socket_command->p2p_system_command_ide_;
      return false;
  }

  byte_buffer.ReadUInt32(&client_socket_command->p2p_system_command_type_);
  byte_buffer.ReadUInt32(&client_socket_command->server_socket_);
  byte_buffer.ReadUInt32(&client_socket_command->client_socket_);
  byte_buffer.ReadUInt32(&client_socket_command->client_connection_ip_);
  byte_buffer.ReadUInt16(&client_socket_command->client_connection_port_);
  byte_buffer.ReadUInt16(&client_socket_command->padding_byte_);
  if(client_socket_command->padding_byte_ != P2P_SYSTEM_COMMAND_PADDING_BYTE)
  {
    LOG(LS_ERROR) << "the padding_byte is error";
    return false;
  }
  return true;
}