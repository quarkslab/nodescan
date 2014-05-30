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

#include <ns/protocols/sip.h>
#include <ns/lvl4_state_machine.h>

namespace nsp = ns::protocols;

static const char* get_options = R"str(OPTIONS sip:sip@127.0.0.1 SIP/2.0
Via: SIP/2.0/UDP 127.0.0.1;branch=z9hG4bKhjhs8ass877
Max-Forwards: 70
To: <sip:my@poney.com>
From: Alice <sip:her@poney.com>;tag=1928301774
Call-ID: a84b4c76e66710
CSeq: 63104 OPTIONS
Contact: <sip:my@poney.com>
Accept: application/sdp
Content-Length: 0


)str";

static const char* sip_20 = "SIP/2.0";

bool nsp::SIP::operator()(ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM&) const
{
	t.send(get_options);
	ns::Lvl4DataAction cb_invalid_header = _cb_invalid_header;
	HeaderDataAction cb_header = _cb_header;
	lvl4sm.set_trigger<ns::CharDataTrigger>('\n',
		[cb_invalid_header,cb_header](ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM& hsm, unsigned char* buf, uint32_t size)
		{
			if (size < 1) {
				return call_cb<false>(cb_invalid_header, t, lvl4sm, hsm, buf, size);
			}
			if (buf[size-1] != '\r') {
				return call_cb<false>(cb_invalid_header, t, lvl4sm, hsm, buf, size);
			}
			buf[size-1] = 0;
			if (size == 1) {
				return false;
			}
			char* dot = (char*) memchr(buf, ':', size);
			if (dot == nullptr) {
				if ((size > strlen(sip_20)) && (memcmp(buf, sip_20, strlen(sip_20)) == 0)) {
					return true;
				}
				return call_cb<false>(cb_invalid_header, t, lvl4sm, hsm, buf, size);
			}
			*dot = 0;
			char* value = dot+1;
			if (*value != 0 && (*(value+1) == ' ')) {
				value++;
			}
			return call_cb<true>(cb_header, t, lvl4sm, hsm, (const char*) buf, value);
		}
		);
	return true;
}
