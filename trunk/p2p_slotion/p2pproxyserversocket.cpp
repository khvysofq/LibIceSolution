/*
* p2p solution
* Copyright 2013, VZ Inc.
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

#include "p2pproxyserversocket.h"
#include <iostream>

const char RTSP_HEADER[]   = "DESCRIBE rtsp://";
const char RTSP_BACKLASH   = '/';
const char RTSP_BREAK_CHAR = '_';
const size_t RTSP_HEADER_LENGTH = sizeof(RTSP_HEADER) - 1;

const char HTTP_GET_HEADER[] = "GET /";
const char HTTP_POST_HEADER[] = "POST /";
const size_t HTTP_POST_HEADER_LENGTH = sizeof(HTTP_POST_HEADER) - 1;
const size_t HTTP_GET_HEADER_LENGTH = sizeof(HTTP_GET_HEADER) - 1;
const char HTTP_A            = '\r';
const char HTTP_N            = '\n';
//////////////////////////////////////////////////////////////////////////
//P2PProxyServerSocket
//////////////////////////////////////////////////////////////////////////

size_t P2PProxyServerSocket::DeleteAllTheSameString(
  char *data,const char *substr)
{
  size_t len = strlen(substr);
  for(char *p = strstr(data,substr);p != NULL; 
    p = strstr(p, substr))
  {
    size_t remain_len = strlen(data) - ((p - data) + len);
    memcpy(p,p + len,remain_len);
    p[remain_len] = '\0';
  }
  return strlen(data);
}

//////////////////////////////////////////////////////////////////////////
//RTSP Proxy Server socket
//////////////////////////////////////////////////////////////////////////
P2PRTSPProxyServerSocket::P2PRTSPProxyServerSocket(
  talk_base::AsyncSocket* socket)
  : P2PProxyServerSocket(socket, KBufferSize),
  is_connect_(false)
{
  //allow handle the request RTSP data
  BufferInput(true);
}

void P2PRTSPProxyServerSocket::SendConnectResult(int result, 
                                                 const std::string &addr_ide)
{
  if(result == 0)
    is_connect_ = true;
  else
    is_connect_ = false;
}

void P2PRTSPProxyServerSocket::ProcessInput(char* data, size_t* len)
{
  data[*len] = '\0';
  LOG_P2P(BASIC_INFOR) << "------------------------";
  LOG_P2P(BASIC_INFOR) << "Original data is ";
  LOG_P2P(BASIC_INFOR) << "\n" << data;
  LOG_P2P(BASIC_INFOR) << "------------------------";
  if(strncmp(RTSP_HEADER,(const char *)data,RTSP_HEADER_LENGTH) == 0){
    if(!ParseRTSPRequest(data,len)){
      SignalCloseEvent(this,-1);
      return;
    }
  }
  BufferInput(false);
  SignalConnectReadEvent(this,(const void *)data,*len);
  BufferInput(true);
  *len = 0;
  /*BufferInput(false);
  SignalReadEvent(this);
  BufferInput(true);*/
}

bool P2PRTSPProxyServerSocket::ParseRTSPRequest(char *data, size_t *len)
{
  LOG_P2P(P2P_RTSP_LOCAL_SERVER) << "parsing RTSP source";
  size_t header_len = RTSP_HEADER_LENGTH;
  size_t backlash_pos = 0;
  size_t break_char_pos = 0;
  char source_ide[64];
  size_t serouce_ide_len = 0;

  memset(source_ide,0,64);

  //1. Find backlash position and break char position
  for(size_t i = header_len; i < *len; i++){
    if(data[i] == RTSP_BACKLASH)
      //add 1 because the / is no a member of server ip
        backlash_pos = i + 1; 
    if(data[i] == RTSP_BREAK_CHAR){
      break_char_pos = i;
      break;
    }
    if(data[i] == '\r' && data[i + 1] == '\n'){
      return false;
    }
  }

  //2. get the server ip and port
  serouce_ide_len = break_char_pos - backlash_pos;
  strncpy(source_ide,data + backlash_pos, serouce_ide_len);

  serouce_ide_len += 1;
  //3. delete the server ip in the data
  for(size_t i = break_char_pos + 1; i < *len; i++){
    data[i - serouce_ide_len] = data[i];
    data[i] = 0;
  }
  *len -= serouce_ide_len;
  
  LOG_P2P(BASIC_INFOR) << "------------------------";
  LOG_P2P(BASIC_INFOR) << "new data is";
  LOG_P2P(BASIC_INFOR) << "\n" << data;
  LOG_P2P(BASIC_INFOR) << "------------------------";
  if(!is_connect_){
    LOG_P2P(P2P_RTSP_LOCAL_SERVER) << "Connect the source " << source_ide;
    SignalConnectRequest(this,source_ide);
  }
  return true;
}


