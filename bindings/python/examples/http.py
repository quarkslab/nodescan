import pyleeloo
import pynodescan

ips = pyleeloo.ip_list_intervals()
ips.add("127.0.0.1")
ips.add("173.194.34.14")

ports = pyleeloo.port_list_intervals()
ports.add(pyleeloo.port(80, pyleeloo.protocol.TCP))

targets = pynodescan.IPV4TargetSet(ips, ports)

engine = pynodescan.AsyncEngine(targets, 100, 1)

class Callable:
    def __init__(self, method):
        self.__method = method

    def __call__(self, *args, **kwargs):
        return self.__method(*args, **kwargs)

class HTTPMethod:
    def __init__(self, method, url, headers):
        self.__method = method
        self.__url = url
        self.__headers = headers
        self._cb_headers = lambda t,k,v: True
        self._cb_content = lambda t,c: None
        self.__properties = pynodescan.Lvl4Properties(dict)

    def __call__(self, target, lvl4sm, hsm):
        s = "%s %s HTTP/1.0\n\n" % (self.__method, self.__url)
        target.send(s)
        self.__properties[target]['code'] = None
        self.__properties[target]['content-length'] = None

        lvl4sm.set_char_data_trigger('\n', Callable(self._on_newline))
        return True

    def on_header(self, cb):
        self._cb_header = cb
        return self

    def on_content(self, cb):
        self._cb_content = cb
        return self

    def _on_newline(self, target, lvl4sm, hsm, buf):
        s = buf.tobytes().decode('ascii')
        if s.startswith("HTTP"):
            fields = s.split(' ')
            self.__properties[target]['code'] = int(fields[1])
            return True

        print("after starts:  " + s)
        header = s.split(':')
        if len(header) == 1:
            cl = self.__properties[target]['content-length'] 
            if cl == None:
                return False
            lvl4sm.set_size_data_trigger(cl, Callable(self._on_content))
            return True
        elif len(header) > 2:
            header = [header[0], ":".join(header[1:])]

        key, value = header
        if key == "Content-Length":
            self.__properties[target]['content-length'] = int(value)
            
        return self._cb_header(target, header[0], header[1])

    def _on_content(self, target, lvl4sm, hsm, buf):
        self._cb_content(target, self.__properties[target]['code'], buf.tobytes())
        self.__properties.remove(target)
        return False

def print_header(t, k, v):
    print(t, k, v)
    return True

engine.set_lvl4_connected_callback(
    HTTPMethod("GET", "/", {'User-agent': "My little poney 1.0"})
        .on_header(print_header)
        .on_content(lambda target, code, content: print(code, content))
    )

engine.launch()
