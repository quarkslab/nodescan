import pyleeloo
import pynodescan

ips = pyleeloo.ip_list_intervals()
ips.add("127.0.0.1")

ports = pyleeloo.port_list_intervals()
ports.add(pyleeloo.tcp_port(5555))
ports.add(pyleeloo.tcp_port(5556))

targets = pynodescan.IPV4TargetSet(ips, ports)

engine = pynodescan.AsyncEngine(targets, 2, 60)
def on_connect(t, l, h):
    print("on_connect %s" % str(t.port()))
    return True
engine.set_lvl4_connected_callback(on_connect)

def timeout_of_target(t):
    if t.port() == pyleeloo.tcp_port(5555):
        return 5
    return 10

engine.set_timeout_of_target(timeout_of_target)
engine.set_lvl4_finish_callback(lambda t, b, e: print("finish for %s/%d" % (str(t.port()), e)))

engine.launch()
