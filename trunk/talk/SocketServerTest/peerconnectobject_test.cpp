#include <Windows.h>
#include "libjingle_p2p_interface_thread.h"
#include <iostream>
#include <crtdbg.h>

const char STUN_SERVER_IP[] =   "stun.endigovoip.com";
const char P2P_SERVER_IP[]  =   "192.168.1.116";
const int  P2P_SERVER_PORT  =   8888;

const char SEND_MESSAGE_STRING[]    =   "Hello world";


//////////////////////////////
const int  PRAGROMM_CLOSE           =   -1;
const int  STATE_INSTANCE           =   0;
const int  STATE_START_TO_CONNECT_PEER        =   1;
const int  STATE_SEND_DATE_TO_PEER       =   2;
const int  STATE_SCUCCEED           =   3;
const int  STATE_WAITE              =   4;
const int  STATE_CLOSE              =   5;
const int  STATE_TUNNEL_CLOSED      =   6;
//////////////////////////////

int        CurrentState;

void OnRecvData(char *data, int len,void *p2p_tunnel_client)
{
    //for(int i = 0; i < len ; i++)
    //    std::cout<< data[i];
    //std::cout<<std::endl;
    //char * data_buffer = new char[len + 1];
    //_memccpy(data_buffer,data,len,sizeof(char));
    //data_buffer[len]    =   '\0';
    //std::cout << data_buffer << std::endl;
    //std::cout << "the length is \t" << len << std::endl;
    //delete data_buffer;
}
void OnOnlinePeer(std::map<int, std::string> peers,void *p2p_tunnel_client)
{
    std::cout<< "---------------------------------"<<std::endl;
    if(!peers.empty())
    {
        for(std::map<int,std::string>::iterator iter = peers.begin();
            iter != peers.end(); iter ++)
            std::cout<< "|\t"<<iter->first <<"\t"<<iter->second << "\t|\n";
    }
    else
        std::cout<<"no online peer on the peer to peer server" << std::endl;
    std::cout<< "---------------------------------"<<std::endl;

}
void OnStateChangeMessage(StateMessageType state_message_type, void *p2p_tunnel_client)
{
    std::cout<<"/////////////////////OnStateChangeMessage"<<std::endl;
    switch(state_message_type)
    {
    case StateMessageType::ERROR_P2P_LOGING_SERVER_FAILURE:
        {
            std::cout<< "ERROR_P2P_LOGING_SERVER_FAILURE" << std::endl;
            CurrentState    =   STATE_START_TO_CONNECT_PEER;
            break;
        }
    case StateMessageType::STATE_P2P_LOGING_SERVER_SUCCEED:
        {
            std::cout<<"STATE_P2P_LOGING_SERVER_SUCCEED"<<std::endl;
            break;
        }
    case StateMessageType::ERROR_P2P_SERVER_NO_THIS_PEER:
        {
            std::cout << "ERROR_P2P_SERVER_NO_THIS_PEER" << std::endl;
            CurrentState    =   STATE_START_TO_CONNECT_PEER;
            break;
        }
    case StateMessageType::STATE_ICE_SEND_DATA_EVENT:
        {
            std::cout << "STATE_ICE_SEND_DATA_EVENT" << std::endl;
            CurrentState    =   STATE_SEND_DATE_TO_PEER;
            break;
        }
    case StateMessageType::ERROR_ICE_PEER_DISCONNECTED:
        {
            std::cout << "ERROR_ICE_PEER_DISCONNECTED" << std::endl;
            CurrentState    =   STATE_START_TO_CONNECT_PEER;
            break;
        }
    case StateMessageType::STATE_ICE_PEER_DISCONNECTED:
        {
            std::cout << "STATE_ICE_PEER_DISCONNECTED" << std::endl;
            CurrentState    =   STATE_WAITE;
            break;
        }
    case StateMessageType::STATE_P2P_SERVER_DISCONNECTED:
        {
            std::cout << "STATE_P2P_SERVER_DISCONNECTED" << std::endl;
            CurrentState    =   STATE_WAITE;
            break;
        }
    }
}

int main(void)
{
    //¼ì²éÄÚ´æÐ¹Â¶
#ifdef _DEBUG
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    //_CrtSetBreakAlloc(1112);
#endif 
    int close_programm = 1;
    int connection_peer;
    ThreadP2PTunnelClient    *tunnel_client = new ThreadP2PTunnelClient();

    //init TestTunnelClient call back function
    ////////////
    CurrentState    =   STATE_INSTANCE;
    tunnel_client->SetCallBackThreadRecvData(OnRecvData);
    tunnel_client->SetCallBackThreadOnlinePeerFunc(OnOnlinePeer);
    tunnel_client->SetCallBackThreadStateChangeMessageFunc(OnStateChangeMessage);
    //set the basic part of the p2p
    tunnel_client->SetDeBugInforLevel(LogInforLevel::LOG_INFO/*,"C:/log_test.txt"*/);
    tunnel_client->SetStunServerAddress(STUN_SERVER_IP,3478);
    tunnel_client->SetP2PServerAddress(P2P_SERVER_IP,P2P_SERVER_PORT);
    tunnel_client->StartLoginP2PServer();


    CurrentState    =   STATE_START_TO_CONNECT_PEER;
    std::string send_message_string;
    send_message_string = "1";
    for(int i = 1; i < 40960; i++)
    {
        send_message_string += "0";
    }

    while(close_programm    !=   PRAGROMM_CLOSE)
    {
        switch(CurrentState)
        {
        case STATE_START_TO_CONNECT_PEER:
            {
                std::cout << "please choose a peers" <<std::endl;
                std::cin >> connection_peer;
                if(connection_peer == PRAGROMM_CLOSE)
                {
                    CurrentState    =   STATE_CLOSE;
                    break;
                }
                else if(connection_peer == 0)
                {
                    CurrentState    =   STATE_SEND_DATE_TO_PEER;
                    break;
                }
                tunnel_client->ConnectToPeer(connection_peer);
                CurrentState    =   STATE_WAITE;
                break;
            }
        case STATE_SEND_DATE_TO_PEER:
            {    
                int ch = 0;
                while(true)
                {
                    if(ch == PRAGROMM_CLOSE)
                    {
                        CurrentState = STATE_CLOSE;
                        break;
                    }
                    while(ch)
                    {
                        tunnel_client->SendData(send_message_string.c_str(),
                            send_message_string.length());
                        Sleep(20);
                        ch --;
                    }
                    std::cout<< "choose to send a message" << std::endl;
                    std::cin >> ch;
                }
                break;
            }
        case STATE_WAITE:
            {
                Sleep(500);
                break;
            }
        case STATE_CLOSE:
            {
                tunnel_client->DisConnectionCurrentPeer();

                //tunnel_client->DisConnctionP2PServer();
                //Sleep(10000);
                CurrentState  =   STATE_WAITE;
                break;
            }
        case STATE_TUNNEL_CLOSED:
            {
                close_programm = PRAGROMM_CLOSE;
                break;
            }
        }
    }
    delete  tunnel_client;
    //delete  tunnel_client;
    return 0;
}

