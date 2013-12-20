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
static const talk_base::SocketAddress KLocalRTSPServer("127.0.0.1",554);

P2PUserClient::P2PUserClient(talk_base::Thread *worker_thread,
                             talk_base::Thread *signal_thread,
                             talk_base::Thread *stream_thread)
                             :worker_thread_(worker_thread),
                             signal_thread_(signal_thread),
                             stream_thread_(stream_thread),
                             initiator_(false)
{
  LOG(LS_INFO) << "+++" << __FUNCTION__;
  receive_buffer_ = new char[RECEIVE_BUFFER_LENGTH];
  p2p_source_management_ = P2PSourceManagement::Instance();
  p2p_server_connection_management_ = P2PServerConnectionManagement::Instance();
  p2p_connection_management_ = P2PConnectionManagement::Instance();

  is_peer_connect_ = false;
}

P2PUserClient::~P2PUserClient(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;

}

void P2PUserClient::Initiatlor(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;

  std::string local_peer_name = GetCurrentComputerUserName();

  ////Create a random string with local peer name
  std::string random_string;
  talk_base::CreateRandomString(4,RANDOM_BASE64,&random_string);

  local_peer_name += random_string;
  local_peer_name += JID_DEFAULT_DOMAIN;

  //translator local_peer_name with lower character
  //because the ICE protect just needs lower character 
  std::transform(local_peer_name.begin(),local_peer_name.end(),
    local_peer_name.begin(),tolower);

  p2p_source_management_->AddNewServerResource("RTSP_SERVER",
    "127.0.0.1",8557,random_string);
  //p2p_source_management_->AddNewServerResource("HTTP_SERVER",
  //  "127.0.0.1",80,random_string);
  p2p_source_management_->SetLocalPeerName(local_peer_name);

  p2p_connection_management_->Initialize(signal_thread_,
    worker_thread_,stream_thread_,false);
  p2p_server_connection_management_->SetIceDataTunnel(
    p2p_connection_management_->GetP2PICEConnection());

}

void P2PUserClient::StartRun(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
  p2p_server_connection_management_->SignInP2PServer(ServerAddr);
}

void P2PUserClient::ConnectionToPeer(int peer_id){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
  std::cout << "Listen " << KLocalRTSPServer.ToString() << std::endl;
  ProxyServerFactory::CreateRTSPProxyServer(signal_thread_->socketserver(),
    KLocalRTSPServer);
  //p2p_connection_management_->Connect(peer_id);
}

void P2PUserClient::Destory(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
}

void P2PUserClient::SendRandomData(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
}

void P2PUserClient::OnMessage(talk_base::Message* msg){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
}
