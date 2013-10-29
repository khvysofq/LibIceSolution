#include "p2p_user_client.h"
#include "peer_connection_ice.h"
#include "peer_connection_server.h"

P2PUserClient::P2PUserClient(talk_base::Thread *worker_thread,
                             talk_base::Thread *signal_thread)
                             :worker_thread_(worker_thread),
                             signal_thread_(signal_thread),
                             p2p_ICE_connection_(NULL),
                             p2p_server_connection_(NULL)
{
  LOG(LS_INFO) << "+++" << __FUNCTION__;
  
}
P2PUserClient::~P2PUserClient(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;

}

void P2PUserClient::Initiatlor(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;

  p2p_ICE_connection_   = new PeerConnectionIce(worker_thread_,
    signal_thread_);
  p2p_server_connection_ = new PeerConnectionServer();
  p2p_ICE_connection_->set_p2p_server_connection(p2p_server_connection_);
  p2p_server_connection_->set_ice_connection(p2p_ICE_connection_);
  
  //p2p ICE part
  p2p_ICE_connection_->SignalStatesChange.connect(this,
    &P2PUserClient::OnStatesChange);
  p2p_ICE_connection_->SignalSendDataToUpLayer.connect(this,
    &P2PUserClient::OnReceiveDataFromLoweLayer);
  SignalSendDataToLowLayer.connect(p2p_ICE_connection_,
    &AbstractICEConnection::OnReceiveDataFromUpLayer);

  //p2p server part
  p2p_server_connection_->SignalStatesChange.connect(this,
    &P2PUserClient::OnStatesChange);
  p2p_server_connection_->SignalOnlinePeers.connect(this,
    &P2PUserClient::OnOnlinePeers);

}

void P2PUserClient::StartRun(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
  p2p_server_connection_->set_server_address(ServerAddr);
  p2p_server_connection_->set_local_peer_name("GuangleiHe@p2p_solution.com");
  p2p_server_connection_->ConnectP2PServer();
}

void P2PUserClient::Destory(){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
  p2p_server_connection_->SignOutP2PServer();
  signal_thread_->ProcessMessages(5000);
  if(p2p_ICE_connection_){
    delete p2p_ICE_connection_;
    p2p_ICE_connection_ = NULL;
  }
  if(p2p_server_connection_){
    delete p2p_server_connection_;
    p2p_server_connection_ = NULL;
  }
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
  case ERROR_CAN_NOT_SEND_MESSAGE:
    {
      std::cout << "\tERROR_CAN_NOT_SEND_MESSAGE" << std::endl;
      break;
    }
 
  }
}

void P2PUserClient::OnReceiveDataFromLoweLayer(char *data, int len){
  LOG(LS_INFO) << "+++" << __FUNCTION__;

}

void P2PUserClient::OnOnlinePeers(const Peers peers){
  LOG(LS_INFO) << "+++" << __FUNCTION__;

  std::cout << "====================================" << std::endl;
  for(Peers::const_iterator iter = peers.begin();
    iter != peers.end();
    ++iter){
      std::cout << iter->first << "\t" << iter->second;
  }
  std::cout << "\n====================================" << std::endl;

}

void P2PUserClient::OnMessage(talk_base::Message* msg){
  LOG(LS_INFO) << "+++" << __FUNCTION__;
}