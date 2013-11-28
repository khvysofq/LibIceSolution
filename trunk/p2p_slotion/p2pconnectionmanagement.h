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

#ifndef P2P_CONNECTION_MANAGEMENT_H_
#define P2P_CONNECTION_MANAGEMENT_H_

#include <set>

#include "talk/base/sigslot.h"
#include "talk/base/socketaddress.h"
#include "p2psourcemanagement.h"
#include "mediator_pattern.h"

class P2PConnectionManagement : public sigslot::has_slots<>
{
public:
  virtual int Connect(talk_base::SocketAddress &addr);
private:
  AbstractVirtualNetwork *IsPeerConnected(int peer_id);
private:

  typedef std::map<int,AbstractVirtualNetwork *> P2PConnections;
  P2PConnections      current_connect_peer_;
  P2PSourceManagement *p2p_source_management_;
public:
  //Singleton Pattern Function
  P2PConnectionManagement(){}
  static P2PConnectionManagement *Instance();
private:
  //Singleton Pattern variable
  static P2PConnectionManagement *p2p_connection_management_;
};


#endif // !P2P_CONNECTION_MANAGEMENT_H_
