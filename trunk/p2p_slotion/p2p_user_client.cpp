#include "talk/base/json.h"
#include "p2p_user_client.h"
#include "virtual_network.h"
#include "virtual_application.h"
#include "peer_connection_ice.h"
#include "peer_connection_server.h"

P2PUserClient::P2PUserClient(talk_base::Thread *worker_thread,
                             talk_base::Thread *signal_thread)
                             :worker_thread_(worker_thread),
                             signal_thread_(signal_thread),
                             p2p_ICE_connection_(NULL),
                             p2p_server_connection_(NULL),
                             p2p_virtual_network_(NULL),
                             initiator_(false)
{
  LOG(LS_INFO) << "+++" << __FUNCTION__;
  receive_buffer_ = new char[RECEIVE_BUFFER_LENGTH];

}
P2PUserClient::~P2PUserClient(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;

}

void P2PUserClient::Initiatlor(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;

  //p2p server
  p2p_server_connection_ = new PeerConnectionServer();
  //the first step is set the local peer source
  //Test_GenerateResource();  
  //SetLocalJidString("Guangleihe@gmail.com");

  std::string first_local_peer_name = GetCurrentComputerUserName();
  first_local_peer_name += JID_DEFAULT_DOMAIN;

  p2p_server_connection_->set_local_peer_name(first_local_peer_name);
  //p2p server user client part
  p2p_server_connection_->SignalStatesChange.connect(this,
    &P2PUserClient::OnStatesChange);
  p2p_server_connection_->SignalOnlinePeers.connect(this,
    &P2PUserClient::OnOnlinePeers);
  //
  //p2p ice
  std::string local_peer_name = p2p_server_connection_->get_local_name();
  p2p_ICE_connection_   = new PeerConnectionIce(worker_thread_,
    signal_thread_,p2p_server_connection_,local_peer_name);
  p2p_ICE_connection_->SignalStatesChange.connect(this,
    &P2PUserClient::OnStatesChange);

  //p2p virtual network
  p2p_virtual_network_ = new VirtualNetwork(p2p_ICE_connection_);
  //p2p virtual application
  p2p_virtual_application_ = new VirtualApplication(p2p_virtual_network_);
}


void P2PUserClient::StartRun(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
  p2p_server_connection_->set_server_address(ServerAddr);
  p2p_server_connection_->SignInP2PServer();
}

void P2PUserClient::ConnectionToPeer(int peer_id){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
  if(!p2p_server_connection_->UpdataPeerInfor("Hello"))
    std::cout << "UpdatePeerInfor failure" << std::endl;
  initiator_ = true;
  std::string remote_peer_name = 
    p2p_server_connection_->get_remote_name(peer_id);
  if(remote_peer_name.empty()){
    LOG(LS_ERROR) << "\t remote_peer_name is empty";
    return ;
  }
  LOG(LS_INFO) << "\t remote peer name is\t" << remote_peer_name;
  p2p_ICE_connection_->ConnectionToRemotePeer(peer_id,remote_peer_name);
  p2p_virtual_application_->ListenATcpPort(554);
}

void P2PUserClient::Destory(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
  p2p_server_connection_->SignOutP2PServer();
  signal_thread_->ProcessMessages(5000);
  delete receive_buffer_;

  if(p2p_virtual_network_){
    delete p2p_virtual_network_;
    p2p_virtual_network_ = NULL;
  }

  if(p2p_ICE_connection_){
    delete p2p_ICE_connection_;
    p2p_ICE_connection_ = NULL;
  }
  if(p2p_server_connection_){
    delete p2p_server_connection_;
    p2p_server_connection_ = NULL;
  }
}

void P2PUserClient::OnReceiveDataFromVirtualNetwokr(int socket,SocketType socket_type,
                                                    const char *data, int len)
{
  LOG(LS_INFO) << "---------------------------------";
  LOG(LS_INFO) << "\t socket number\t" << socket;
  LOG(LS_INFO) << "\t socket_type\t" << socket_type;
  for(int i = 0; i < len ; i++){
    std::cout << data[i] ;
  }
  std::cout << std::endl;
  LOG(LS_INFO) << "---------------------------------";
}

