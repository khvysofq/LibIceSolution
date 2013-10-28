#ifndef MEDIATOR_PATTERN_H_
#define MEDIATOR_PATTERN_H_
#include <iostream>
#include <map>
#include "talk/base/sigslot.h"
#include "defaults.h"

typedef std::map<int,std::string> Peers;

////////////////////////////////////////////////////////////////////////
//  Abstract p2p server class declaration
////////////////////////////////////////////////////////////////////////
class AbstractP2PServerConnection;

class AbstractICEConnection
  :public sigslot::has_slots<>
{
public:
  AbstractICEConnection(AbstractP2PServerConnection *);
  virtual ~AbstractICEConnection() {
    remote_peers_.clear();
  };
  /////////////////////////////////////////////////////////
  //user interface 
  sigslot::signal1<StatesChangeType>  SignalStatesChange;


  const Peers get_remote_peers() const { 
    return remote_peers_; 
  }
  int set_local_peer_name(std::string local_peer_name);
  std::string get_local_peer_name() const {
    return local_peer_name_;
  }

  /////////////////////////////////////////////////////////
  //ice to p2p server interface
  virtual void OnReceiveMessageFromRemotePeer(std::string,int)  = 0;
  sigslot::signal2<std::string,int> SignalSendMessageToRemote;

  /////////////////////////////////////////////////////////
  //ice to up layer interface
  virtual void OnReceiveDataFromUpLayer(char *, int) = 0;
  sigslot::signal2<char *, int> SignalSendDataToUpLayer;
protected:
  void Add_remote_peer(int peer_id, std::string peer_name){
    remote_peers_.insert(
      std::pair<int,std::string>(peer_id,peer_name));
  };
  int GetRemotePeerIdByName(std::string peer_name) const;
protected:
  AbstractP2PServerConnection *p2p_server_connection_;
  Peers                       remote_peers_;
  //the peer_name string don't occur '/', '@' or '.' character
  std::string                 local_peer_name_;
};


////////////////////////////////////////////////////////////////////////
//  Abstract p2p server class declaration
////////////////////////////////////////////////////////////////////////
class AbstractP2PServerConnection
  :public sigslot::has_slots<>
{
public:
  AbstractP2PServerConnection(AbstractICEConnection * ice_connection);
  virtual ~AbstractP2PServerConnection(){
    online_peers_.clear();
  }

  //user interface
  sigslot::signal1<StatesChangeType>  SignalStatesChange;
  sigslot::signal1<Peers>             SignalOnlinePeers;

  //ice to p2p server interface
  virtual void OnSendMessageToRemotePeer(std::string, int) = 0;
  sigslot::signal2<std::string,int> SignalReceiveMessageFromRemotePeer;

protected:
  AbstractICEConnection     *ice_connection_;
  Peers                       online_peers_;
};
#endif