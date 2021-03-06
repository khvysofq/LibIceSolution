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
#include "talk/base/bytebuffer.h"

#include "defaults.h"

class P2PProxySocket;
class ProxySocketBegin;
class P2PConnectionImplementator;
class P2PConnectionManagement;
class P2PSystemCommandFactory;
class ProxyServerFactory;
class SocketTableManagement;


class ProxyP2PSession : public sigslot::has_slots<>,
  public talk_base::MessageHandler
{
public:
  ProxyP2PSession(talk_base::StreamInterface *stream,
    const std::string &remote_peer_name,
    talk_base::Thread *signal_thread,
    talk_base::Thread *worker_thread,
    bool is_mix_data_mode = true);
  ~ProxyP2PSession();
  void RegisterProxySocket(ProxySocketBegin *proxy_socket_begin);
  void DeleteProxySocketBegin(ProxySocketBegin *proxy_socket_begin);

  ProxySocketBegin* GetProxySocketBegin(uint32 local_socket){
    ProxySocketBeginMap::iterator iter = proxy_socket_begin_map_.find(local_socket);
    if(iter == proxy_socket_begin_map_.end() || iter->second == NULL)
      return NULL;
    return proxy_socket_begin_map_[local_socket];
  };
  P2PConnectionImplementator *GetP2PConnectionImplementator() const{
    return p2p_connection_implementator_;
  }
  bool IsMe(const std::string remote_peer_name)const ;
  size_t CurrentConnectSize() const {return proxy_socket_begin_map_.size();}

  //p2p system management
  void CreateClientSocketConnection(uint32 socket,
    const talk_base::SocketAddress& addr);
  void ReplayClientSocketCreateSucceed(uint32 server_socket, 
    uint32 client_socket,const talk_base::SocketAddress &addr);
  
  void P2PSocketClose(uint32 socket, bool is_server);
  void MixP2PSocketClose(uint32 socket, bool is_server);
  void IndependentP2PSocketClose(uint32 socket, bool is_server);

  void P2PSocketCloseSucceed(uint32 socket, bool is_server);
  void P2PSocketConnectFailure(uint32 server_socket,
    uint32 client_socket);

  sigslot::signal1<talk_base::StreamInterface *> 
    SignalIndependentStreamRead;

private:
  void Destory();
  virtual void OnMessage(talk_base::Message *msg);
  //p2p system management
  bool ProceesSystemCommand(const char *data, uint16 len);
  void CloseP2PSocket(uint32 socket);
  void CloseP2PSocketSucceed(uint32 socket);
  void ConnectProxySocketFailure(uint32 socket);
  void IsAllProxySocketClosed();
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

  void OnIndependentStreamRead(talk_base::StreamInterface *stream);

  void SendCommand(talk_base::ByteBuffer *byte_buffer);

  void CloseAllProxySokcet(talk_base::StreamInterface *stream);
private:
  enum{
    P2P_SOCKET_START,
    P2P_SOCKET_CONNECTING_PEER,
    P2P_SOCKET_PEER_CONNECTED,
    P2P_SOCKET_CONNECTING_PROXY_SOCKET,
    P2P_SOCKET_PROXY_CONNECTED,
    P2P_SOCKET_CLOSING,
    P2P_SOCKET_CLOSED
  }independent_mode_state_;

  static const size_t BUFFER_SIZE = KBufferSize;
  typedef std::map<uint32, ProxySocketBegin*> ProxySocketBeginMap;
  typedef std::map<uint32, P2PProxySocket *> P2PProxySockets;
  ProxySocketBeginMap          proxy_socket_begin_map_;
  P2PProxySockets              p2p_proxy_sockets_;

  talk_base::FifoBuffer        *command_send_buffer_;

  P2PConnectionImplementator   *p2p_connection_implementator_;
  P2PConnectionManagement      *p2p_connection_management_;
  P2PSystemCommandFactory      *p2p_system_command_factory_;
  SocketTableManagement        *socket_table_management_;
  bool                         is_self_close;
  bool                         is_mix_data_mode_;
  bool                         independent_mode_connected_;

  talk_base::Thread            *signal_thread_;
  talk_base::Thread            *worker_thread_;
};

#endif // !PROXY_P2P_SESSION_H_
