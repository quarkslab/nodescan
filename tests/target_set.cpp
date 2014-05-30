#include <leeloo/ip_list_intervals.h>
#include <leeloo/port_list_intervals.h>

#include <iostream>
#include <sstream>

#include <ns/target_set.h>
#include <ns/host_state_machine.h>


int main(int argc, char** argv)
{
	leeloo::ip_list_intervals ips;
	leeloo::port_list_intervals ports;

	ips.add("10/8");
	ports.add(10, 200, leeloo::port::protocol_enum::TCP);

	ns::IPV4TargetSet set(ips, ports);

	set.init();

	for (int i = 0; i < 100; i++) {
		ns::Target t = set.next_target();
		if (i % 10 != 0) {
			ns::HostSM hsm;
			set.target_finished(t, hsm);
		}
	}

	std::stringstream ss;
	set.save_state(ss);
	
	std::cout << ss.str() << std::endl;
	ss.seekg(0, std::stringstream::beg);

	leeloo::ip_list_intervals new_ips;
	leeloo::port_list_intervals new_ports;
	ns::IPV4TargetSet deser(new_ips, new_ports);
	deser.restore_state(ss);

	std::stringstream ss2;
	deser.save_state(ss2);
	std::cout << ss2.str() << std::endl;

	/*if (ss != ss2) {
		std::cerr << "Deserialize - >serialize -> deserialize gave different results" << std::endl;
		return 1;
	}*/

	for (int i = 0; i < 50; i++) {
		if (set.next_target() != deser.next_target()) {
			std::cerr << "Issue at " << i << std::endl;
			return 1;
		}
	}

	ips.clear();
	ips.add("127.0.0.1");
	ns::RepeatableTargetSet<ns::IPV4TargetSet> rsets(ips, ports);
	rsets.init();
	ns::Target t;
	do {
		t = rsets.next_target();
		std::cout << t.ipv4() << "/" << t.is_end() << std::endl;
	}
	while (!t.is_end());

	return 0;
}
