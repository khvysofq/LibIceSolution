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

#include "peer_connection_client.h"

#include "defaults.h"
#include "talk/base/common.h"
#include "talk/base/nethelpers.h"
#include "talk/base/logging.h"
#include "talk/base/stringutils.h"

#ifdef WIN32
#include "talk/base/win32socketserver.h"
#endif

using talk_base::sprintfn;

namespace {

// This is our magical hangup signal.
const char kByeMessage[] = "BYE";
// Delay between server connection retries, in milliseconds
const int kReconnectDelay = 2000;

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

PeerConnectionClient::PeerConnectionClient()
  : callback_(NULL),
    resolver_(NULL),
    state_(NOT_CONNECTED),
    my_id_(-1),
    romete_id_(-1){
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
}

PeerConnectionClient::~PeerConnectionClient() {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
}

void PeerConnectionClient::InitSocketSignals() {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  ASSERT(control_socket_.get() != NULL);
  ASSERT(hanging_get_.get() != NULL);                                                   
  control_socket_->SignalCloseEvent.connect(this,
      &PeerConnectionClient::OnClose);
  hanging_get_->SignalCloseEvent.connect(this,
      &PeerConnectionClient::OnClose);
  control_socket_->SignalConnectEvent.connect(this,
      &PeerConnectionClient::OnConnect);
  hanging_get_->SignalConnectEvent.connect(this,
      &PeerConnectionClient::OnHangingGetConnect);
  control_socket_->SignalReadEvent.connect(this,
      &PeerConnectionClient::OnRead);
  hanging_get_->SignalReadEvent.connect(this,
      &PeerConnectionClient::OnHangingGetRead);
}

int PeerConnectionClient::id() const {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  return my_id_;
}

bool PeerConnectionClient::is_connected() const {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  return my_id_ != -1;
}

const Peers& PeerConnectionClient::peers() const {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  return peers_;
}

void PeerConnectionClient::RegisterObserver(
    PeerConnectionClientObserver* callback) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  ASSERT(!callback_);
  callback_ = callback;
}

void PeerConnectionClient::Connect(const std::string& server, int port,
                                   const std::string& client_name) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  ASSERT(!server.empty());
  ASSERT(!client_name.empty());

  //check nothing
  if (state_ != NOT_CONNECTED) {
    LOG(WARNING)
        << "The client must not be connected before you can call Connect()";
    callback_->OnServerConnectionFailure();
    return;
  }

  if (server.empty() || client_name.empty()) {
    LOG(WARNING)
        << "The server ip or client name is empty. Are you sure you are entry those?";
    callback_->OnServerConnectionFailure();
    return;
  }

  //initialize port
  if (port <= 0)
    port = kDefaultServerPort;

  //initialize server_address ip address
  server_address_.SetIP(server);
  //initialize server_address port
  server_address_.SetPort(port);
  //initialize client_name by getpeername function
  client_name_ = client_name;

  //DNS server
  if (server_address_.IsUnresolved()) {
    state_ = RESOLVING;
    resolver_ = (void *)( new talk_base::AsyncResolver());
    ((talk_base::AsyncResolver*)(resolver_))->SignalWorkDone.connect(this,
                                      &PeerConnectionClient::OnResolveResult);
    ((talk_base::AsyncResolver*)(resolver_))->set_address(server_address_);
    ((talk_base::AsyncResolver*)(resolver_))->Start();
  } else {
    DoConnect();
  }
}

void PeerConnectionClient::OnResolveResult(talk_base::SignalThread *t) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  if (((talk_base::AsyncResolver*)(resolver_))->error() != 0) {
    callback_->OnServerConnectionFailure();
    ((talk_base::AsyncResolver*)(resolver_))->Destroy(false);
    resolver_ = NULL;
    state_ = NOT_CONNECTED;
  } else {
    server_address_ = ((talk_base::AsyncResolver*)(resolver_))->address();
    DoConnect();
  }
}

void PeerConnectionClient::DoConnect() {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__ << "\tLogin to "
        <<server_address_.ToString();
  control_socket_.reset(CreateClientSocket(server_address_.ipaddr().family()));
  hanging_get_.reset(CreateClientSocket(server_address_.ipaddr().family()));
  InitSocketSignals();
  char buffer[1024];
  sprintfn(buffer, sizeof(buffer),
           "GET /sign_in?%s HTTP/1.0\r\n\r\n", client_name_.c_str());
  onconnect_data_ = buffer;

  bool ret = ConnectControlSocket();
  if (ret)
    state_ = SIGNING_IN;
  if (!ret) {
    callback_->OnServerConnectionFailure();
  }
}
void PeerConnectionClient::SendMeesageToPeer(int peer_id, 
                                             const std::string& message)
{
    romete_id_ = peer_id;
    std::string* msg = new std::string(message);
    if (msg) {
        // For convenience, we always run the message through the queue.
        // This way we can be sure that messages are sent to the server
        // in the same order they were signaled without much hassle.
        pending_messages_.push_back(msg);
    }
    if(!pending_messages_.empty())
        talk_base::Thread::Current()->PostDelayed(20,this,
        P2P_SEND_PENDING_MESSAGE);
}

