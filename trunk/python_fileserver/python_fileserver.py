import socket
import sys
#################################
SERVER_HOST = "127.0.0.1"
SERVER_PORT = 3214
CLIENT_HOST = "127.0.0.1"
CLIENT_PORT = 1234
#################################
FILE_NAME = "E:\\google"
CHUNKSIZE = 1024

s = socket.socket()
s.bind((SERVER_HOST,SERVER_PORT))

while True:
    sc, address = s.accept()

    print address
    i=1
    f = open('file_'+ str(i)+".pdf",'wb') #open in binary
    i=i+1
    while (True):
        l = sc.recv(1024)
        while (l):
                f.write(l)
                l = sc.recv(1024)
    f.close()
    sc.close()
s.close()