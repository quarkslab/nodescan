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

#ifndef NS_ASYNC_SCAN_H
#define NS_ASYNC_SCAN_H

#include <stdint.h>
#include <stdlib.h>

#include <vector>
#include <map>

#include <ns/action.h>
#include <ns/engine.h>
#include <ns/timestamp.h>
#include <ns/host_state_machine.h>
#include <ns/lvl4_state_machine.h>

struct epoll_event;

namespace ns {

class Target;
class TargetSet;

class AsyncEngine: public Engine
{
public:
	typedef std::map<uint32_t, HostSM>  hosts_sms_type;
	typedef std::map<int, Lvl4SM> lvl4_sms_type;
	typedef std::map<int, Target> sockets_targets_type;

public:
	AsyncEngine(TargetSet& targets, uint32_t nsockets = 1000, uint32_t timeout = 5);

public:
	// Implement the interface of an engine
	void launch();
	void launch_shrd(uint32_t idx, uint32_t total);

public:
	void set_lvl4_connected_callback(Lvl4Action const& a)
	{
		_callback_lvl4_connected = a;
	}

	void set_lvl4_finish_callback(Lvl4Finish const& f) { _callback_finish = f; }

	void set_status_display_callback(StatusDisplay const& f, uint32_t timeout)
	{
		_callback_status = f;
		_timeout_status_display = timeout;
	}

	void set_watch_timeout(WatchTimeout const& f, uint32_t timeout)
	{
		_watch_timeout = timeout;
		_watch_timeout_cb = f;
	}

	void set_timeout_of_target(TimeoutTarget const& f)
	{
		_callback_timeout_target = f;
	}

	void ensure_available_sockets(const size_t n);

	void set_buffer_size_limit(uint32_t limit) { _buf_limit = limit; }

public:
	uint32_t timeout() const { return _timeout; }
	uint32_t watch_timeout() const { return _watch_timeout; }
	uint32_t nsockets() const { return _nsockets; }

private:
	void do_async_scan();
	void init_sockets();
	void connect_ip(uint32_t const ip, uint16_t const port);

private:
	void free_socket(int s);

	HostSM& host_sm(uint32_t ip)
	{
		auto it = _hosts_sms.find(ip);
		if (it == _hosts_sms.end()) {
			return _hosts_sms.insert(std::make_pair(ip, HostSM())).first->second;
		}
		return it->second;
	}

	void del_host_sm(Target const& t)
	{
		_hosts_sms.erase(t.ipv4());
	}

	void add_connected_socket(int s);
	void add_connecting_socket(int s);
	void remove_connecting_socket(int s);
	void remove_connected_socket(int s);

	inline int& epoll() { return _epoll; }

	bool process_free_socks();
	int process_events();
	size_t process_dirty_and_timeouts();

	void process_connecting_ready(int s, Lvl4SM& lvl4sm);
	void process_connected_ready(int s, Target const& target, Lvl4SM& lvl4sm);

	void reconnect(int s, Target const& target, Lvl4SM const& lvl4sm);
	int create_socket(int& s, Target const& target);

	Target const& target_from_socket(int s) const
	{
		sockets_targets_type::const_iterator it = _sockets_targets.find(s);
		assert(it != _sockets_targets.end());
		return it->second;
	}

	Lvl4SM& new_lvl4_sm(int s)
	{
		return _lvl4_sms.insert(std::make_pair(s, Lvl4SM())).first->second;
	}

	void del_lvl4_sm(int s)
	{
		_lvl4_sms.erase(s);
	}

	Lvl4SM& lvl4_sm(epoll_event const& ev);

	Lvl4SM& lvl4_sm(int s)
	{
		return _lvl4_sms[s];
	}

	void callback_finish(Target const& t, Lvl4Buffer const& buf, int error)
	{
		if (_callback_finish) {
			_callback_finish(t, buf.begin(), buf.size(), error);
		}
	}

	void socket_finished(int s, int err);

	bool should_call_status_display();

	bool has_watch_timedout(time_t ts, int s, Lvl4SM& lvl4sm) const;

	inline uint32_t timeout_of_target(Target const& t) const
	{
		if (_callback_timeout_target) {
			return _callback_timeout_target(t);
		}
		return _timeout;
	}

private:
	hosts_sms_type _hosts_sms;
	lvl4_sms_type _lvl4_sms;
	sockets_targets_type _sockets_targets;
	uint32_t _avail_socks;
	int _epoll;
	Lvl4Action _callback_lvl4_connected;

	// Configuration
	uint32_t _nsockets;
	uint32_t _timeout;
	uint32_t _watch_timeout;
	WatchTimeout _watch_timeout_cb;

	// Status
	size_t _nlaunched;
	size_t _ndone;

	// Errors
	Lvl4Finish _callback_finish;

	// Timeout of target
	TimeoutTarget _callback_timeout_target;

	// Status display
	StatusDisplay _callback_status;
	uint32_t _timeout_status_display; // in seconds
	time_t _last_time_status_display;

	// Limits
	uint32_t _buf_limit;
};

}

#endif
