/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/12/10      9:49
 * Filename : F:\GitHub\trunk\p2p_slotion\registerlocalserver.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: registerlocalserver
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
#include <map>

#include "talk/base/thread.h"
#include "talk/base/messagehandler.h"
#include "talk/base/sigslot.h"


struct ServerResources;
class ServerRegisterQuestion;
class P2PSourceManagement;

const std::string REGISTER_SUCCEED = "1 REGISTER SUCCEED";
const std::string REGISTER_FAILURE = "0 REGISTER FAILURE";

typedef std::map<const std::string, talk_base::SocketAddress> 
  OnlineServerResources;

class RegisterLocalServer 
  :public talk_base::MessageHandler,
  public sigslot::has_slots<>
{
  RegisterLocalServer(talk_base::Thread *signal_thread,
    talk_base::Thread *worker_thread);
  ~RegisterLocalServer();
private:
  void CreateRegisterServer_w();
  void NewServersRegister_s(ServerResources *server_resources);
  bool AddOnlineServerResource_w(ServerResources *server_resources);
  virtual void OnMessage(talk_base::Message *msg);

  //TCP Server Socket signal function
  virtual void OnNewConnect_w(talk_base::AsyncPacketSocket *server_socket,
    talk_base::AsyncPacketSocket *new_socket);

  //ServerRegisterQuestion
  virtual void OnSignalRegisterServerResources(
    ServerResources *server_resources);

  talk_base::Thread     *signal_thread_;
  talk_base::Thread     *worker_thread_;
  P2PSourceManagement   *p2p_source_management_;
  OnlineServerResources online_server_resource_;
};

class ServerRegisterQuestion : public sigslot::has_slots<>,
  public talk_base::MessageHandler
{
public:
  ServerRegisterQuestion(talk_base::AsyncPacketSocket *socket);
  sigslot::signal1<ServerResources*> SignalRegisterServerResources;


private:
  //Thread message function
  virtual void OnMessage(talk_base::Message *msg);

  //Signal function for AsyncPacketSocket
  void OnSignalReadPacket(talk_base::AsyncPacketSocket *socket, 
    const char *data, size_t len, const talk_base::SocketAddress &addr);
  void OnSignalReadyToSend(talk_base::AsyncPacketSocket *socket);
  void OnSignalClose(talk_base::AsyncPacketSocket *socket,int err);

  //help function
  bool ParseServerResources(ServerResources *server_resources,
    const char *data, size_t len);
  
  talk_base::AsyncPacketSocket *socket_;
  bool                          realy_to_send_;
};
