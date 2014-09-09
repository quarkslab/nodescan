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

#ifndef NS_TARGET_SET_H
#define NS_TARGET_SET_H

#include <ns/target.h>
#include <ns/lvl4_buffer.h>

#include <leeloo/ip_list_intervals.h>
#include <leeloo/port.h>
#include <leeloo/port_list_intervals.h>
#include <leeloo/list_intervals_random.h>
#include <leeloo/uni.h>

#include <queue>
#include <set>

namespace ns {

class Engine;
class HostSM;

struct NextTargetWouldBlock
{ };

class TargetSet
{
public:
	virtual void init() = 0;
	virtual void init_shrd(uint32_t shrd_idx, uint32_t shrd_count) = 0;

	virtual void save_state(const char* file);
	virtual void restore_state(const char* file);

	virtual void save_state(std::ostream& os) = 0;
	virtual void restore_state(std::istream& is) = 0;

	virtual Target next_target() = 0;
	virtual bool target_finished(Target const& target, HostSM& hsm) = 0;

	virtual void init_host_sm(Target const&, HostSM&) { };
};

class IPV4TargetSet: public TargetSet
{
	friend class Engine;

	typedef leeloo::ip_list_intervals::size_type v4_size_type;
	typedef leeloo::port_list_intervals::size_type port_size_type;

	typedef leeloo::list_intervals_random_promise<leeloo::ip_list_intervals, leeloo::uni> v4_random_state;
	typedef leeloo::list_intervals_random<leeloo::port_list_intervals, leeloo::uni> port_random_state;

public:
	IPV4TargetSet(leeloo::ip_list_intervals& ipv4s, leeloo::port_list_intervals& ports);

public:
	leeloo::ip_list_intervals const& ipv4s() const { return _ipv4s; }
	leeloo::port_list_intervals const& ports() const { return _ports; }

	/*void add_ipv4(uint32_t ip);
	void add_ipv4s(uint32_t start, uint32_t end);
	void add_ipv4s(const char* ip);

	void add_port(uint16_t port);
	void add_ports(uint16_t start, uint16_t end);*/

public:
	void init() override;
	void init_shrd(uint32_t shrd_idx, uint32_t shrd_count);

	void save_state(std::ostream& os) override;
	void restore_state(std::istream& is) override;

	Target next_target() override;
	bool target_finished(Target const& target, HostSM& hsm); 

	void init_host_sm(Target const& target, HostSM& hsm);

	inline uint32_t ports_count() const { return _rand_port.size_original(); }

private:
	//void reaggregate_ips();
	//void reaggregate_ports();

private:
	leeloo::ip_list_intervals& _ipv4s;
	leeloo::port_list_intervals& _ports;

	v4_random_state _rand_v4;
	port_random_state _rand_port;
};

class SimpleTargetSet: public TargetSet
{
	typedef std::set<ns::Target> target_storage_type;

public:
	void add_target(Target const& target)
	{
		_targets.insert(target);
	}
	
	void remove_target(Target const& target)
	{
		target_storage_type::iterator it = _targets.find(target);
		if (it == _targets.end()) {
			return;
		}
		if (it == _it) {
			++_it;
		}
		_targets.erase(it);
	}

	virtual void init()
	{
		_it = _targets.begin();
	}

	virtual void init_shrd(uint32_t /*shrd_idx*/, uint32_t /*shrd_count*/) { }

	virtual void save_state(std::ostream&)
	{ }
	virtual void restore_state(std::istream&)
	{ }

	virtual Target next_target()
	{
		if (_it == _targets.end()) {
			return Target::end();
		}
		Target ret = *_it;
		++_it;
		return ret;
	}

	virtual bool target_finished(Target const&, HostSM&) { return true; }

private:
	target_storage_type _targets;
	target_storage_type::const_iterator _it;
};

template <class T>
class RepeatableTargetSet: public T
{
public:
	typedef T target_set_base_type;

public:
	using T::T;

public:
	Target next_target() override
	{
		Target ret = target_set_base_type::next_target();
		if (ret.is_end()) {
			this->init();
			ret = target_set_base_type::next_target();
		}
		return ret;
	}
};

template <class T>
class ReinjectableTargetSet: public T
{
public:
	typedef T target_set_base_type;

public:
	using T::T;

public:
	void emplace_target(Target&& t)
	{
		_targets.emplace(std::move(t));
	}

	void add_target(Target const& t)
	{
		_targets.push(t);
	}

public:
	Target next_target() override
	{
		// Added targets are done first
		if (_targets.size() > 0) {
			Target ret = std::move(_targets.front());
			_targets.pop();
			return ret;
		}
		return target_set_base_type::next_target();
	}

private:
	std::queue<Target> _targets;
};

}

#endif
