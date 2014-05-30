#include <leeloo/ip_list_intervals.h>
#include <leeloo/port_list_intervals.h>

#include <ns/async_engine.h>
#include <ns/data_triggers.h>
#include <ns/lvl4_properties_storage.h>
#include <ns/target_set.h>
#include <ns/protocols/sip.h>

#include <iostream>
#include <fstream>


int main()
{
	leeloo::ip_list_intervals ips;
	ips.add("91.121.152.104");
	ips.aggregate();
	std::cout << "IPs count: " << ips.size() << std::endl;

	leeloo::port_list_intervals ports;
	ports.add(leeloo::udp_port(5060));

	ns::IPV4TargetSet targets(ips, ports);
	ns::AsyncEngine engine(targets, 10000, 120);

	engine.set_lvl4_connected_callback(
		ns::protocols::SIP()
			.on_header(
				[](ns::ConnectedTarget const&, ns::Lvl4SM&, ns::HostSM&, const char* key, const char* value)
				{
					std::cout << key << ": " << value << std::endl;
					return true;
				}
			)
			.on_invalid_header(
				[](ns::ConnectedTarget const&, ns::Lvl4SM&, ns::HostSM&, unsigned char* buf, size_t n)
				{
					std::cout << "invalid header: " << std::endl;
					std::cout.write((const char*) buf, n); 
					std::cout.flush();
					return true;
				}
			)
	);

	engine.launch();

	return 0;
}
