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

#ifndef PEERCONNECTION_SAMPLES_CLIENT_PEER_CONNECTION_SERVER_H_
#define PEERCONNECTION_SAMPLES_CLIENT_PEER_CONNECTION_SERVER_H_
#pragma once

#include <map>
#include <string>

//#include "talk/base/nethelpers.h"
#include "talk/base/signalthread.h"
#include "talk/base/sigslot.h"
//#include "talk/base/physicalsocketserver.h"
#include "talk/base/scoped_ptr.h"
#include "mediator_pattern.h"

class talk_base::AsyncSocket;
class talk_base::SignalThread;
class talk_base::SocketAddress;
//class talk_base::AsyncResolver;

enum P2PServerMessageType
{
    P2P_SEND_REMOTE_MESSAGE,
    P2P_UPDATE_MESSAGE
};
class MessageDataRemoteID : public talk_base::MessageData
{
public:
  MessageDataRemoteID(int remote_peer_id)
    :remote_peer_id_(remote_peer_id){}
  int remote_peer_id_;
};
class PeerConnectionServer : public talk_base::MessageHandler,
                             public AbstractP2PServerConnection{
 public:
  enum State {
    NOT_CONNECTED,
    RESOLVING,
    SIGNING_IN,
    CONNECTED,
    SIGNING_OUT_WAITING,
    SIGNING_OUT,
  };

  struct PendMessage{
    PendMessage(int remote_id, P2PServerMessageType message_type,std::string *message)
      :remote_id_(remote_id),message_(message),message_type_(message_type){}
    int         remote_id_;
    P2PServerMessageType    message_type_;
    std::string *message_;
  };

  PeerConnectionServer();
  ~PeerConnectionServer();
  

  int id() const;
  bool is_connected() const;
  const PeerInfors& peers() const;


  //connect check the state and then call the doconnect function
  virtual void SignInP2PServer();

  virtual void OnSendMessageToRemotePeer(const std::string& message, int peer_id);
  //call the SendToPeer function the peer_id is it
  //the messages is a magical hangup signal.
  //const char kByeMessage[] = "BYE";
  //I don't known how is it.
  bool SendHangUp(int peer_id);

  //send the sign out message
  //Maybe I known that ConnectControlSocket function send data with
  //onconnect_data by there.
  bool SignOutP2PServer();

  // implements the MessageHandler interface
  //only call the function doConnect
  void OnMessage(talk_base::Message* msg);

  //void ShowServerConnectionPeer();
  //void set_server_ip(talk_base::SocketAddress server_address);
  virtual bool UpdataPeerInfor(std::string infor);

 protected:
   //send message to p2p server
   bool SendMessageToP2PServer();
   //send update message
   bool SendUpdateMessage(std::string infor);
  //check the message
  bool IsSendingMessage();
     //initialize onconnect_data_ and then call the connectontrolSocket function
  bool SendToPeer(int peer_id, const std::string& message);
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

  //read and close and then connect
  void OnHangingGetRead(talk_base::AsyncSocket* socket);

  // Parses a single line entry in the form "<name>,<id>,<connected>"
  bool ParseEntry(const std::string& entry, std::string* name, int* id,
    std::string *resource,bool* connected);

  int GetResponseStatus(const std::string& response);

  bool ParseServerResponse(const std::string& response, size_t content_length,
                           size_t* peer_id, size_t* eoh);

  void OnClose(talk_base::AsyncSocket* socket, int err);

  void OnResolveResult(talk_base::SignalThread *t);

  void* resolver_;
  talk_base::scoped_ptr<talk_base::AsyncSocket> control_socket_;
  talk_base::scoped_ptr<talk_base::AsyncSocket> hanging_get_;
  std::string onconnect_data_;
  std::string control_data_;
  std::string notification_data_;
  State state_;
  int   my_id_;
  std::deque<PendMessage *>       pending_messages_;
};

#endif  // PEERCONNECTION_SAMPLES_CLIENT_PEER_CONNECTION_CLIENT_H_