bool P2PRTSPProxyServerSocket::ParseRTSPRequestAddress(char *data, size_t *len){
  LOG_P2P(P2P_RTSP_LOCAL_SERVER) << "parsing RTSP source";
  char new_data[2046]   = {0};
  size_t new_data_len   = 0;

  char turn_ip[64]      = {0};
  size_t turn_ip_len    = 0;

  char pass_word[128]   = {0};
  size_t pass_word_len  = 0;

  char p2p_ip[64]       = {0};
  size_t p2p_ip_len     = 0;

  char *temp            = NULL;
  char *current_pos     = data + RTSP_HEADER_LENGTH;

  //1. get the turn_ip
  temp = strchr(current_pos,'/') + 1;
  turn_ip_len = temp - current_pos;
  memcpy(turn_ip,current_pos,turn_ip_len);
  current_pos = temp;

  for(size_t i = 0; i < turn_ip_len; i ++){
    if(turn_ip[i] == ':'){
      memset(turn_ip + i,0, 64 - i - 1);
      turn_ip[i] = '/';
      turn_ip_len = i + 1;
      break;
    }
  }

  //2. getting the pass_word and name
  temp = strchr(current_pos,'@') + 1;
  pass_word_len = temp - current_pos;
  memcpy(pass_word,current_pos,pass_word_len);
  current_pos = temp;

  //3. getting the p2p ip
  temp = strchr(current_pos,'/') + 1;
  p2p_ip_len = temp - current_pos;
  memcpy(p2p_ip,current_pos,p2p_ip_len);
  current_pos = temp;

  //4. check whether the p2p ip string has ':'
  for(size_t i = 0; i < p2p_ip_len; i ++){
    if(p2p_ip[i] == ':'){
      memset(p2p_ip + i,0, 64 - i - 1);
      break;
    }
    if(p2p_ip[i] == '/'){
      memset(p2p_ip + i,0, 64 - i - 1);
      break;
    }
  }

  //save those result
  //1. save the pass word and username
  memcpy(new_data,RTSP_HEADER,RTSP_HEADER_LENGTH);
  new_data_len = RTSP_HEADER_LENGTH;
  memcpy(new_data + new_data_len,pass_word,pass_word_len);
  new_data_len += pass_word_len;
  memcpy(new_data + new_data_len,turn_ip,turn_ip_len);
  new_data_len += turn_ip_len;
  memcpy(new_data + new_data_len,
    current_pos,*len - (current_pos - data));
  new_data_len += *len - (current_pos - data);

  //
  //std::cout << "------------------------\n";
  //std::cout << "Original data is \n";
  //for(size_t i = 0; i < *len ; i++)
  //  std::cout <<data[i];
  //std::cout << "\n";
  //LOG_P2P(P2P_RTSP_LOCAL_SERVER) << "new data is \n" << new_data;
  //std::cout << "------------------------\n";

  memcpy(data,new_data,new_data_len);
  *len = new_data_len;

  if(!is_connect_){
    LOG_P2P(P2P_RTSP_LOCAL_SERVER) << "Connect the source " << p2p_ip;
    SignalConnectRequest(this,p2p_ip);
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////
//HTTP Porxy Server Socket
//////////////////////////////////////////////////////////////////////////

P2PHTTPProxyServerSocket::P2PHTTPProxyServerSocket(
  talk_base::AsyncSocket* socket)
  : P2PProxyServerSocket(socket, KBufferSize),
  is_connect_(false)
{
  //allow handle the request RTSP data
  BufferInput(true);
}

void P2PHTTPProxyServerSocket::SendConnectResult(int result, 
                                                 const std::string &addr_ide)
{
  if(result == 0)
    is_connect_ = true;
  else
    is_connect_ = false;
}

void P2PHTTPProxyServerSocket::ProcessInput(char* data, size_t* len)
{
  LOG_P2P(P2P_HTTP_SOCKET_LOGIC | BASIC_INFOR | P2P_HTTP_SOCKET_DATA) 
    << "Read input data :"; 

  data[*len] = '\0';
  LOG_P2P(BASIC_INFOR) << "------------------------";
  LOG_P2P(BASIC_INFOR) << "Original data is ";
  LOG_P2P(BASIC_INFOR) << "\n" << data;
  LOG_P2P(BASIC_INFOR) << "------------------------";

  if(strncmp(HTTP_POST_HEADER,(const char *)data,
    HTTP_POST_HEADER_LENGTH) == 0){
      if(!ParseHTTPRequest(data,len,HTTP_POST_HEADER_LENGTH)){
        SignalCloseEvent(this,SOCKET_ERROR);
        return ;
      }
  }
  else if(strncmp(HTTP_GET_HEADER,(const char *)data,
    HTTP_GET_HEADER_LENGTH) == 0){
      if(!ParseHTTPRequest(data,len,HTTP_GET_HEADER_LENGTH)){
        SignalCloseEvent(this,SOCKET_ERROR);
        return ;
      }
  }

  LOG_P2P(BASIC_INFOR) << "------------------------";
  LOG_P2P(BASIC_INFOR) << "new data is ";
  LOG_P2P(BASIC_INFOR) << "\n" << data;
  LOG_P2P(BASIC_INFOR) << "------------------------";
  SignalConnectReadEvent(this,(const void *)data,*len);
}

bool P2PHTTPProxyServerSocket::ParseHTTPRequest(char *data, 
                                                size_t *len, 
                                                size_t header_length)
{
  if(is_connect_){
    *len = DeleteAllTheSameString(data,source_ide_.c_str());
    return true;
  }
  LOG_P2P(P2P_RTSP_LOCAL_SERVER) << "parsing HTTP source";
  size_t header_len = header_length;
  size_t backlash_pos = header_len;
  size_t break_char_pos = 0;
  char source_ide[64];
  size_t serouce_ide_len = 0;

  memset(source_ide,0,64);

  //1. Find backlash position and break char position
  for(size_t i = header_len; i < *len; i++){
    if(data[i] == RTSP_BACKLASH)
    {
      //add 1 because the / is no a member of server ip
      break_char_pos = i + 1;
      break;
    }
    if(data[i] == ' ')
    {
      //add 1 because the / is no a member of server ip
      break_char_pos = i; 
      if(i == header_len)
        return false;
      break;
    }
    if(data[i] == '\r' && data[i + 1] == '\n'){
      return false;
    }
  }

  //2. get the server ip and port
  serouce_ide_len = break_char_pos - backlash_pos;
  strncpy(source_ide,data + backlash_pos, serouce_ide_len);

  if(!is_connect_){
    source_ide_ = source_ide;
    *len = DeleteAllTheSameString(data,source_ide_.c_str());
    if(source_ide[serouce_ide_len - 1] == RTSP_BACKLASH)
      source_ide[serouce_ide_len - 1] = 0;
    SignalConnectRequest(this,source_ide);
  }
  return true;
}