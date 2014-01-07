# Echo client program
import socket
import time

def random_str(randomlength=8):
    str = ''
    chars = 'AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789'
    length = len(chars) - 1
    random = Random()
    for i in range(randomlength):
        str+=chars[random.randint(0, length)]
    return str

HOST = '127.0.0.1'    # The remote host
PORT = 9632              # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

msg = random_str(1024);

# Get the size of the socket's send buffer
bufsize = s.getsockopt( socket.SOL_SOCKET, socket.SO_SNDBUF )
print bufsize
# Get the state of the SO_REUSEADDR option
state = s.getsockopt( socket.SOL_SOCKET, socket.SO_REUSEADDR )
print state
tcp_nodelay = s.getsockopt(socket.SOL_SOCKET,socket.TCP_NODELAY);
print tcp_nodelay;
#while True:
while True:
  s.sendall(msg)
s.close()
print 'Received', repr(data)
