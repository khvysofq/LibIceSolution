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
  talk_base::Thread *signal_thread,
  talk_base::Thread *worker_thread,
  bool mix_connect_mode)
{
  if(mix_connect_mode == true)
    peer_connection_mode_ = MIX_DATA_MODE;
  else
    peer_connection_mode_ = INDEPENDENT_MODE;

  signal_thread_ = signal_thread;
  worker_thread_ = worker_thread;

  p2p_ice_connection_ = new PeerConnectionIce(signal_thread_,worker_thread_);
}


ProxyP2PSession * P2PConnectionManagement::ConnectBySourceIde(
  const std::string &source_id, talk_base::SocketAddress *addr,
  const std::string &server_type,bool *is_existed)
{ 
  //1. Find the peer id by resource server address
  ProxyP2PSession *proxy_p2p_session = NULL;
  std::string remote_peer_name;
  const ServerResource * res = 
    p2p_source_management_->SreachPeerBySourceIde(source_id,server_type,
    &remote_peer_name);
  if(!res){
    return NULL;
  }
  if(!addr){
    LOG(LS_ERROR) << "the socket address can't set empty";
    return false;
  }
  addr->SetIP(res->server_ip_);
  addr->SetPort(res->server_port_);

  LOG_P2P(P2P_CONNECT_LOGIC) << "\t search peer name is" << remote_peer_name;
  //2. Whether the peer is connected
  proxy_p2p_session = WhetherThePeerIsExisted(remote_peer_name);

  //Existed
  if(proxy_p2p_session){
    LOG_P2P(P2P_CONNECT_LOGIC|BASIC_INFOR) << "The peer connection is existed";
    *is_existed = true;
    return proxy_p2p_session;
  }
  *is_existed = false;

  //connect this peer
  talk_base::StreamInterface *stream = 
    p2p_ice_connection_->ConnectionToRemotePeer(remote_peer_name);

  //Create ProxyP2PSession Object
  if(peer_connection_mode_ == MIX_DATA_MODE)
    proxy_p2p_session = new ProxyP2PSession(stream,
    remote_peer_name,signal_thread_,worker_thread_);
  else
    proxy_p2p_session = new ProxyP2PSession(stream,
    remote_peer_name,signal_thread_,worker_thread_,false);

  //Insert this session to session map
  proxy_p2p_sessions_.insert(proxy_p2p_session);


  LOG_P2P(P2P_CONNECT_LOGIC|BASIC_INFOR) << "------------------------------";
  LOG_P2P(P2P_CONNECT_LOGIC|BASIC_INFOR) <<"current proxy p2p session size = "
    << proxy_p2p_sessions_.size();
  LOG_P2P(P2P_CONNECT_LOGIC|BASIC_INFOR) << "------------------------------";

  return proxy_p2p_session;
}
//
//ProxyP2PSession *P2PConnectionManagement::MixConnectBySourceIde(
//  const std::string &source_id,talk_base::SocketAddress *addr, bool *is_existed)
//{
//  //1. Find the peer id by resource server address
//  ProxyP2PSession *proxy_p2p_session = NULL;
//  std::string remote_peer_name;
//  const ServerResource * res = 
//    p2p_source_management_->SreachPeerBySourceIde(source_id,&remote_peer_name);
//  if(!res){
//    return NULL;
//  }
//  if(!addr){
//    LOG(LS_ERROR) << "the socket address can't set empty";
//    return false;
//  }
//  addr->SetIP(res->server_ip_);
//  addr->SetPort(res->server_port_);
//
//  std::cout << __FUNCTION__ << "\t search peer name is" << remote_peer_name
//    << std::endl;
//  //2. Whether the peer is connected
//  proxy_p2p_session = WhetherThePeerIsExisted(remote_peer_name);
//
//  //Existed
//  if(proxy_p2p_session){
//    *is_existed = true;
//    return proxy_p2p_session;
//  }
//  *is_existed = false;
//
//  //connect this peer
//  talk_base::StreamInterface *stream = 
//    p2p_ice_connection_->ConnectionToRemotePeer(remote_peer_name);
//
//  //Create ProxyP2PSession Object
//  proxy_p2p_session = new ProxyP2PSession(stream,remote_peer_name);
//  //Insert this session to session map
//  proxy_p2p_sessions_.insert(proxy_p2p_session);
//
//  return proxy_p2p_session;
//}
//
//ProxyP2PSession *P2PConnectionManagement::IdependentConnectBySourceIde(
//  const std::string &source_id,talk_base::SocketAddress *addr, bool *is_existed)
//{
//  *is_existed = false;
//  //1. Find the peer id by resource server address
//  ProxyP2PSession *proxy_p2p_session = NULL;
//  std::string remote_peer_name;
//  const ServerResource * res = 
//    p2p_source_management_->SreachPeerBySourceIde(source_id,&remote_peer_name);
//  if(!res){
//    return NULL;
//  }
//  if(!addr){
//    LOG(LS_ERROR) << "the socket address can't set empty";
//    return false;
//  }
//  addr->SetIP(res->server_ip_);
//  addr->SetPort(res->server_port_);
//
//  std::cout << __FUNCTION__ << "\t search peer name is" << remote_peer_name
//    << std::endl;
//
//  //connect this peer
//  talk_base::StreamInterface *stream = 
//    p2p_ice_connection_->ConnectionToRemotePeer(remote_peer_name);
//
//  //Create ProxyP2PSession Object
//  proxy_p2p_session = new ProxyP2PSession(stream,remote_peer_name,false);
//  //Insert this session to session map
//  proxy_p2p_sessions_.insert(proxy_p2p_session);
//
//  return proxy_p2p_session;
//}


