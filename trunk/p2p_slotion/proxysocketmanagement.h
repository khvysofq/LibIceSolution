/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/20      10:57
 * Filename : F:\GitHub\trunk\p2p_slotion\proxysocketmanagement.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: proxysocketmanagement
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

#ifndef P2P_SLOTION_PROXY_SOCKET_MANAGEMENT_H_
#define P2P_SLOTION_PROXY_SOCKET_MANAGEMENT_H_

#include "talk/base/socketadapters.h"
//#include "proxyserverfactory.h"

#include "p2psystemcommand.h"
#include "sockettablemanagement.h"

class AsyncP2PSocket;

class ProxySocketBegin : public sigslot::has_slots<>
{
public:
  ProxySocketBegin(AsyncP2PSocket *p2p_socket,
    talk_base::AsyncSocket *int_socket);
  //p2p socket signal function
  virtual void OnP2PReceiveData(const char *data, uint16 len);
  bool IsMe(uint32 socket);

protected:
  //Internal Socket Signal function
  virtual void OnInternalRead(talk_base::AsyncSocket* socket);
  virtual void OnInternalWrite(talk_base::AsyncSocket* socket);
  virtual void OnInternalClose(talk_base::AsyncSocket* socket, int err);

  //Data Change Function
  virtual void ReadSocketDataToBuffer(talk_base::AsyncSocket *socket,
    talk_base::FifoBuffer *buffer);
  virtual void ReadP2PDataToBuffer(const char *data, uint16 len, 
    talk_base::FifoBuffer *buffer);
  virtual void WriteBufferDataToSocket(talk_base::AsyncSocket *socket,
    talk_base::FifoBuffer *buffer);
  virtual void WriteBufferDataToP2P(talk_base::FifoBuffer *buffer);

protected:
  static const int KBufferSize = 4 * 1024;
  talk_base::scoped_ptr<talk_base::AsyncSocket> int_socket_;
  talk_base::scoped_ptr<AsyncP2PSocket> p2p_socket_;
  talk_base::FifoBuffer out_buffer_;
  talk_base::FifoBuffer in_buffer_;

  SocketTableManagement        *socket_table_management_;
  P2PSystemCommandFactory      *p2p_system_command_factory_;
private:
  DISALLOW_EVIL_CONSTRUCTORS(ProxySocketBegin);
};

///////////////////////////////////////////////////////////////////////////
//TODO:(GuangleiHe) TIME: 11/20/2013
//Should implement P2PSocketFactory and inherit by SocketFactory
//
///////////////////////////////////////////////////////////////////////////
class ProxySocketManagement
{
public:
  ProxySocketManagement(){};
  void RegisterProxySocket(uint32 local_socket, 
    ProxySocketBegin *proxy_socket_begin);
  const ProxySocketBegin* GetProxySocketBegin(uint32 local_socket);
  bool  RunSocketProccess(uint32 socket, SocketType socket_type,
    const char *data, uint16 len);

  typedef std::map<uint32, ProxySocketBegin*> ProxySocketBeginMap;
  void DestoryAll();
private:
  ProxySocketBeginMap proxy_socket_begin_map_;

  DISALLOW_EVIL_CONSTRUCTORS(ProxySocketManagement);
};

#endif