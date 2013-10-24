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

#ifndef PEERCONNECTION_SAMPLES_CLIENT_PEER_CONNECTION_CLIENT_H_
#define PEERCONNECTION_SAMPLES_CLIENT_PEER_CONNECTION_CLIENT_H_
#pragma once

#include <map>
#include <string>

//#include "talk/base/nethelpers.h"
#include "talk/base/signalthread.h"
#include "talk/base/sigslot.h"
//#include "talk/base/physicalsocketserver.h"
#include "talk/base/scoped_ptr.h"

class talk_base::AsyncSocket;
class talk_base::SignalThread;
class talk_base::SocketAddress;
//class talk_base::AsyncResolver;

enum P2PServerMessageType
{
    P2P_SEND_PENDING_MESSAGE
};
typedef std::map<int, std::string> Peers;

struct PeerConnectionClientObserver {
  virtual void OnSignedIn() = 0;  // Called when we're logged on.
  virtual void OnDisconnected() = 0;
  virtual void OnPeerConnected(int id, const std::string& name) = 0;
  virtual void OnPeerDisconnected(int peer_id) = 0;
  virtual void OnMessageFromPeer(int peer_id, const std::string& message) = 0;
  virtual void OnMessageSent(int err) = 0;
  virtual void OnServerConnectionFailure() = 0;
  virtual void OnServerConnectionSucceed() = 0;

 protected:
  virtual ~PeerConnectionClientObserver() {}
};

class PeerConnectionClient : public sigslot::has_slots<>,
                             public talk_base::MessageHandler {
 public:
  enum State {
    NOT_CONNECTED,
    RESOLVING,
    SIGNING_IN,
    CONNECTED,
    SIGNING_OUT_WAITING,
    SIGNING_OUT,
  };

  PeerConnectionClient();
  ~PeerConnectionClient();
  

  int id() const;
  bool is_connected() const;
  const Peers& peers() const;

  //initialize observer
  //set the callback class
  //callback class is a basic class that define above.
  void RegisterObserver(PeerConnectionClientObserver* callback);

  //connect check the state and then call the doconnect function
  void Connect(const std::string& server, int port,
               const std::string& client_name);


  //initialize onconnect_data_ and then call the connectontrolSocket function
  bool SendToPeer(int peer_id, const std::string& message);
  void SendMeesageToPeer(int peer_id, const std::string& message);
  //call the SendToPeer function the peer_id is it
  //the messagex is a magical hangup signal.
  //const char kByeMessage[] = "BYE";
  //I don't known how is it.
  bool SendHangUp(int peer_id);
  //check the message
  bool IsSendingMessage();

  //send the sign out message
  //Maybe I known that ConnectControlSocket function send data with
  //onconnect_data by there.
  bool SignOut();

  // implements the MessageHandler interface
  //onle call the function doConnect
  void OnMessage(talk_base::Message* msg);

  void ShowServerConnectionPeer();
  void set_server_ip(talk_base::SocketAddress server_address);

 protected:
     // initialize control_socket_ and hanging_get_ as the same address
     // send sign in data by function ConnectControlSocket with onconnection_data
  void DoConnect();
  // release all
  void Close();
  //set the siglot mechanism
  void InitSocketSignals();
  //using the control_socket_ to connect the server_address_
  bool ConnectControlSocket();
  //send the onconnect_data_ by the socket and clear it.
  void OnConnect(talk_base::AsyncSocket* socket);
  //send the wait command using the socket programmer
  void OnHangingGetConnect(talk_base::AsyncSocket* socket);
  //as the name means.
  void OnMessageFromPeer(int peer_id, const std::string& message);

  // Quick and dirty support for parsing HTTP header values.
  bool GetHeaderValue(const std::string& data, size_t eoh,
                      const char* header_pattern, size_t* value);

  bool GetHeaderValue(const std::string& data, size_t eoh,
                      const char* header_pattern, std::string* value);

  // Returns true if the whole response has been read.
  bool ReadIntoBuffer(talk_base::AsyncSocket* socket, std::string* data,
                      size_t* content_length);

  void OnRead(talk_base::AsyncSocket* socket);

  //read and colse and then connect
  void OnHangingGetRead(talk_base::AsyncSocket* socket);

  // Parses a single line entry in the form "<name>,<id>,<connected>"
  bool ParseEntry(const std::string& entry, std::string* name, int* id,
                  bool* connected);

  int GetResponseStatus(const std::string& response);

  bool ParseServerResponse(const std::string& response, size_t content_length,
                           size_t* peer_id, size_t* eoh);

  void OnClose(talk_base::AsyncSocket* socket, int err);

  void OnResolveResult(talk_base::SignalThread *t);

  PeerConnectionClientObserver* callback_;
  talk_base::SocketAddress server_address_;
  void* resolver_;
  talk_base::scoped_ptr<talk_base::AsyncSocket> control_socket_;
  talk_base::scoped_ptr<talk_base::AsyncSocket> hanging_get_;
  std::string onconnect_data_;
  std::string control_data_;
  std::string notification_data_;
  std::string client_name_;
  State state_;
  int my_id_;
  int romete_id_;
  Peers peers_;
  std::deque<std::string*>       pending_messages_;

};

#endif  // PEERCONNECTION_SAMPLES_CLIENT_PEER_CONNECTION_CLIENT_H_