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

#ifndef NS_TARGET_H
#define NS_TARGET_H

#include <cstdint>
#include <netinet/in.h>
#include <leeloo/port.h>

namespace ns {

class Target
{
public:
	Target() { }

	Target(uint32_t ipv4, leeloo::port port_):
		_ipv4(ipv4),
		_port(port_)
	{ }

public:
	inline uint32_t ipv4() const { return _ipv4; }
	inline leeloo::port port() const { return _port; }
	inline uint16_t port_value() const { return port().value(); }
	inline uint16_t port_protocol() const { return port().protocol(); }

	inline bool is_end() const { return _ipv4 == 0xFFFFFFFF && port_value() == 0xFFFF; }

	static Target from_socket(int s);
	sockaddr_in to_sockaddr_in() const;

public:
	// Make this usable in an std::map
	inline bool operator<(Target const& o) const
	{
		if (ipv4() == o.ipv4()) {
			return port().as_u32() < o.port().as_u32();
		}
		return ipv4() < o.ipv4();
	}

	inline bool operator==(Target const& o) const
	{
		return ipv4() == o.ipv4() and port() == o.port();
	}

	inline bool operator!=(Target const& o) const
	{
		return ipv4() != o.ipv4() or port() != o.port();
	}

public:
	static Target end();

private:
	uint32_t _ipv4;
	leeloo::port _port;
};

}

#endif
