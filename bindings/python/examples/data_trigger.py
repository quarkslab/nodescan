import pyleeloo
import pynodescan

ips = pyleeloo.ip_list_intervals()
ips.add("127.0.0.1")

ports = pyleeloo.port_list_intervals()
ports.add(pyleeloo.port(2000, pyleeloo.protocol.TCP))

targets = pynodescan.IPV4TargetSet(ips, ports)

engine = pynodescan.AsyncEngine(targets)

def trigger(target, lvl4sm, hsm, buf):
    print(buf.tobytes())
    return False

def print_true(s):
    print(s)
    return False

class NewlineDataTrigger():
    def __init__(self, func):
        self.__func = func

    def process_buffer(self, target, lvl4sm, hsm, buf, prev_pos):
        s = buf.data().tobytes().decode('ascii')
        ret = True
        if '\n' in s:
            lines = s.split('\n')[:-1]
            n = 0
            for l in lines:
                n += len(l)+1
                ret = self.__func(l)
            buf.pop_by(n)
        return ret

def connected_callback(target, lvl4sm, hsm):
    target.send("hello")

    lvl4sm.set_data_trigger(NewlineDataTrigger(lambda s: print_true(s)))
    return True

engine.set_lvl4_connected_callback(connected_callback)

engine.launch()
