#include <string>
#include <iostream>
#include <stdio.h>
#include "tunnelclient.h"

#define MY_WEBRTC_CONNECT   "132.177.123.6:3478"
#define MY_WEBRTC_SERVER    "42.121.127.71"
const talk_base::SocketAddress
    KServerAddr("192.168.1.116",8888);

const int NO_CMD_INPUT  =   0;
const int CMD_OVER      =   -10;
int thread_cmd_choice   =   NO_CMD_INPUT;
const char *hello_world =   "Hello world!";
const int  SEND_DATA_LENGTH =   kBlockSize;
char  temp_buffer[kBlockSize];

const int KStartSendLen =   128;
const int KEndSendLen   =   256;

int total_send = 0;

class TestTunnelClient : public sigslot::has_slots<>
{
public:
    TestTunnelClient()
    {
        signal_thread_  =   talk_base::Thread::Current();
        is_initator_    =   false;

        my_client_      =   new MyClient(signal_thread_,signal_thread_);

        //my_client_->SignalStreamEvent.connect(this,
        //    &TestTunnelClient::OnStreamEvent);
        my_client_->SignalRecvData.connect(this,
            &TestTunnelClient::OnSignalRecvData);

        my_client_->SignalSendData.connect(this,
            &TestTunnelClient::OnSignalSendData);

        my_client_->SignalOnlinePeer.connect(this,
            &TestTunnelClient::OnSignalOnlinePeer);

        my_client_->SignalServerConnectionFailure.connect(
            this,&TestTunnelClient::OnSignalServerConnectionFailure);


        my_client_->SignalServerConnectionSucceed.connect(
            this,&TestTunnelClient::OnSignalServerConnectionSucceed);
        recv_file_stream_.Open("recv.txt","wb",NULL);
    }
    ~TestTunnelClient()
    {
        my_client_->DisconnectFromServer();
        worker_thread_->Stop();
        recv_file_stream_.Close();
        delete worker_thread_;
        delete my_client_;
    }

    void Init()
    {
        my_client_->StartLogin(KServerAddr.ipaddr().ToString()
            ,KServerAddr.port());
    }
    void OnSignalSendData()
    {
        
        LOG(INFO) <<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\t"<< __FUNCTION__;
        static int i =  KStartSendLen;
        if(i < KEndSendLen)
        {
            total_send += i;
            std::string random_string = talk_base::CreateRandomString(i++);
            std::cout<<"\t\t"<<random_string<<std::endl;
            my_client_->SendData(random_string.c_str(),random_string.length(),NULL);
        }
        LOG(INFO) <<">>>>>>>>>>>>>>>total send\t"<<total_send;
    }


    void ConnectToPeer(int peer_id)
    {
        std::cout<<"Please input the send file name" << std::endl;
        //std::cin>>send_file_name_;
        is_initator_    =   true;
        //send_file_stream_.Open(send_file_name_,
        //    "rb",NULL);
        my_client_->ConnectToPeer(peer_id);
    }
    void OnSignalRecvData(char *data, int len)
    {
        LOG(INFO) <<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\t"<< len;
    }
    void OnSignalOnlinePeer(std::map<int, std::string> peers)
    {
        //显示当前连接上的peer
        std::cout<<"-----------------------------------------------\n";
        for(std::map<int, std::string>::iterator iter = peers.begin();
            iter != peers.end(); iter ++)
            std::cout<< iter->first << "\t" << iter->second << std::endl;
        std::cout<<"-----------------------------------------------\n";
    }

    void OnSignalServerConnectionFailure()
    {
        std::cout <<__FUNCTION__ <<std::endl;
    }

    void OnSignalServerConnectionSucceed()
    {
        std::cout <<__FUNCTION__ <<std::endl;
    }


private:
    talk_base::Thread   *signal_thread_;
    talk_base::Thread   *worker_thread_;
    MyClient            *my_client_;
    bool                is_initator_;

    talk_base::FileStream send_file_stream_;
    talk_base::FileStream recv_file_stream_;
    std::string          send_file_name_;
};


class RunThreadClass : public talk_base::MessageHandler
{
public:
    RunThreadClass()
    {
    }

    void OnMessage(talk_base::Message* msg) 
    {
        std::cout<<"2 \tsend message to peer.\n";
        std::cout<<"Please choice a comand!\n";
        std::cin>>thread_cmd_choice;
    }
private:
};

int main(void)
{

    talk_base::FileStream log_stream;
    log_stream.Open("C:/log.txt",
        "wb",NULL);
    bool debug = true;
    if (debug) {
        talk_base::LogMessage::LogToStream(&log_stream,talk_base::INFO);
    }
    talk_base::Thread	*main_thread    =   talk_base::Thread::Current();
    talk_base::Thread   CMD_thread;

    TestTunnelClient    tunnel_client;
    RunThreadClass  cmd_run;

    tunnel_client.Init();
    CMD_thread.Start();
    CMD_thread.Post(&cmd_run,0,NULL);

    while(true)
    {
        if(thread_cmd_choice != NO_CMD_INPUT)
        {
            switch(thread_cmd_choice)
            {
            case 2:
                {
                    int send_peer;
                    LOG(LS_INFO)<<"Please input the peer ID";
                    std::cin>>send_peer;
                    tunnel_client.ConnectToPeer(send_peer);
                    thread_cmd_choice = CMD_OVER;
                    main_thread->Run();
                    break;
                }
            default :
                LOG(LS_INFO)<<"This is a invalue comand.";
                break;
            }
        }
        main_thread->ProcessMessages(1000);
    }
    main_thread->Run();

    CMD_thread.Stop();
    return 0;
}