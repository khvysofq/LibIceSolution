/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/27      11:07
 * Filename : F:\GitHub\trunk\p2p_slotion\p2pconnectionmanagement.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: p2pconnectionmanagement
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
/*
  This class management all p2p connection that create by user question. This
  class used the Singleton Pattern that means there is only one p2p connection
  management object in this program.
  1. It create ProxyP2PSession object and destroy it. 
  2. It manage p2p ice class, 
*/

#ifndef P2P_CONNECTION_MANAGEMENT_H_
#define P2P_CONNECTION_MANAGEMENT_H_

#include <set>

#include "talk/base/sigslot.h"
#include "talk/base/socketaddress.h"
#include "mediator_pattern.h"
#include "p2pconnectionimplementator.h"

class P2PSourceManagement;
class ProxySocketBegin;
class ProxyP2PSession;



class P2PConnectionManagement : public sigslot::has_slots<>
{
public:
  ///////////////////////////////////////////////////////////////////////////
  //BUG NOTE (GuangleiHe, 11/28/2013)
  //Maybe there has a bug.
  //Because the order of the call with Initialize are Anarchy Reigns
  ///////////////////////////////////////////////////////////////////////////
  //You must add some source to p2p source management
  //after call the function. Because It instance PeerConnectionIce
  //object.
  void Initialize(talk_base::Thread *signal_thread,
    talk_base::Thread *worker_thread,bool mix_connect_mode = true);
  AbstractICEConnection *GetP2PICEConnection() const ;
  //When you call this function, you must be sure that the peer id 
  //is correct.
  //virtual bool Connect(ProxySocketBegin *proxy_socket_begin,
  //  const talk_base::SocketAddress& addr, ProxyP2PSession **proxy_p2p_session);
  virtual ProxyP2PSession *ConnectBySourceIde(const std::string &source_id,
    talk_base::SocketAddress *addr, bool *is_existed);


  //There
  //bool CreateP2PConnectionImplementator(const std::string &remote_jid,
  //  talk_base::StreamInterface *stream);
  
  bool CreateProxyP2PSession(const std::string &remote_jid,
    talk_base::StreamInterface *stream);
  void DeleteProxyP2PSession(ProxyP2PSession *proxy_p2p_session);

  ProxyP2PSession *WhetherThePeerIsExisted(const std::string remote_peer_name);
private:
  ProxyP2PSession *MixConnectBySourceIde(const std::string &source_id,
    talk_base::SocketAddress *addr, bool *is_existed);
  ProxyP2PSession *IdependentConnectBySourceIde(const std::string &source_id,
    talk_base::SocketAddress *addr, bool *is_existed);

  bool MixCreateProxyP2PSession(const std::string &remote_jid,
    talk_base::StreamInterface *stream);
  bool IdependentCreateProxyP2PSession(const std::string &remote_jid,
    talk_base::StreamInterface *stream);


  P2PConnectionImplementator *IsPeerConnected(int remote_peer_id);
  void OnStatesChange(StatesChangeType states_type);

  enum {
    MIX_DATA_MODE,
    INDEPENDENT_MODE
  } peer_connection_mode_;

private:

  typedef std::set<ProxyP2PSession *> ProxyP2PSessions;
  ProxyP2PSessions    proxy_p2p_sessions_;
  P2PSourceManagement *p2p_source_management_;

  AbstractICEConnection   *p2p_ice_connection_;
  talk_base::Thread       *signal_thread_;
  talk_base::Thread       *worker_thread_;

  //////////////////////////////////////////////////////////////////////////
public:
  //Singleton Pattern Function
  P2PConnectionManagement();
  static P2PConnectionManagement *Instance();
private:
  //Singleton Pattern variable
  static P2PConnectionManagement *p2p_connection_management_;
};

#endif // !P2P_CONNECTION_MANAGEMENT_H_
