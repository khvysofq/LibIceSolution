#include "mediator_pattern.h"
#include "talk/base/logging.h"

AbstractICEConnection::AbstractICEConnection(
  AbstractP2PServerConnection *p2p_server_connection)
  :p2p_server_connection_(p2p_server_connection)
{
  //p2p server to ice part initiator
  p2p_server_connection_->SignalReceiveMessageFromRemotePeer.connect(
    this,&AbstractICEConnection::OnReceiveMessageFromRemotePeer);
}

int AbstractICEConnection::set_local_peer_name(
  std::string local_peer_name)
{
  if(local_peer_name.find('/')
    || local_peer_name.find('@')
    || local_peer_name.find('.')){
    LOG(LS_ERROR) << "the peer_name string didn't occur '/', '@' or '.' character";
    return -1;
  }
  local_peer_name_ = local_peer_name;
  return 0;
}
int AbstractICEConnection::GetRemotePeerIdByName(std::string peer_name) const
{
  for(Peers::const_iterator iter = remote_peers_.begin();
    iter != remote_peers_.end(); ++iter)
    if(iter->second == peer_name)
      return iter->first;
  return 0;
}

AbstractP2PServerConnection::AbstractP2PServerConnection(
  AbstractICEConnection *ice_connection)
  :ice_connection_(ice_connection)
{
  //ice to p2p server part initiator
  ice_connection_->SignalSendMessageToRemote.connect(this,
    &AbstractP2PServerConnection::OnSendMessageToRemotePeer);
}