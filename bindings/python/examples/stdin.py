import pyleeloo
import pynodescan
import ipaddress

targets = pynodescan.ReinjectableTargetStdin(pyleeloo.tcp_port(1245))
engine = pynodescan.AsyncEngine(targets, 1, 100)

def ip2str(ip):
    return str(ipaddress.ip_address(ip))

def send_hello(self, target, lvl4sm, hsm):
    print("send hello to %s" % ip2str(target.ipv4()))
    s = "hello"
    target.send(s)
    return False

n = 4
def finish(target, err):
    print("finished for %s/%s: %d" % (ip2str(target.ipv4()), str(target.port()), err))
    global n
    if n > 0:
        targets.add_target(pynodescan.Target(target.ipv4(), pyleeloo.tcp_port(1246)))
        n -= 1

engine.set_lvl4_connected_callback(send_hello)
engine.set_lvl4_finish_callback(finish)
engine.launch()
