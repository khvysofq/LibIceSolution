/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/27      11:38
 * Filename : F:\GitHub\trunk\p2p_slotion\p2psourcemanagement.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: p2psourcemanagement
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
#include <iostream>

#include "p2psourcemanagement.h"
#include "talk/base/logging.h"
#include "talk/base/json.h"
//////////////////////////////////////////////////////////////////////////
//Singleton Pattern 
P2PSourceManagement *P2PSourceManagement::p2p_source_management_ = NULL;

P2PSourceManagement *P2PSourceManagement::Instance(){
  if(!p2p_source_management_)
    p2p_source_management_ = new P2PSourceManagement();
  return p2p_source_management_;
}
//////////////////////////////////////////////////////////////////////////

void P2PSourceManagement::SetLocalPeerName(const std::string &peer_name){
  //defualt set the peer id is 0;
  local_peer_resource_.peer_id_  = 0;
  local_peer_resource_.peer_jid_ = peer_name;
}

bool P2PSourceManagement::AddNewServerResource(ServerResource *server_resource){
  //
  for(ServerResources::iterator iter = local_peer_resource_.server_resources_.begin();
    iter != local_peer_resource_.server_resources_.end(); iter++)
  {
    if((*iter)->server_ip_ == server_resource->server_ip_
      && (*iter)->server_port_ == server_resource->server_port_){
        LOG(LS_ERROR) << "The Server Resource Existed";
        return false;
    }
  }

  local_peer_resource_.server_resources_.insert(server_resource);
  return true;
}

bool P2PSourceManagement::AddNewServerResource(const std::string &server_name,
                                            const std::string &server_ip, 
                                            int server_port,
                                            const std::string &server_type)
{
  ServerResource *server_resouce = new ServerResource(server_name,
    server_ip,server_port,server_type);
  if(!AddNewServerResource(server_resouce)){
    delete server_resouce;
    return false;
  }
  return true;
}

bool P2PSourceManagement::DeleteServerResourceByAddr(
  const talk_base::SocketAddress &addr)
{
  //1. Find the source
  for(ServerResources::iterator iter = local_peer_resource_.server_resources_.begin();
    iter != local_peer_resource_.server_resources_.end(); iter++)
  {
    if((*iter)->server_ip_ == addr.ipaddr().ToString()
      && (*iter)->server_port_ == addr.port()){
        delete (*iter);
        local_peer_resource_.server_resources_.erase(iter);
        return true;
    }
  }

  LOG(LS_ERROR) << "Delete Failure The Server Resource Not Existed";
  return false;
}

bool P2PSourceManagement::DeleteServerResourceByName(const std::string &server_name){
  //1. Find the source
  for(ServerResources::iterator iter = local_peer_resource_.server_resources_.begin();
    iter != local_peer_resource_.server_resources_.end(); iter++)
  {
    if((*iter)->server_name_ == server_name){
      delete (*iter);
      local_peer_resource_.server_resources_.erase(iter);
      return true;
    }
  }
  LOG(LS_ERROR) << "Delete Failure The Server Resource Not Existed";
  return false;
}

void P2PSourceManagement::DeleteAllServerResource(ServerResources &server_resource){
  //1. Find the source
  for(ServerResources::iterator iter = server_resource.begin();
    iter != server_resource.end(); iter++)
  {
    delete *iter;
    //server_resource.erase(iter);
  }
}

//////////////////////////////////////////////////////////////////////////

bool P2PSourceManagement::ThePeerResourceIsExisited(PeerResource *peer_resource){
  for(PeerResources::iterator iter = remote_peer_resources_.begin();
    iter != remote_peer_resources_.end(); iter++)
  {
    if((*iter)->peer_id_ == peer_resource->peer_id_ 
      &&(*iter)->peer_jid_ == peer_resource->peer_jid_){
        return true;
    }
  }
  return false;
}

void P2PSourceManagement::OnOnlinePeers(const PeerInfors &peers){
  LOG(LS_ERROR) << __FUNCTION__;
  for(PeerInfors::const_iterator iter = peers.begin();
    iter != peers.end();++iter){
      PeerResource *peer_resource = new PeerResource();

      ParseJosonString(peer_resource,iter->first,
        iter->second.peer_name_,iter->second.resource_);
      if(!ThePeerResourceIsExisited(peer_resource))
        remote_peer_resources_.insert(peer_resource);
      else
        delete peer_resource;
  }
  ShowAllInfors();
}

void P2PSourceManagement::OnAPeerLogin(int peer_id,const PeerInfor &peer){
  LOG(LS_ERROR) << __FUNCTION__;
  //1. Find the remote peer by peer id and peer name
  for(PeerResources::iterator iter = remote_peer_resources_.begin();
    iter != remote_peer_resources_.end(); iter++)
  {
    if((*iter)->peer_id_ == peer_id 
      &&(*iter)->peer_jid_ == peer.peer_name_){
        LOG(LS_ERROR) << "The Peer Existed";
        return ;
    }
  }
  if(peer.resource_.empty()){
    LOG(LS_ERROR) << "Peer Resource is empty";
    return ;
  }
  //2. Insert the peer to remote peer resources
  PeerResource *peer_resource = new PeerResource();
  ParseJosonString(peer_resource,peer_id,peer.peer_name_,peer.resource_);
  remote_peer_resources_.insert(peer_resource);
  ShowAllInfors();
}

