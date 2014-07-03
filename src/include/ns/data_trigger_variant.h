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

#ifndef NS_TRIGGER_VARIANT_H
#define NS_TRIGGER_VARIANT_H

#include <ns/data_trigger_base.h>
#include <ns/static_max_size.h>

namespace ns {

class DataTrigger;

// TODO: we can win space by using a simple integer to remember on which type
// to cast (and do not use any virtual methods).
// This requires some C++11 magic :)
template <class... Types>
class DataTriggerVariant
{
	static constexpr size_t max_size = static_max_size<Types...>::value;

public:
	DataTriggerVariant()
	{
		new (storage()) DataTrigger();
	}

	~DataTriggerVariant()
	{
		storage()->~DataTrigger();
	}

public:
	template <class T, typename... Args>
	inline void set(Args && ... args)
	{
		storage()->~DataTrigger();
		new (storage()) T(std::forward<Args>(args)...);
	}

	template <typename... Args>
	inline int process_buffer(Args && ... args)
	{
		return storage()->process_buffer(std::forward<Args>(args)...);
	}

	inline DataTrigger const* trigger()  const { return reinterpret_cast<DataTrigger const*>(&_storage[0]); }

private:
	inline DataTrigger* storage() { return reinterpret_cast<DataTrigger*>(&_storage[0]); }

private:
	uint8_t _storage[max_size];
};

}

#endif
