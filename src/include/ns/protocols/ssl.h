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

#ifndef NS_PROTOCOLS_SSL_H
#define NS_PROTOCOLS_SSL_H

#include <ns/action.h>
#include <ns/protocols/ssl_structs.h>

namespace ns {

namespace protocols {

class SSL 
{
public:
	typedef std::function<bool(ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM& hsm, uint16_t cipher)> CipherDataAction;
public:
	SSL();

public:
	SSL& on_protocol_failure(Lvl4DataAction const& f)
	{
		_on_failure = f;
		return *this;
	}

	SSL& on_cipher(CipherDataAction const& f)
	{
		_on_cipher = f;
		return *this;
	}

	SSL& on_certificate(ns::Lvl4DataAction const& f)
	{
		_on_certif = f;
		return *this;
	}

public:
	bool operator()(ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM&) const;

private:
	bool on_tls_header(ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM&, unsigned char* buf, uint32_t size, ns::Lvl4DataAction const& on_hs) const;

	bool on_handshake1(ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM&, unsigned char* buf, uint32_t size) const;

	bool on_handshake2(ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM& hsm, unsigned char* buf, uint32_t size) const;

private:
	template <bool def, class Action, class... Args>
	static bool call_cb(Action const& action, Args && ... args)
	{
		if (action) {
			return action(std::forward<Args>(args)...);
		}
		return def;
	}

private:
	uint8_t client_hello[sizeof(__impl::TLS)+sizeof(__impl::Handshake)+sizeof(__impl::ClientHello)];
	ns::Lvl4DataAction _on_certif;
	CipherDataAction _on_cipher;
	ns::Lvl4DataAction _on_failure;
};

} // protocols

} // leeloo

#endif
