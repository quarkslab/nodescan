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

#ifndef NS_BUFFER_H
#define NS_BUFFER_H

#include <type_traits>
#include <cstdlib>
#include <utility>

#include <assert.h>
#include <string.h>

namespace ns {

template <class T, class SizeType>
class Buffer
{
	static_assert(std::is_pod<T>::value, "T must be a POD");

	typedef SizeType size_type;
	typedef T value_type;
	typedef T* pointer_type;
	typedef T const* const_pointer_type;

public:
	Buffer():
		_buffer(nullptr),
		_buf_size(0),
		_buf_pos(0)
	{ }

	Buffer(Buffer const& o):
		Buffer()
	{
		copy(o);
	}

	Buffer(Buffer&& o):
		_buffer(nullptr)
	{
		move(std::move(o));
	}

	~Buffer()
	{
		if (_buffer) {
			::free(_buffer);
		}
	}

public:
	void free()
	{
		if (_buffer) {
			::free(_buffer);
			_buffer = NULL;
			_buf_pos = 0;
			_buf_size = 0;
		}
	}

public:
	Buffer& operator=(Buffer const& o)
	{
		if (&o != this) {
			copy(o);
		}
		return *this;
	}

	Buffer& operator=(Buffer&& o)
	{
		if (&o != this) {
			move(std::move(o));
		}
		return *this;
	}

public:
	// TODO: replace by malloc_usable_size and save memory!
	inline size_type allocated_size() const { return _buf_size; }
	inline size_type size() const { return _buf_pos; }

	inline pointer_type begin() { return _buffer; }
	inline const_pointer_type begin() const { return _buffer; }

	inline pointer_type end() { return _buffer + size(); }
	inline const_pointer_type end() const { return _buffer + size(); }

	inline pointer_type at(size_type const i) { return begin()+i; }
	inline const_pointer_type at(size_type const i) const { return begin()+i; }

	inline pointer_type operator[](size_type const i) { return at(i); }
	inline const_pointer_type operator[](size_type const i) const { return at(i); }

public:
	// Returns a pointer to the previous position of the possibly new allocated buffer
	pointer_type grow_by(size_type n)
	{
		size_type old_pos = size();
		allocate(size() + n);
		size_() += n;
		return begin()+old_pos;
	}

	void pop_by(size_type n)
	{
		assert(n <= _buf_pos);
		const size_type rem = size()-n;
		memmove(_buffer, _buffer + n, rem);
		size_() -= n;
	}

private:
	inline size_type& size_() { return _buf_pos; }
	inline size_type& allocated_size_() { return _buf_size; }

	static inline size_type size_buf(size_type n) { return n*sizeof(size_type); }

	void allocate(size_type n)
	{
		if (_buffer) {
			if (allocated_size() < n) {
				_buffer = reinterpret_cast<pointer_type>(realloc(_buffer, size_buf(n)));
				allocated_size_() = n;
			}
		}
		else {
			_buffer = reinterpret_cast<pointer_type>(malloc(size_buf(n)));
			allocated_size_() = n;
		}
	}

	void copy(Buffer const& o)
	{
		allocate(o.size());
		memcpy(_buffer, o._buffer, size_buf(o.size()));
		size_() = o.size();
	}

	void move(Buffer&& o)
	{
		if (_buffer) {
			::free(_buffer);
		}
		_buffer = o._buffer;
		allocated_size_() = o.allocated_size();
		size_() = o.size();
		o._buffer = nullptr;
		o.size_() = 0;
		o.allocated_size_() = 0;
	}

private:
	pointer_type _buffer;
	size_type _buf_size;
	size_type _buf_pos;
};

}

#endif
