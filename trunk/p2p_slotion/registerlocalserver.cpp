/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/12/10      9:49
 * Filename : F:\GitHub\trunk\p2p_slotion\registerlocalserver.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: registerlocalserver
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

#include "registerlocalserver.h"

#include "talk/p2p/base/basicpacketsocketfactory.h"
#include "talk/base/asyncpacketsocket.h"
#include "talk/base/json.h"
#include "p2psourcemanagement.h"

//////////////////////////////////////////////////////////////////////////
//JSON String


//////////////////////////////////////////////////////////////////////////
const int CREATE_REGISTER_SERVER        = 0;
const int NEW_REGISTER_SERVER_RESOURCE  = 1;


const talk_base::SocketAddress KLocalSocketAddr();
const int DEFUALT_PORT  = 9999;
const int CHECOUT_PORT  = 9998;

struct MyMessageData : public talk_base::MessageData{
  MyMessageData(ServerResources *server_resources):
    server_resources_(server_resources){}
    ServerResources *server_resources_;
};

//////////////////////////////////////////////////////////////////////////
//signal thread function
RegisterLocalServer::RegisterLocalServer(talk_base::Thread *signal_thread,
                                         talk_base::Thread *worker_thread)
                                         :signal_thread_(signal_thread),
                                          worker_thread_(worker_thread)
{
  p2p_source_management_ = P2PSourceManagement::Instance();
}

RegisterLocalServer::~RegisterLocalServer(){

}

void RegisterLocalServer::NewServersRegister_s(
  ServerResources *server_resources)
{
  for(ServerResources::iterator iter = server_resources->begin();
    iter != server_resources->end(); iter++){
      delete (*iter);
  }
  delete server_resources;
}

//////////////////////////////////////////////////////////////////////////
//worker thread function

void RegisterLocalServer::CreateRegisterServer_w(){
  ASSERT(worker_thread_ == talk_base::Thread::Current());

  //1. Create BasicPacketSocketFactory using the current thread
  talk_base::BasicPacketSocketFactory 
    basic_packet_socket_fatory(worker_thread_);

  //2. Create TCP Server
  talk_base::AsyncPacketSocket *async_tcp_packet_socket
    = basic_packet_socket_fatory.CreateServerTcpSocket(
    talk_base::SocketAddress(),DEFUALT_PORT,DEFUALT_PORT,0);

  async_tcp_packet_socket->SignalNewConnection.connect(this,
    &RegisterLocalServer::OnNewConnect_w);
}

void RegisterLocalServer::OnNewConnect_w(
  talk_base::AsyncPacketSocket *server_socket,
  talk_base::AsyncPacketSocket *new_socket)
{
  ASSERT(worker_thread_ == talk_base::Thread::Current());
  //no needs release at here. itself will be release
  new ServerRegisterQuestion(new_socket);
}

bool RegisterLocalServer::AddOnlineServerResource_w(
  ServerResources *server_resources)
{
  for(ServerResources::iterator iter = server_resources->begin();
    iter != server_resources->end(); iter)
  {
    OnlineServerResources::iterator f_iter =
      online_server_resource_.find((*iter)->server_ide_);

    if(f_iter != online_server_resource_.end()){
      LOG(LS_ERROR) << "The source Existed";
      return false;
    }
    talk_base::SocketAddress addr((*iter)->server_ip_,(*iter)->server_port_);

    online_server_resource_.insert(
      OnlineServerResources::value_type((*iter)->server_ide_,addr));
  }
  return true;
}

void RegisterLocalServer::OnSignalRegisterServerResources(
  ServerResources *server_resources)
{
  ASSERT(worker_thread_ == talk_base::Thread::Current());
  AddOnlineServerResource_w(server_resources);
  signal_thread_->Post(this,NEW_REGISTER_SERVER_RESOURCE,
    new MyMessageData(server_resources));
}


//////////////////////////////////////////////////////////////////////////
//Thread Message
void RegisterLocalServer::OnMessage(talk_base::Message *msg){

  switch(msg->message_id){
  case CREATE_REGISTER_SERVER:
    {
      CreateRegisterServer_w();
      break;
    }
  case NEW_REGISTER_SERVER_RESOURCE:
    {
      MyMessageData *my_message_data = 
        static_cast<MyMessageData*>(msg->pdata);

      NewServersRegister_s(my_message_data->server_resources_);

      delete my_message_data;
      break;
    }
  }
}





//////////////////////////////////////////////////////////////////////////

ServerRegisterQuestion::ServerRegisterQuestion(
  talk_base::AsyncPacketSocket *socket)
  :socket_(socket),realy_to_send_(false)
{
  socket_->SignalReadPacket.connect(this,
    &ServerRegisterQuestion::OnSignalReadPacket);

  socket_->SignalReadyToSend.connect(this,
    &ServerRegisterQuestion::OnSignalReadyToSend);
  
  socket_->SignalClose.connect(this,
    &ServerRegisterQuestion::OnSignalClose);
}

void ServerRegisterQuestion::OnSignalReadPacket(
  talk_base::AsyncPacketSocket *socket,const char *data, size_t len, 
  const talk_base::SocketAddress &addr)
{
  //1. Create some basic competent
  ServerResources *server_resources = new ServerResources;

  //2. Parse this data by json string;
  if(ParseServerResources(server_resources,data,len) && realy_to_send_){
    socket_->Send(REGISTER_SUCCEED.c_str(),REGISTER_SUCCEED.length(),
      talk_base::DiffServCodePoint::DSCP_DEFAULT);

    SignalRegisterServerResources(server_resources);
    socket_->Close();
    //Destroy itself
    talk_base::Thread::Current()->Post(this);
  }
  else{
    LOG(LS_ERROR) << "Send register failure message";
    socket_->Send(REGISTER_FAILURE.c_str(),REGISTER_FAILURE.length(),
      talk_base::DiffServCodePoint::DSCP_DEFAULT);
  }
}

void ServerRegisterQuestion::OnSignalReadyToSend(
  talk_base::AsyncPacketSocket *socket)
{
  realy_to_send_ = true;
}

void ServerRegisterQuestion::OnSignalClose(
  talk_base::AsyncPacketSocket *socket,int err)
{
  socket_->Close();
}

//////////////////////////////////////////////////////////////////////////
//Help function
bool ServerRegisterQuestion::ParseServerResources(
  ServerResources *server_resources,const char *data, size_t len)
{
  Json::Value root;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(data,root);
  if ( !parsingSuccessful )
  {
    // report to the user the failure and their locations in the document.
    LOG(LS_ERROR)  << "Failed to parse configuration\n"
      << reader.getFormattedErrorMessages();
    return false;
  }

  Json::Value joson_array = root[SERVER_RESOURCE_ARRAY];
  int size = joson_array.size();
  for(int i = 0; i < size; i++){
    server_resources->insert(new ServerResource(
      joson_array[i][SERVER_NAME].asCString(),joson_array[i][SERVER_IP].asCString(),
      joson_array[i][SERVER_PORT].asInt(),joson_array[i][SOURCE_IDE].asCString()));
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////
//Thread Message
void ServerRegisterQuestion::OnMessage(talk_base::Message *msg){
  delete this;
}