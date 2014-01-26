# Echo client program
import socket
import time

MSG = """
******************MEAN******************
0 Getting the client current state
1 start p2p solution program
2 End p2p solution program
3 start cmd mode, type quit to end
----------------------------------------"""

START_COMMAND_MODE      =   '3'
END_COMMAND_MODE        =   'quit'

HOST = '192.168.1.225'     # The remote host
PORT = 14725              # The same port as used by the server
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print "Start the program manager server"
server_socket.bind((HOST,PORT))
print "Bind the ",HOST,":",PORT
server_socket.listen(5)
print "wait a client connect... ..."
while True:
  connection,address = server_socket.accept()
  print "new connection arrived"
  while True:
    print MSG
    input_buffer = raw_input("input a choose :");
    connection.sendall(input_buffer)
    if(input_buffer == START_COMMAND_MODE):
      while True:
        input_buffer = raw_input("CURRENT_DIR/ >>> ");
        connection.sendall(input_buffer)
        if(input_buffer == END_COMMAND_MODE):
          print "End command model"
          break;
        receve_buffer = connection.recv(1024);
        print receve_buffer

    receve_buffer = connection.recv(1024);
    print "RESULT : ",receve_buffer
