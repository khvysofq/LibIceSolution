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
#pragma once

#include <string>
#include <map>


//when you used the RunMessageProcess function
//the FOREVER means naver back.
const int KOREVER    =   -1;

enum LogInforLevel { LOG_SENSITIVE, 
    LOG_VERBOSE, 
    LOG_INFO, 
    LOG_WARNING, 
    LOG_ERROR
};
enum StateMessageType
{
    //////////////////////////////////////
    ERROR_P2P_SERVER_NO_THIS_PEER,
    ERROR_P2P_LOGING_SERVER_FAILURE,
    ERROR_ICE_CONNECTION_FAILURE,
    ERROR_ICE_PEER_DISCONNECTED,

    STATE_P2P_START_PEER_CONNECTION,
    STATE_P2P_LOGING_SERVER_SUCCEED,
    STATE_ICE_SEND_DATA_EVENT,
    STATE_ICE_PEER_DISCONNECTED,
    
    //////////////////////////////////////
    STATE_P2P_SERVER_DISCONNECTED,

};

const int STUN_SERVER_PORT  =   3478;
const std::string   content_type = "Data_transport_test";

class P2PTunnelClient;
class ThreadP2PTunnelClient;
typedef void(*RecvDataFunc)(char *,int, void *);
typedef void(*OnlinePeerFunc)(std::map<int, std::string>,void *);
typedef void(*StateChangeMessageFunc)(StateMessageType ,void *);

typedef void(*ThreadRecvDataFunc)(char *,int,void *);
typedef void(*ThreadOnlinePeerFunc)(std::map<int, std::string>,void *);


const unsigned short kDefaultServerPort = 8888;

//the help function
std::string GetEnvVarOrDefault(const char* env_var_name,
                               const char* default_value);
//get the stun server ip and port
//std::string GetPeerConnectionString();
//get the server ip
//std::string GetDefaultServerName();
//get current user name
std::string GetPeerName();

//#define MY_WEBRTC_CONNECT   "42.121.127.71:3478"
//#define MY_WEBRTC_CONNECT   "stunserver.org:3478"


#endif  // PEERCONNECTION_SAMPLES_CLIENT_DEFAULTS_H_
