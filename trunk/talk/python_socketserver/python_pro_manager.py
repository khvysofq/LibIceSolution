import os
import time
import subprocess
import socket

############################################
##Basic define
HOST = '192.168.1.225'     # The remote host
PORT =  14725              # The same port as used by the server
PROGRAM_PATH = "F:/GitHub/trunk/talk/Debug/p2p_slotion.exe"

##constant command define
GETTING_CURRENT_STATE   =   '0'
START_P2P_SLOUTION      =   '1'
END_P2P_SLOUTION        =   '2'

START_COMMAND_MODE      =   '3'
END_COMMAND_MODE        =   'quit'

##constant with current state
JUST_START              = "just start the socket"
RUNNING_PROGRAM         = "running the p2p_sloution"
END_PROGRAM             = "end the p2p_sloution"
ERROR_HAD_RUN           = "the p2p solution had run, you can't double start it."
ERROR_HAD_END           = "the p2p solution had end, you have nothing to end."

##---------------------------------
##FUNCATION that connect the server by TCP always retry
def ConnectServer(SERVER_HOST,SERVER_PORT):
  server_connect = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
  while 1:
    print "Start to connect the server ",SERVER_HOST,":",SERVER_PORT
    try:
      server_connect.connect((SERVER_HOST,SERVER_PORT))
    except Exception,e:
      print "This server connect error"
      continue
    return server_connect
############################################


if __name__ == '__main__':
  
  
  current_state = JUST_START
  server_socket = ConnectServer(HOST,PORT)


  while True:
    try:
      receive_buffer = server_socket.recv(1024);
      
      print receive_buffer
      ############################################
      if(receive_buffer == GETTING_CURRENT_STATE):
        print "Get current state"
        server_socket.sendall(current_state);
      if(receive_buffer == ""):
        server_socket.close()
        server_socket = ConnectServer(HOST,PORT)
        continue
      ############################################
      elif (receive_buffer == START_P2P_SLOUTION):
        if(current_state == RUNNING_PROGRAM):
          print "Error, the program has run"
          server_socket.sendall(ERROR_HAD_RUN);
          continue
        child = subprocess.Popen(PROGRAM_PATH)
        print "run the program",PROGRAM_PATH
        current_state = RUNNING_PROGRAM
        server_socket.sendall(current_state + PROGRAM_PATH);
      ############################################
      elif (receive_buffer == END_P2P_SLOUTION):
        if(current_state == END_PROGRAM):
          print "Error, the program has end"
          server_socket.sendall(ERROR_HAD_END);
          continue
        print "close this program"
        child.kill()
        current_state = END_PROGRAM
        server_socket.sendall(current_state);
      elif (receive_buffer == START_COMMAND_MODE):
        print "start command model"
        while True:
          receive_buffer = server_socket.recv(1024);
          if(receive_buffer == ""):
            break
          print receive_buffer
          if(receive_buffer == END_COMMAND_MODE):
            print "end command model"
            server_socket.sendall("Ok! end of the command model");
            break
          print "..........................."
          output = os.popen(receive_buffer,"r");
          result =  output.read()
          if(result == ""):
            result = "this is not recognized as an internal or external command, operable program or batch file." 
          print result
          server_socket.sendall(result);
          print "..........................."
    except Exception,e:
      server_socket.close()
      server_socket = ConnectServer(HOST,PORT)
