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

#define _BSD_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include <ns/async_engine.h>
#include <ns/host_state_machine.h>
#include <ns/ipstr.h>
#include <ns/target.h>
#include <ns/log.h>
#include <ns/errors.h>

#define MAX_EVENTS 1024

static uint32_t socket_bytes_avail(int s)
{
	int ret;
	ioctl(s, FIONREAD, &ret);
	return ret;
}

static int new_socket(int type, int proto)
{
	int s = socket(AF_INET, type, proto);
	fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);
	return s;
}

ns::AsyncEngine::AsyncEngine(TargetSet& targets, uint32_t nsockets, uint32_t timeout):
	Engine(targets),
	_nsockets(nsockets),
	_timeout(timeout),
	_timeout_status_display(0)
{
	init_sockets();
}

void ns::AsyncEngine::init_sockets()
{
	_avail_socks = nsockets();
	/*
	free_socks().reserve(nsockets());
	for (uint32_t i = 0; i < nsockets(); i++) {
		const int s = new_socket();
		_lvl4_sms.insert(std::make_pair(s, Lvl4SM()));
		free_socks().push_back(s);
	}
	*/
}

void ns::AsyncEngine::remove_connected_socket(int s)
{
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = s;
	epoll_ctl(epoll(), EPOLL_CTL_DEL, s, &ev);
}

void ns::AsyncEngine::free_socket(int s)
{
	remove_connecting_socket(s);
	remove_connected_socket(s);

	close(s);
	del_lvl4_sm(s);
	_avail_socks++;
	_sockets_targets.erase(s);
}

void ns::AsyncEngine::socket_finished(int s, int err)
{
	_ndone++;
	Target t = target_from_socket(s);
	Lvl4SM& lvl4sm = lvl4_sm(s);
	Lvl4Buffer const& buf = lvl4sm.buffer();
	callback_finish(t, buf, err);
	lvl4sm.free_buffer();
	if (target_finished(t, host_sm(t.ipv4()))) {
		del_host_sm(t);
	}
	free_socket(s);
}

void ns::AsyncEngine::process_connecting_ready(int s, Lvl4SM& lvl4sm)
{
	_D(BOOST_LOG_NAMED_SCOPE("AsyncEngine::process_connecting_ready"));

	int err;
	socklen_t size = sizeof(int);
	getsockopt(s, SOL_SOCKET, SO_ERROR, &err, &size);
	if (err != 0) {
		//BOOST_LOG_TRIVIAL(trace) << "Error with " << ipstr(ipv4) << ": " << strerror(err) << std::endl;
		socket_finished(s, err);
		return;
	}
	 
	Target target = target_from_socket(s);
	const uint32_t ipv4 = target.ipv4();
	_D(BOOST_LOG_TRIVIAL(trace) << "Connected to " << ipstr(ipv4) << std::endl);

	const bool ret = lvl4sm.on_connect(s, target, host_sm(ipv4));

	if (ret == false) {
		// The end for him
		socket_finished(s, 0);
		_D(BOOST_LOG_TRIVIAL(trace) << "on_connect action returned false for " << ipstr(ipv4) << std::endl);
	}
	else {
		remove_connecting_socket(s);
		add_connected_socket(s);
		lvl4sm.update_ts();
	}
}

void ns::AsyncEngine::reconnect(int s, Target const& target, Lvl4SM const& lvl4sm)
{
	_D(BOOST_LOG_TRIVIAL(trace) << "reconnect" << std::endl);
	Lvl4Action cur_action = lvl4sm.get_on_connect();

	remove_connected_socket(s);
	close(s);
	_avail_socks++;

	create_socket(s, target);
	lvl4_sm(s).set_on_connect(cur_action);
	add_connecting_socket(s);
}

int ns::AsyncEngine::create_socket(int& s, Target const& target)
{
	assert(_avail_socks > 0);
	/*struct in_addr addr_;
	addr_.s_addr = htonl(target.ipv4());
	std::cerr << "Connecting to " << inet_ntoa(addr_) << "..." << std::endl;*/

	leeloo::port port = target.port();
	s = new_socket(port.socket_type(), port.socket_proto());
	if (s == -1) {
		return errno;
	}
	_avail_socks--;
	sockaddr_in addr = target.to_sockaddr_in();
	int ret = connect(s, (const sockaddr*) &addr, sizeof(struct sockaddr_in));
	Lvl4SM& lvl4sm = new_lvl4_sm(s);
	lvl4sm.set_valid(true);
	lvl4sm.set_on_connect(_callback_lvl4_connected);
	lvl4sm.update_ts();
	_sockets_targets.insert(std::make_pair(s, target));
	return ret;
}

