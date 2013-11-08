#include <iostream>
#include "talk/base/stream.h"
#include "talk/base/helpers.h"

const char *GenerateRandomData(size_t len){
  std::string random_string = talk_base::CreateRandomString(1024);
  std::string *return_string = new std::string(random_string);
  return return_string->c_str();
}

static const int WRITE_DATA_LENGTH = 256;
static const int READ_DATA_LENGTH  = 128;

int main(void){
  talk_base::FifoBuffer memory_stream(1024);
  while(true){
    std::cout << "1. generate random string that the string length is 32 \n";
    std::string random_string = talk_base::CreateRandomString(WRITE_DATA_LENGTH);

    std::cout << "2. print the random_string" << std::endl;
    std::cout << random_string << std::endl;

    std::cout << "3 . write string to the memory_stream" << std::endl;
    size_t write_count = 0;
    talk_base::StreamResult res 
      = memory_stream.Write(random_string.c_str(),random_string.length(),
      &write_count,NULL);
    if(res != talk_base::SR_SUCCESS){
      std::cout << "ERROR:\t can't write data to memory stream" << std::endl;
    } else {
      size_t pos = 0;
      size_t available = 0;
      size_t memory_size = 0;
      memory_stream.GetPosition(&pos);
      memory_stream.GetAvailable(&available);
      memory_stream.GetSize(&memory_size);
      std::cout << "-----------------------------------------\n";
      std::cout << "Write Data Succeed ... ...\n" 
        <<"write data length is " << write_count << std::endl
        << "current memory position is " << pos << std::endl
        << "Current memory available is " << available << std::endl
        << "Current memory size is " << memory_size << std::endl;
      std::cout << "-----------------------------------------\n";
    }
    // Test One
    // memory_stream.SetPosition(0);
    //
    std::cout << "4. get read from the memory stream" << std::endl; 
    size_t read_data_size = 0;
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    const char *read_pos = (const char *)memory_stream.GetReadData(&read_data_size);
    std::cout << "readable data size is " << read_data_size << std::endl;
    for(int i = 0; i < read_data_size; ++i)
      std::cout << read_pos[i];
    std::cout << std::endl;
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

    std::cout << "5. read data from the memory stream" << std::endl; 
    size_t read_count = 0;
    char read_buffer[WRITE_DATA_LENGTH];
    memset(read_buffer,0,WRITE_DATA_LENGTH);
    res = memory_stream.Read(read_buffer,READ_DATA_LENGTH,&read_count,NULL);
    if(res != talk_base::SR_SUCCESS){
      std::cout << "ERROR:\t can't read data to memory stream " << read_count
        << std::endl;
    } else {
      size_t pos;
      size_t available;
      size_t memory_size;
      memory_stream.GetPosition(&pos);
      memory_stream.GetAvailable(&available);
      memory_stream.GetSize(&memory_size);
      std::cout << "*****************************************\n";
      std::cout << "read Data Succeed ... ...\n" 
        <<"write data length is " << write_count << std::endl
        << "current memory position is " << pos << std::endl
        << "Current memory available is " << available << std::endl
        << "Current memory size is " << memory_size << std::endl;
      std::cout << "......................................" << std::endl;
      std::cout << read_buffer << std::endl;
      std::cout << "*****************************************\n";
    }

    int is_continue;
    std::cout << "Is continue <0 = no; other = yes>" << std::endl;
    std::cin >> is_continue;
    if(!is_continue)
      break;
  }
  return 0;
}