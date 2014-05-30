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

#include <unistd.h>
#include <string.h>

#include <boost/log/trivial.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/attributes/named_scope.hpp>

#include <ns/lvl4_state_machine.h>

ns::Lvl4SM::Lvl4SM():
	_reconnect(false),
	_reprocess(false)
{
}

ns::Lvl4SM::~Lvl4SM()
{
}

bool ns::Lvl4SM::process_lvl4_data(int s, uint32_t navail, Target const& target, HostSM& hsm)
{
	BOOST_LOG_NAMED_SCOPE("Lvl4SM::process_lvl4_data");

	uint32_t prev_pos = _buf.size();
	unsigned char* buf_read = _buf.grow_by(navail);
	int nread = read(s, buf_read, navail);
	if (nread <= 0) {
		// TODO: callback
		BOOST_LOG_TRIVIAL(trace) << s << "invalid read size: " << strerror(errno) << std::endl;
		return false;
	}
	int ret;
	do {
		ret = _trigger.process_buffer(ConnectedTarget(s, target), *this, hsm, _buf, prev_pos);
		prev_pos = 0;
	}
	while (ret == DataTrigger::CanProcessAgain);
	return ret == DataTrigger::NoMoreToProcess;
}

bool ns::Lvl4SM::process_buffer(int s, Target const& target, HostSM& hsm)
{
	int ret;
	do {
		ret = _trigger.process_buffer(ConnectedTarget(s, target), *this, hsm, _buf, 0);
	}
	while (ret == DataTrigger::CanProcessAgain);
	return ret == DataTrigger::NoMoreToProcess;
}
