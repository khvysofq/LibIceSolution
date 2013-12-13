/*
* p2p solution
* Copyright 2013, VZ Inc.
* 
* Author   : GuangLei He
* Email    : guangleihe@gmail.com
* Created  : 2013/12/10      18:50
* Filename : F:\GitHub\trunk\LibRegitster\libregister.cpp
* File path: F:\GitHub\trunk\LibRegitster
* File base: libregister
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

#include "libregister.h"

#include <string>
#include <iostream>
#include <sstream>

#include "talk/base/thread.h"
#include "talk/base/socketaddress.h"
#include "talk/p2p/base/basicpacketsocketfactory.h"
#include "talk/base/asyncpacketsocket.h"
#include "talk/base/json.h"

const std::string SERVER_NAME           = "SERVER_NAME";
const std::string SERVER_IP             = "SERVER_IP";
const std::string SERVER_PORT           = "SERVER_PORT";
const std::string SOURCE_IDE            = "SOURCE_IDE";
const std::string PEER_JID              = "PEER_JID";
const std::string SERVER_RESOURCE_ARRAY = "SERVER_RESOURCE_ARRAY";

const int MESSAGE_CREATE_SOCKET         = 0;

class RegisterMenagement :public talk_base::MessageHandler {

public:
  bool InitRegister(const char *peer_ip, int register_port,int update_port,
    const char *server_type, const char *server_ip,
    int server_port, const char *idetifier){
      
      worker_thread_->Start();
      is_init_ = true;
      
      peer_register_addr_.SetIP(peer_ip);
      peer_register_addr_.SetPort(register_port);

      peer_update_addr_.SetIP(peer_ip);
      peer_update_addr_.SetPort(update_port);

      local_server_addr_.SetIP(server_ip);
      local_server_addr_.SetPort(server_port);

      server_type_ = server_type;
      server_idetifier_ = idetifier;

      register_string_ = ParseInformationToString();
  }

private:
  const std::string ParseInformationToString();
  virtual void OnMessage(talk_base::Message *msg);

  bool                     is_init_;
  talk_base::SocketAddress peer_register_addr_;
  talk_base::SocketAddress peer_update_addr_;
  talk_base::SocketAddress local_server_addr_;
  std::string              server_idetifier_;
  std::string              server_type_;
  std::string              register_string_;

  talk_base::Thread        *signal_thread_;
  talk_base::Thread        *worker_thread_;
  //////////////////////////////////////////////////////////////////////////
private:
  RegisterMenagement();
  static RegisterMenagement * register_management_;
public:
  static RegisterMenagement *Instance();
};
//////////////////////////////////////////////////////////////////////////
RegisterMenagement *RegisterMenagement::register_management_ = NULL;

RegisterMenagement *RegisterMenagement::Instance(){
  if(!register_management_)
    register_management_ = new RegisterMenagement();
  return register_management_;
}
//////////////////////////////////////////////////////////////////////////

RegisterMenagement::RegisterMenagement():is_init_(false){
  signal_thread_ = talk_base::Thread::Current();
  worker_thread_ = new talk_base::Thread();
};

const std::string RegisterMenagement::ParseInformationToString()
{
  std::stringstream joson_string;
  joson_string << "{";

  //Add server resource Array identifier
  joson_string << "\"" << SERVER_RESOURCE_ARRAY << "\"" << ": [";

  joson_string << "{" << "\"" << SERVER_NAME << "\" : "
    << "\"" << server_type_<< "\","
    << "\"" << SERVER_IP << "\" : "
    << "\"" << local_server_addr_.ipaddr().ToString()<< "\","
    << "\"" << SERVER_PORT << "\" : "
    << local_server_addr_.port() << ","
    << "\"" << SOURCE_IDE << "\" : "
    << "\"" << server_idetifier_<< "\""
    << "}";
  joson_string << "]";
  joson_string << "}";
  return joson_string.str();
}

void RegisterMenagement::OnMessage(talk_base::Message *msg){

}

//////////////////////////////////////////////////////////////////////////
bool LoginPeer(const char *peer_ip, int register_port,int update_port,
               const char *server_type, const char *server_ip,
               int server_port, const char *idetifier)
{
  RegisterMenagement *register_management = RegisterMenagement::Instance(); 

  register_management->InitRegister(peer_ip,register_port,update_port,
    server_type,server_ip,server_port,idetifier);

}
//////////////////////////////////////////////////////////////////////////