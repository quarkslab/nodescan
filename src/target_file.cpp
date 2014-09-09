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

#include <ns/target_file.h>

#include <leeloo/ips_parser.h>

#include <fstream>

#include <sys/ioctl.h>
#include <unistd.h>

#if 0
ns::TargetFile::TargetFile(const char* file)
{
	open_file(file);
}

void ns::TargetFile::open_file(const char* file)
{
	_storage.reset(new std::ifstream(file, std::ifstream::in));
	_stream = _storage.get();
	_cur_line = 0;
}

ns::TargetFile::TargetFile(std::istream& is):
	_stream(&is),
	_cur_line(0)
{
}

void ns::TargetFile::init()
{
	_cur_line = 0;
}

void ns::TargetFile::init_shrd(uint32_t /*shrd_idx*/, uint32_t /*shrd_count*/)
{
}

void ns::TargetFile::save_state(std::ostream& os)
{
	os << _cur_line;
}

void ns::TargetFile::restore_state(std::istream& is)
{
	is >> _cur_line;
}
#endif

// Async file
//

ns::TargetAsyncFile::TargetAsyncFile(int fd, leeloo::port const& port):
	_fd(fd),
	_def_port(port)
{
}

void ns::TargetAsyncFile::init()
{
	fcntl(fd(), F_SETFL, fcntl(fd(), F_GETFL) | O_NONBLOCK);
}

void ns::TargetAsyncFile::init_shrd(uint32_t, uint32_t)
{
}

ns::Target ns::TargetAsyncFile::next_target()
{
	if (_buf.size() > 0) {
		Target ret = next_target_in_buf();
		if (ret != Target::end()) {
			return ret;
		}
	}

	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(STDIN_FILENO, &rfds);

	struct timeval tv;
	memset(&tv, 0, sizeof(struct timeval));
	int sel_ret = select(STDIN_FILENO+1, &rfds, nullptr, nullptr, &tv);
	if (sel_ret == -1) {
		return Target::end();
	}
	if (sel_ret == 0) {
		throw NextTargetWouldBlock();
	}

	int navail;
	if (ioctl(fd(), FIONREAD, &navail) == -1) {
		return Target::end();
	}
	std::cout << "navail: " << navail << std::endl;
	if (navail == 0) {
		return Target::end();
	}

	unsigned char* buf_read = _buf.grow_by(navail);
	int nread = read(fd(), buf_read, navail);
	if (nread <= 0) {
		return Target::end();
	}

	Target ret = next_target_in_buf();
	if (ret == Target::end()) {
		throw NextTargetWouldBlock();
	}
	return ret;
}

ns::Target ns::TargetAsyncFile::next_target_in_buf()
{
	const unsigned char* newline = (const unsigned char*) memchr(_buf.begin(), '\n', _buf.size());
	if (!newline) {
		return Target::end();
	}

	const size_t size = (uintptr_t)(newline) - (uintptr_t)(_buf.begin());
	bool valid = false;
	uint32_t ipv4 = leeloo::ips_parser::ipv4toi((const char*) _buf.begin(), size, valid);
	_buf.pop_by(size+1);
	if (!valid) {
		return Target::end();
	}
	return Target(ipv4, _def_port);

}

ns::TargetStdin::TargetStdin(leeloo::port const& def_port):
	TargetAsyncFile(STDIN_FILENO, def_port)
{
}
