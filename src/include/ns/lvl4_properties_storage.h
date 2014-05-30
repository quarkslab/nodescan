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

#ifndef LVL4_PROPERTIES_STORAGE_H
#define LVL4_PROPERTIES_STORAGE_H

#include <ns/target.h>
#include <ns/connected_target.h>

#include <map>

namespace ns {

template <class Properties>
class Lvl4PropertiesStorage
{
	typedef Properties properties_type;
	typedef std::map<Target, properties_type> storage_type;

public:
	typedef std::function<properties_type()> func_new_property;

public:
	Lvl4PropertiesStorage():
		_new([] { return properties_type(); })
	{ }

	Lvl4PropertiesStorage(func_new_property const& f):
		_new(f)
	{ }


public:
	properties_type& properties_of(Target const& target)
	{
		typename storage_type::iterator it = _storage.find(target);
		if (it != _storage.end()) {
			return it->second;
		}
		return _storage.insert(std::make_pair(target, std::move(_new()))).first->second;
	}

	properties_type& operator[](Target const& target) { return properties_of(target); }

	void remove(Target const& target)
	{
		typename storage_type::iterator it = _storage.find(target);
		if (it != _storage.end()) {
			_storage.erase(it);
		}
	}

	void clear()
	{
		_storage->~storage_type();
		new (&_storage) storage_type();
	}

private:
	storage_type _storage;
	func_new_property _new;
};

}

#endif
