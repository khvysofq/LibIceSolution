# Echo server program
import socket

HOST = '127.0.0.1'                 # Symbolic name meaning all available interfaces
PORT = 554              # Arbitrary non-privileged port
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
print HOST,PORT
s.listen(5)
conn, addr = s.accept()
print 'Connected by', addr
while 1:
    data = conn.recv(1024)
    if not data: break
    print data
conn.close()