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

//////////////////////////////////////////////////////////////////////////
//Singleton Pattern Function
P2PConnectionManagement *P2PConnectionManagement::p2p_connection_management_ = NULL;

P2PConnectionManagement *P2PConnectionManagement::Instance(){
  if(!p2p_connection_management_)
    p2p_connection_management_ = new P2PConnectionManagement();
  return p2p_connection_management_;
}
//////////////////////////////////////////////////////////////////////////


int P2PConnectionManagement::Connect(talk_base::SocketAddress &addr){
  //0. Get the instance of the P2PSourceManagement
  p2p_source_management_ = P2PSourceManagement::Instance();

  //1. whether the peer exists.
  int peer_id = p2p_source_management_->SreachPeerByServerResource(addr);
  if(peer_id == 0){
    LOG(LS_ERROR) << "Can't found the server addr";
    return -1;
  }
  //2. Whether the peer is connected.
  AbstractVirtualNetwork *virtual_network = IsPeerConnected(peer_id);
  if(!virtual_network){
    LOG(LS_VERBOSE) << "The Peer Can't connected";
  }
  //3. Create new connect request

  //4. return the virtual network object.
  return 0;
}

AbstractVirtualNetwork *P2PConnectionManagement::IsPeerConnected(int peer_id){
  P2PConnections::iterator iter = current_connect_peer_.find(peer_id);
  if(iter != current_connect_peer_.end())
    return iter->second;
  return NULL;
}