/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/18      11:27
 * Filename : F:\GitHub\trunk\p2p_slotion\sockettablemanagement.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: sockettablemanagement
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

#include "talk/base/logging.h"
#include "sockettablemanagement.h"
#include <iostream>

SocketTableManagement *SocketTableManagement::socket_table_management_ = NULL;

SocketTableManagement *SocketTableManagement::Instance(){
  if(!socket_table_management_)
    socket_table_management_ = new SocketTableManagement();
  return socket_table_management_;
}


SocketTableManagement::SocketTableManagement(){

}

bool SocketTableManagement::AddNewLocalSocket(uint32 local_socket, uint32 remote_socket,
                                              SocketType socket_type)
{
  SocketTableMap::iterator iter;
  //First, check out whither have the same socket in socket table map. 
  iter = remote_socket_map_.find(remote_socket);
  if(iter != remote_socket_map_.end()){
    LOG(LS_ERROR) << "Have a same socket in remote socket map";
    return false;
  }
  iter = local_socket_map_.find(local_socket);
  if(iter != local_socket_map_.end()){
    LOG(LS_ERROR) << "Have a same socket in local socket map";
    return false;
  }

  //Second, create new socket table object
  SocketTable *new_socket_table 
    = new SocketTable(local_socket,remote_socket,socket_type);
  
  //third, add new socket table to socket map
  //1. add to remote_socket_map_;
  remote_socket_map_.insert(SocketTableMap::value_type(
    remote_socket,new_socket_table));
  //2. add to local_socket_map_;
  local_socket_map_.insert(SocketTableMap::value_type(
    local_socket,new_socket_table));
  return true;
}

bool SocketTableManagement::DeleteASocket(uint32 local_socket){
  SocketTableMap::iterator iter;
  //First, find local socket at local socket map
  iter = local_socket_map_.find(local_socket);
  if(iter == local_socket_map_.end()){
    LOG(LS_ERROR) << "We are not found this socket in socket map";
    return false;
  }

  //Second, save the remote_socket that use to delete remote socket map object
  uint32 remote_socket = iter->second->remote_socket_;

  //Third, delete this socket object and erase it in local socket map
  delete iter->second;
  local_socket_map_.erase(iter);

  //fourth, find the object in remote socket map and erase it. 
  iter = remote_socket_map_.find(remote_socket);
  if(iter == remote_socket_map_.end()){
    LOG(LS_ERROR) << "Have a same socket in remote socket map";
    return false;
  }
  remote_socket_map_.erase(iter);
  return true;
}

void SocketTableManagement::DeleteAllSockets(){
  //First, delete all socket table object and erase in local socket map
  for(SocketTableMap::iterator iter = local_socket_map_.begin();
    iter != local_socket_map_.end(); ++iter){
      delete iter->second;
      local_socket_map_.erase(iter);
  }

  //Second, erase all socket table object in remote socket map
  for(SocketTableMap::iterator iter = remote_socket_map_.begin();
    iter != local_socket_map_.end(); ++iter){
      remote_socket_map_.erase(iter);
  }
}

uint32 SocketTableManagement::GetLocalSocket(uint32 remote_socket){
  SocketTableMap::iterator iter = remote_socket_map_.find(remote_socket);
  if(iter == remote_socket_map_.end()){
    //LOG(LS_ERROR) << "Can't found local socket in socket map";
    return NON_SOCKET;
  }
  return iter->second->local_socket_;
}

uint32 SocketTableManagement::GetRemoteSocket(uint32 local_socket){
  SocketTableMap::iterator iter = local_socket_map_.find(local_socket);
  if(iter == local_socket_map_.end()){
    //LOG(LS_ERROR) << "Can't found remote socket in socket map";
    return NON_SOCKET;
  }
  return iter->second->remote_socket_;
}