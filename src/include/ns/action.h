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

#ifndef NS_ACTION_H
#define NS_ACTION_H

#include <functional>
#include <cstdint>

namespace ns {

class HostSM;
class Lvl4SM;
class ConnectedTarget;
class Target;

typedef std::function<void(HostSM& sm)> HostAction;
typedef std::function<bool(ConnectedTarget const& target, Lvl4SM& lvl4sm, HostSM& hsm, unsigned char*, uint32_t size)> Lvl4DataAction;
typedef std::function<bool(ConnectedTarget const& target, Lvl4SM& lvl4sm, HostSM& hsm)> Lvl4Action;

typedef std::function<void(Target const& target, const unsigned char* buf_rem, uint32_t buf_size, int error)> Lvl4Finish;
typedef std::function<void(uint32_t nlaunched, uint32_t ndone)> StatusDisplay;
typedef std::function<bool(ConnectedTarget const& t)> WatchTimeout;
typedef std::function<uint32_t(Target const& t)> TimeoutTarget;

}

#endif
