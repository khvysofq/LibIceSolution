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
#include "talk/base/stream.h"
#include "defaults.h"
//#include "proxyserverfactory.h"


class SocketTableManagement;
class P2PSystemCommandFactory;
class P2PConnectionManagement;
class ProxyP2PSession;
class P2PConnectionImplementator;

class ProxySocketBegin : public sigslot::has_slots<>,
  public talk_base::MessageHandler
{
public:
  ProxySocketBegin(talk_base::AsyncSocket *int_socket = NULL);
  void Destory();
  ~ProxySocketBegin();

  intptr_t GetSocketNumber() const{return (intptr_t)(int_socket_.get());}

  //virtual bool StartConnect(const talk_base::SocketAddress& addr);
  virtual bool StartConnectBySourceIde(const std::string &source,
    const std::string &server_type);
  virtual void OnP2PPeerConnectSucceed(ProxyP2PSession *proxy_p2p_session);
  virtual void OnP2PSocketConnectSucceed(ProxyP2PSession *proxy_p2p_session);
  
  //p2p socket signal function
  virtual void OnP2PRead(const char *data, uint16 len);
  bool IsMe(intptr_t socket);

  virtual void OnIndependentRead(talk_base::StreamInterface *stream);

  virtual void OnP2PWrite(talk_base::StreamInterface *stream);
  virtual void OnP2PClose(talk_base::StreamInterface *stream);
  virtual void OnOtherSideSocketCloseSucceed(
    talk_base::StreamInterface *stream);
  virtual void OnProxySocketConnectFailure(
    talk_base::StreamInterface *stream);
  
protected:

  virtual void CloseP2PSocket();
  virtual void SendCloseSocketSucceed();
  virtual void InternalSocketError(talk_base::AsyncSocket* socket, int err);

  //Internal Socket Signal function
  virtual void OnInternalRead(talk_base::AsyncSocket* socket);
  virtual void OnInternalWrite(talk_base::AsyncSocket* socket);
  virtual void OnInternalClose(talk_base::AsyncSocket* socket, int err);

  //Data Change Function
  virtual void ReadSocketDataToBuffer(talk_base::AsyncSocket *socket,
    talk_base::FifoBuffer *buffer);
  virtual void ReadP2PDataToBuffer(const char *data, uint16 len, 
    talk_base::FifoBuffer *buffer);

  virtual void ReadP2PStreamDatatoBuffer(
    talk_base::StreamInterface *stream,talk_base::FifoBuffer *buffer);

  virtual void WriteBufferDataToSocket(talk_base::AsyncSocket *socket,
    talk_base::FifoBuffer *buffer);
  virtual void WriteBufferDataToP2P(talk_base::FifoBuffer *buffer);
  virtual void OnMessage(talk_base::Message* msg);

  void SetProxyP2PSession(ProxyP2PSession *proxy_p2p_session);
protected:
  enum{
    P2P_SOCKET_START,
    P2P_SOCKET_CONNECTING_PEER,
    P2P_SOCKET_PEER_CONNECTED,
    P2P_SOCKET_CONNECTING_PROXY_SOCKET,
    P2P_SOCKET_PROXY_CONNECTED,
    P2P_SOCKET_CLOSING,
    P2P_SOCKET_CLOSED
  } p2p_socket_state_;

  enum{
    INT_SCOKET_START,
    INT_SOCKET_CONNECTING,
    INT_SOCKET_CONNECTED,
    INT_SOCKET_CLOSED
  }int_socket_state_;

  talk_base::scoped_ptr<talk_base::AsyncSocket> int_socket_;
  talk_base::FifoBuffer out_buffer_;
  talk_base::FifoBuffer in_buffer_;

  SocketTableManagement        *socket_table_management_;
  P2PSystemCommandFactory      *p2p_system_command_factory_;
  P2PConnectionManagement      *p2p_connection_management_;
  talk_base::SocketAddress      remote_peer_addr_;
//private:
  //bool                          internal_date_wait_receive_;
  ///////////////////////////////////////////////////////////////////////////
  //BUSINESS LOGIC NOTE (GuangleiHe, 12/2/2013)
  //remove the is_server_ control in a function to set by derived classes.
  ///////////////////////////////////////////////////////////////////////////
  bool                          is_server_;
  ProxyP2PSession              *proxy_p2p_session_;
  P2PConnectionImplementator   *p2p_connection_implementator_;
  talk_base::Thread            *current_thread_;
  DISALLOW_EVIL_CONSTRUCTORS(ProxySocketBegin);
};

#endif
