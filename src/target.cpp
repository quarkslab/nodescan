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

#include <ns/target.h>
#include <leeloo/port.h>
#include <leeloo/ips_parser.h>

#include <string.h>

#include <sys/socket.h>

ns::Target::Target(const char* ipv4, leeloo::port port_):
	_port(port_)
{
	bool valid;
	_ipv4 = leeloo::ips_parser::ipv4toi(ipv4, valid);
	if (!valid) {
		*this = end();
	}
}

ns::Target ns::Target::end()
{
	return Target(0xFFFFFFFF, leeloo::port(0xFFFF, leeloo::port::protocol_enum::UNSUPPORTED));
}

ns::Target ns::Target::from_socket(int s)
{
	struct sockaddr_in sa;
	socklen_t size = sizeof(sockaddr_in);
	getpeername(s, (sockaddr*) &sa, &size);

	int type;
	unsigned int length = sizeof(int);
	getsockopt(s, SOL_SOCKET, SO_TYPE, &type, &length);

	int proto;
	length = sizeof(int);
	getsockopt(s, SOL_SOCKET, SO_PROTOCOL, &proto, &length);

	return Target(ntohl(sa.sin_addr.s_addr), leeloo::port(ntohs(sa.sin_port), leeloo::port::protocol_from_socket_type(type, proto)));
}

sockaddr_in ns::Target::to_sockaddr_in() const
{
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;

	addr.sin_addr.s_addr = htonl(ipv4());
	addr.sin_port = htons(port_value());

	return addr;
}
