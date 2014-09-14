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

#ifndef NS_LVL4_STATE_MACHINE
#define NS_LVL4_STATE_MACHINE

#include <cstdint>
#include <time.h>

#include <ns/buffer.h>
#include <ns/state_machine.h>
#include <ns/host_state_machine.h>
#include <ns/lvl4_buffer.h>
#include <ns/data_triggers.h>
#include <ns/connected_target.h>

namespace ns {

class Lvl4SM: public StateMachine
{
public:
	Lvl4SM();
	~Lvl4SM();

public:
	bool process_lvl4_data(int s, uint32_t navail, Target const& target, HostSM& hsm);
	bool process_buffer(int s, Target const& target, HostSM& hsm);

	void set_reconnect(bool v) { _reconnect = v; }
	bool reconnect() const { return _reconnect; }

	void set_on_connect(Lvl4Action const f) { _func_connect = f; }
	inline bool on_connect(int s, Target const& target, HostSM& hsm)
	{
		if (!_func_connect) {
			std::cerr << "Warning: no on_connect func for " << target.ipv4() << std::endl;
			return false;
		}
		return _func_connect(ConnectedTarget(s, target), *this, hsm);
	}

	inline void free_buffer() { _buf.free(); }

	template <class T, typename... Args>
	void set_trigger(Args && ... args)
	{
		_trigger.set<T>(std::forward<Args>(args)...);
		_reprocess = true;
	}

	DataTrigger const* data_trigger() const { return _trigger.trigger(); }

	bool should_process_buffer()
	{
		if (_reprocess) {
			_reprocess = false;
			return true;
		}
		return false;
	}

	Lvl4Buffer const& buffer() const { return _buf; }

	void remove_data_trigger()
	{
		set_trigger<DataTrigger>();
		_reprocess = false;
	}

private:
	bool _reconnect;
	bool _reprocess;

	Lvl4Action _func_connect;
	DataTriggersVariant _trigger;

	Lvl4Buffer _buf;
};

}


#endif
