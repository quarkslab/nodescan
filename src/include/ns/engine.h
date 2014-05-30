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

#ifndef NS_ENGINE_H
#define NS_ENGINE_H

#include <stdint.h>
#include <stdlib.h>

#include <vector>
#include <map>

#include <leeloo/uni.h>
#include <leeloo/ip_list_intervals.h>

#include <ns/host_state_machine.h>
#include <ns/target.h>
#include <ns/target_set.h>

namespace ns {

class Engine
{
public:
	Engine(TargetSet& targets);

public:
	// Engine interface
	virtual void launch() = 0;
	virtual void launch_shrd(uint32_t idx, uint32_t total) = 0;

	void auto_save_state(const char* file, uint32_t time_s);

	void save_state(const char* file) { _targets.save_state(file); }
	void restore_state(const char* file) { _targets.restore_state(file); }
	
protected:
	inline void init_scan() { _targets.init(); }
	inline void init_shrd_scan(uint32_t shrd_idx, uint32_t shrd_count) { _targets.init_shrd(shrd_idx, shrd_count); }

	//inline void save_state(std::ostream& os) { _targets.save_state(os); }
	//inline void restore_state(std::istream& is) { _targets.restore_state(is); }

	inline Target next_target() { return _targets.next_target(); }
	bool target_finished(Target const& t, HostSM& hsm);
	inline void init_host_sm(Target const& t, HostSM& hsm) { _targets.init_host_sm(t, hsm); }

	bool should_save_state();

	const char* file_autosave() const { return _file_autosave.c_str(); }

private:
	TargetSet& _targets;
	std::string _file_autosave;
	time_t _last_save;
	time_t _time_autosave; // in seconds
};

}

#endif
