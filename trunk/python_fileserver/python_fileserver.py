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
print """����Կ�������ʾ����Ϣ��ÿһ�б�ʾһ��IP��ַ��״̬,��һ��ΪIP��ַ���ڶ���Ϊ���õ�״̬��������Ϊ���IP��һ�����ߵ�ʱ�䡣���У����ڵڶ���״̬��˵:
  =======================================================================
  \tavaliable\t����ʹ��\t���IP��ַû����ʹ�ù���
  \tunavailable\t������ʹ��\t���IP��ַ��ǰ���ڱ�ʹ�á�
  \tmaybe available Ҳ�����ʹ��\t��������ʹ�ù����IP��ַ�������ڲ����ߡ�����Կ������ĵ�¼ʱ��,����������
  =======================================================================
  \t����������һ̨����������ÿ��15���ӷ�ping����IP��ַһ�Σ����кܶ���õ��Ľ����
  \t��������������������ڵ�ǰĿ¼�±���һ��ping_ressult.csv���ļ�����excel���߼��±��򿪣���Ҳ���Կ��������"""
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