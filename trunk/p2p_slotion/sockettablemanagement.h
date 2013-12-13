/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/18      11:27
 * Filename : F:\GitHub\trunk\p2p_slotion\sockettablemanagement.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: sockettablemanagement
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

#ifndef SOCKET_TABLE_MANAGEMENT_H_
#define SOCKET_TABLE_MANAGEMENT_H_
#include "talk/base/basictypes.h"
#include "talk/base/asyncpacketsocket.h"
#include "defaults.h"
#include <map>

//SocketTable
struct SocketTable{
  SocketTable(){}
  SocketTable(int local_socket,int remote_socket,SocketType socket_type)
    :local_socket_(local_socket),remote_socket_(remote_socket),
    socket_type_(socket_type){}
  int local_socket_;
  int remote_socket_;
  SocketType socket_type_;
};

///////////////////////////////////////////////////////////////////////////
//TODO:(GuangleiHe) TIME: 12/1/2013
//remove the socket table management to a normal class. Not a singleton 
//pattern. Because, each proxy session has it own socket table.
///////////////////////////////////////////////////////////////////////////
class SocketTableManagement
{
public:
  SocketTableManagement();
  static SocketTableManagement *Instance();


  bool   AddNewLocalSocket(uint32 local_socket, uint32 remote_socket,
    SocketType socket_type);
  bool   UpdateRemoteSocketTable(uint32 local_socket, uint32 new_remote_socket);
  bool   DeleteASocket(uint32 local_socket);
  void   DeleteAllSockets();
  
  uint32 GetRemoteSocket(uint32 local_socket);
  //
  typedef std::map<uint32,SocketTable *> SocketTableMap;
private:
  static SocketTableManagement *socket_table_management_;
  SocketTableMap               local_socket_map_;
};








#endif
