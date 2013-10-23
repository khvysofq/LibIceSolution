#ifndef MEDIATOR_PATTERN_H_
#define MEDIATOR_PATTERN_H_
#include "talk/base/sigslot.h"
#include <iostream>
#include <map>

typedef std::map<int,std::string> Peers;
class P2PMediator{
public:
    virtual ~P2PMediator();
    
    //virtual void OnReciveMessage(std::string, int) = 0;
    //ICE part
    sigslot::signal2<std::string,int> SignalSendMessageToRemoteByServer;
    sigslot::signal2<std::string,int> SignalReciveRemoteMessageByServer;

    //peer to peer server part
    sigslot::signal2<std::string,int> SignalSendMessageToPeer;
    sigslot::signal2<std::string,int> SignalReciveMessageFromPeer;
    sigslot::signal1<int>             SignalStateChange;
    sigslot::signal1<Peers>           SignalOnlinePeers;
};


class P2PColleague{
public:
    P2PColleague(P2PMediator *p2p_mediator)
        :p2p_mediator_(p2p_mediator){};
protected:
    P2PMediator *p2p_mediator_;
};

#endif