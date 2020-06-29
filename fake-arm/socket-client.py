import socket
import sys

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = ('localhost', 5002)
print('connecting to %s port %s' % server_address)
sock.connect(server_address)
try:
    
    # Send data
    message = b'r:mode'
    print('sending "%s"' % message)
    sock.sendall(message)

    data = sock.recv(1024)
    print('received "%s"' % data)

    # Send data
    message = b'r:mode! 1'
    print('sending "%s"' % message)
    sock.sendall(message)

    data = sock.recv(1024)
    print('received "%s"' % data)

    # Send data
    message = b'r:mode'
    print('sending "%s"' % message)
    sock.sendall(message)

    data = sock.recv(1024)
    print('received "%s"' % data)

finally:
    print('closing socket')
    sock.close()
