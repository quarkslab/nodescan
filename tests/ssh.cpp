#include <leeloo/ip_list_intervals.h>
#include <leeloo/port_list_intervals.h>

#include <ns/async_engine.h>
#include <ns/data_triggers.h>
#include <ns/lvl4_properties_storage.h>
#include <ns/target_set.h>
#include <ns/protocols/ssh.h>

#include <iostream>
#include <fstream>

int main()
{
	leeloo::ip_list_intervals ips;
	ips.add("127.0.0.1");
	ips.add("91.121.152.104");
	ips.aggregate();
	std::cout << "IPs count: " << ips.size() << std::endl;

	leeloo::port_list_intervals ports;
	ports.add(leeloo::tcp_port(22));

	ns::IPV4TargetSet targets(ips, ports);
	ns::AsyncEngine engine(targets, 10000, 120);

	engine.set_lvl4_connected_callback(
		ns::protocols::SSH().on_certificate(
			[](ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM&, unsigned char* buf, uint32_t size)
			{
				std::cerr.write((const char*) buf, size);
				std::cerr.flush();
				return false;
			}
		)
	);

	engine.launch();

	return 0;
}
