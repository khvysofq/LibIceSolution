/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/19      17:07
 * Filename : F:\GitHub\trunk\p2p_slotion\P2PSystemCommandFactory.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: P2PSystemCommandFactory
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

#ifndef P2P_SYSTEM_COMMAND_H_
#define P2P_SYSTEM_COMMAND_H_

#include "talk/base/bytebuffer.h"
#include "talk/base/basictypes.h"
#include "talk/base/socketaddress.h"
/*

*/
static const int P2P_SYSTEM_COMMAND_IDE         = 0X0F123456; 
static const int P2P_SYSTEM_COMMAND_PADDING_BYTE= 0XFF;


const int P2P_SYSTEM_CREATE_RTSP_CLIENT         = 1;
const int P2P_SYSTEM_CREATE_RTSP_CLIENT_SUCCEED = 2;
struct P2PRTSPCommand{
  uint32 p2p_system_command_ide_;
  uint32 p2p_system_command_type_;
  uint32 server_socket_;
  uint32 client_socket_;
  uint32 client_connection_ip_;
  uint16 client_connection_port_;
  uint16 padding_byte_;
};


const uint16 P2PRTSPCOMMAND_LENGTH 
  = sizeof(P2PRTSPCommand);

///////////////////////////////////////////////////////////////////////////
//TODO:(GuangleiHe) TIME: 12/1/2013
//Remove the P2P System Command Factory to a normal class. every function in  
//the class as a static function.
///////////////////////////////////////////////////////////////////////////
class P2PSystemCommandFactory
{
public:
  P2PSystemCommandFactory();
  static P2PSystemCommandFactory *Instance();

  bool IsP2PSystemCommand(const char *data, int len);

  talk_base::ByteBuffer *CreateRTSPClientSocket(
    uint32 socket,const talk_base::SocketAddress &addr);
  talk_base::ByteBuffer *ReplyRTSPClientSocketSucceed(uint32 server_socket,
    uint32 client_socket);

  void DeleteRTSPClientCommand(talk_base::ByteBuffer *data);

  bool ParseCommand(const char *data, 
    uint16 len,
    uint32 *p2p_system_command_type,
    uint32 *server_socket,
    uint32 *client_socket,
    uint32 *client_connection_ip,
    uint16 *client_connection_port);

private:
  static P2PSystemCommandFactory *p2p_system_command_factory_; 

  DISALLOW_EVIL_CONSTRUCTORS(P2PSystemCommandFactory);
};


#endif