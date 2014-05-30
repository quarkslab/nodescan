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

#include <ns/connected_target.h>
#include <ns/lvl4_state_machine.h>
#include <ns/protocols/ssl.h>

#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

namespace nsp = ns::protocols;
using nsp::__impl::TLS;
using nsp::__impl::Handshake;
using nsp::__impl::ClientHello;
using nsp::__impl::Certificate;

#pragma pack(pop)

nsp::SSL::SSL()
{
	TLS tls;
	tls.msg_type = 0x16; // handshake
	tls.client_version = {3, 0};
	tls.length = htons(sizeof(Handshake)+sizeof(ClientHello));

	Handshake hs;
	hs.msg_type = 1; // client_hello
	hs.length2 = 0;
	hs.length1 = 0;
	hs.length0 = sizeof(ClientHello);

	ClientHello ch;
	ch.client_version = {3, 0};
	ch.random.gmt_unix_time = htonl(time(NULL));
	memset(&ch.random.random_bytes[0], 0xAA, 28);
	ch.session_id.length = 0;
	ch.cipher_suites.length = htons(sizeof(ch.cipher_suites.data));
	ch.cipher_suites.data[0] = 0x0100;
	ch.cipher_suites.data[1] = 0x0200;
	ch.cipher_suites.data[2] = 0x3B00;
	ch.cipher_suites.data[3] = 0x0400;
	ch.cipher_suites.data[4] = 0x0500;
	ch.cipher_suites.data[5] = 0x0A00;
	ch.cipher_suites.data[6] = 0x2F00;
	ch.cipher_suites.data[7] = 0x3500;
	ch.cipher_suites.data[8] = 0x3C00;
	ch.cipher_suites.data[9] = 0x3D00;
	ch.compression_methods = {1, 0};
	ch.ext_length = 0;

	memcpy(client_hello, &tls, sizeof(TLS));
	memcpy(client_hello+sizeof(TLS), &hs, sizeof(Handshake));
	memcpy(client_hello+sizeof(TLS)+sizeof(Handshake), &ch, sizeof(ClientHello));
}

bool nsp::SSL::operator()(ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM&) const
{
	t.send((unsigned char*) client_hello, sizeof(client_hello));
	lvl4sm.set_trigger<ns::SizeDataTrigger>(sizeof(TLS),
		[this](ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM& hsm, unsigned char* buf, uint32_t size) -> bool
		{ return this->on_tls_header(t, lvl4sm, hsm, buf, size, [this](ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM& hsm, unsigned char* buf, uint32_t size) { return this->on_handshake1(t, lvl4sm, hsm, buf, size); }); });

	return true;
}

bool nsp::SSL::on_tls_header(ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM& hsm, unsigned char* buf, uint32_t size, ns::Lvl4DataAction const& on_hs) const
{
	TLS const* tls = reinterpret_cast<TLS const*>(buf);
	if (tls->msg_type != 0x16) { // Handshake
		return call_cb<false>(_on_failure, t, lvl4sm, hsm, buf, size);
	}
	lvl4sm.set_trigger<ns::SizeDataTrigger>(ntohs(tls->length), on_hs);
	return true;
}

bool nsp::SSL::on_handshake1(ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM& hsm, unsigned char* buf, uint32_t size) const
{
	if (size < sizeof(Handshake)) {
		return call_cb<false>(_on_failure, t, lvl4sm, hsm, buf, size);
	}
	size -= sizeof(Handshake);
	Handshake const* hs = reinterpret_cast<Handshake const*>(buf);
	if (hs->msg_type != 2) { // Server hello
		return call_cb<false>(_on_failure, t, lvl4sm, hsm, buf, size);
	}
	int length = hs->get_length();
	if (length != size) {
		return call_cb<false>(_on_failure, t, lvl4sm, hsm, buf, size);
	}
	buf += sizeof(Handshake);
	uint16_t cipher = *((uint16_t*)(&buf[size-3]));
	if (!call_cb<true>(_on_cipher, t, lvl4sm, hsm, cipher)) {
		return false;
	}

	lvl4sm.set_trigger<ns::SizeDataTrigger>(sizeof(TLS),
		[this](ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM& hsm, unsigned char* buf, uint32_t size) -> bool
		{ return on_tls_header(t, lvl4sm, hsm, buf, size, [this](ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM& hsm, unsigned char* buf, uint32_t size) { return this->on_handshake2(t, lvl4sm, hsm, buf, size); }); });
	return true;
}

bool nsp::SSL::on_handshake2(ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM& hsm, unsigned char* buf, uint32_t size) const
{
	if (size < sizeof(Handshake)) {
		return call_cb<false>(_on_failure, t, lvl4sm, hsm, buf, size);
	}
	size -= sizeof(Handshake);
	Handshake const* hs = reinterpret_cast<Handshake const*>(buf);
	int length = hs->get_length();
	if (hs->msg_type != 11) { // Certificates
		return call_cb<false>(_on_failure, t, lvl4sm, hsm, buf, size);
	}
	if (length != size) {
		return call_cb<false>(_on_failure, t, lvl4sm, hsm, buf, size);
	}
	buf += sizeof(Handshake);
	if (size < 3) {
		return call_cb<false>(_on_failure, t, lvl4sm, hsm, buf, size);
	}
	Certificate const* crts = reinterpret_cast<Certificate const*>(buf);
	int crts_len = crts->get_length();
	size -= 3;
	buf += 3;
	if (crts_len != size) {
		return call_cb<false>(_on_failure, t, lvl4sm, hsm, buf, size);
	}
	while (crts_len > 0) {
		Certificate const* crt = reinterpret_cast<Certificate const*>(buf);
		if (crts_len < 3) {
			return call_cb<false>(_on_failure, t, lvl4sm, hsm, buf, size);
		}
		int crt_len = crt->get_length();
		if (crts_len < crt_len) {
			return call_cb<false>(_on_failure, t, lvl4sm, hsm, buf, size);
		}
		buf += 3;
		crts_len -= 3;
		if (!call_cb<true>(_on_certif, t, lvl4sm, hsm, buf, crt_len)) {
			return false;
		}
		buf += crt_len;
		crts_len -= crt_len;
	}
	return false;
}
