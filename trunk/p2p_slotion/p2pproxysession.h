/*
* p2p solution
* Copyright 2013, VZ Inc.
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
#ifndef P2P_PROXY_SESSION_H_
#define P2P_PROXY_SESSION_H_

#include <queue>

#include "talk/base/thread.h"
#include "talk/base/basictypes.h"
#include "talk/base/bytebuffer.h"
#include "defaults.h"

class P2PProxySocket;
class P2PProxyStartSocket;
class P2PProxyEndSocket;
class P2PConnectionImplementator;
class P2PConnectionManagement;
class SocketTableManagement;

struct P2PCommandData{
  P2PCommandData(const char *data, uint16 len);
  ~P2PCommandData();
  char *data_;
  uint16 len_;
};

typedef std::queue<P2PCommandData*> CommandDataBuffers;

//////////////////////////////////////////////////////////////////////////
//P2PProxySession
//////////////////////////////////////////////////////////////////////////
class P2PProxySession : public sigslot::has_slots<>,
  public talk_base::MessageHandler
{
public:
  P2PProxySession(talk_base::StreamInterface *stream,
    const std::string &remote_peer_name,
    talk_base::Thread *signal_thread,
    talk_base::Thread *worker_thread,
    bool is_mix_data_mode = true);
  ~P2PProxySession();

  virtual bool RegisterP2PProxySocket(P2PProxySocket *p2p_proxy_socket,
    bool is_server);
  void ReleaseP2PProxySocket(P2PProxySocket *p2p_proxy_socket);
  void RegisterNewSocketTable(uint32 local_socket, uint32 remote_socket);
  void DeleteSocketTable(uint32 local_socket);

  P2PProxySocket *GetP2PProxySocket(uint32 socket_number){
    P2PProxySockets::iterator iter = p2p_proxy_sockets_.find(socket_number);
    if(iter == p2p_proxy_sockets_.end() || iter->second == NULL)
      return NULL;
    return iter->second;
  }

  P2PConnectionImplementator *GetP2PConnectionImplementator() const{
    return p2p_connection_implementator_;
  }
  bool IsMe(const std::string remote_peer_name)const ;
  size_t CurrentConnectSize() const {return p2p_proxy_sockets_.size();}

  void SendSystemCommand(const char *p, size_t len);

  sigslot::signal1<talk_base::StreamInterface *> SignalIndependentStreamRead;
  sigslot::signal1<P2PProxySession *> SignalPeerConnectSucceed;
  sigslot::signal1<P2PProxySession *> SignalPeerConnectFailure;
protected:
  bool ParseCommand(const char *data, uint16 len,
                                           uint32 *p2p_system_command_type,
                                           uint32 *server_socket,
                                           uint32 *client_socket,
                                           uint32 *client_connection_ip,
                                           uint16 *client_connection_port);
  void Destory();
  virtual void OnMessage(talk_base::Message *msg);
  //p2p system management
  virtual bool ProceesSystemCommand(const char *data, uint16 len) = 0;
  void IsAllProxySocketClosed();
  //
  bool  RunSocketProccess(uint32 socket, SocketType socket_type,
    const char *data, uint16 len);
  //
  void OnP2PConnectSucceed(talk_base::StreamInterface *stream);
  void OnStreamRead(uint32 socket_number,SocketType socket_type,
    const char *data, uint16 len);
  //All proxy get the close events.
  void OnStreamClose(talk_base::StreamInterface *stream);
  //All proxy get the write events.
  void OnStreamWrite(talk_base::StreamInterface *stream);

  void OnIndependentStreamRead(talk_base::StreamInterface *stream);


  void CloseAllProxySokcet(talk_base::StreamInterface *stream);
public:

  enum{
    P2P_CONNECTING,
    P2P_CONNECTED,
    P2P_CLOSING,
    P2P_CLOSE
  }state_;

  static const size_t BUFFER_SIZE = KBufferSize;
  typedef std::map<uint32,P2PProxySocket *> P2PProxySockets;
  P2PProxySockets              p2p_proxy_sockets_;

  CommandDataBuffers           command_data_buffers_;

  P2PConnectionImplementator   *p2p_connection_implementator_;
  P2PConnectionManagement      *p2p_connection_management_;
  SocketTableManagement        *socket_table_management_;
  bool                         is_self_close;
  bool                         is_mix_data_mode_;
  bool                         independent_mode_connected_;

  talk_base::Thread            *signal_thread_;
  talk_base::Thread            *worker_thread_;
};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
class P2PProxyServerSession : public P2PProxySession{
public:
  P2PProxyServerSession(talk_base::StreamInterface *stream,
    const std::string &remote_peer_name,
    talk_base::Thread *signal_thread,
    talk_base::Thread *worker_thread,
    bool is_mix_data_mode = true);
private:
  //p2p system management
  virtual bool ProceesSystemCommand(const char *data, uint16 len);
};

class P2PProxyClientSession : public P2PProxySession{
public:
  P2PProxyClientSession(talk_base::StreamInterface *stream,
    const std::string &remote_peer_name,
    talk_base::Thread *signal_thread,
    talk_base::Thread *worker_thread,
    bool is_mix_data_mode = true);
private:
  virtual bool ProceesSystemCommand(const char *data, uint16 len);

};

#endif // !P2P_PROXY_SESSION_H_