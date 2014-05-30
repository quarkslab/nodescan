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

#include <ns/data_triggers.h>
#include <ns/connected_target.h>
#include <ns/lvl4_state_machine.h>
#include <ns/host_state_machine.h>

#include <boost/python/extract.hpp>

#include <algorithm>

ns::DataTrigger::ProcessReturnCode ns::SizeDataTrigger::process_buffer(ConnectedTarget const& target, Lvl4SM& lvl4sm, HostSM& hsm, Lvl4Buffer& buf, uint32_t)
{
	if (buf.size() < _size) {
		return NoMoreToProcess;
	}
	assert(_f);
	const size_t size = _size;
	const bool ret = _f(target, lvl4sm, hsm, buf.begin(), size);
	if (!ret) {
		return ConnectionEnd;
	}
	buf.pop_by(size);
	return CanProcessAgain;
}

ns::DataTrigger::ProcessReturnCode ns::CharDataTrigger::process_buffer(ConnectedTarget const& target, Lvl4SM& lvl4sm, HostSM& hsm, Lvl4Buffer& buf, uint32_t prev_pos)
{
	unsigned char* pc = std::find(buf.begin()+prev_pos, buf.end(), _c);
	if (pc == buf.end()) {
		return NoMoreToProcess;
	}
	const uintptr_t pos = ((uintptr_t)pc - (uintptr_t)buf.begin());
	const bool ret = _f(target, lvl4sm, hsm, buf.begin(), pos);
	if (!ret) {
		return ConnectionEnd;
	}
	buf.pop_by(pos+1);
	return CanProcessAgain;
}

ns::DataTrigger::ProcessReturnCode ns::PythonDataTrigger::process_buffer(ConnectedTarget const& target, Lvl4SM& lvl4sm, HostSM& hsm, Lvl4Buffer& buf, uint32_t prev_pos)
{
	return boost::python::extract<ns::DataTrigger::ProcessReturnCode>(_trigger.attr("process_buffer")(target, boost::ref(lvl4sm), boost::ref(hsm), boost::ref(buf), prev_pos));
}
