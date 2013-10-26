import socket
HOST = "127.0.0.1"
PORT = 1234
s = socket.socket(socket.AF_INET,socket.SOCK_STREAM);
print "initialize socket succeed"
s.connect((HOST,PORT))
#s.bind((HOST,PORT))
print "socket connect succeed"
while True:
  data = s.recv(1024)
  if not data:
    continue
  print data
  s.sendall(data)
s.close()