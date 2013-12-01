/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/12/01      12:34
 * Filename : F:\GitHub\trunk\p2p_slotion\proxyp2psession.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: proxyp2psession
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
#ifndef PROXY_P2P_SESSION_H_
#define PROXY_P2P_SESSION_H_

#include "talk/base//thread.h"

#include "talk/base/basictypes.h"
#include "defaults.h"

class ProxySocketBegin;
class P2PConnectionImplementator;
class P2PSystemCommandFactory;
class ProxyServerFactory;
class SocketTableManagement;


class ProxyP2PSession : public sigslot::has_slots<>{
public:
  ProxyP2PSession(P2PConnectionImplementator *p2p_connection_implementator);

  void RegisterProxySocket(ProxySocketBegin *proxy_socket_begin);

  ProxySocketBegin* GetProxySocketBegin(uint32 local_socket){
    return proxy_socket_begin_map_[local_socket];
  };
  P2PConnectionImplementator *GetP2PConnectionImplementator() const{
    return p2p_connection_implementator_;
  }
  void DestoryAll();
  bool IsMe(const std::string remote_peer_name)const ;

  //p2p system management
  void CreateClientSocketConnection(uint32 socket,
    const talk_base::SocketAddress& addr);
  void ReplayClientSocketCreateSucceed(uint32 client_socket, 
    uint32 server_socket,const talk_base::SocketAddress &addr);

private:
  //p2p system management
  bool ProceesSystemCommand(const char *data, uint16 len);
  
  //
  bool  RunSocketProccess(uint32 socket, SocketType socket_type,
    const char *data, uint16 len);
  //
  void OnConnectSucceed(talk_base::StreamInterface *stream);
  void OnStreamRead(uint32 socket_number,SocketType socket_type,
    const char *data, uint16 len);
  //All proxy get the close events.
  void OnStreamClose(talk_base::StreamInterface *stream);
  //All proxy get the write events.
  void OnStreamWrite(talk_base::StreamInterface *stream);
private:
  typedef std::map<uint32, ProxySocketBegin*> ProxySocketBeginMap;
  ProxySocketBeginMap          proxy_socket_begin_map_;

  P2PConnectionImplementator   *p2p_connection_implementator_;
  P2PSystemCommandFactory      *p2p_system_command_factory_;
  talk_base::Thread            *current_thread_;
  SocketTableManagement        *socket_table_management_;

};

#endif // !PROXY_P2P_SESSION_H_
