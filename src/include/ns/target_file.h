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

#ifndef NS_TARGET_FILE_H
#define NS_TARGET_FILE_H

#include <ns/target_set.h>

#include <leeloo/port.h>

namespace ns {

class Engine;
class HostSM;

#if 0
class TargetFile: public TargetSet
{
	friend class Engine;

public:
	TargetFile(const char* file);
	TargetFile(std::istream& is);

public:
	void init() override;
	void init_shrd(uint32_t shrd_idx, uint32_t shrd_count);

	void save_state(std::ostream& os) override;
	void restore_state(std::istream& is) override;

	Target next_target() override;
	bool target_finished(Target const& target, HostSM& hsm); 

	void init_host_sm(Target const& target, HostSM& hsm);

private:
	void open_file(const char* file);
	std::istream& stream() { return *_stream; }

private:
	boost::shared_ptr<std::istream> _storage;
	std::istream* _stream;
	size_t _cur_line;
};
#endif

class TargetAsyncFile: public TargetSet
{
public:
	TargetAsyncFile(int fd, leeloo::port const& def_port);

public:
	virtual void init() override;
	virtual void init_shrd(uint32_t shrd_idx, uint32_t shrd_count) override;

	virtual void save_state(const char*) override { };
	virtual void restore_state(const char*) override { };

	virtual void save_state(std::ostream&) override { };
	virtual void restore_state(std::istream&) override { };

	virtual Target next_target() override;

	virtual bool target_finished(Target const&, HostSM&) override
	{
		return true;
	}

private:
	inline int fd() const { return _fd; }
	Target next_target_in_buf();

private:
	Lvl4Buffer _buf;
	int _fd;
	leeloo::port _def_port;
};

class TargetStdin: public TargetAsyncFile
{
public:
	TargetStdin(leeloo::port const& def_port);
};

}

#endif
