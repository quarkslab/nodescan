import pyleeloo
import pynodescan
import ipaddress

ips = pyleeloo.ip_list_intervals()
ips.add("127.0.0.1")

ports = pyleeloo.port_list_intervals()
ports.add(pyleeloo.port(1245, pyleeloo.protocol.TCP))

targets = pynodescan.IPV4TargetSet(ips, ports)
engine = pynodescan.AsyncEngine(targets, 1, 600)

def ip2str(ip):
    return str(ipaddress.ip_address(ip))

def p(t, l, h, buf):
    print("recv: " + buf.tobytes().decode('ascii'))
    return True

def send_hello(target, lvl4sm, hsm):
    print("send hello to %s" % ip2str(target.ipv4()))
    s = "hello"
    target.send(s)
    lvl4sm.set_char_data_trigger('Z', p)
    return True

def finish(target, buf, err):
    print("finish with errcode %d and buffer %s" % (err, buf.tobytes().decode('ascii')))

n = 0
def watch_timeout(t):
    global n
    print("watch timeout for %s" % (ip2str(t.ipv4())))
    n += 1
    return n < 4

engine.set_watch_timeout(watch_timeout, 1)
engine.set_lvl4_connected_callback(send_hello)
engine.set_lvl4_finish_callback(finish)

engine.launch()
