#include "talk/base/logging.h"
#include "mediator_pattern.h"
#include "defaults.h"

//AbstractICEConnection::AbstractICEConnection(AbstractP2PServerConnection *
//                                             p2p_server_connection)
//  :p2p_server_connection_(p2p_server_connection)
//{
//    p2p_server_connection_ = p2p_server_connection;
//    p2p_server_connection_->SignalReceiveMessageFromRemotePeer.connect(
//      this,&AbstractICEConnection::OnReceiveMessageFromRemotePeer);
//    SignalSendMessageToRemote.connect(p2p_server_connection_,
//      &AbstractP2PServerConnection::OnSendMessageToRemotePeer);
//  //p2p server to ice part initiator
//}
//
//int AbstractICEConnection::set_local_peer_name(
//  std::string local_peer_name)
//{
//  //if(local_peer_name.find('/')
//  //  || local_peer_name.find('@')
//  //  || local_peer_name.find('.')){
//  //  LOG(LS_ERROR) << "the peer_name string can't occur '/', '@' or '.' character";
//  //  return -1;
//  //}
//  local_peer_name_ = local_peer_name;
//  return 0;
//}
//int AbstractICEConnection::GetRemotePeerIdByName(std::string peer_name) const
//{
//  for(PeerInfors::const_iterator iter = remote_peers_.begin();
//    iter != remote_peers_.end(); ++iter)
//    if(iter->second.peer_name_ == peer_name)
//      return iter->first;
//  return 0;
//}
