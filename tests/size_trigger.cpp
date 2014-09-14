#include <leeloo/ip_list_intervals.h>
#include <leeloo/port_list_intervals.h>

#include <ns/async_engine.h>
#include <ns/data_triggers.h>
#include <string.h>

#include <iostream>

int main()
{
	leeloo::ip_list_intervals ips;
	ips.add("127.0.0.1");

	leeloo::port_list_intervals ports;
	ports.add(2000, 2009, leeloo::port::protocol_enum::TCP);

	ns::IPV4TargetSet targets(ips, ports);
	ns::AsyncEngine engine(targets);
	engine.set_lvl4_connected_callback(
		[](ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM&)
		{
			std::cout << "send hello to port " << t.port_value() << std::endl;
			const char* req = "hello";
			t.send((unsigned char*) req, strlen(req));

			std::cout << "wait for hi on port " << t.port_value() << std::endl;
			lvl4sm.set_trigger<ns::SizeDataTrigger>(2,
				[](ns::ConnectedTarget const& t, ns::Lvl4SM&, ns::HostSM&, unsigned char* buf, uint32_t size)
				{
					std::cout << "received '";
					std::cout.write(reinterpret_cast<const char*>(buf), static_cast<std::streamsize>(size));
					std::cout << "' on port " << t.port_value() << std::endl;
					return false;
				}
			);

			return true;
		}
	);
	engine.set_lvl4_finish_callback(
		[](ns::Target const& target, const unsigned char*, size_t, int err)
		{
			std::cout << "finish for port " << target.port_value() << ": " << strerror(err) << std::endl;
		});
	engine.launch();

	return 0;
}
