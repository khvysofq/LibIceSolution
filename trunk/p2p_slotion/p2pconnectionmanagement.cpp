/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/27      11:08
 * Filename : F:\GitHub\trunk\p2p_slotion\p2pconnectionmanagement.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: p2pconnectionmanagement
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

#include "p2pconnectionmanagement.h"
#include "p2psourcemanagement.h"
#include "peer_connection_ice.h"
#include "proxyp2psession.h"
#include "proxysocketmanagement.h"

//////////////////////////////////////////////////////////////////////////
//Singleton Pattern Function
P2PConnectionManagement *P2PConnectionManagement::p2p_connection_management_ = NULL;

P2PConnectionManagement *P2PConnectionManagement::Instance(){
  if(!p2p_connection_management_)
    p2p_connection_management_ = new P2PConnectionManagement();
  return p2p_connection_management_;
}
//////////////////////////////////////////////////////////////////////////

P2PConnectionManagement::P2PConnectionManagement()
{
  p2p_source_management_ = P2PSourceManagement::Instance();
}

void P2PConnectionManagement::Initialize(
  talk_base::Thread *signal_thread,talk_base::Thread *worker_thread)
{
  signal_thread_ = signal_thread;
  worker_thread_ = worker_thread;

  p2p_ice_connection_ = new PeerConnectionIce(worker_thread_,signal_thread_);

}

bool P2PConnectionManagement::Connect(ProxySocketBegin *proxy_socket_begin,
                                     const talk_base::SocketAddress& addr,
                                     ProxyP2PSession **proxy_p2p_session)
{
  //1. Find the peer id by resource server address
  std::string remote_peer_name = 
    p2p_source_management_->SreachPeerByServerResource(addr);
  if(remote_peer_name.empty())
    return false;

  //2. Whether the peer is connected
  *proxy_p2p_session = WhetherThePeerIsExisted(remote_peer_name);
  
  //Existed
  if(*proxy_p2p_session){
    return true;
  }

  //else
  int remote_peer_id = 
    p2p_source_management_->GetRemotePeerIdByPeerName(remote_peer_name);

  //connect this peer
  p2p_ice_connection_->ConnectionToRemotePeer(proxy_socket_begin,remote_peer_id);

  return true;
}

bool P2PConnectionManagement::ConnectBySourceIde(ProxySocketBegin *proxy_socket_begin,
                                                 const std::string &source_id, 
                                                 ProxyP2PSession **proxy_p2p_session,
                                                 talk_base::SocketAddress *addr)
{
  //1. Find the peer id by resource server address
  std::string remote_peer_name;
  const ServerResource * res = 
    p2p_source_management_->SreachPeerBySourceIde(source_id,&remote_peer_name);
  if(!res)
    return false;
  if(!addr){
    LOG(LS_ERROR) << "the socket address can't set empty";
    return false;
  }
  addr->SetIP(res->server_ip_);
  addr->SetPort(res->server_port_);

  //2. Whether the peer is connected
  *proxy_p2p_session = WhetherThePeerIsExisted(remote_peer_name);

  //Existed
  if(*proxy_p2p_session){
    return true;
  }

  //else
  int remote_peer_id = 
    p2p_source_management_->GetRemotePeerIdByPeerName(remote_peer_name);

  //connect this peer
  p2p_ice_connection_->ConnectionToRemotePeer(proxy_socket_begin,remote_peer_id);

  return true;
}

ProxyP2PSession *P2PConnectionManagement::WhetherThePeerIsExisted(
  const std::string remote_peer_name)
{
  for(ProxyP2PSessions::iterator iter = proxy_p2p_sessions_.begin();
    iter != proxy_p2p_sessions_.end(); iter++)
  {
    if((*iter)->IsMe(remote_peer_name))
      return (*iter);
  }
  return NULL;
}

AbstractICEConnection *P2PConnectionManagement::GetP2PICEConnection() const{
  return p2p_ice_connection_;
}

bool P2PConnectionManagement::CreateProxyP2PSession(
  ProxySocketBegin *proxy_socket_begin,const std::string &remote_jid,
  talk_base::StreamInterface *stream)
{
  //1. Check the connection is existed.
  ProxyP2PSession *proxy_p2p_session = WhetherThePeerIsExisted(
    remote_jid);
  if(proxy_p2p_session){
    LOG(LS_WARNING) << "The peer is existed";
    if(proxy_socket_begin)
      proxy_socket_begin->OnP2PPeerConnectSucceed(proxy_p2p_session);
    return false;
  }

  //2. create p2p connection implementator
  P2PConnectionImplementator *p2p_connection_implementator =
    new P2PConnectionImplementator(remote_jid,stream);

  //3. create proxy p2p session
  proxy_p2p_session = new ProxyP2PSession(p2p_connection_implementator);

  //4. register this proxy
  ///////////////////////////////////////////////////////////////////////////
  //BUSINESS LOGIC NOTE (GuangleiHe, 12/2/2013)
  //
  ///////////////////////////////////////////////////////////////////////////
  if(proxy_socket_begin)
    proxy_p2p_session->RegisterProxySocket(proxy_socket_begin);

  //5. Then add this proxy p2p session to ProxyP2PSessions
  proxy_p2p_sessions_.insert(proxy_p2p_session);
  return true;
}

//bool P2PConnectionManagement::CreateP2PConnectionImplementator(
//  const std::string &remote_jid,talk_base::StreamInterface *stream)
//{
//  //1. Find is connected
//  for(P2PConnections::iterator iter = current_connect_peer_.begin();
//    iter != current_connect_peer_.end(); iter++){
//    if((*iter)->IsMe(remote_jid)){
//      LOG(LS_ERROR) << "The remote peer is connected, you didn't create.";
//      return false;
//    }
//  }
//
//  P2PConnectionImplementator *p2p_connection =
//    new P2PConnectionImplementator(remote_jid,stream);
//
//  current_connect_peer_.insert(p2p_connection);
//
//  return true;
//}


P2PConnectionImplementator *P2PConnectionManagement::IsPeerConnected(
  int remote_peer_id)
{
  //1. find remote peer name, used remote peer id by P2PSourceManagement
  const std::string remote_peer_name = 
    p2p_source_management_->GetRemotePeerNameByPeerId(remote_peer_id);
  return NULL;
}

void P2PConnectionManagement::OnStatesChange(StatesChangeType states_type){
  switch(states_type){
  case STATES_ICE_START_PEER_CONNECTION:
    {
      std::cout << "\tSTATES_ICE_START_PEER_CONNECTION" << std::endl;
      break;
    }
  case STATES_ICE_TUNNEL_SEND_DATA:
    {
      std::cout << "\tSTATES_ICE_TUNNEL_SEND_DATA" << std::endl;
      break;
    }
  case STATES_ICE_TUNNEL_CLOSED:
    {
      std::cout << "\tSTATES_ICE_TUNNEL_CLOSED" << std::endl;
      break;
    }
  }
}

//////////////////////////////////////////////////////////////////////////
