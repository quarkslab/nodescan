#!/usr/bin/python3.3
#

import socket, random


TCP_IP = '127.0.0.1'
TCP_PORT = 1245

server_sock = None
def main():
    global server_sock
    server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_sock.bind((TCP_IP, TCP_PORT))
    server_sock.listen(1)

    l = ['a', 'b', 'c', 'd']
    random.shuffle(l)
    random_str = ''.join(l)

    conn, addr = server_sock.accept()
    print 'Connection address:', addr
    data = conn.recv(5)
    str_ = data.decode('ascii')
    if str_ != "break":
        print("invalid proto")
        return
    conn.send(random_str.encode('ascii'))
    print("sent " + random_str)
    conn.close()

    conn, addr = server_sock.accept()
    print 'Connection address step2:', addr
    data = conn.recv(4)
    str_ = data.decode('ascii')
    if str_ != random_str:
        print("bad string returned: " + str_)
        return
    print("Received good str!")
    conn.close()

try:
    main()
except KeyboardInterrupt:
    pass

if server_sock:
    server_sock.close()
