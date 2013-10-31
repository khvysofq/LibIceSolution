#include "virtual_network.h"

 VirtualNetwork::VirtualNetwork(AbstractICEConnection *p2p_ice_connection):
    AbstractVirtualNetwork(p2p_ice_connection)
 {
   network_header_ = new NetworkHeader();
   network_header_->header_ide_ = P2P_NETWORKER_HEADER_IDE;
 }
 void VirtualNetwork::Destory(){
   delete network_header_;
 }
 VirtualNetwork::~VirtualNetwork(){
   Destory();
 }

 void VirtualNetwork::OnReceiveDataFromLowLayer(talk_base::StreamInterface*
   stream){

 }

 void VirtualNetwork::OnReceiveDataFromUpLayer(int socket,char *data, 
   int len){

 }