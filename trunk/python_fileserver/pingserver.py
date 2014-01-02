#coding=gbk
import sys
import threading
import SocketServer
import subprocess
import time
import python_ping

start_time = time.strftime("%Y-%m-%d %H:%M:%S")
############################################################################
##Basic type define

#define the host node information structure
class HostNode:
  def __init__(self,host_ip,host_status,last_used):
    self.host_ip_       = host_ip
    self.host_status_   = host_status
    self.last_used_     = last_used

#define the host ip part
HOST_PART             = "192.168.1."
HOST_MIN_IP_ARANGD    = 1
HOST_MAX_IP_ARANGD    = 255

HOST_NONE_PING        = "non ping"
HOST_UNAVAILABLE      = "unavailable"
HOST_AVAILABLE        = "available"
HOST_MAYBE            = "maybe available"

LAST_USED             = "never used"

SERVER_FILE_NAME      = "./ping_server.csv"
############################################################################
##part one run the ping test
class PingTask:
  def __init__(self):
    self.host_nodes_ = []
    self.InitHostNode();

  def InitHostNode(self):
    for i in range(HOST_MIN_IP_ARANGD,HOST_MAX_IP_ARANGD):
      host_ip       = HOST_PART + str(i)
      host_status   = HOST_NONE_PING
      last_used     = LAST_USED
      host_node = HostNode(host_ip,host_status,last_used)
      self.host_nodes_.append(host_node);
      #self.SaveToFile()

  def SaveToFile(self):
    #save the data to file
    output = open(SERVER_FILE_NAME,'w')
    output.write("IP ADDRESS,CURRENT_STATUS,LAST USED\n");
    for x in self.host_nodes_:
      string = ""
      string += x.host_ip_
      string += ","
      string += x.host_status_
      string += ","
      string += x.last_used_
      string += "\n"
      output.write(string);
    output.close()

  def RunPingTask(self):
    #init the host node structure array
    i = 0;
    while True:
      i += 1
      print ">>>>> ping at the ",i,"times :",time.strftime("%Y-%m-%d %H:%M:%S")
      for x in self.host_nodes_:

        #output = subprocess.Popen(["ping.exe","-n","1",x.host_ip_],
        #                          stdout = subprocess.PIPE).communicate()[0]
        ##print output
        result = python_ping.verbose_ping(x.host_ip_,1,1)
        
        if("TIME_OUT" == result):
          if(x.host_status_ == HOST_NONE_PING):
            x.host_status_ = HOST_AVAILABLE
          if(x.host_status_ == HOST_UNAVAILABLE):
            x.host_status_ = HOST_MAYBE
        else:
          x.host_status_ = HOST_UNAVAILABLE
          x.last_used_   = time.strftime("%Y-%m-%d %H:%M:%S")
        #print x.host_ip_,x.host_status_,x.last_used_
      self.SaveToFile()
      print "wait 5 minutes after start"
      time.sleep(60 * 5)



############################################################################
#part two net worker server... ...
class EchoRequestHandler(SocketServer.BaseRequestHandler):
  def handle(self):
    #data = self.request.recv(1024)
    read_file = open(SERVER_FILE_NAME,'r')
    data = read_file.read();
    self.request.send(data)
    read_file.close()
    return

class ThreadedEchoServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    pass
############################################################################
if __name__ == '__main__':

  address = ('localhost', 9999) # let the kernel give us a port
  server = ThreadedEchoServer(address, EchoRequestHandler)
  ip, port = server.server_address # find out what port we were given

  networker_thread = threading.Thread(target=server.serve_forever)
  networker_thread.setDaemon(False) # don't hang on exit
  networker_thread.start()

  print "This server started at ",start_time
  print "--------------------------------------------"
  ping_task = PingTask()
  ping_task.RunPingTask();