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

#ifndef P2P_RTSP_PROXY_SERVER_SOCKET_H_
#define P2P_RTSP_PROXY_SERVER_SOCKET_H_

#include "talk/base/socketadapters.h"

#include "defaults.h"

class P2PProxyServerSocket : public talk_base::BufferedReadAdapter
{ 
public:
  P2PProxyServerSocket(AsyncSocket* socket, size_t buffer_size)
      : BufferedReadAdapter(socket, buffer_size) {}
  sigslot::signal2<P2PProxyServerSocket*,
                   const std::string &>  SignalConnectRequest;
  sigslot::signal3<P2PProxyServerSocket*,
                   const void *, size_t>  SignalConnectReadEvent;
  virtual void SendConnectResult(int err, const std::string &addr_ide) = 0;
  virtual const std::string GetProxyType() const = 0;
protected:
  size_t DeleteAllTheSameString(char *data,const char *substr);
  DISALLOW_EVIL_CONSTRUCTORS(P2PProxyServerSocket);
};

class P2PRTSPProxyServerSocket : public P2PProxyServerSocket
{
public:
  explicit P2PRTSPProxyServerSocket(talk_base::AsyncSocket* socket);
private:
  virtual void SendConnectResult(int result,const std::string &addr_ide);
  virtual const std::string GetProxyType() const {return RTSP_SERVER;}
  virtual void ProcessInput(char* data, size_t* len);
  virtual bool ParseRTSPRequest(char *data, size_t *len);
  virtual bool ParseRTSPRequestAddress(char *data, size_t *len);

  bool is_connect_;
  DISALLOW_EVIL_CONSTRUCTORS(P2PRTSPProxyServerSocket);
};

class P2PHTTPProxyServerSocket : public P2PProxyServerSocket{
public:
  explicit P2PHTTPProxyServerSocket(talk_base::AsyncSocket* socket);
private:
  virtual void SendConnectResult(int result,const std::string &addr_ide);

  virtual bool ParseHTTPRequest(char *data, size_t *len, size_t header_length);
  virtual const std::string GetProxyType() const {return HTTP_SERVER;}
  virtual void ProcessInput(char* data, size_t* len);
  bool is_connect_;
  std::string source_ide_;
  DISALLOW_EVIL_CONSTRUCTORS(P2PHTTPProxyServerSocket);
};
#endif