bool PeerConnectionClient::SendToPeer(int peer_id, const std::string& message) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  if (state_ != CONNECTED)
    return false;

  ASSERT(is_connected());
  ASSERT(control_socket_->GetState() == talk_base::Socket::CS_CLOSED);
  if (!is_connected() || peer_id == -1)
    return false;
  LOG(LS_INFO) << "1~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
  LOG(LS_INFO) << message;
  LOG(LS_INFO) << "1~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
  char headers[2048];
  sprintfn(headers, sizeof(headers),
      "POST /message?peer_id=%i&to=%i HTTP/1.0\r\n"
      "Content-Length: %i\r\n"
      "Content-Type: text/plain\r\n"
      "\r\n",
      my_id_, peer_id, message.length());
  onconnect_data_ = headers;
  onconnect_data_ += message;
  LOG(LS_INFO) << "2~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
  LOG(LS_INFO) << onconnect_data_;
  LOG(LS_INFO) << "2~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";



  return ConnectControlSocket();
}

bool PeerConnectionClient::SendHangUp(int peer_id) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  return SendToPeer(peer_id, kByeMessage);
}

bool PeerConnectionClient::IsSendingMessage() {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  return state_ == CONNECTED &&
         control_socket_->GetState() != talk_base::Socket::CS_CLOSED;
}

bool PeerConnectionClient::SignOut() {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  if (state_ == NOT_CONNECTED || state_ == SIGNING_OUT)
    return true;

  if (hanging_get_->GetState() != talk_base::Socket::CS_CLOSED)
    hanging_get_->Close();

  if (control_socket_->GetState() == talk_base::Socket::CS_CLOSED) {
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
    state_ = SIGNING_OUT_WAITING;
  }

  return true;
}

void PeerConnectionClient::Close() {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  control_socket_->Close();
  hanging_get_->Close();
  onconnect_data_.clear();
  peers_.clear();
  if (resolver_ != NULL) {
    ((talk_base::AsyncResolver*)(resolver_))->Destroy(false);
    resolver_ = NULL;
  }
  my_id_ = -1;
  state_ = NOT_CONNECTED;
}
int ConnectControlSocket_count = 0;
int OnConnect_count = 0;
bool PeerConnectionClient::ConnectControlSocket() {
  LOG(LS_INFO) <<"@@@"<<__FUNCTION__ << " ConnectControlSocket_count = "
      << ConnectControlSocket_count ++;
  ASSERT(control_socket_->GetState() == talk_base::Socket::CS_CLOSED);
  int err = control_socket_->Connect(server_address_);
  if (err == SOCKET_ERROR) {
    LOG(LS_ERROR) << "@@@" << __FUNCTION__ << "\tSOCKET_ERROR";
    Close();
    return false;
  }
  return true;
}

void PeerConnectionClient::OnConnect(talk_base::AsyncSocket* socket) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__ << " OnConnect_count = "
        << OnConnect_count++;
  ASSERT(!onconnect_data_.empty());
  LOG(LS_INFO) << "++++++++++++++++++++++++++++++++++++++++++++";
  LOG(LS_INFO) << onconnect_data_;
  LOG(LS_INFO) << "++++++++++++++++++++++++++++++++++++++++++++";
  size_t sent = socket->Send(onconnect_data_.c_str(), onconnect_data_.length());
  ASSERT(sent == onconnect_data_.length());
  UNUSED(sent);
  onconnect_data_.clear();
}

void PeerConnectionClient::OnHangingGetConnect(talk_base::AsyncSocket* socket) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
    char buffer[1024];
  sprintfn(buffer, sizeof(buffer),
           "GET /wait?peer_id=%i HTTP/1.0\r\n\r\n", my_id_);
  int len = strlen(buffer);
  int sent = socket->Send(buffer, len);
  callback_->OnServerConnectionSucceed();
  ASSERT(sent == len);
  UNUSED2(sent, len);
}

void PeerConnectionClient::OnMessageFromPeer(int peer_id,
                                             const std::string& message) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  if (message.length() == (sizeof(kByeMessage) - 1) &&
      message.compare(kByeMessage) == 0) {
    callback_->OnPeerDisconnected(peer_id);
  } else {
    callback_->OnMessageFromPeer(peer_id, message);
  }
}

bool PeerConnectionClient::GetHeaderValue(const std::string& data,
                                          size_t eoh,
                                          const char* header_pattern,
                                          size_t* value) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  ASSERT(value != NULL);
  size_t found = data.find(header_pattern);
  if (found != std::string::npos && found < eoh) {
    *value = atoi(&data[found + strlen(header_pattern)]);
    return true;
  }
  return false;
}

bool PeerConnectionClient::GetHeaderValue(const std::string& data, size_t eoh,
                                          const char* header_pattern,
                                          std::string* value) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
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

