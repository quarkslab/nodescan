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

#ifndef NS_PROTOCOLS_SSH_H
#define NS_PROTOCOLS_SSH_H

#include <ns/action.h>

namespace ns {

class ConnectedTarget;
class Lvl4SM;
class HostSM;

namespace protocols {

class SSH
{
public:
	SSH& on_certificate(Lvl4DataAction const& a) { _cb_certif = a; return *this; }
	SSH& on_invalid_answer(Lvl4DataAction const& a) { _cb_invalid = a; return *this; }

public:
	bool operator()(ns::ConnectedTarget const& t, ns::Lvl4SM& lvl4sm, ns::HostSM&) const;

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
	ns::Lvl4DataAction _cb_certif;
	ns::Lvl4DataAction _cb_invalid;
};

} // protocols

} // ns

#endif
