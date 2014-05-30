#include <leeloo/ip_list_intervals.h>
#include <leeloo/port_list_intervals.h>

#include <ns/async_engine.h>
#include <ns/data_triggers.h>
#include <ns/lvl4_properties_storage.h>
#include <ns/target_set.h>
#include <ns/protocols/ssl.h>

#include <iostream>
#include <fstream>

int main()
{
	leeloo::ip_list_intervals ips;
	ips.add("27.122.0.0/22");
	ips.add("61.5.208.0/24");
	ips.add("61.5.209.32/27");
	ips.add("61.5.209.64/26");
	ips.add("61.5.209.128/25");
	ips.add("61.5.210.0/30");
	ips.add("61.5.210.8/30");
	ips.add("61.5.210.160/27");
	ips.add("61.5.211.0/24");
	ips.add("61.5.212.0/26");
	ips.add("61.5.221.0/25");
	ips.add("61.5.223.0/24");
	ips.add("101.101.0.0/18");
	ips.add("103.2.184.0/23");
	ips.add("103.2.187.0/24");
	ips.add("103.17.44.0/22");
	ips.add("103.23.52.0/22");
	ips.add("103.29.152.0/22");
	ips.add("113.20.32.0/19");
	ips.add("113.21.96.0/20");
	ips.add("114.69.176.0/20");
	ips.add("114.69.192.0/22");
	ips.add("114.69.200.0/22");
	ips.add("114.69.204.0/23");
	ips.add("114.69.206.0/24");
	ips.add("114.69.208.0/21");
	ips.add("114.69.216.0/22");
	ips.add("115.126.160.0/19");
	ips.add("118.179.224.0/19");
	ips.add("175.158.128.0/18");
	ips.add("180.214.96.0/19");
	ips.add("193.51.249.0/24");
	ips.add("194.214.55.0/24");
	ips.add("194.254.189.0/24");
	ips.add("195.221.84.0/24");
	ips.add("202.0.157.0/24");
	ips.add("202.22.128.0/19");
	ips.add("202.22.224.0/20");
	ips.add("202.87.128.0/30");
	ips.add("202.87.128.20/30");
	ips.add("202.87.128.24/30");
	ips.add("202.87.128.44/30");
	ips.add("202.87.128.60/30");
	ips.add("202.87.128.68/30");
	ips.add("202.87.128.108/30");
	ips.add("202.87.128.112/30");
	ips.add("202.87.128.128/30");
	ips.add("202.87.128.136/29");
	ips.add("202.87.128.144/29");
	ips.add("202.87.128.160/30");
	ips.add("202.87.128.172/30");
	ips.add("202.87.128.200/30");
	ips.add("202.87.128.208/29");
	ips.add("202.87.129.0/24");
	ips.add("202.87.130.0/27");
	ips.add("202.87.131.80/30");
	ips.add("202.87.131.196/30");
	ips.add("202.87.131.224/28");
	ips.add("202.87.132.0/25");
	ips.add("202.87.132.184/29");
	ips.add("202.87.133.16/28");
	ips.add("202.87.133.48/28");
	ips.add("202.87.134.0/26");
	ips.add("202.87.134.120/29");
	ips.add("202.87.134.128/26");
	ips.add("202.87.134.248/29");
	ips.add("202.87.138.0/24");
	ips.add("202.87.139.28/30");
	ips.add("202.87.139.36/30");
	ips.add("202.87.139.40/30");
	ips.add("202.87.139.148/30");
	ips.add("202.87.139.160/30");
	ips.add("202.87.140.0/22");
	ips.add("202.87.144.0/22");
	ips.add("202.87.148.0/24");
	ips.add("202.87.150.120/29");
	ips.add("202.87.150.248/30");
	ips.add("202.87.156.0/24");
	ips.add("202.166.176.0/21");
	ips.add("202.171.64.0/20");
	ips.add("203.80.48.0/21");
	ips.add("203.147.64.0/20");
	ips.add("203.147.80.0/21");
	ips.add("220.156.160.0/20");
	ips.add("223.29.128.0/19");
	ips.add("223.29.160.0/21");
	ips.add("223.29.168.0/22");
	ips.add("223.29.172.0/23");
	ips.aggregate();
	std::cout << "IPs count: " << ips.size() << std::endl;

	leeloo::port_list_intervals ports;
	ports.add(leeloo::tcp_port(443));

	ns::IPV4TargetSet targets(ips, ports);
	ns::AsyncEngine engine(targets, 100, 10);

	engine.set_lvl4_connected_callback(
		ns::protocols::SSL().on_certificate([](ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM&, unsigned char* buf, uint32_t size)
			{
				std::stringstream path;
				path << "cert_" << t.ipv4() << "_" << t.port().value();
				FILE* f = fopen(path.str().c_str(), "w+");
				fwrite(buf, 1, size, f);
				fclose(f);
				// This crashes for an obscure reason in -03...
				// std::ofstream of(path.str(), std::ofstream::out | std::ofstream::trunc);
				// of.write((const char*) buf, size);
				// of.close();
				return true;
			})
	);

	engine.launch();

	return 0;
}