void P2PUserClient::OnStatesChange(StatesChangeType states_type){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
  switch(states_type){
  case STATES_ICE_START_PEER_CONNECTION:
    {
      std::cout << "\tSTATES_ICE_START_PEER_CONNECTION" << std::endl;
      break;
    }
  case STATES_P2P_SERVER_LOGIN_SUCCEED:
    {
      std::cout << "\tSTATES_P2P_SERVER_LOGIN_SUCCEED" << std::endl;
      ////2. server resource
      //for(int i = 0; i < 10; i++){
      //  ServerResource server_resource;
      //  server_resource.peer_ip_     = "127.0.0.2";
      //  server_resource.peer_name_   = "Server";
      //  server_resource.peer_port_   = i * 100;
      //  server_resource.peer_source_ = "Nothing";
      //  AddServerMember(server_resource);
      //}
      //p2p_server_connection_->UpdataPeerInfor(GetResourceJosonString());
      break;
    }
  case STATES_P2P_PEER_CONNECTION:
    {
      std::cout << "\tSTATES_P2P_PEER_CONNECTION" << std::endl;
      break;
    }
  case STATES_P2P_REMOTE_PEER_DISCONNECTED:
    {
      std::cout << "\tSTATES_P2P_REMOTE_PEER_DISCONNECTED" << std::endl;
      break;
    }
  case STATES_P2P_ONLINE_PEER_CONNECTION:
    {
      std::cout << "\tSTATES_P2P_ONLINE_PEER_CONNECTION" << std::endl;
      break;
    }
  case STATES_P2P_PEER_SIGNING_OUT:
    {
      std::cout << "\tSTATES_P2P_PEER_SIGNING_OUT" << std::endl;
      break;
    }
  case ERROR_P2P_SERVER_TIME_OUT:
    {
      std::cout << "\tERROR_P2P_SERVER_TIME_OUT" << std::endl;
      break;
    }
  case ERROR_P2P_SERVER_LOGIN_SERVER_FAILURE:
    {
      std::cout << "\tERROR_P2P_SERVER_LOGIN_SERVER_FAILURE" << std::endl;
      break;
    }
  case ERROR_P2P_PEER_NO_THIS_PEER:
    {
      std::cout << "\tERROR_P2P_PEER_NO_THIS_PEER" << std::endl;
      break;
    }
  case ERROR_P2P_CAN_NOT_SEND_MESSAGE:
    {
      std::cout << "\tERROR_CAN_NOT_SEND_MESSAGE" << std::endl;
      break;
    }
  case STATES_ICE_TUNNEL_SEND_DATA:
    {
      std::cout << "\tSTATES_ICE_TUNNEL_SEND_DATA" << std::endl;
      //if(initiator_)
      //  SendRandomData();
      break;
    }
  case STATES_ICE_TUNNEL_CLOSED:
    {
      std::cout << "\tSTATES_ICE_TUNNEL_CLOSED" << std::endl;
      break;
    }
  }
}
void P2PUserClient::SetLocalJidString(const std::string &jid){
  local_peer_resource_.peer_jid_ = jid;
}

void P2PUserClient::AddServerMember(const ServerResource &server_resource){
  ServerResource *new_server_resource = new ServerResource(server_resource);
  local_peer_resource_.server_resources_.push_back(new_server_resource);
}

