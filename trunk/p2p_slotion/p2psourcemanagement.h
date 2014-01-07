/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/27      11:38
 * Filename : F:\GitHub\trunk\p2p_slotion\p2psourcemanagement.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: p2psourcemanagement
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

#ifndef P2P_SOURCE_MANAGEMENT_H_
#define P2P_SOURCE_MANAGEMENT_H_
#include <vector>
#include <set>

#include "talk/base/sigslot.h"
#include "talk/base/socketaddress.h"
#include "defaults.h"

//For Joson 
const std::string SERVER_NAME           = "SERVER_NAME";
const std::string SERVER_IP             = "SERVER_IP";
const std::string SERVER_PORT           = "SERVER_PORT";
const std::string SOURCE_IDE            = "SOURCE_IDE";
const std::string PEER_JID              = "PEER_JID";
const std::string SERVER_RESOURCE_ARRAY = "SERVER_RESOURCE_ARRAY";

struct ServerResource{

  ServerResource():server_name_(""),server_ip_(""),server_port_(0),
    server_ide_(""){}

  ServerResource(const std::string &server_name,
    const std::string &server_ip, int server_port,
    const std::string &server_ide)
    :server_name_(server_name),server_ip_(server_ip),
    server_port_(server_port),server_ide_(server_ide){}

  std::string server_name_;
  std::string server_ip_;
  int         server_port_;
  std::string server_ide_;
};
typedef std::set<ServerResource* > ServerResources;

struct PeerResource{
  int               peer_id_;
  std::string       peer_jid_;
  ServerResources   server_resources_;
};
typedef std::set<PeerResource* > PeerResources;

//This class 
//1. Manage local source information
//2. Manage all remote source information
//3. 
class P2PSourceManagement : public sigslot::has_slots<>
{
public:
  ~P2PSourceManagement(){
    DeleteAllServerResource(local_peer_resource_.server_resources_);
  }
  sigslot::signal1<const std::string &> SignalRegisterServerResources;
  //P2PConnectionManagement interface
  const std::string SreachPeerByServerResource(
    const talk_base::SocketAddress &addr);
  const ServerResource *SreachPeerBySourceIde(const std::string &source_ide,
    const std::string &server_type,std::string *remote_peer_name);

  bool SreachServerResourceByAddr(const ServerResources &server_resources, 
    const talk_base::SocketAddress& addr);
  const ServerResource *SreachServerResourceByIde(
    const ServerResources &server_resources, const std::string &server_type,
    const std::string &source_ide);


  //This is remote peer resource update function.
  //Those are call back function called by p2p_connection_server
  //
  //OnOnlinePeers will be call at the first login p2p server
  void OnOnlinePeers(const PeerInfors &peers);
  //OnAPeerLogin called at a new peer login p2p server
  void OnAPeerLogin(int peer_id,const PeerInfor &peer);
  //OnAPeerLogout called at a peer logout p2p server
  void OnAPeerLogout(int peer_id,const PeerInfor &peer);

  bool ThePeerResourceIsExisited(PeerResource *peer_resource);

  //
  void ShowAllInfors();
  void ShowAllServerResourceInfor(const ServerResources &server_resources);

  //Help Function that Parse Joson string to PeerInfor struct
  //The function will new a PeerResource Object
  bool ParseJosonString(PeerResource *peer_resource,int peer_id,
    const std::string &peer_name,const std::string &resource_string);
  const std::string GetServerResourceString();

  //Manage for local peer_resourece
  //defualt set the peer id is 0;
  void SetLocalPeerName(const std::string &peer_name);
  const std::string GetLocalPeerName() const;
  void SetLocalPeerId(int peer_id);
  bool IsSetLocalPeerName() const ;
  const std::string GetRemotePeerNameByPeerId(int peer_id);
  int  GetRemotePeerIdByPeerName(const std::string &remote_peer_name);

  //
  bool AddNewServerResources(ServerResources *server_resources);
  bool AddNewServerResource(ServerResource *server_resource);
  bool AddNewServerResource(const std::string &server_name,
    const std::string &server_ip, int server_port,
    const std::string &server_type);
  bool DeleteServerResourceByAddr(const talk_base::SocketAddress &addr);
  bool DeleteServerResourceByName(const std::string &server_name);
  void DeleteAllServerResource(ServerResources &server_resource);
  void DeleteAllOnlinePeerResource();

private:
  PeerResource    local_peer_resource_;
  PeerResources   remote_peer_resources_;

public:
  //Singleton Pattern Function
  static P2PSourceManagement *Instance();
private:
  P2PSourceManagement(){};
  
  //Singleton Pattern variable
  static P2PSourceManagement *p2p_source_management_;

  DISALLOW_EVIL_CONSTRUCTORS(P2PSourceManagement);
};
#endif // !P2P_SOURCE_MANAGEMENT_H_
