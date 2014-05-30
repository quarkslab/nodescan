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

#ifndef NS_DATA_TRIGGERS_H
#define NS_DATA_TRIGGERS_H

#include <ns/data_trigger_base.h>
#include <ns/data_trigger_variant.h>

#include <boost/python/object.hpp>

namespace ns {

class SizeDataTrigger: public DataTrigger
{
public:
	SizeDataTrigger(uint32_t size, Lvl4DataAction const& f):
		_f(f),
		_size(size)
	{ }

public:
	virtual ProcessReturnCode process_buffer(ConnectedTarget const& target, Lvl4SM&, HostSM&, Lvl4Buffer&, uint32_t prev_pos) override;

private:
	Lvl4DataAction _f;
	uint32_t _size;
};

class CharDataTrigger: public DataTrigger
{
public:
	CharDataTrigger(unsigned char c, Lvl4DataAction const& f):
		_f(f),
		_c(c)
	{ }

public:
	virtual ProcessReturnCode process_buffer(ConnectedTarget const& target, Lvl4SM&, HostSM&, Lvl4Buffer&, uint32_t prev_pos) override;

private:
	Lvl4DataAction _f;
	unsigned char _c;
};

class PythonDataTrigger: public DataTrigger
{
public:
	PythonDataTrigger(boost::python::object const& obj):
		_trigger(obj)
	{ }

public:
	virtual ProcessReturnCode process_buffer(ConnectedTarget const&, Lvl4SM&, HostSM&, Lvl4Buffer&, uint32_t prev_pos) override;

private:
	boost::python::object _trigger;
};

typedef DataTriggerVariant<SizeDataTrigger, CharDataTrigger, PythonDataTrigger> DataTriggersVariant;

}

#endif
