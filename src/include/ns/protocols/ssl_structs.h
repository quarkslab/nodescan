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

#ifndef NS_PROTOCOLS_SSL_STRUCTS_H
#define NS_PROTOCOLS_SSL_STRUCTS_H

#include <cstdint>

namespace ns {
namespace protocols {
namespace __impl {

#pragma pack(push)
#pragma pack(1)
struct ProtocolVersion
{
	uint8_t major;
	uint8_t minor;
};

struct Random
{
	uint32_t gmt_unix_time;
	uint8_t random_bytes[28];
};

struct SessionID
{
	uint8_t length;
};

struct CipherSuite
{
	uint16_t length;
	uint16_t data[10];
};

struct CompressionMethod
{
	uint8_t length;
	uint8_t data;
};

struct ClientHello
{
	ProtocolVersion client_version;
	Random random;
	SessionID session_id;
	CipherSuite cipher_suites;
	CompressionMethod compression_methods;
	uint16_t ext_length;
};

struct TLS
{
	uint8_t msg_type;
	ProtocolVersion client_version;
	uint16_t length;
};

struct Handshake
{
	uint32_t msg_type: 8;
	uint32_t length2: 8;
	uint32_t length1: 8;
	uint32_t length0: 8;

	uint32_t get_length() const { return (length2 << 16) | (length1 << 8) | (length0); }
};

struct Certificate
{
	uint32_t length2: 8;
	uint32_t length1: 8;
	uint32_t length0: 8;

	uint32_t get_length() const { return (length2 << 16) | (length1 << 8) | (length0); }
};

} // __impl
} // protocols 
} // ns 


#endif
