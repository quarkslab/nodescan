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

#ifndef NS_CONNECTED_TARGET_H
#define NS_CONNECTED_TARGET_H

#include <ns/target.h>
#include <leeloo/port.h>

namespace ns {

class Target;

class ConnectedTarget
{
public:
	ConnectedTarget(int s, Target const& t):
		_t(t),
		_s(s)
	{ }

public:
	int send(const char* buf) const;
	int send(unsigned char* buf, size_t s) const;

	inline uint32_t ipv4() const { return _t.ipv4(); }
	inline leeloo::port port() const { return _t.port(); }
	inline uint16_t port_value() const { return port().value(); }
	inline uint16_t port_protocol() const { return port().protocol(); }

	operator Target const&() const { return _t; }
	Target const& target() const { return _t; }

	bool operator<(ConnectedTarget const& o)  const { return _s <  o._s; }
	bool operator==(ConnectedTarget const& o) const { return _s == o._s; }

private:
	Target const& _t;
	int _s;
};

}

#endif