ProxyP2PSession *P2PConnectionManagement::WhetherThePeerIsExisted(
  const std::string remote_peer_name)
{
  //TEST CONNECT
  if(peer_connection_mode_ == INDEPENDENT_MODE)
    return NULL;

  for(ProxyP2PSessions::iterator iter = proxy_p2p_sessions_.begin();
    iter != proxy_p2p_sessions_.end(); iter++)
  {
    if((*iter)->IsMe(remote_peer_name)){
      if(peer_connection_mode_ == MIX_DATA_MODE){
        LOG_P2P(P2P_CONNECT_LOGIC) << "MIX_DATA_MODE The session existed";
        return (*iter);
      }
      else{
        if((*iter)->CurrentConnectSize() == 0){
          LOG_P2P(P2P_CONNECT_LOGIC) 
            << "INDEPENDENT_MODE The session existed";
          return (*iter);
        }
      }
    }
  }
  LOG_P2P(P2P_CONNECT_LOGIC) << "The session not existed";
  return NULL;
}

AbstractICEConnection *P2PConnectionManagement::GetP2PICEConnection() const{
  return p2p_ice_connection_;
}

bool P2PConnectionManagement::CreateProxyP2PSession(
  const std::string &remote_jid,talk_base::StreamInterface *stream)
{
  LOG_P2P(P2P_CONNECT_LOGIC) << "\t Start Create New Session";
  //1. Check the connection is existed.
  ProxyP2PSession *proxy_p2p_session = WhetherThePeerIsExisted(
    remote_jid);
  if(proxy_p2p_session){
    //never rech here
    LOG(LS_ERROR) << "The peer is existed";
    ASSERT(0);
    return false;
  }

  ////2. create p2p connection implementator
  //P2PConnectionImplementator *p2p_connection_implementator =
  //  new P2PConnectionImplementator(remote_jid,stream);

  //3. create proxy p2p session
  if(peer_connection_mode_ == MIX_DATA_MODE)
    proxy_p2p_session = new ProxyP2PSession(stream,remote_jid,
    signal_thread_,worker_thread_);
  else
    proxy_p2p_session = new ProxyP2PSession(stream,remote_jid,
    signal_thread_,worker_thread_,false);


  //4. register this proxy
  ///////////////////////////////////////////////////////////////////////////
  //BUSINESS LOGIC NOTE (GuangleiHe, 12/2/2013)
  //The ProxySocketBegin is repetition, RegisterProxySocket called 
  //maybe repetitive
  ///////////////////////////////////////////////////////////////////////////

  //5. Then add this proxy p2p session to ProxyP2PSessions
  proxy_p2p_sessions_.insert(proxy_p2p_session);


  LOG_P2P(P2P_CONNECT_LOGIC) << "------------------------------";
  LOG_P2P(P2P_CONNECT_LOGIC) << __FUNCTION__ <<"current proxy p2p session size = " 
    << proxy_p2p_sessions_.size();
  LOG_P2P(P2P_CONNECT_LOGIC) << "------------------------------";

  return true;
}
//
//bool P2PConnectionManagement::MixCreateProxyP2PSession(
//  const std::string &remote_jid,talk_base::StreamInterface *stream)
//{
//  std::cout << __FUNCTION__ << "\t Start Create New Session"
//    << std::endl;
//  //1. Check the connection is existed.
//  ProxyP2PSession *proxy_p2p_session = WhetherThePeerIsExisted(
//    remote_jid);
//  if(proxy_p2p_session){
//    //never rech here
//    LOG(LS_ERROR) << "The peer is existed";
//    ASSERT(0);
//    return false;
//  }
//
//  ////2. create p2p connection implementator
//  //P2PConnectionImplementator *p2p_connection_implementator =
//  //  new P2PConnectionImplementator(remote_jid,stream);
//
//  //3. create proxy p2p session
//  proxy_p2p_session = new ProxyP2PSession(stream,remote_jid);
//
//  //4. register this proxy
//  ///////////////////////////////////////////////////////////////////////////
//  //BUSINESS LOGIC NOTE (GuangleiHe, 12/2/2013)
//  //The ProxySocketBegin is repetition, RegisterProxySocket called 
//  //maybe repetitive
//  ///////////////////////////////////////////////////////////////////////////
//
//  //5. Then add this proxy p2p session to ProxyP2PSessions
//  proxy_p2p_sessions_.insert(proxy_p2p_session);
//  return true;
//}
//
//bool P2PConnectionManagement::IdependentCreateProxyP2PSession(
//  const std::string &remote_jid,talk_base::StreamInterface *stream)
//{
//  std::cout << __FUNCTION__ << "\t Start Create New Session"
//    << std::endl;
//  ///////////////////////////////////////////////////////////////////////////
//  //BUSINESS LOGIC NOTE (GuangleiHe, 12/2/2013)
//  //The ProxySocketBegin is repetition, RegisterProxySocket called 
//  //maybe repetitive
//  ///////////////////////////////////////////////////////////////////////////
//
//  //Then add this proxy p2p session to ProxyP2PSessions
//  proxy_p2p_sessions_.insert(new ProxyP2PSession(stream,remote_jid,false));
//  return true;
//}


void P2PConnectionManagement::DeleteProxyP2PSession(
  ProxyP2PSession *proxy_p2p_session)
{
  //1. Find this proxy_p2p_session
  ProxyP2PSessions::iterator iter = proxy_p2p_sessions_.find(
    proxy_p2p_session);
  if(iter == proxy_p2p_sessions_.end()){
    LOG(LS_ERROR) << "Can't Find this session";
    return ;
  }
  delete proxy_p2p_session;
  proxy_p2p_sessions_.erase(iter);
  LOG_P2P(P2P_CONNECT_LOGIC) << "------------------------------";
  LOG_P2P(P2P_CONNECT_LOGIC) <<"current proxy p2p session size = " 
    << proxy_p2p_sessions_.size();
  LOG_P2P(P2P_CONNECT_LOGIC) << "------------------------------";
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


//////////////////////////////////////////////////////////////////////////
