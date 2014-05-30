#include <leeloo/ip_list_intervals.h>
#include <leeloo/port_list_intervals.h>

#include <ns/async_engine.h>
#include <ns/data_triggers.h>
#include <ns/lvl4_properties_storage.h>
#include <ns/target_set.h>

#include <iostream>

int main()
{
	leeloo::ip_list_intervals ips;
	ips.add("127.0.0.1");

	leeloo::port_list_intervals ports;
	ports.add(leeloo::tcp_port(80));

	ns::IPV4TargetSet targets(ips, ports);
	ns::AsyncEngine engine(targets);

	ns::Lvl4PropertiesStorage<uint32_t> props;

	engine.set_lvl4_connected_callback(
		[&props](ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM&)
		{
			const char* req = "GET / HTTP/1.0\n\n";
			t.send((unsigned char*) req, strlen(req));
			props[t] = 0;

			lvl4sm.set_trigger<ns::CharDataTrigger>('\n',
				[&props](ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM&, unsigned char* buf, uint32_t size)
				{
					if (size == 0) {
						std::cout << "received empty line on port " << t.port_value() << std::endl;
						return true;
					}
					if (buf[size-1] == '\r') {
						size--;
					}
					std::cout << "received '";
					std::cout.write(reinterpret_cast<const char*>(buf), static_cast<std::streamsize>(size));
					std::cout << "' on port " << t.port_value() << std::endl;

					if (size == 0) {
						uint32_t& content_length = props[t];
						if (content_length == 0) {
							std::cerr << "No content-length provided!" << std::endl;
							return false;
						}
						lvl4sm.set_trigger<ns::SizeDataTrigger>(content_length,
							[](ns::ConnectedTarget const&, ns::Lvl4SM&, ns::HostSM&, unsigned char* buf, uint32_t size)
							{
								std::cout << "received content:" << std::endl;
								std::cout.write((const char*) buf, size);
								return false;
							});
						return true;
					}
					char* dpts = (char*) memchr(buf, ':', size);
					if (dpts == nullptr) {
						if (size >= 4 && buf[0] == 'H' && buf[1] == 'T' && buf[2] == 'T' && buf[3] == 'P') {
							return true;
						}
						std::cout << "invalid header" << std::endl;
						return false;
					}
					*dpts = 0;
					if (strcasecmp((const char*) buf, "Content-length") == 0) {
						buf[size] = 0;
						props[t] = atoll(dpts+1);
					}
					return true;
				}
			);

			return true;
		}
	);
	engine.launch();

	return 0;
}