bool PeerConnectionClient::ReadIntoBuffer(talk_base::AsyncSocket* socket,
                                          std::string* data,
                                          size_t* content_length) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  char buffer[0xffff];
  do {
    int bytes = socket->Recv(buffer, sizeof(buffer));
    if (bytes <= 0)
      break;
    data->append(buffer, bytes);
  } while (true);

  bool ret = false;
  size_t i = data->find("\r\n\r\n");
  if (i != std::string::npos) {
    LOG(INFO) << "Headers received";
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
void PeerConnectionClient::OnRead(talk_base::AsyncSocket* socket) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
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
          while (pos < control_data_.size()) {
            size_t eol = control_data_.find('\n', pos);
            if (eol == std::string::npos)
              break;
            int id = 0;
            std::string name;
            bool connected;
            if (ParseEntry(control_data_.substr(pos, eol - pos), &name, &id,
                           &connected) && id != my_id_) {
              peers_[id] = name;
              LOG(LS_INFO)<<"Current Connection Peer\t"<<id<<"\t"<<name;
              callback_->OnPeerConnected(id, name);
            }
            pos = eol + 1;
          }
        }
        ASSERT(is_connected());
        callback_->OnSignedIn();
      } else if (state_ == SIGNING_OUT) {
        Close();
        callback_->OnDisconnected();
      } else if (state_ == SIGNING_OUT_WAITING) {
        SignOut();
      }
    }

    control_data_.clear();

    if (state_ == SIGNING_IN) {
        ASSERT(hanging_get_->GetState() == talk_base::Socket::CS_CLOSED);
        state_ = CONNECTED;
        if(!pending_messages_.empty())
            talk_base::Thread::Current()->PostDelayed(20,this,
            P2P_SEND_PENDING_MESSAGE);
        hanging_get_->Connect(server_address_);
    }
  }
}
void PeerConnectionClient::ShowServerConnectionPeer()
{
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
    for(Peers::iterator iter =  peers_.begin();
        iter != peers_.end();iter++)
        LOG(LS_INFO)<<iter->first<<"\t"<<iter->second;
}
void PeerConnectionClient::set_server_ip(talk_base::SocketAddress server_address)
{
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
    server_address_ =   server_address;
}


void PeerConnectionClient::OnHangingGetRead(talk_base::AsyncSocket* socket) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
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
        bool connected = false;
        if (ParseEntry(notification_data_.substr(pos), &name, &id,
                       &connected)) {
          if (connected) {
            peers_[id] = name;
            callback_->OnPeerConnected(id, name);
          } else {
            peers_.erase(id);
            callback_->OnPeerDisconnected(id);
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

bool PeerConnectionClient::ParseEntry(const std::string& entry,
                                      std::string* name,
                                      int* id,
                                      bool* connected) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
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
    }
  }
  return !name->empty();
}

int PeerConnectionClient::GetResponseStatus(const std::string& response) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  int status = -1;
  size_t pos = response.find(' ');
  if (pos != std::string::npos)
    status = atoi(&response[pos + 1]);
  return status;
}

bool PeerConnectionClient::ParseServerResponse(const std::string& response,
                                               size_t content_length,
                                               size_t* peer_id,
                                               size_t* eoh) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  int status = GetResponseStatus(response.c_str());
  if (status != 200) {
    LOG(LS_ERROR) << "Received error from server";
    Close();
    callback_->OnDisconnected();
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

void PeerConnectionClient::OnClose(talk_base::AsyncSocket* socket, int err) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;
  socket->Close();

#ifdef WIN32
  if (err != WSAECONNREFUSED) {
#else
  if (err != ECONNREFUSED) {
#endif
    if (socket == hanging_get_.get()) {
      if (state_ == CONNECTED) {
        LOG(LS_INFO) <<"@@@"<<__FUNCTION__ << " state_ == CONNECTED ";
        hanging_get_->Close();
        hanging_get_->Connect(server_address_);
      }
    } else {
      //LOG(LS_WARNING) << "@@@" <<__FUNCTION__ <<"\t" << err;
      //callback_->OnServerConnectionFailure();
      callback_->OnMessageSent(err);
    }
  } else {
    if (socket == control_socket_.get()) {
      LOG(WARNING) << "Connection refused; retrying in 2 seconds";
      talk_base::Thread::Current()->PostDelayed(kReconnectDelay, this, 0);
    } else {
      Close();
      callback_->OnDisconnected();
    }
  }
}

void PeerConnectionClient::OnMessage(talk_base::Message* msg) {
    LOG(LS_INFO) <<"@@@"<<__FUNCTION__;

    if(msg->message_id == P2PServerMessageType::P2P_SEND_PENDING_MESSAGE)
    {
        if (!pending_messages_.empty() && !IsSendingMessage()) {
            std::string *send_msg = pending_messages_.front();
            pending_messages_.pop_front();
            LOG(LS_INFO) <<">>>"<<__FUNCTION__ << "\tpeer_id_ =" << romete_id_;
            if (!SendToPeer(romete_id_, *send_msg) && romete_id_ != -1) {
                LOG(LS_ERROR) << "SendToPeer failed";
                callback_->OnMessageSent(-1);
            }
            delete send_msg;
        }
        if(!pending_messages_.empty())
            talk_base::Thread::Current()->PostDelayed(20,this,
            P2P_SEND_PENDING_MESSAGE);
    }
    else
        DoConnect();
}
