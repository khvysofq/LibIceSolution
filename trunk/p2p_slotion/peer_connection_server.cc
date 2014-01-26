/*
* libjingle
* Copyright 2012, Google Inc.
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

#include "peer_connection_server.h"

#include "defaults.h"
#include "talk/base/common.h"
#include "talk/base/nethelpers.h"
#include "talk/base/logging.h"
#include "talk/base/stringutils.h"

#ifdef WIN32
#include "talk/base/win32socketserver.h"
#endif

#include "p2psourcemanagement.h"

using talk_base::sprintfn;

namespace {

  // This is our magical hangup signal.
  const char kByeMessage[] = "BYE";
  // Delay between server connection retries, in milliseconds
  const int kReconnectDelay   = 2000;
  const int KHeartbeat        = 30 * 1000;
  const int SEND_MESSAGE      = 1;
  const int RECONNECT_MESSAGE = 2;
  const int HEARTBEAT         = 3;

  const char HEARTBEAT_DATA[]           = "Hello";
  const char HEARTBEAT_REPEAT[]         = "ok";
  const size_t HEARBEAT_LENGTH          = 6;
  const size_t HEARBEAT_REPEAT_LENGTH   = 3;

  talk_base::AsyncSocket* CreateClientSocket(int family) {
    //#ifdef WIN32
    //  talk_base::Win32Socket* sock = new talk_base::Win32Socket();
    //  sock->CreateT(family, SOCK_STREAM);
    //  return sock;
    //#elif defined(POSIX)
    talk_base::Thread* thread = talk_base::Thread::Current();
    ASSERT(thread != NULL);
    return thread->socketserver()->CreateAsyncSocket(family, SOCK_STREAM);
    //#else
    //#error Platform not supported.
    //#endif
  }

}

PeerConnectionServer::PeerConnectionServer()
  :resolver_(NULL),
  state_(NOT_CONNECTED),
  my_id_(-1),
  has_heart_(false)
{
  LOG_P2P(CREATE_DESTROY_INFOR | P2P_SERVER_LOGIC_INFOR)
    << "Create a p2p server object";
  p2P_source_management_ = P2PSourceManagement::Instance();
}

PeerConnectionServer::~PeerConnectionServer() {
  LOG_P2P(CREATE_DESTROY_INFOR | P2P_SERVER_LOGIC_INFOR)
    << "Destroy p2p server object";
}

void PeerConnectionServer::InitSocketSignals() {
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "Initiator p2p server socket";
  ASSERT(control_socket_.get() != NULL);
  ASSERT(hanging_get_.get() != NULL);
  control_socket_->SignalCloseEvent.connect(this,
    &PeerConnectionServer::OnClose);
  hanging_get_->SignalCloseEvent.connect(this,
    &PeerConnectionServer::OnClose);
  control_socket_->SignalConnectEvent.connect(this,
    &PeerConnectionServer::OnConnect);
  hanging_get_->SignalConnectEvent.connect(this,
    &PeerConnectionServer::OnHangingGetConnect);
  control_socket_->SignalReadEvent.connect(this,
    &PeerConnectionServer::OnRead);
  hanging_get_->SignalReadEvent.connect(this,
    &PeerConnectionServer::OnHangingGetRead);
}

int PeerConnectionServer::id() const {
  return my_id_;
}

bool PeerConnectionServer::is_connected() const {
  return my_id_ != -1;
}

const PeerInfors& PeerConnectionServer::peers() const {
  return online_peers_;
}


void PeerConnectionServer::SignInP2PServer() 
{
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "Signing p2p server";

  //check nothing
  if (state_ != NOT_CONNECTED) {
    LOG(WARNING)
      << "The client must not be connected before you can call Connect()";
    SignalStatesChange(ERROR_P2P_SERVER_LOGIN_SERVER_FAILURE);
    return;
  }

  //initialize port
  if (server_address_.port() <= 0)
    server_address_.SetPort(kDefaultServerPort);

  //DNS server
  if (server_address_.IsUnresolved()) {
    LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "resolved the p2p server socket address";
    state_ = RESOLVING;
    resolver_ = (void *)( new talk_base::AsyncResolver());
    ((talk_base::AsyncResolver*)(resolver_))->SignalWorkDone.connect(this,
      &PeerConnectionServer::OnResolveResult);

    //This call is the new call for newest libjingle library
    ((talk_base::AsyncResolver*)(resolver_))->Start(server_address_);
  } else {
    DoConnect();
  }
}

void PeerConnectionServer::OnResolveResult(talk_base::SignalThread *t) {
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "get the really server ip address";
  //this call is past libjingle call
  //if (((talk_base::AsyncResolver*)(resolver_))->error() != 0) {

  //this call is newest call 
  if (((talk_base::AsyncResolver*)(resolver_))->GetError() != 0) {  
    SignalStatesChange(ERROR_P2P_SERVER_LOGIN_SERVER_FAILURE);
    ((talk_base::AsyncResolver*)(resolver_))->Destroy(false);
    resolver_ = NULL;
    state_ = NOT_CONNECTED;
  } else {
    server_address_ = ((talk_base::AsyncResolver*)(resolver_))->address();
    DoConnect();
  }
}

void PeerConnectionServer::DoConnect() {
  std::cout << "\tLogin to " <<server_address_.ToString() << std::endl;
  control_socket_.reset(CreateClientSocket(server_address_.ipaddr().family()));
  hanging_get_.reset(CreateClientSocket(server_address_.ipaddr().family()));
  InitSocketSignals();

  local_peer_name_ = p2P_source_management_->GetLocalPeerName();
  if(local_peer_name_.empty()){
    LOG(LS_ERROR) << "Local peer name is not set";
    return ;
  }

  char buffer[1024];
  sprintfn(buffer, sizeof(buffer),
    "GET /sign_in?%s HTTP/1.0\r\n\r\n", local_peer_name_.c_str());
  onconnect_data_ = buffer;
  
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) 
    << "Generate signing data :" << onconnect_data_;
  bool ret = ConnectControlSocket();
  if (ret)
    state_ = SIGNING_IN;
  if (!ret) {
    SignalStatesChange(ERROR_P2P_SERVER_LOGIN_SERVER_FAILURE);
  }
}
void PeerConnectionServer::OnSendMessageToRemotePeer(
  const std::string& message, int peer_id)
{
  std::string* msg = new std::string(message);
  if (msg) {
    // For convenience, we always run the message through the queue.
    // This way we can be sure that messages are sent to the server
    // in the same order they were signaled without much hassle.
    pending_messages_.push_back(new PendMessage(peer_id,P2P_SEND_REMOTE_MESSAGE,msg));
  }
  if(!pending_messages_.empty())
    talk_base::Thread::Current()->Post(this,SEND_MESSAGE);
}

bool PeerConnectionServer::SendToPeer(int peer_id, const std::string& message) {
  LOG_P2P(P2P_SERVER_DATA_INFOR) << "Send data to other peer "
    << "Peer ID is " << peer_id << "message is " << message;
  if (state_ != CONNECTED)
    return false;

  ASSERT(is_connected());
  ASSERT(control_socket_->GetState() == talk_base::Socket::CS_CLOSED);
  if (!is_connected() || peer_id == -1)
    return false;
  char headers[2048];
  sprintfn(headers, sizeof(headers),
    "POST /message?peer_id=%i&to=%i HTTP/1.0\r\n"
    "Content-Length: %i\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n",
    my_id_, peer_id, message.length());
  onconnect_data_ = headers;
  onconnect_data_ += message;
  return ConnectControlSocket();
}

bool PeerConnectionServer::SendHangUp(int peer_id) {
  return SendToPeer(peer_id, kByeMessage);
}

bool PeerConnectionServer::IsSendingMessage() {
  return state_ == CONNECTED &&
    control_socket_->GetState() != talk_base::Socket::CS_CLOSED;
}

bool PeerConnectionServer::SignOutP2PServer() {
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "Log out to p2p server";
  if (state_ == NOT_CONNECTED || state_ == SIGNING_OUT)
    return true;

  if (hanging_get_->GetState() != talk_base::Socket::CS_CLOSED)
    hanging_get_->Close();

  if (control_socket_->GetState() == talk_base::Socket::CS_CLOSED) {
    LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "Setting the state_ is SIGNING_OUT";
    state_ = SIGNING_OUT;

    if (my_id_ != -1) {
      char buffer[1024];
      sprintfn(buffer, sizeof(buffer),
        "GET /sign_out?peer_id=%i HTTP/1.0\r\n\r\n", my_id_);
      onconnect_data_ = buffer;
      return ConnectControlSocket();
    } else {
      // Can occur if the app is closed before we finish connecting.
      return true;
    }
  } else {
    LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "Setting the state_ is SIGNING_OUT_WAITING";
    state_ = SIGNING_OUT_WAITING;
  }
  return true;
}

void PeerConnectionServer::Close() {
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) 
    << "Close all socket connect, setting the state_ is NOT_CONNECTED";
  control_socket_->Close();
  hanging_get_->Close();
  onconnect_data_.clear();
  if (resolver_ != NULL) {
    ((talk_base::AsyncResolver*)(resolver_))->Destroy(false);
    resolver_ = NULL;
  }
  my_id_ = -1;
  state_ = NOT_CONNECTED;
}
bool PeerConnectionServer::ConnectControlSocket() {
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "Connect p2p server by control_socket";
  ASSERT(control_socket_->GetState() == talk_base::Socket::CS_CLOSED);
  int err = control_socket_->Connect(server_address_);
  if (err == SOCKET_ERROR) {
    LOG(LS_ERROR) << "\tSOCKET_ERROR";
    Close();
    return false;
  }
  return true;
}

void PeerConnectionServer::OnConnect(talk_base::AsyncSocket* socket) {
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << 
    "Control_socket connect succeed, then send the signing data" ;
  ASSERT(!onconnect_data_.empty());
  size_t sent = socket->Send(onconnect_data_.c_str(), 
    onconnect_data_.length());
  ASSERT(sent == onconnect_data_.length());
  UNUSED(sent);
  onconnect_data_.clear();
}

void PeerConnectionServer::OnHangingGetConnect(talk_base::AsyncSocket* socket) {
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "Hanging_socket connect server succeed";
  char buffer[1024];
  sprintfn(buffer, sizeof(buffer),
    "GET /wait?peer_id=%i HTTP/1.0\r\n\r\n", my_id_);
  int len = strlen(buffer);
  int sent = socket->Send(buffer, len);
  if(has_heart_ == false){
    has_heart_ = true;
    talk_base::Thread::Current()->PostDelayed(KHeartbeat,this,HEARTBEAT);
  }
  ASSERT(sent == len);
  UNUSED2(sent, len);
}

void PeerConnectionServer::OnMessageFromPeer(int peer_id,
                                             const std::string& message) 
{
  if (message.length() == (sizeof(kByeMessage) - 1) &&
    message.compare(kByeMessage) == 0) {
      SignalStatesChange(STATES_P2P_REMOTE_PEER_DISCONNECTED);
  } else {
    LOG_P2P(P2P_SERVER_DATA_INFOR) << "Get the remote peer message";
    SignalReceiveMessageFromRemotePeer(message,peer_id);
  }
}

bool PeerConnectionServer::GetHeaderValue(const std::string& data,
                                          size_t eoh,
                                          const char* header_pattern,
                                          size_t* value) 
{
  ASSERT(value != NULL);
  size_t found = data.find(header_pattern);
  if (found != std::string::npos && found < eoh) {
    *value = atoi(&data[found + strlen(header_pattern)]);
    return true;
  }
  return false;
}

bool PeerConnectionServer::GetHeaderValue(const std::string& data, size_t eoh,
                                          const char* header_pattern,
                                          std::string* value) 
{
  ASSERT(value != NULL);
  size_t found = data.find(header_pattern);
  if (found != std::string::npos && found < eoh) {
    size_t begin = found + strlen(header_pattern);
    size_t end = data.find("\r\n", begin);
    if (end == std::string::npos)
      end = eoh;
    value->assign(data.substr(begin, end - begin));
    return true;
  }
  return false;
}

bool PeerConnectionServer::ReadIntoBuffer(talk_base::AsyncSocket* socket,
                                          std::string* data,
                                          size_t* content_length) 
{
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "check whether the data is complete";
  char buffer[0xffff];
  do {
    int bytes = socket->Recv(buffer, sizeof(buffer));
    if (bytes <= 0)
      break;
    if(strncmp(buffer,HEARTBEAT_REPEAT,HEARBEAT_REPEAT_LENGTH) == 0){
      std::cout << "receive heartbeat" << std::endl;
      return false;
    }
    data->append(buffer, bytes);
  } while (true);

  bool ret = false;
  size_t i = data->find("\r\n\r\n");
  if (i != std::string::npos) {
    if (GetHeaderValue(*data, i, "\r\nContent-Length: ", content_length)) {
      size_t total_response_size = (i + 4) + *content_length;
      if (data->length() >= total_response_size) {
        ret = true;
        std::string should_close;
        const char kConnection[] = "\r\nConnection: ";
        if (GetHeaderValue(*data, i, kConnection, &should_close) &&
          should_close.compare("close") == 0) {
            socket->Close();
            // Since we closed the socket, there was no notification delivered
            // to us.  Compensate by letting ourselves know.
            OnClose(socket, 0);
        }
      } else {
        // We haven't received everything.  Just continue to accept data.
      }
    } else {
      LOG(LS_ERROR) << "No content length field specified by the server.";
    }
  }
  return ret;
}
// The callback function are calling by control_socket_
void PeerConnectionServer::OnRead(talk_base::AsyncSocket* socket) {
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "Reading control_socket data from server";
  size_t content_length = 0;
  if (ReadIntoBuffer(socket, &control_data_, &content_length)) {
    size_t peer_id = 0, eoh = 0;
    bool ok = ParseServerResponse(control_data_, content_length, &peer_id,
      &eoh);
    if (ok) {
      if (my_id_ == -1) {
        // First response.  Let's store our server assigned ID.
        ASSERT(state_ == SIGNING_IN);
        my_id_ = peer_id;
        ASSERT(my_id_ != -1);

        // The body of the response will be a list of already connected peers.
        if (content_length) {
          size_t pos = eoh + 4;
          bool has_member = false;
          while (pos < control_data_.size()) {
            size_t eol = control_data_.find('\n', pos);
            if (eol == std::string::npos)
              break;
            int id = 0;
            std::string name;
            std::string resource;
            bool connected;
            if (ParseEntry(control_data_.substr(pos, eol - pos), &name, &id,
              &resource,&connected) && id != my_id_) {
                online_peers_[id].peer_name_ = name;
                online_peers_[id].resource_ = resource;
                has_member = true;
                LOG_P2P(P2P_SERVER_LOGIC_INFOR)
                  << "Current Connection Peer\t"<<id<<"\t"<<name;
            }
            pos = eol + 1;
          }
          if(has_member)
            SignalOnlinePeers(online_peers_);
        }
        ASSERT(is_connected());
      } else if (state_ == SIGNING_OUT) {
        Close();
        SignalStatesChange(STATES_P2P_PEER_SIGNING_OUT);
      } else if (state_ == SIGNING_OUT_WAITING) {
        SignOutP2PServer();
      }
    }

    control_data_.clear();

    if (state_ == SIGNING_IN) {
      ASSERT(hanging_get_->GetState() == talk_base::Socket::CS_CLOSED);
      LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "setting the state_ is CONNECTED "
        << "use the hanging_socket to connect the server";
      state_ = CONNECTED;
      hanging_get_->Connect(server_address_);
      SignalStatesChange(STATES_P2P_SERVER_LOGIN_SUCCEED);
    }
  }
}

void PeerConnectionServer::OnHangingGetRead(talk_base::AsyncSocket* socket) {
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "Reading hanging_socket data from server";

  size_t content_length = 0;
  if (ReadIntoBuffer(socket, &notification_data_, &content_length)) {
    size_t peer_id = 0, eoh = 0;
    bool ok = ParseServerResponse(notification_data_, content_length,
      &peer_id, &eoh);

    if (ok) {
      // Store the position where the body begins.
      size_t pos = eoh + 4;

      if (my_id_ == static_cast<int>(peer_id)) {
        // A notification about a new member or a member that just
        // disconnected.
        int id = 0;
        std::string name;
        std::string resource;
        bool connected = false;
        if (ParseEntry(notification_data_.substr(pos), &name, &id,&resource,
          &connected)) {
            if (connected) {
              online_peers_[id].peer_name_ = name;
              online_peers_[id].resource_ = resource;
              SignalAPeerLogin(id,online_peers_[id]);
            } else {
              SignalAPeerLogout(id,online_peers_[id]);
              online_peers_.erase(id);
            }
        }
      } else {
        OnMessageFromPeer(peer_id, notification_data_.substr(pos));
      }
    }

    notification_data_.clear();
  }

  if (hanging_get_->GetState() == talk_base::Socket::CS_CLOSED &&
    state_ == CONNECTED) {
      hanging_get_->Connect(server_address_);
  }
}

bool PeerConnectionServer::ParseEntry(const std::string& entry,
                                      std::string* name,
                                      int* id,
                                      std::string *resource,
                                      bool* connected) 
{

  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "Parse member data";
  ASSERT(name != NULL);
  ASSERT(id != NULL);
  ASSERT(connected != NULL);
  ASSERT(!entry.empty());

  *connected = false;
  size_t separator = entry.find(',');
  if (separator != std::string::npos) {
    *id = atoi(&entry[separator + 1]);
    name->assign(entry.substr(0, separator));
    separator = entry.find(',', separator + 1);
    if (separator != std::string::npos) {
      *connected = atoi(&entry[separator + 1]) ? true : false;
      separator = entry.find(',', separator + 1);
      if (separator != std::string::npos) {
        resource->assign(entry.substr(separator + 1, std::string::npos));
      }
    }
  }
  return !name->empty();
}

int PeerConnectionServer::GetResponseStatus(const std::string& response) {
  LOG_P2P(P2P_SERVER_LOGIC_INFOR)
    << "get the response status by parse response";
  int status = -1;
  size_t pos = response.find(' ');
  if (pos != std::string::npos)
    status = atoi(&response[pos + 1]);
  return status;
}

bool PeerConnectionServer::ParseServerResponse(const std::string& response,
                                               size_t content_length,
                                               size_t* peer_id,
                                               size_t* eoh) 
{
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "Parsing this response";
  int status = GetResponseStatus(response.c_str());
  if (status != 200) {
    LOG(LS_ERROR) << "Received error from server";
    Close();
    SignalStatesChange(STATES_P2P_REMOTE_PEER_DISCONNECTED);
    return false;
  }

  *eoh = response.find("\r\n\r\n");
  ASSERT(*eoh != std::string::npos);
  if (*eoh == std::string::npos)
    return false;

  *peer_id = -1;

  // See comment in peer_channel.cc for why we use the Pragma header and
  // not e.g. "X-Peer-Id".
  GetHeaderValue(response, *eoh, "\r\nPragma: ", peer_id);

  return true;
}
void PeerConnectionServer::ReturnInit(){
  my_id_ = -1;
  
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "setting the state_ is NOT_CONNECTED";
  state_ = NOT_CONNECTED;
  online_peers_.clear();
}

void PeerConnectionServer::OnClose(talk_base::AsyncSocket* socket, int err) {
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "Close the socket connect";
  socket->Close();

#ifdef WIN32
  if(err == WSAETIMEDOUT || err == WSAECONNABORTED){
    LOG(WARNING) << "Connection refused; retrying in 2 seconds ";
    ReturnInit();
    SignalStatesChange(ERROR_P2P_SERVER_LOGIN_SERVER_FAILURE);
    talk_base::Thread::Current()->PostDelayed(kReconnectDelay, 
      this, RECONNECT_MESSAGE);
  }
  if (err != WSAECONNREFUSED) {
#else
  if(err == ETIMEDOUT || err == ECONNABORTED){
    ReturnInit();
    SignalStatesChange(ERROR_P2P_SERVER_LOGIN_SERVER_FAILURE);
    LOG(WARNING) << "Connection refused; retrying in 2 seconds ";
    talk_base::Thread::Current()->PostDelayed(kReconnectDelay, 
      this, RECONNECT_MESSAGE);
  }
  if (err != ECONNREFUSED) {
#endif
    if (socket == hanging_get_.get()) {
      if (state_ == CONNECTED) {
        LOG_P2P(P2P_SERVER_LOGIC_INFOR) 
          << "Hanging_socket close";
        hanging_get_->Close();
        hanging_get_->Connect(server_address_);
      }
    } 
    else {
      LOG_P2P(P2P_SERVER_LOGIC_INFOR) 
        << "control_socket close";
    }
  } 
  else {
    if (socket == control_socket_.get()) {
      LOG(WARNING) << "Connection refused; retrying in 2 seconds ";
      talk_base::Thread::Current()->PostDelayed(kReconnectDelay, 
        this, RECONNECT_MESSAGE);
    } else {
      Close();
      SignalStatesChange(ERROR_P2P_SERVER_LOGIN_SERVER_FAILURE);
    }
  }
}

bool PeerConnectionServer::UpdataPeerInfor(const std::string &infor){
  LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "put Update peer information to pending message";
  std::cout << infor << std::endl;
  std::string* msg = new std::string(infor);
  if (msg) {
    // For convenience, we always run the message through the queue.
    // This way we can be sure that messages are sent to the server
    // in the same order they were signaled without much hassle.
    pending_messages_.push_back(new PendMessage(0,P2P_UPDATE_MESSAGE,msg));
  }
  if(!pending_messages_.empty())
    talk_base::Thread::Current()->Post(this,SEND_MESSAGE);
  return true;
}

bool PeerConnectionServer::SendUpdateMessage(std::string infor){
  LOG_P2P(P2P_SERVER_DATA_INFOR|P2P_SERVER_LOGIC_INFOR) 
    << "Send update information by control_socket";
  if (state_ != CONNECTED)
    return false;

  ASSERT(is_connected());
  ASSERT(control_socket_->GetState() == talk_base::Socket::CS_CLOSED);
  if (!is_connected())
    return false;
  char headers[2048];
  sprintfn(headers, sizeof(headers),
    "POST /update?peer_id=%i HTTP/1.0\r\n"
    "Content-Length: %i\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n",my_id_,infor.length());
  onconnect_data_ = headers;
  onconnect_data_ += infor;
  return ConnectControlSocket();
}

bool PeerConnectionServer::SendMessageToP2PServer(){
  LOG_P2P(P2P_SERVER_LOGIC_INFOR|P2P_SERVER_DATA_INFOR) 
    << "getting message and try to send";

  if (!pending_messages_.empty() && !IsSendingMessage()) {

    PendMessage *send_msg = pending_messages_.front();
    pending_messages_.pop_front();
    if(send_msg->message_type_ == P2P_SEND_REMOTE_MESSAGE){
      if (!SendToPeer(send_msg->remote_id_, *send_msg->message_) 
        && send_msg->remote_id_ != -1) {
          LOG(LS_ERROR) << "SendToPeer failed";
          SignalStatesChange(ERROR_P2P_CAN_NOT_SEND_MESSAGE);
      } 
    } 
    else if(send_msg->message_type_ == P2P_UPDATE_MESSAGE){
      if (!SendUpdateMessage(*send_msg->message_)) {
        LOG(LS_ERROR) << "SendToPeer failed";
        SignalStatesChange(ERROR_P2P_CAN_NOT_SEND_MESSAGE);
      }
    }
    delete send_msg->message_;
    delete send_msg;
  } 
  else {
    LOG_P2P(P2P_SERVER_LOGIC_INFOR) << "Can't Send Message ...";
  }
  return true;
}

void PeerConnectionServer::OnMessage(talk_base::Message* msg) {
  switch(msg->message_id){
  case SEND_MESSAGE:
    {
      SendMessageToP2PServer();
      if(!pending_messages_.empty())
        talk_base::Thread::Current()->PostDelayed(100,this,SEND_MESSAGE);
      break;
    }
  case RECONNECT_MESSAGE:
    {
      //state_ = NOT_CONNECTED;
      DoConnect();
      break;
    }
  case HEARTBEAT:
    {
      hanging_get_->Send(HEARTBEAT_DATA,HEARBEAT_LENGTH);
      talk_base::Thread::Current()->PostDelayed(KHeartbeat,this,HEARTBEAT);
      break;
    }
  }

}
