import socket
#################################
SERVER_HOST = "127.0.0.1"
SERVER_PORT = 3214
CLIENT_HOST = "127.0.0.1"
CLIENT_PORT = 1234
#################################

FILE_NAME   = "E:\\DBankFile\\p2pchanneltest.txt"

s = socket.socket()
s.connect((SERVER_HOST,SERVER_PORT))
print "connection succeed..."
f=open (FILE_NAME, "rb") 
l = f.read(1024)
while (l):
    s.send(l)
    print "send 1024 bytes"
    l = f.read(1024)
s.close()
f.close()