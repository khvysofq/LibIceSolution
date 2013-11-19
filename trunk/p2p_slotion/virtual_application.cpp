/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/18      11:24
 * Filename : F:\GitHub\trunk\p2p_slotion\virtual_application.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: virtual_application
 * File ext : cpp
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

#include "virtual_application.h"
#include "asynctcpsocketnormal.h"
#include "talk/base/thread.h"
#include "talk/base/asynctcpsocket.h"
#include "talk/base/bytebuffer.h"
#include "talk/base/dscp.h"

static const int SEND_BUFFER_LENGTH = 64 * 1024;

VirtualApplication::VirtualApplication(AbstractVirtualNetwork *virtual_network)
  :AbstarctVirtualApplication(virtual_network),rtsp_proxy_server_(NULL)
{
  LOG(LS_INFO) << "---" << __FUNCTION__;
  current_thread_ = talk_base::Thread::Current();
  is_server_    = false;
  p2p_system_command_factory_ = P2PSystemCommandFactory::Instance();

}
void VirtualApplication::Destory(){
  LOG(LS_INFO) << "---" << __FUNCTION__;

}
VirtualApplication::~VirtualApplication(){
  LOG(LS_INFO) << "---" << __FUNCTION__;
  Destory();

}
void VirtualApplication::OnReceiveDateFromLowLayer(uint32 socket, 
                                                   SocketType socket_type,
                                                   const char *data, uint16 len)
{
  LOG(LS_INFO) << "---" << __FUNCTION__;
  if(socket == 0){
    // It must be a create socket command.
    CreateRTSPClientSocketCommand rtsp_client_command;
    if(p2p_system_command_factory_->ParseCommand(&rtsp_client_command,
      data,len)){
        LOG(LS_ERROR) << "Parse Create Command Error";
        return ;
    }

  }
}



bool VirtualApplication::CreateRTSPServer(const talk_base::SocketAddress 
                                          &rtsp_server)
{
  LOG(LS_INFO) << "---" << __FUNCTION__;
  is_server_  = true;
  ASSERT(rtsp_proxy_server_ == NULL);
  rtsp_proxy_server_ = new RTSPProxyServer(virtual_network_,
    current_thread_->socketserver(),rtsp_server);

  
  return true;
}


void VirtualApplication::OnMessage(talk_base::Message *msg){
  LOG(LS_INFO) << "---" << __FUNCTION__;

}