/*
 * Copyright (c) 2014, Quarkslab
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * 
 * * Neither the name of the {organization} nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ns/target_set.h>
#include <ns/host_state_machine.h>

#include <leeloo/random.h>
#include <leeloo/uni.h>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/random.hpp>

#include <fstream>

boost::random::mt19937 g_mt_rand_ips;
boost::random::mt19937 g_mt_rand_ports;

void ns::TargetSet::save_state(const char* file)
{
	std::ofstream os(file, std::ofstream::out | std::ofstream::trunc);
	save_state(os);
	os.close();
}

void ns::TargetSet::restore_state(const char* file)
{
	std::ifstream is(file, std::ifstream::in);
	restore_state(is);
	is.close();
}

ns::IPV4TargetSet::IPV4TargetSet(leeloo::ip_list_intervals& ipv4s, leeloo::port_list_intervals& ports):
	_ipv4s(ipv4s),
	_ports(ports)
{
}

void ns::IPV4TargetSet::init()
{
	_ipv4s.aggregate();
	_ports.aggregate();

	_ipv4s.create_index_cache(256);
	_ports.create_index_cache(16);

	_rand_v4.init(_ipv4s, leeloo::random_engine<uint32_t>(g_mt_rand_ips));
	_rand_port.init(_ports, leeloo::random_engine<uint32_t>(g_mt_rand_ports));
}

void ns::IPV4TargetSet::init_shrd(uint32_t /*shrd_idx*/, uint32_t /*shrd_count*/)
{
}

ns::Target ns::IPV4TargetSet::next_target()
{
	if (_rand_port.end()) {
		_rand_v4.next();
		_rand_port.init(_ports, leeloo::random_engine<uint32_t>(g_mt_rand_ports));
	}
	if (_rand_v4.end()) {
		return Target::end();
	}

	const uint32_t ip = _rand_v4.get_current(_ipv4s);
	const leeloo::port port = leeloo::port(_rand_port(_ports));

	return Target(ip, port);
}

void ns::IPV4TargetSet::save_state(std::ostream& os)
{
	boost::archive::xml_oarchive xmlo(os);
	xmlo << boost::serialization::make_nvp("ipv4s", _ipv4s);
	xmlo << boost::serialization::make_nvp("ports", _ports);
	_rand_v4.save_state(xmlo);
	_rand_port.save_state(xmlo);
}

void ns::IPV4TargetSet::restore_state(std::istream& is)
{
	boost::archive::xml_iarchive xmli(is);
	xmli >> boost::serialization::make_nvp("ipv4s", _ipv4s);
	xmli >> boost::serialization::make_nvp("ports", _ports);
	_rand_v4.restore_state(xmli, _ipv4s, leeloo::random_engine<uint32_t>(g_mt_rand_ips));
	_rand_port.restore_state(xmli, _ports, leeloo::random_engine<uint32_t>(g_mt_rand_ports));

	_ipv4s.create_index_cache(256);
	_ports.create_index_cache(16);
}

bool ns::IPV4TargetSet::target_finished(Target const& target, HostSM& hsm)
{
	hsm.port_done(target.port());
	if (hsm.ports_done_count() == ports_count()) {
		// Target is done
		_rand_v4.step_done(hsm.rand_idx_ip());
		// true means that the HostSM object can be deleted
		return true;
	}
	return false;
}

void ns::IPV4TargetSet::init_host_sm(Target const&, HostSM& hsm)
{
	hsm.set_rand_idx_ip(_rand_v4.get_current_step());
}
