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

#ifndef PEERCONNECTION_SAMPLES_CLIENT_CONDUCTOR_H_
#define PEERCONNECTION_SAMPLES_CLIENT_CONDUCTOR_H_
#pragma once

#include <iostream>
#include <deque>
#include <map>
#include <set>
#include <string>

#include "peer_connection_client.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/base/scoped_ptr.h"

#include "talk/base/helpers.h"
#include "talk/base/logging.h"
#include "talk/base/physicalsocketserver.h"
#include "talk/base/proxyserver.h"
#include "talk/base/socketaddress.h"
#include "talk/base/thread.h"
#include "talk/p2p/base/p2ptransportchannel.h"
#include "talk/p2p/client/basicportallocator.h"
#include "talk/p2p/base/basicpacketsocketfactory.h"
#include "talk/p2p/base/transportdescription.h"
#include "talk/p2p/base/transportdescriptionfactory.h"

const int STUN_SERVER_PORT  =   3478;

// The address of the public STUN server.
static const talk_base::SocketAddress
    kStunAddr("stun.endigovoip.com", STUN_SERVER_PORT);

static const talk_base::SocketAddress
    KServerAddr("42.121.127.71",8888);

const int SIGNALING_DELAY	=	0;

// Based on ICE_UFRAG_LENGTH
static const char* kIceUfrag[4] = {"TESTICEUFRAG0000", "TESTICEUFRAG0001",
    "TESTICEUFRAG0002", "TESTICEUFRAG0003"};
// Based on ICE_PWD_LENGTH
static const char* kIcePwd[4] = {"TESTICEPWD00000000000000",
    "TESTICEPWD00000000000001",
    "TESTICEPWD00000000000002",
    "TESTICEPWD00000000000003"};

////////////////////////////////////////////////////////////////////////////
const int   NO_CMD_INPUT        =   -1;
const int   EXIT_THE_PROGRAMM   =   -1;
const int   CMD_RUN_SUCCESS     =   1;
const int   IS_COMMUNICATION    =   1;
const int   NOT_COMMUNICATION   =   0;
////////////////////////////////////////////////////////////////////////////
enum {
  MY_SEND_MESSAGE_ = 1,
  MY_RECIVE_MESSAGE_,
  MY_READY_TO_SEND
};


struct CandidateData : public talk_base::MessageData {
    CandidateData(cricket::TransportChannel* ch,
        const char *data = NULL,size_t len = 0, int flags = 0)
        : channel_(ch),data_(data),len_(len),flags_(flags) {
    }
    cricket::TransportChannel* channel_;
    const char *data_;
    size_t      len_;
    int         flags_;
};


class Conductor : public PeerConnectionClientObserver
    ,public sigslot::has_slots<>
    ,public talk_base::MessageHandler{
public:
    enum CallbackID {
        MEDIA_CHANNELS_INITIALIZED = 1,
        PEER_CONNECTION_CLOSED,
        SEND_MESSAGE_TO_PEER,
        PEER_CONNECTION_ERROR,
        NEW_STREAM_ADDED,
        STREAM_REMOVED,
    };

    Conductor(talk_base::Thread* signaling_thread,
               talk_base::Thread* worker_thread);

    bool connection_active() const;

    virtual void Close();
    ~Conductor();

public:

    void SendData_w(cricket::TransportChannel* channel);
    void RecvData_W(cricket::TransportChannel* channel, const char* data,
                             size_t len, int flags);
    //Create peer connection Factory with peer_connection_factory_
    //create ice server 
    bool InitializePeerConnection();
    void DeletePeerConnection();


    virtual void OnSignedIn();
    virtual void OnDisconnected();
    virtual void OnPeerConnected(int id, const std::string& name);
    virtual void OnPeerDisconnected(int id);
    virtual void OnMessageFromPeer(int peer_id, const std::string& message);
    virtual void OnMessageSent(int err);
    virtual void OnServerConnectionFailure();

    //
    // MainWndCallback implementation.
    //

    virtual void StartLogin(const std::string& server, int port);

    virtual void DisconnectFromServer();

    virtual void ConnectToPeer(int peer_id);

    virtual void DisconnectFromCurrentPeer();

    //p2p_transport------------------------------------------
        //
    void OnSignalCandidatesAllocationDone(cricket::Transport* channel);
    //
    void OnSignalRequestSignaling(cricket::Transport* channel) ;
    void OnSignalCandidatesReady(cricket::Transport* ch,
        const std::vector<cricket::Candidate> &candidates) ;
    //
    void OnSignalRoleConflict() ;
    //
    void OnSignalReadableState(cricket::Transport* trasport);
    //
    void OnSignalWritableState(cricket::Transport* trasport);
    //
    void OnSignalRouteChange(cricket::Transport* transport,
        int component,  // component
        const cricket::Candidate& candidate);
    void OnSignalConnecting(cricket::Transport* trasport);


    
    //p2p_transport------------------------------------------



    //  sigslot::signal1<TransportChannel*> SignalReadyToSend;
    void OnReadyToSend(cricket::TransportChannel* channel) ;
    void OnReadPacket(cricket::TransportChannel* channel, const char* data,
        size_t len, int flags) ;
    void OnMessage(talk_base::Message* msg) ;
    // Send a message to the remote peer.
    void SendMessageToPeer(const std::string& json_object);
    std::string TransportDescriptionToJsonString(cricket::TransportDescription 
        *transport_description);
     cricket::TransportDescription * JsonStringToTransportDescription(
         const std::string &JsonString);


protected:

    int                             peer_id_;
    PeerConnectionClient            *client_;
    std::deque<std::string*>        pending_messages_;
    std::string                     server_;

    cricket::PortAllocator          *allocator_;
    talk_base::NetworkManager	    *network_namager_;
    cricket::TransportChannelImpl	*channel_;
    talk_base::Thread	            *signaling_thread_;
    talk_base::Thread               *worker_thread_;
    cricket::P2PTransport           *p2p_transport_;
    std::vector<cricket::Candidate> romete_candidates_;
    std::vector<cricket::Candidate> local_candidates_;
    
    cricket::TransportDescriptionFactory    transport_description_factory_;
    cricket::TransportDescription           *transport_description_; 
    cricket::TransportOptions               transport_options_;
    cricket::TransportDescription           *romete_transport_description_;
    cricket::TransportChannel*              current_send_channel_;

    std::string                     send_data_;

    bool                            is_peer_connect;
};

#endif  // PEERCONNECTION_SAMPLES_CLIENT_CONDUCTOR_H_