void ns::AsyncEngine::process_connected_ready(int s, Target const& target, Lvl4SM& lvl4sm)
{
	_D(BOOST_LOG_NAMED_SCOPE("AsyncEngine::process_connected_ready"));
	uint32_t navail = socket_bytes_avail(s);
	const uint32_t ipv4 = target.ipv4();
	if (navail == 0) {
		_D(BOOST_LOG_TRIVIAL(trace) << ipstr(ipv4) << " remote host deconnected" << std::endl);
		if (lvl4sm.reconnect()) {
			Lvl4Buffer const& buf = lvl4sm.buffer();
			callback_finish(target, buf, (int) errors::WILL_RECONNECT);
			lvl4sm.free_buffer();
			reconnect(s, target, lvl4sm);
			return;
		}
		socket_finished(s, ECONNRESET);
		return;
	}

	// Returns true to go on
	//         false to remove
	bool ret = lvl4sm.process_lvl4_data(s, navail, target, host_sm(ipv4));
	_D(BOOST_LOG_TRIVIAL(trace) << ipstr(ipv4) << " process_lvl4_data returned " << ret << std::endl);
	lvl4sm.update_ts();

	if (ret == false) {
		_D(BOOST_LOG_TRIVIAL(trace) << ipstr(ipv4) << " free socket!" << std::endl);
		// Free socket, this is the end for this one!
		socket_finished(s, 0);
	}
}

void ns::AsyncEngine::add_connecting_socket(int s)
{
	struct epoll_event ev;
	ev.events = EPOLLOUT;
	ev.data.fd = s;
	epoll_ctl(epoll(), EPOLL_CTL_ADD, s, &ev);
}

void ns::AsyncEngine::add_connected_socket(int s)
{
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = s;
	epoll_ctl(epoll(), EPOLL_CTL_ADD, s, &ev);
}

void ns::AsyncEngine::remove_connecting_socket(int s)
{
	struct epoll_event ev;
	// &ev isn't mandatory, but there is a bug before 2.6.9 that
	// makes epoll crashes if &ev == nullptr
	epoll_ctl(epoll(), EPOLL_CTL_DEL, s, &ev);
}

void ns::AsyncEngine::launch()
{
	init_scan();
	do_async_scan();
}

void ns::AsyncEngine::launch_shrd(uint32_t, uint32_t)
{
	//init_shrd_scan(idx, total);
	do_async_scan();
}

bool ns::AsyncEngine::process_free_socks()
{
	std::vector<int>::const_iterator it_socks;
	_D(BOOST_LOG_TRIVIAL(trace) << "begin free socks" << std::endl);
	const uint32_t avail_socks = _avail_socks;
	try {
		for (uint32_t i = 0; i < avail_socks; i++) {
			int ret, s;
			Target cur_target;
			while (true) {
				cur_target = next_target();
				if (cur_target.is_end()) {
					return false;
				}
				_nlaunched++;
				ret = create_socket(s, cur_target);
				if (ret == 0) {
					break;
				}
				else {
					if (errno == EINPROGRESS) {
						break;
					}
					socket_finished(s, errno);
					_D(BOOST_LOG_TRIVIAL(trace) << s << " Error connecting to " << ipstr(cur_target.ipv4()) << ": " << errno << " " << strerror(errno) << std::endl);
				}
			}
			_D(BOOST_LOG_TRIVIAL(trace) << "Connecting to " << ipstr(cur_target.ipv4()) << std::endl);
			HostSM& hsm = host_sm(cur_target.ipv4());
			init_host_sm(cur_target, hsm);

			add_connecting_socket(s);
		}
	}
	catch (NextTargetWouldBlock const&) {
		return true;
	}
	return true;
}

