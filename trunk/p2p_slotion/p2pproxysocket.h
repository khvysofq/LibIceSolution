/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/27      9:11
 * Filename : F:\GitHub\trunk\p2p_slotion\asyncp2ppackagesocket.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: asyncp2ppackagesocket
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

#ifndef P2P_PROXY_SOCKET_H_
#define P2P_PROXY_SOCKET_H_

#include "talk/base/asyncsocket.h"
#include "p2pproxysession.h"

typedef uint16 SocketType;
class P2PConnectionManagement;

class P2PProxySocket 
  : public talk_base::AsyncSocket,
  public sigslot::has_slots<>
{
 public:
   P2PProxySocket();
   P2PProxySocket(P2PProxySession *p2p_proxy_session,
     uint32 other_side_socket);
  uint32 GetSocketNumber() const {return ((uint32)this);}

  void OnP2PRead(const char *data, uint16 len);
  void OnP2PClose(P2PProxySession *p2p_proxy_session);
  void OnP2PWrite(P2PProxySession *p2p_proxy_session);

  sigslot::signal2<const char *, uint16> SignalReadMixData;
  
  virtual void OnP2PConnectSucceed(P2PProxySession *p2p_proxy_session) = 0;
  virtual void OnP2PSocketConnectSucceed(P2PProxySession *p2p_proxy_session,
    uint32 other_side_socket) = 0;
  virtual void OnP2PSocketConnectFailure(P2PProxySession *p2p_proxy_session) 
    = 0;
 public:
   //for AsyncSocket
   virtual talk_base::AsyncSocket* Accept(talk_base::SocketAddress* paddr);
   //for Socket Class

   // Returns the address to which the socket is bound.  If the socket is not
   // bound, then the any-address is returned.
   // Local address is the local peer address and port
   virtual talk_base::SocketAddress GetLocalAddress() const;

   // Returns the address to which the socket is connected.  If the socket is
   // not connected, then the any-address is returned.
   // remote address is the relay connect address, not the remote peer address
   virtual talk_base::SocketAddress GetRemoteAddress() const;

   //IEEE Std 1003.1
   //Upon successful completion, bind() shall return 0; 
   //otherwise, -1 shall be returned and errno set to indicate the error.
   virtual int Bind(const talk_base::SocketAddress& addr);

   //IEEE Std 1003.1
   //Upon successful completion, connect() shall return 0; 
   //otherwise, -1 shall be returned and errno set to indicate the error.
   //addr is the relay connect addr, not the peer addr.
   virtual int Connect(const talk_base::SocketAddress& addr);

   //IEEE Std 1003.1
   //Upon successful completion, send() shall return the number of bytes sent. 
   //Otherwise, -1 shall be returned and errno set to indicate the error.
   virtual int Send(const void *pv, size_t cb);
   virtual int SendTo(const void *pv, size_t cb, const talk_base::SocketAddress& addr);

   //IEEE Std 1003.1
   //Upon successful completion, recv() shall return the length of the message 
   //in bytes. If no messages are available to be received and the peer has 
   //performed an orderly shutdown, recv() shall return 0. Otherwise, -1 shall 
   //be returned and errno set to indicate the error.
   virtual int Recv(void *pv, size_t cb);
   virtual int RecvFrom(void *pv, size_t cb, talk_base::SocketAddress *paddr);

   //IEEE Std 1003.1
   //Upon successful completions, listen() shall return 0; otherwise, -1 shall 
   //be returned and errno set to indicate the error.
   virtual int Listen(int backlog);

   //virtual talk_base::Socket *Accept(talk_base::SocketAddress *paddr);

   //IEEE Std 1003.1
   //Upon successful completion, 0 shall be returned; otherwise, -1 shall
   //be returned and errno set to indicate the error.
   virtual int Close();

   virtual int GetError() const;
   virtual void SetError(int error);
   
   virtual ConnState GetState() const ;

   // Fills in the given uint16 with the current estimate of the MTU along the
   // path to the address to which this socket is connected. NOTE: This method
   // can block for up to 10 seconds on Windows.
   virtual int EstimateMTU(uint16* mtu);

   //enum Option {
   //  OPT_DONTFRAGMENT,
   //  OPT_RCVBUF,      // receive buffer size
   //  OPT_SNDBUF,      // send buffer size
   //  OPT_NODELAY,     // whether Nagle algorithm is enabled
   //  OPT_IPV6_V6ONLY, // Whether the socket is IPv6 only.
   //  OPT_DSCP         // DSCP code
   //};
   enum ConnState{
     CS_SOCKET_CONNECTING,
     CS_SOCKET_CONNECTED,

     CS_P2P_CONNECTED,
     CS_P2P_CONNECTING,

     CS_CLOSED
   };
   //If successful, return 0; otherwise return -1;
   virtual int GetOption(Option opt, int* value);
   virtual int SetOption(Option opt, int value);
protected:
  void CreateP2PSystemCommand(uint32 command_type,
    uint32 server_socket,uint32 client_socket,
    const talk_base::SocketAddress &addr);
protected:
  int                       error_;
  ConnState                 connect_state_;
  talk_base::SocketAddress  remote_addr_;
  talk_base::SocketAddress  local_addr_;
  uint32                    other_side_socket_;

  P2PConnectionManagement     *p2p_connection_management_;
  P2PConnectionImplementator  *p2p_connection_implementator_;
  P2PProxySession             *p2p_proxy_session_;
};


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
class P2PProxyStartSocket : public P2PProxySocket{
public:
  P2PProxyStartSocket();

  int ConnectPeer(const std::string &addr_ide,
     const std::string &proxy_type);
  virtual void OnP2PConnectSucceed(P2PProxySession *p2p_proxy_session);

  void OnP2PSocketConnectSucceed(P2PProxySession *p2p_proxy_session,
    uint32 other_side_socket);

  void OnP2PSocketConnectFailure(P2PProxySession *p2p_proxy_session);

  virtual int Connect(const talk_base::SocketAddress &addr);
  
private:
};

//////////////////////////////////////////////////////////////////////////
//P2P Proxy End Socket
//////////////////////////////////////////////////////////////////////////
class P2PProxyEndSocket : public P2PProxySocket{
public:
  P2PProxyEndSocket(P2PProxyClientSession *p2p_proxy_session,
                               uint32 other_side_socket);
  virtual void OnP2PSocketConnectSucceed(
    P2PProxySession *p2p_proxy_session,uint32 other_side_socket){};
  virtual void OnP2PSocketConnectFailure(
    P2PProxySession *p2p_proxy_session){};

  virtual void OnP2PConnectSucceed(P2PProxySession *p2p_proxy_session){};

  void SocketConnectSucceed();
  void SocketConnectFailure();
};
#endif // P2P_PROXY_SOCKET_H_