std::string P2PUserClient::GetResourceJosonString(){
  std::stringstream joson_string;
  joson_string << "{";
  // Add Peer jid
  joson_string << "\"" << PEER_JID << "\""<< ":" <<"\"" 
    << local_peer_resource_.peer_jid_<< "\",";

  //Add server resource Array identifier
  joson_string << "\"" << SERVER_RESOURCE_ARRAY << "\"" << ": [";
  int size = local_peer_resource_.server_resources_.size();
  for(int i = 0; i < size; ++i){
    joson_string << "{" << "\"" << SERVER_NAME << "\" : "
      << "\"" << local_peer_resource_.server_resources_[i]->peer_name_<< "\","
      << "\"" << SERVER_IP << "\" : "
      << "\"" << local_peer_resource_.server_resources_[i]->peer_ip_<< "\","
      << "\"" << SERVER_PORT << "\" : "
      << local_peer_resource_.server_resources_[i]->peer_port_ << ","
      << "\"" << SERVER_SOURCE << "\" : "
      << "\"" << local_peer_resource_.server_resources_[i]->peer_source_<< "\""
      << "}";
    if(size - i > 1){
      joson_string << ",";
    }
  }
  joson_string << "]";
  joson_string << "}";
  return joson_string.str();
}
void P2PUserClient::Test_GenerateResource(){
  //1. set LocalJidString
  SetLocalJidString("Guangleihe@gmail.com");
  //2. server resource
  for(int i = 0; i < 10; i++){
    ServerResource server_resource;
    server_resource.peer_ip_     = "127.0.0.2";
    server_resource.peer_name_   = "Server";
    server_resource.peer_port_   = i * 100;
    server_resource.peer_source_ = "Nothing";
    AddServerMember(server_resource);
  }
  std::cout << GetResourceJosonString() << std::endl;

  Json::Value root;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(GetResourceJosonString(),root);
  if ( !parsingSuccessful )
  {
    // report to the user the failure and their locations in the document.
    std::cout  << "Failed to parse configuration\n"
      << reader.getFormattedErrorMessages();
    return;
  }

  std::string jid = root[PEER_JID].asString();
  std::cout << "PEER_JID\t" << jid << std::endl;
  Json::Value joson_array = root[SERVER_RESOURCE_ARRAY];
  int size = joson_array.size();
  for(int i = 0; i < size; i++){
    std::cout << joson_array[i][SERVER_NAME] << std::endl;
    std::cout << joson_array[i][SERVER_IP] << std::endl;
    std::cout << joson_array[i][SERVER_PORT] << std::endl;
    std::cout << joson_array[i][SERVER_SOURCE] << std::endl;
  }
  // Get the value of the member of root named 'encoding', return a 'null' value if
  // there is no such member.
}

void P2PUserClient::ParseJosonString(const std::string joson){
  Json::Value root;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(joson,root);
  if ( !parsingSuccessful )
  {
    // report to the user the failure and their locations in the document.
    std::cout  << "Failed to parse configuration\n"
      << reader.getFormattedErrorMessages();
    return;
  }

  //std::string jid = root[PEER_JID].asString();
  //std::cout << "PEER_JID\t" << jid << std::endl;
  Json::Value joson_array = root[SERVER_RESOURCE_ARRAY];
  int size = joson_array.size();
  for(int i = 0; i < size; i++){
    std::cout << "\t\t" << joson_array[i][SERVER_NAME];
    std::cout << "\t\t" << joson_array[i][SERVER_IP];
    std::cout << "\t\t" << joson_array[i][SERVER_PORT];
    std::cout << "\t\t" << joson_array[i][SERVER_SOURCE] << std::endl;
  }
  // Get the value of the member of root named 'encoding', return a 'null' value if
  // there is no such member.
}

void P2PUserClient::OnReceiveDataFromLoweLayer(talk_base::StreamInterface* stream){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
  size_t res = 0;
  stream->ReadAll(receive_buffer_,RECEIVE_BUFFER_LENGTH,&res,NULL);
  LOG(LS_INFO) << "\t receive data \t" << res;
}

void P2PUserClient::SendRandomData(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
  //SignalSendDataToLowLayer(receive_buffer_,1024);
  std::string msg = "Hello world Hello world Hello world Hello world \
                    Hello world Hello world Hello world Hello world";
  p2p_virtual_application_->SignalSendDataToLowLayer(1,TCP_SOCKET,
    receive_buffer_,8000);
  //p2p_virtual_network_->OnReceiveDataFromUpLayer(1,1000,msg.c_str(),
  //  msg.length());
}

void P2PUserClient::OnOnlinePeers(const PeerInfors peers){
  LOG(LS_INFO) << "+++" << __FUNCTION__;

  std::cout << "====================================" << std::endl;
  for(PeerInfors::const_iterator iter = peers.begin();
    iter != peers.end();++iter){
      std::cout << iter->first << "\t" << iter->second.peer_name_ << std::endl;
      std::cout << iter->second.resource_ << std::endl;
      //ParseJosonString(iter->second.resource_);
  }
  std::cout << "\n====================================" << std::endl;
}

void P2PUserClient::OnMessage(talk_base::Message* msg){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
}