int ns::AsyncEngine::process_events()
{
	static struct epoll_event events[MAX_EVENTS];
	_D(BOOST_LOG_TRIVIAL(trace) << "begin epoll" << std::endl);
	// Poll this
	const int nfds = epoll_wait(epoll(), events, MAX_EVENTS, 1);
	for (int i = 0; i < nfds; i++) {
		struct epoll_event& ev = events[i];
		const int fd = ev.data.fd;
		if ((ev.events & EPOLLOUT) == EPOLLOUT) {
			process_connecting_ready(fd, lvl4_sm(fd));
		}
		else
		if ((ev.events & EPOLLIN) == EPOLLIN) {
			process_connected_ready(fd, target_from_socket(fd), lvl4_sm(fd));
		}
	}

	return nfds;
}

bool ns::AsyncEngine::has_watch_timedout(time_t ts, int s, Lvl4SM& lvl4sm) const
{
	if (!_watch_timeout_cb || (_watch_timeout == 0)) {
		return false;
	}

	if ((ts-lvl4sm.watch_ts()) >= watch_timeout()) {
		if (!_watch_timeout_cb(ConnectedTarget(s, Target::from_socket(s)))) {
			return true;
		}
		lvl4sm.update_watch_ts(ts);
	}
	return false;
}

size_t ns::AsyncEngine::process_dirty_and_timeouts()
{
	_D(BOOST_LOG_TRIVIAL(trace) << "begin timeouts" << std::endl);
	lvl4_sms_type::iterator it;
	const time_t ts = timestamp();
	size_t n_lvl4_sms_valid = 0;
	std::vector<std::pair<int, Lvl4SM*>> to_process;
	std::vector<int> timeouted;
	for (it = _lvl4_sms.begin(); it != _lvl4_sms.end(); it++) {
		Lvl4SM& p = it->second;
		if (p.valid()) {
			if (p.should_process_buffer()) {
				const int s = it->first;
				to_process.push_back(std::make_pair(s, &p));
			}
			if (((ts-p.ts()) >= timeout()) || has_watch_timedout(ts, it->first, p)) {
				const int s = it->first;
				timeouted.push_back(s);
			}
			else {
				n_lvl4_sms_valid++;
			}
		}
	}

	// Do this out of this loop as some Lvl4SM can be deleted in the way.
	for (auto const& tp: to_process) {
		const int s = tp.first;
		Target target = target_from_socket(s);
		bool ret = tp.second->process_buffer(s, target, host_sm(target.ipv4()));
		if (!ret) {
			socket_finished(s, 0);
			continue;
		}
	}

	for (int s: timeouted) {
		socket_finished(s, (int) errors::NS_TIMEOUT);
	}

	return n_lvl4_sms_valid;
}

bool ns::AsyncEngine::should_call_status_display()
{
	if (!_callback_status || _timeout_status_display == 0) {
		return false;
	}

	const time_t now = timestamp();
	if (now-_last_time_status_display >= _timeout_status_display) {
		_last_time_status_display = now;
		return true;
	}

	return false;
}

void ns::AsyncEngine::do_async_scan()
{
	_D(BOOST_LOG_NAMED_SCOPE("AsyncEngine::do_async_scan"));

	epoll() = epoll_create(nsockets());
	
	_nlaunched = 0;
	_ndone = 0;

	_last_time_status_display = 0;

	process_free_socks();

	while (true) {

		// Events processing
		const int nfds = process_events();

		// Dirties and timeouts
		const size_t n_lvl4_sms_valid = process_dirty_and_timeouts();

		// IP connect. It is done after the events processing because some of
		// them might have add new targets into the original set.
		const bool end_targets = !process_free_socks();

		_D(BOOST_LOG_TRIVIAL(trace) << "nfds == " << nfds << " "
		                            << "n_lvl4_sms_valid == " << n_lvl4_sms_valid << " "
		                            << "end_targets == " << end_targets << std::endl);
		// Check for the end
		if (nfds == 0 && n_lvl4_sms_valid == 0 && end_targets) {
			break;
		}

		if (should_call_status_display()) {
			_callback_status(_nlaunched, _ndone);
		}

		if (should_save_state()) {
			save_state(file_autosave());
		}
	}
}

ns::Lvl4SM& ns::AsyncEngine::lvl4_sm(epoll_event const& ev)
{
	return lvl4_sm(ev.data.fd);
}
