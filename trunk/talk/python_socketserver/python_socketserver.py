# Echo client program
import socket
import time

HOST = '192.168.1.225'    # The remote host
PORT = 554              # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
msg = "Hello, worldHello, worldHello"
#while True:
while True:
  s.sendall(msg)
  print "send data...",len(msg)
  data = s.recv(1024)
  print "receive data...",len(msg)
  time.sleep(0.2)
s.close()
print 'Received', repr(data)
