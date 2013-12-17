/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/22      9:47
 * Filename : F:\GitHub\trunk\p2p_slotion\peer_connection_ice.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: peer_connection_ice
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

#ifndef PEER_CONNECTION_ICE_H_
#define PEER_CONNECTION_ICE_H_

#include "talk/base/thread.h"
#include "talk/p2p/base/sessionmanager.h"
#include "talk/p2p/client/httpportallocator.h"
#include "talk/session/tunnel/tunnelsessionclient.h"

#include "mediator_pattern.h"

class SendDataBuffer;
class P2PSourceManagement;
class P2PConnectionManagement;
class ProxySocketBegin;


class PeerConnectionIce
  :public AbstractICEConnection,
  public talk_base::MessageHandler
{
public:     //user interface
  PeerConnectionIce(talk_base::Thread *worker_thread,
    talk_base::Thread *signal_thread);
  ~PeerConnectionIce();
  virtual talk_base::StreamInterface *ConnectionToRemotePeer(
    const std::string remote_peer_name);

private:    //p2p server function and some help function
  void Destroy();
  //receive messages that from remote peer by p2p server
  void OnReceiveMessageFromRemotePeer(const std::string msg,int peer_id);
private:  //libjingle system function
  // implements the MessageHandler interface
  void OnMessage(talk_base::Message* msg);
private:    //ICE part function
  enum {REMOTE_PEER_MESSAGE};
  //interior message that start college transport information
  void OnRequestSignaling();

  //interior message that should send to remote peer by p2p server
  void OnOutgoingMessage(cricket::SessionManager* manager,
    const buzz::XmlElement* stanza);

  // Accept the tunnel when it arrives and wire up the stream.
  void OnIncomingTunnel(cricket::TunnelSessionClient* client,
    buzz::Jid jid, std::string description,
    cricket::Session* session);
  //data event

private:    //ICE part member
  talk_base::Thread               *worker_thread_;
  talk_base::Thread               *signal_thread_;
  talk_base::BasicNetworkManager  *basic_network_manager_;
  cricket::SessionManager         *session_manager_;
  cricket::TunnelSessionClient    *tunnel_session_client_;
  cricket::BasicPortAllocator     *basic_prot_allocator_;
  
private:    //p2p server member
  P2PSourceManagement             *p2p_source_management_;
  P2PConnectionManagement         *p2p_connection_management_;
};





#endif