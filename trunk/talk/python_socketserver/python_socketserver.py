# Echo client program
import socket
import time

HOST = '127.0.0.1'    # The remote host
PORT = 554              # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
while True:
  s.sendall('Hello, world')
  print "send data ..."
  #data = s.recv(1024)
  time.sleep(1);
s.close()
print 'Received', repr(data)