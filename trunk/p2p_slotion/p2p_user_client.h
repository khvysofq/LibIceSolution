/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/27      11:52
 * Filename : F:\GitHub\trunk\p2p_slotion\p2p_user_client.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: p2p_user_client
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
#ifndef P2P_SOLUTION_P2P_USER_CLIENT_H_
#define P2P_SOLUTION_P2P_USER_CLIENT_H_

#include "talk/base/sigslot.h"
#include "talk/base/messagehandler.h"
#include "talk/base/thread.h"
#include "talk/base/stream.h"

#include "defaults.h"
#include "mediator_pattern.h"
class P2PConnectionManagement;
class P2PSourceManagement;
class P2PServerConnectionManagement;

//const talk_base::SocketAddress ServerAddr("192.168.1.225",8888);
//const talk_base::SocketAddress ServerAddr("192.168.1.116",8888);
const talk_base::SocketAddress ServerAddr("42.121.127.71",8888);
const int RECEIVE_BUFFER_LENGTH   = 1024 * 16;

class P2PUserClient :public sigslot::has_slots<>,
  public talk_base::MessageHandler
{
public:
  P2PUserClient(talk_base::Thread *signal_thread,
    talk_base::Thread *worker_thread);
  ~P2PUserClient();
  //user interface
  void Initiatlor();
  void StartRun();
  void ConnectionToPeer(int peer_id);

  
  // implements the MessageHandler interface
  void OnMessage(talk_base::Message* msg);

  bool                        is_peer_connect_;
private:
  const talk_base::SocketAddress ReadingConfigureFile(const std::string &config_file,
    const std::string &random_string);

  talk_base::Thread           *worker_thread_;
  talk_base::Thread           *signal_thread_;
  bool                        initiator_;
  char                        *receive_buffer_;
  talk_base::SocketAddress    p2p_server_addr_;

  P2PSourceManagement             *p2p_source_management_;
  P2PServerConnectionManagement   *p2p_server_connection_management_;
  P2PConnectionManagement         *p2p_connection_management_;
};

#endif