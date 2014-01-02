#coding=gbk

import time
import socket
from random import Random
import os

FILE_NAME = "./ping_result.csv"

def random_str(randomlength=8):
  str = ''
  chars = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789'
  length = len(chars) - 1
  random = Random()
  for i in range(randomlength):
      str+=chars[random.randint(0, length)]
  return str
msg = random_str(2046);

###################################################################################
#client receive data

HOST = '192.168.1.235'    # The remote host
PORT = 9999               # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
#data = s.recv(4096)
#print "receive data...",len(msg)
#s.sendall(msg)
data = s.recv(10240)
print data
#time.sleep(30.0/1000);
s.close()
write_file = open(FILE_NAME,'w');
write_file.write(data);
write_file.close();
print """你可以看上面显示的信息，每一行表示一个IP地址的状态,第一列为IP地址，第二列为可用的状态，第三列为这个IP上一次在线的时间。其中，对于第二列状态来说:
  =======================================================================
  \tavaliable\t可以使用\t这个IP地址没有人使用过。
  \tunavailable\t不可以使用\t这个IP地址当前正在被使用。
  \tmaybe available 也许可以使用\t曾今有人使用过这个IP地址，但现在不在线。你可以看它最后的登录时间,再做决定。
  =======================================================================
  \t局域网内有一台服务器，它每隔15分钟分ping所有IP地址一次，运行很多天得到的结果。
  \t当你运行这个程序，它会在当前目录下保存一个ping_ressult.csv的文件，用excel或者记事本打开，你也可以看到结果。"""
os.system("pause")

####################################################################################
#server send data

#HOST = '127.0.0.1'                 # Symbolic name meaning all available interfaces
#PORT = 554              # Arbitrary non-privileged port
#s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#s.bind((HOST, PORT))
#print HOST,PORT
#s.listen(5)
#conn, addr = s.accept()
#print 'Connected by', addr
#while 1:
#  conn.sendall(msg)
#  print "send data ...",len(msg)
#  #data = conn.recv(1024)
#  time.sleep(120.0/1000);
#  #data = s.recv(1024)
#conn.close()