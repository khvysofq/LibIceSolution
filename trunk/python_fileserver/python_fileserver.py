# Echo server program
import socket
import time
from random import Random

def random_str(randomlength=8):
  str = ''
  chars = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789'
  length = len(chars) - 1
  random = Random()
  for i in range(randomlength):
      str+=chars[random.randint(0, length)]
  return str

#client coding
msg = random_str(2046)
HOST = '42.121.127.71'    # The remote host
PORT = 554              # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

while True:
  #data = s.recv(4096)
  #print "receive data...",len(msg)
  #s.sendall(msg)
  s.recv(4096)
  #time.sleep(30.0/1000);
s.close()
print 'Received', repr(data)

#server coding

#msg = random_str(1024);

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
#  #time.sleep(20.0/1000);
#  #data = s.recv(1024)
#conn.close()