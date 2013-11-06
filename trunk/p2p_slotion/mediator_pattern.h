#ifndef MEDIATOR_PATTERN_H_
#define MEDIATOR_PATTERN_H_
#include <iostream>
#include <map>
#include "talk/base/sigslot.h"
#include "talk/base/socketaddress.h"
#include "talk/base/stream.h"

#include "defaults.h"

typedef std::map<int,std::string> Peers;

////////////////////////////////////////////////////////////////////////
//  Abstract p2p server class declaration
////////////////////////////////////////////////////////////////////////
class AbstractP2PServerConnection
  :public sigslot::has_slots<>
{
public:
  AbstractP2PServerConnection();
  virtual ~AbstractP2PServerConnection(){
    online_peers_.clear();
  }

  //user interface
  //void set_ice_connection(AbstractICEConnection * ice_connection);
  void set_local_peer_name(std::string local_peer_name){
    local_peer_name_ = local_peer_name;
  }
  std::string get_local_name();

  void set_server_address(talk_base::SocketAddress server_address){
    server_address_ = server_address;
  }
  void set_server_address(const std::string &server, int port){
    server_address_.SetIP(server);
    server_address_.SetPort(port);
  }

  std::string get_remote_name(int peer_id) const;

  virtual void SignInP2PServer() = 0;
  virtual bool SignOutP2PServer() = 0;
  sigslot::signal1<StatesChangeType>  SignalStatesChange;
  sigslot::signal1<const PeerInfors>  SignalOnlinePeers;

  virtual bool UpdataPeerInfor(std::string ) = 0;
  //ice to p2p server interface
  virtual void OnSendMessageToRemotePeer(const std::string&, int) = 0;
  sigslot::signal2<const std::string,int> SignalReceiveMessageFromRemotePeer;


protected:
  PeerInfors                  online_peers_;
  talk_base::SocketAddress    server_address_;
  std::string                 local_peer_name_;
  DISALLOW_EVIL_CONSTRUCTORS(AbstractP2PServerConnection);
};


////////////////////////////////////////////////////////////////////////
//  Abstract p2p ICE class declaration
////////////////////////////////////////////////////////////////////////
class AbstractICEConnection
  :public sigslot::has_slots<>
{
public:
  AbstractICEConnection(AbstractP2PServerConnection *p2p_server_connection);
  virtual ~AbstractICEConnection() {
    remote_peers_.clear();
  };
  /////////////////////////////////////////////////////////
  //user interface 
  sigslot::signal1<StatesChangeType>  SignalStatesChange;
  
  virtual void DestroyPeerConnectionIce() = 0;
  virtual void ConnectionToRemotePeer(int remote_peer_id, 
    std::string remote_peer_name) = 0;
  //void set_p2p_server_connection(AbstractP2PServerConnection
  //  *p2p_server_connection);
  const PeerInfors get_remote_peers() const { 
    return remote_peers_; 
  }
  int set_local_peer_name(std::string local_peer_name);
  std::string get_local_peer_name() const {
    return local_peer_name_;
  }

  /////////////////////////////////////////////////////////
  //ice to p2p server interface
  virtual void OnReceiveMessageFromRemotePeer(const std::string,int)  = 0;
  sigslot::signal2<const std::string&,int> SignalSendMessageToRemote;

  /////////////////////////////////////////////////////////
  //ice to up layer interface
  virtual void OnReceiveDataFromUpLayer(const char *, int) = 0;
  sigslot::signal1<talk_base::StreamInterface*> SignalSendDataToUpLayer;
protected:
  void Add_remote_peer(int peer_id, std::string peer_name){
    //remote_peers_.insert(
    //  std::pair<int,std::string>(peer_id,PeerInfor(peer_name,"")));
  };
  int GetRemotePeerIdByName(std::string peer_name) const;
protected:
  AbstractP2PServerConnection *p2p_server_connection_;
  PeerInfors                  remote_peers_;
  //the peer_name string don't occur '/', '@' or '.' character
  std::string                 local_peer_name_;
  DISALLOW_EVIL_CONSTRUCTORS(AbstractICEConnection);
};





////////////////////////////////////////////////////////////////////////
//  Abstract Virtual Network
////////////////////////////////////////////////////////////////////////
class AbstractVirtualNetwork
  :public sigslot::has_slots<>
{
public:
  AbstractVirtualNetwork(AbstractICEConnection *p2p_ice_connection):
    p2p_ice_connection_(p2p_ice_connection){

      p2p_ice_connection_->SignalSendDataToUpLayer.connect(this,
        &AbstractVirtualNetwork::OnReceiveDataFromLowLayer);
      SignalSendDataToLowLayer.connect(p2p_ice_connection_,
        &AbstractICEConnection::OnReceiveDataFromUpLayer);
  }
public:
  //application layer
  sigslot::signal4<int ,SocketType,const char *, int> SignalSendDataToUpLayer;
  virtual void OnReceiveDataFromUpLayer(int,SocketType,const char *, int) = 0;
public:
  //ice part 
  sigslot::signal2<const char *, int> SignalSendDataToLowLayer;
  virtual void OnReceiveDataFromLowLayer(talk_base::StreamInterface* ) = 0;

private:
  AbstractICEConnection *p2p_ice_connection_;
  DISALLOW_EVIL_CONSTRUCTORS(AbstractVirtualNetwork);
};



////////////////////////////////////////////////////////////////////////
//  Abstract Virtual Application Layer
////////////////////////////////////////////////////////////////////////
class AbstarctVirtualApplication :public sigslot::has_slots<>
{
public:
  AbstarctVirtualApplication(AbstractVirtualNetwork *virtual_network)
    :virtual_network_(virtual_network){

      virtual_network_->SignalSendDataToUpLayer.connect(this,
        &AbstarctVirtualApplication::OnReceiveDateFromLowLayer);
      SignalSendDataToLowLayer.connect(virtual_network_,
        &AbstractVirtualNetwork::OnReceiveDataFromUpLayer);
  }
  virtual bool ListenATcpPort(int port) = 0;
  //public:
  //  virtual void BindTcpPort(int tcp_port) = 0;
  //  virtual void BindUdpPort(int udp_port) = 0;
public: // virtual network interface
  sigslot::signal4<int ,SocketType,const char *, int> SignalSendDataToLowLayer;
  virtual void OnReceiveDateFromLowLayer(int socket, SocketType socket_type,
    const char *data, int len) = 0;
private:
  AbstractVirtualNetwork  *virtual_network_;
  DISALLOW_EVIL_CONSTRUCTORS(AbstarctVirtualApplication);
};

#endif