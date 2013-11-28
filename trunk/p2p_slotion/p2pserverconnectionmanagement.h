/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/28      11:47
 * Filename : F:\GitHub\trunk\p2p_slotion\p2pserverconnectionmanagement.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: p2pserverconnectionmanagement
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

#ifndef P2P_SERVER_CONNECTION_MANAGEMENT_H_
#define P2P_SERVER_CONNECTION_MANAGEMENT_H_

#include "mediator_pattern.h"

class P2PSourceManagement;

class P2PServerConnectionManagement : public sigslot::has_slots<>
{
public:
  bool SignOutP2PServer();
  void SignInP2PServer(const talk_base::SocketAddress &server_addr);
  bool UpdatePeerInfor(const std::string &);
  void SetIceDataTunnel(AbstractICEConnection *ice_connection);
  //All resource are manage by P2PResourceManagement class
  //so delete those function.
  //void SetLocalName(const std::string &local_name);
  //const std::string GetLocalName() const;

  //sigslot::signal2<int,const std::string &> 
  //  SignalReceiveMessageFromRemotePeer;
  ////By default the p2p server connection object is PeerConnectionServer
  ////who define at peer_connection_server.h
  ////bool Initialize(AbstractP2PServerConnection *p2p_server_connection);
  //void OnSendMessageToRemotePeer(int peer_id, const std::string &msg);


  //void OnReceiveMessageFromRemotePeer(const std::string msg,int peer_id);
private:

  void OnServerStatesChange(StatesChangeType state_type);

  enum{
    NO_SERVER_OBJECT,
    SERVER_OBJECT_INITIALIZE,
    SERVER_CONNECTING,
    LOCAL_UPDATA_INFO,
    SERVER_CONNECTING_SUCCEED
  }state_;
  AbstractP2PServerConnection *p2p_server_connection_;
  P2PSourceManagement         *p2p_source_management_;

  //////////////////////////////////////////////////////////////////////////
public:
  //Singleton Pattern Function
  static P2PServerConnectionManagement *Instance();
private:
  P2PServerConnectionManagement();
  //Singleton Pattern variable
  static P2PServerConnectionManagement *p2p_server_connection_management_;
  //////////////////////////////////////////////////////////////////////////
  DISALLOW_EVIL_CONSTRUCTORS(P2PServerConnectionManagement);
};



#endif // !P2P_SERVER_CONNECTION_MANAGEMENT_H_
