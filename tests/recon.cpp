#include <leeloo/ip_list_intervals.h>
#include <leeloo/port_list_intervals.h>
#include <ns/async_engine.h>
#include <ns/data_triggers.h>
#include <ns/lvl4_properties_storage.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

int main()
{
	leeloo::ip_list_intervals ips;
	ips.add("127.0.0.1");

	leeloo::port_list_intervals ports;
	ports.add(leeloo::tcp_port(1245));

	typedef std::array<unsigned char, 4> random_str_type;
	ns::Lvl4PropertiesStorage<random_str_type> props;

	ns::IPV4TargetSet targets(ips, ports);
	ns::AsyncEngine engine(targets);
	engine.set_lvl4_connected_callback(
		[&props](ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM&)
		{
			const char* payload = "break";
			t.send((unsigned char*) payload, strlen(payload));

			std::cout << "payload sent" << std::endl;

			lvl4sm.set_trigger<ns::SizeDataTrigger>(4,
				[&props](ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM&, unsigned char* buf, uint32_t)
				{
					memcpy(std::begin(props[t]), buf, 4);
					std::cout << "received '";
					std::cout.write((const char*) buf, 4);
					std::cout << "'" << std::endl;
					lvl4sm.set_reconnect(true);
					lvl4sm.set_on_connect(
						[&props](ns::ConnectedTarget const& t, ns::Lvl4SM&, ns::HostSM&)
						{
							std::cout << "reconnected to server" << std::endl;
							t.send((unsigned char*) std::begin(props[t]), 4);
							return false;
						});
					return true;
				});
			return true;
		});
	engine.launch();

	return 0;
}
