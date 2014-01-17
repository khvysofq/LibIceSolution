/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/27      11:52
 * Filename : F:\GitHub\trunk\p2p_slotion\p2p_user_client.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: p2p_user_client
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

#include <fstream>

#include "talk/base/json.h"
#include "talk/base/helpers.h"

#include "p2p_user_client.h"
#include "peer_connection_ice.h"
#include "peer_connection_server.h"

#include "sockettablemanagement.h"
#include "p2psourcemanagement.h"
#include "p2pconnectionmanagement.h"
#include "p2pserverconnectionmanagement.h"
#include "proxyserverfactory.h"


static const int TEST_SEND_BUFFER   = 4096;
//static const talk_base::SocketAddress KLocalRTSPServer("127.0.0.1",554);
static const talk_base::SocketAddress KLocalRTSPServer("127.0.0.1",554);
static const talk_base::SocketAddress KLocalHTTPServer("127.0.0.1",80);

P2PUserClient::P2PUserClient(talk_base::Thread *signal_thread,
                             talk_base::Thread *worker_thread)
                             :signal_thread_(signal_thread),
                             worker_thread_(worker_thread),
                             initiator_(false)
{
  LOG_P2P(P2P_BASIC_PART_LOGIC|CREATE_DESTROY_INFOR )
    << "Create P2PUserClient";

  receive_buffer_ = new char[RECEIVE_BUFFER_LENGTH];
  p2p_source_management_ = P2PSourceManagement::Instance();
  p2p_server_connection_management_ = P2PServerConnectionManagement::Instance();
  p2p_connection_management_ = P2PConnectionManagement::Instance();

  is_peer_connect_ = false;
}

P2PUserClient::~P2PUserClient(){
  LOG_P2P(P2P_BASIC_PART_LOGIC|CREATE_DESTROY_INFOR) 
    << "Destroy P2PUserClient";
}

void P2PUserClient::Initiatlor(){
  LOG_P2P(P2P_BASIC_PART_LOGIC) << "Initiator basic part of p2p solution";

  //Create a random string with local peer name
  //and generate local_peer_jid
  std::string local_peer_name = GetCurrentComputerUserName();
  std::string random_string;
  talk_base::CreateRandomString(4,RANDOM_BASE64,&random_string);
  local_peer_name += random_string;
  local_peer_name += JID_DEFAULT_DOMAIN;

  //translator local_peer_name with lower character
  //because the ICE protect just needs lower character 
  std::transform(local_peer_name.begin(),local_peer_name.end(),
    local_peer_name.begin(),tolower);
  
  srand(static_cast<unsigned int>(time(NULL)));
  std::ostringstream random_ip_string;
  random_ip_string << rand() %255 << ".";
  random_ip_string << rand() %255 << ".";
  random_ip_string << rand() %255 << ".";
  random_ip_string << rand() %255;
  p2p_server_addr_ = ReadingConfigureFile("configure.file",
    random_ip_string.str());

  p2p_source_management_->SetLocalPeerName(local_peer_name);
  p2p_connection_management_->Initialize(signal_thread_,
    signal_thread_,true);

  p2p_server_connection_management_->SetIceDataTunnel(
    p2p_connection_management_->GetP2PICEConnection());

}

void P2PUserClient::StartRun(){
  LOG_P2P(P2P_BASIC_PART_LOGIC) << "Start run the p2p solution";
  p2p_server_connection_management_->SignInP2PServer(p2p_server_addr_);
}

void P2PUserClient::ConnectionToPeer(int peer_id){
  LOG_P2P(P2P_BASIC_PART_LOGIC) << "listen a local port " 
    << KLocalRTSPServer.ToString();
  
  ProxyServerFactory::CreateP2PRTSPProxyServer(signal_thread_->socketserver(),
    KLocalRTSPServer);
  ProxyServerFactory::CreateP2PHTTPProxyServer(signal_thread_->socketserver(),
    KLocalHTTPServer);
}

const talk_base::SocketAddress P2PUserClient::ReadingConfigureFile(
  const std::string &config_file,const std::string &random_string)
{
  LOG_P2P(P2P_BASIC_PART_LOGIC) << "Reading Configure file";
  talk_base::SocketAddress server_addr;

  std::ifstream read_handle(config_file.c_str());
  //read_handle.open(
  if(!read_handle.is_open()){
    LOG(LS_ERROR) << "Can't open the configure file.";
    std::system("pause");
    std::exit(0);
  }

  //The first line must be P2P_SERVER_CONFIGURE

  std::string p2p_server_configure;
  read_handle >> p2p_server_configure;
  if(!p2p_server_configure.empty() && (p2p_server_configure != P2P_SERVER_CONFIGURE)){
    LOG(LS_ERROR) << "The first line of the configure file must be a p2p server address";
    std::system("pause");
    std::exit(0);
  } 
  else{
    std::string server_ip;
    uint16 server_port;
    read_handle >> server_ip;
    read_handle >> server_port;
    server_addr.SetIP(server_ip);
    server_addr.SetPort(server_port);
  }

  while(true){
    std::string server_configure;
    read_handle >> server_configure;
    if(server_configure.empty()){
      break;
    }
    if(server_configure != SERVER_CONFIGURE){
      LOG(LS_ERROR) << "reading the server configure getting error";
      std::system("pause");
      std::exit(0);
    }

    std::string server_name;
    std::string server_ip;
    uint16 server_port;
    std::string server_ide;

    read_handle >> server_name;
    read_handle >> server_ip;
    read_handle >> server_port;
    read_handle >> server_ide;
    if(server_name.empty() || server_ip.empty()){
      LOG(LS_ERROR) << "reading the server configure getting error";
      std::system("pause");
      std::exit(0);
    }
    p2p_source_management_->AddNewServerResource(server_name,
      server_ip,server_port,server_ide);
  }
  return server_addr;
}

void P2PUserClient::OnMessage(talk_base::Message* msg){
}
