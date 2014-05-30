import ipaddress

class ProtoRouter:
    def __init__(self, routes):
        self.__routes = routes
        self.__default_route = None

    @property
    def routes(self): return self.__routes

    def default_route(self, cb):
        self.__default_route = cb
        return self

    def __call__(self, target, lvl4sm, hsm):
        route = self.routes.get(target.port(), self.__default_route)
        if route == None:
            return False
        return route(target, lvl4sm, hsm)

import pyleeloo
import pynodescan

ips = pyleeloo.ip_list_intervals()
ips.add("103.2.184.0/22")
ips.add("115.126.160.0/19")
ips.add("103.2.184.0/23")
ips.add("127.0.0.1")

ports = pyleeloo.port_list_intervals()
tcp_22 = pyleeloo.port(22, pyleeloo.protocol.TCP)
tcp_443 = pyleeloo.port(443, pyleeloo.protocol.TCP)
ports.add(tcp_22)
ports.add(tcp_443)

targets = pynodescan.IPV4TargetSet(ips, ports)

def ip_target(t):
    return str(ipaddress.ip_address(t.ipv4()))

def save_certif(name):
    def _cb(t, lvl4sm, hsm, buf):
        with open("%s_%s" % (name, ip_target(t)), "wb+") as f:
            f.write(buf.tobytes())
        return False
    return _cb

engine = pynodescan.AsyncEngine(targets, 40000, 60)
engine.set_lvl4_connected_callback(
    ProtoRouter({tcp_22:  pynodescan.protocols.SSH().on_certificate(save_certif("ssh")).on_invalid_answer(lambda t, lvl4sm, hsm, buf: print("SSH failed for %s" % ip_target(t))),
                 tcp_443: pynodescan.protocols.SSL().on_certificate(save_certif("ssl"))})
)

try:
    engine.auto_save_state("scan_state.xml", 60)
    engine.launch()
except KeyboardInterrupt:
    engine.save_state("scan_state.xml")
