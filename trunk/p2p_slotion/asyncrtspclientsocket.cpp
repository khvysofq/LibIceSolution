/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/19      19:14
 * Filename : F:\GitHub\trunk\p2p_slotion\asyncrtspclientsocket.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: asyncrtspclientsocket
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

#include "talk/base/bytebuffer.h"

#include "asyncrtspclientsocket.h"
#include "proxyserverfactory.h"


//////////////////////////////////////////////////////////////////////////
RTSPClientSocket::RTSPClientSocket(
  AsyncP2PSocket *p2p_socket,talk_base::AsyncSocket *int_socket,
  uint32 server_socket_number,const talk_base::SocketAddress &server_addr)
  :ProxySocketBegin(p2p_socket,int_socket),server_socket_number_(server_socket_number)
{
  
  int_socket_->SignalConnectEvent.connect(this,
    &RTSPClientSocket::OnInternalConnect);

  int_socket_->Connect(server_addr);
}



void RTSPClientSocket::OnInternalConnect(
  talk_base::AsyncSocket* socket)
{
  //The Client connect was accept
  //Then Send the connect succeed to remote peer
  ASSERT(int_socket_.get() == socket);
  //generate a reply string
  socket_table_management_->AddNewLocalSocket((uint32)socket,
    server_socket_number_,TCP_SOCKET);

  talk_base::ByteBuffer *reply_string = 
    p2p_system_command_factory_->ReplyRTSPClientSocketSucceed(
    server_socket_number_,(uint32)int_socket_.get());
  //send this string to remote peer

  p2p_socket_->Send((uint32)socket,TCP_SOCKET, reply_string->Data(),
    P2PRTSPCOMMAND_LENGTH,NULL);
  //delete the string
  p2p_system_command_factory_->DeleteRTSPClientCommand(reply_string);
}