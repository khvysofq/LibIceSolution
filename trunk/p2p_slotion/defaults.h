/*
 * libjingle
 * Copyright 2011, Google Inc.
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

#ifndef PEERCONNECTION_SAMPLES_CLIENT_DEFAULTS_H_
#define PEERCONNECTION_SAMPLES_CLIENT_DEFAULTS_H_

#include <string>
#include <map>
#include "talk/base/stream.h"
#include "talk/base/logging.h"

static const char RANDOM_BASE64[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '0'
};

//////////////////////////////////////////////////////////////////////////
//Define logging

typedef uint32 LOG_FILTER;

const int ALWAYS_INFOR          = (0X1);
//operation for object create and destroy
const int OFOI                  = ((0X1<<1));
const int SERVER_DATA_INFOR     = ((0X1<<2));
const int P2P_ICE_DATA_INFOR    = ((0X1<<3));
const int P2P_TUNNEL_DATA_INFOR = ((0X1<<4));
const int PROXY_SOCKET_INFOR    = ((0X1<<5));

extern LOG_FILTER log_filter;
#define LOG_F_S(sev,level) (log_filter & level)?LOG_F(sev):LOG(LS_SENSITIVE)



//////////////////////////////////////////////////////////
const int kDefaultServerPort = 8888;
const talk_base::SocketAddress  KStunAddr("stun.endigovoip.com",3478);

const size_t DEAFULT_BUFFER_LENGTH = 64 * 1024;

struct PeerInfor{
  PeerInfor(){};
  PeerInfor(const std::string &peer_name, const std::string &resource)
    :peer_name_(peer_name),resource_(resource){}
  std::string peer_name_;
  std::string resource_;
};

typedef std::map<int,PeerInfor> PeerInfors;


const int P2P_NETWORKER_HEADER_IDE        = 0X01234567;
const int P2P_NETWORKER_HEADER_IDE_HEAD   = P2P_NETWORKER_HEADER_IDE >> 24;
const int P2P_NETWORKER_HEADER_IDE_MIDDLE = (P2P_NETWORKER_HEADER_IDE >> 8) & 0X0000FFFF;
const int P2P_NETWORKER_HEADER_IDE_LAST   = P2P_NETWORKER_HEADER_IDE & 0X000000FF;
const int RECEIVE_BUFFER_LEN       =  1024 * 64;
struct SOCKETHeader{
  int header_ide_;
  int remote_socket_;
  int local_socket_;
  int data_len_;
};

typedef uint16 SocketType;

struct NetworkHeader{
  uint32      header_ide_;
  uint32      remote_socket_;
  uint32      local_socket_;
  SocketType  socket_type_; //uint16
  uint16      data_len_;
};
const int NETWORKHEADER_LENGTH = sizeof(NetworkHeader);
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//ICE help define
const std::string HTTP_USER_AGENT     = "HTTP_USER_AGENT";
const std::string JID_DEFAULT_DOMAIN  = "@vzenith.com/p2p";
const std::string DEFAULT_DECRIBE     = "p2p solution test";

//virtual network layer help define
const int NON_SOCKET                 =  0;
const SocketType          TCP_SOCKET =  1;
const SocketType          UDP_SOCKET =  2;

std::string GetEnvVarOrDefault(const char* env_var_name,
                               const char* default_value);
//get user computer name
std::string GetCurrentComputerUserName();

enum StatesChangeType
{
  ///////////////////////////////////////////////
  //p2p server states message
  STATES_P2P_SERVER_SEND_WAIT_SUCCEED,
  STATES_P2P_SERVER_LOGIN_SUCCEED,
  STATES_P2P_PEER_CONNECTION,
  STATES_P2P_REMOTE_PEER_DISCONNECTED,
  STATES_P2P_ONLINE_PEER_CONNECTION,
  STATES_P2P_PEER_SIGNING_OUT,

  ERROR_P2P_SERVER_TIME_OUT,
  ERROR_P2P_SERVER_LOGIN_SERVER_FAILURE,
  ERROR_P2P_PEER_NO_THIS_PEER,
  ERROR_P2P_CAN_NOT_SEND_MESSAGE,
  //////////////////////////////////////////////
  
  //////////////////////////////////////////////
  STATES_ICE_START_PEER_CONNECTION,
  STATES_ICE_TUNNEL_SEND_DATA,
  STATES_ICE_TUNNEL_CLOSED
};

//some help function
std::string StreamResultToString(const talk_base::StreamResult res);


#endif  // PEERCONNECTION_SAMPLES_CLIENT_DEFAULTS_H_