void P2PSourceManagement::OnAPeerLogout(int peer_id,const PeerInfor &peer){
  LOG(LS_ERROR) << __FUNCTION__;
  for(PeerResources::iterator iter = remote_peer_resources_.begin();
    iter != remote_peer_resources_.end(); iter++)
  {
    if((*iter)->peer_id_ == peer_id 
      &&(*iter)->peer_jid_ == peer.peer_name_){
        //1. delete server resource at the peer;
        DeleteAllServerResource((*iter)->server_resources_);
        //2. delete the peer resource
        //delete &(*iter)->server_resources_;
        delete *(iter);
        remote_peer_resources_.erase(iter);
        ShowAllInfors();
        return ;
    }
  }
  LOG(LS_ERROR) << "The Peer Not Existed";
}

bool P2PSourceManagement::ParseJosonString(PeerResource *peer_resource,
  int peer_id,const std::string &peer_name,const std::string &resource_string)
{
  //1.new a PeerResource Object
  peer_resource->peer_id_ = peer_id;
  peer_resource->peer_jid_ = peer_name;

  Json::Value root;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(resource_string,root);
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
    peer_resource->server_resources_.insert(new ServerResource(
      joson_array[i][SERVER_NAME].asCString(),joson_array[i][SERVER_IP].asCString(),
      joson_array[i][SERVER_PORT].asInt(),joson_array[i][SOURCE_TYPE].asCString()));
  }
  return true;
}

std::string P2PSourceManagement::GetServerResourceString()
{
  std::stringstream joson_string;
  joson_string << "{";
  // Add Peer jid
  joson_string << "\"" << PEER_JID << "\""<< ":" <<"\"" 
    << local_peer_resource_.peer_jid_<< "\",";

  //Add server resource Array identifier
  joson_string << "\"" << SERVER_RESOURCE_ARRAY << "\"" << ": [";
  for(ServerResources::iterator iter = local_peer_resource_.server_resources_.begin();
    iter != local_peer_resource_.server_resources_.end(); ){
    joson_string << "{" << "\"" << SERVER_NAME << "\" : "
      << "\"" << (*iter)->server_name_<< "\","
      << "\"" << SERVER_IP << "\" : "
      << "\"" << (*iter)->server_ip_<< "\","
      << "\"" << SERVER_PORT << "\" : "
      << (*iter)->server_port_ << ","
      << "\"" << SOURCE_TYPE << "\" : "
      << "\"" << (*iter)->server_type_<< "\""
      << "}";
    //There very specially
    iter++;
    if(iter != local_peer_resource_.server_resources_.end()){
      joson_string << ",";
    }
  }
  joson_string << "]";
  joson_string << "}";
  return joson_string.str();
}

void P2PSourceManagement::ShowAllServerResourceInfor(
  const ServerResources &server_resources)
{
  for(ServerResources::const_iterator iter = server_resources.begin();
    iter != server_resources.end(); iter++)
  {
    std::cout <<"\t" << SERVER_NAME << "\t" << (*iter)->server_name_<< std::endl;
    std::cout <<"\t" << SERVER_IP << "\t" << (*iter)->server_ip_<< std::endl;
    std::cout <<"\t" << SERVER_PORT << "\t" << (*iter)->server_port_<< std::endl;
    std::cout <<"\t" << SOURCE_TYPE << "\t" << (*iter)->server_type_<< std::endl;
  }
}

void P2PSourceManagement::ShowAllInfors(){
  std::cout << "----------------------------------" << std::endl;
  for(PeerResources::iterator iter = remote_peer_resources_.begin();
    iter != remote_peer_resources_.end(); iter++){
      std::cout << "PEER_ID\t" << (*iter)->peer_id_ << std::endl;
      std::cout << "PEER_JID\t" << (*iter)->peer_jid_<< std::endl;
      std::cout << ".........................." << std::endl;
      ShowAllServerResourceInfor((*iter)->server_resources_);
      std::cout << "..........................\n" << std::endl;
  }
  std::cout << "----------------------------------" << std::endl;
}


//////////////////////////////////////////////////////////////////////////
//For P2PConnectionManagement Interface
int P2PSourceManagement::SreachPeerByServerResource(
  const talk_base::SocketAddress &addr)
{
  for(PeerResources::iterator iter = remote_peer_resources_.begin();
    iter != remote_peer_resources_.end(); iter++){
      if(SreachServerResource((*iter)->server_resources_,addr)){
        return (*iter)->peer_id_;
      }
  }
  return 0;
}

bool P2PSourceManagement::SreachServerResource(
  const ServerResources &server_resources, const talk_base::SocketAddress& addr)
{
  const std::string server_ip = addr.ipaddr().ToString();
  for(ServerResources::const_iterator iter = server_resources.begin();
    iter != server_resources.end(); iter++)
  {
    if((*iter)->server_ip_ == server_ip && (*iter)->server_port_ == addr.port()){
      return true;
    }
  }
  return false;
}