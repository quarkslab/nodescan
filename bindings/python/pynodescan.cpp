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

#include <boost/python.hpp>


#include <ns/async_engine.h>
#include <ns/action.h>
#include <ns/target_set.h>
#include <ns/lvl4_properties_storage.h>

#include <ns/protocols/ssh.h>
#include <ns/protocols/ssl.h>
#include <ns/protocols/sip.h>

namespace nsp = ns::protocols;

using namespace boost::python;

static object object_from_ro_mem(unsigned char* buf, size_t const n)
{
#if PY_VERSION_HEX < 0x03000000
	return boost::python::object(boost::python::handle<>(PyBuffer_FromReadWriteMemory((void*) buf, n)));
#else
	return boost::python::object(boost::python::handle<>(PyMemoryView_FromMemory((char*) buf, n, PyBUF_READ)));
#endif
}

struct NSTargetSetWrap: ns::TargetSet, wrapper<ns::TargetSet>
{
	virtual void init() override
	{ this->get_override("init")(); }

	virtual void init_shrd(uint32_t shrd_idx, uint32_t shrd_count) override
	{ this->get_override("init_shrd")(shrd_idx, shrd_count); }

	virtual void save_state(const char* file) override
	{ this->get_override("save_state")(file); }


	virtual void restore_state(const char* file) override
	{ this->get_override("restore_state")(file); }

	virtual void save_state(std::ostream&) override
	{ }

	virtual void restore_state(std::istream&) override
	{ }

	virtual ns::Target next_target()
	{ return this->get_override("next_target")(); }

	virtual bool target_finished(ns::Target const& target, ns::HostSM& hsm)
	{ return this->get_override("target_finished")(target, hsm); }
};

static void connected_target_send(ns::ConnectedTarget const& t, std::string const& s)
{
	t.send((unsigned char*) s.c_str(), s.size());
}

static void async_engine_set_lvl4_connected_callback(ns::AsyncEngine& e, boost::python::object const& f)
{
	e.set_lvl4_connected_callback(
		[f](ns::ConnectedTarget const& target, ns::Lvl4SM& lvl4sm, ns::HostSM& hsm) -> bool
		{ 
			return extract<bool>(f(target, boost::ref(lvl4sm), boost::ref(hsm)));
		});
}

static void async_engine_set_lvl4_finish_callback(ns::AsyncEngine& e, boost::python::object const& f)
{
	e.set_lvl4_finish_callback(
		[f](ns::Target const& target, int code) -> bool
		{ 
			return extract<bool>(f(target, code));
		});
}

void lvl4sm_set_char_data_trigger(ns::Lvl4SM& lvl4sm, std::string const& c, boost::python::object const& f)
{
	if (c.size() != 1) {
		PyErr_SetString(PyExc_ValueError, "string must contain only one character");
		throw_error_already_set();
	}
	lvl4sm.set_trigger<ns::CharDataTrigger>(c[0],
		[f](ns::ConnectedTarget const& target, ns::Lvl4SM& lvl4sm_, ns::HostSM& hsm, unsigned char* buf, uint32_t size) -> bool
		{
			return extract<bool>(f(target, boost::ref(lvl4sm_), boost::ref(hsm), object_from_ro_mem(buf, size)));
		});
}

void lvl4sm_set_size_data_trigger(ns::Lvl4SM& lvl4sm, size_t const n, boost::python::object const& f)
{
	lvl4sm.set_trigger<ns::SizeDataTrigger>(n,
		[f](ns::ConnectedTarget const& target, ns::Lvl4SM& lvl4sm_, ns::HostSM& hsm, unsigned char* buf, uint32_t size) -> bool
		{
			return extract<bool>(f(target, boost::ref(lvl4sm_), boost::ref(hsm), object_from_ro_mem(buf, size)));
		});
}

void lvl4sm_set_on_connect(ns::Lvl4SM& lvl4sm, boost::python::object const& f)
{
	lvl4sm.set_on_connect(
		[f](ns::ConnectedTarget const& target, ns::Lvl4SM& lvl4sm_, ns::HostSM& hsm) -> bool
		{
			return extract<bool>(f(target, boost::ref(lvl4sm_), boost::ref(hsm)));
		});
}

void lvl4sm_set_python_data_trigger(ns::Lvl4SM& lvl4sm, boost::python::object const& obj)
{
	lvl4sm.set_trigger<ns::PythonDataTrigger>(obj);
}

boost::python::object lvl4buffer_data(ns::Lvl4Buffer& buf)
{
	return object_from_ro_mem(buf.begin(), buf.size());
}


typedef ns::Lvl4PropertiesStorage<boost::python::object> Lvl4PythonPropertiesStorage;

object& python_properties_storage_get(Lvl4PythonPropertiesStorage& s, ns::ConnectedTarget const& t)
{
	return s[t];
}

void python_properties_storage_set(Lvl4PythonPropertiesStorage& s, ns::Target const& t, boost::python::object const& o)
{
	s[t] = o;
}

void python_properties_storage_remove(Lvl4PythonPropertiesStorage& s, ns::ConnectedTarget const& t)
{
	s.remove(t);
}

boost::shared_ptr<Lvl4PythonPropertiesStorage> python_properties_init(object const& type)
{
	return boost::shared_ptr<Lvl4PythonPropertiesStorage>(new Lvl4PythonPropertiesStorage([type] { return type(); }));
}

void (ns::AsyncEngine::*async_engine_save_state)(const char* file) = &ns::AsyncEngine::save_state;
void (ns::AsyncEngine::*async_engine_restore_state)(const char* file) = &ns::AsyncEngine::restore_state;

//void (ns::TargetSet::*target_set_save_state)(const char* file) = &NSTargetSetWrap::save_state;
//void (ns::TargetSet::*target_set_restore_state)(const char* file) = &ns::TargetSet::restore_state;

struct PythonProtocols
{ };

nsp::SSL& ssl_on_certificate(nsp::SSL& ssl, object const& f)
{
	return ssl.on_certificate(
		[f](ns::ConnectedTarget const& target, ns::Lvl4SM& lvl4sm_, ns::HostSM& hsm, unsigned char* buf, uint32_t size) -> bool
		{
			return extract<bool>(f(target, boost::ref(lvl4sm_), boost::ref(hsm), object_from_ro_mem(buf, size)));
		});
}

nsp::SSL& ssl_on_cipher(nsp::SSL& ssl, object const& f)
{
	return ssl.on_cipher(
		[f](ns::ConnectedTarget const& target, ns::Lvl4SM& lvl4sm_, ns::HostSM& hsm, uint16_t cipher) -> bool
		{
			return extract<bool>(f(target, boost::ref(lvl4sm_), boost::ref(hsm), cipher));
		});
}

nsp::SSL& ssl_on_protocol_failure(nsp::SSL& ssl, object const& f)
{
	return ssl.on_protocol_failure(
		[f](ns::ConnectedTarget const& target, ns::Lvl4SM& lvl4sm_, ns::HostSM& hsm, unsigned char* buf, uint32_t size) -> bool
		{
			return extract<bool>(f(target, boost::ref(lvl4sm_), boost::ref(hsm), object_from_ro_mem(buf, size)));
		});
}

nsp::SSH& ssh_on_invalid_answer(nsp::SSH& ssh, object const& f)
{
	return ssh.on_invalid_answer(
		[f](ns::ConnectedTarget const& target, ns::Lvl4SM& lvl4sm_, ns::HostSM& hsm, unsigned char* buf, uint32_t size) -> bool
		{
			return extract<bool>(f(target, boost::ref(lvl4sm_), boost::ref(hsm), object_from_ro_mem(buf, size)));
		});
}

nsp::SSH& ssh_on_certificate(nsp::SSH& ssh, object const& f)
{
	return ssh.on_certificate(
		[f](ns::ConnectedTarget const& target, ns::Lvl4SM& lvl4sm_, ns::HostSM& hsm, unsigned char* buf, uint32_t size) -> bool
		{
			return extract<bool>(f(target, boost::ref(lvl4sm_), boost::ref(hsm), object_from_ro_mem(buf, size)));
		});
}

nsp::SIP& sip_on_header(nsp::SIP& sip, object const& f)
{
	return sip.on_header(
		[f](ns::ConnectedTarget const& target, ns::Lvl4SM& lvl4sm_, ns::HostSM& hsm, const char* key, const char* value) -> bool
		{
			return extract<bool>(f(target, boost::ref(lvl4sm_), boost::ref(hsm), key, value));
		});
}

nsp::SIP& sip_on_invalid_header(nsp::SIP& sip, object const& f)
{
	return sip.on_invalid_header(
		[f](ns::ConnectedTarget const& target, ns::Lvl4SM& lvl4sm_, ns::HostSM& hsm, unsigned char* buf, uint32_t size) -> bool
		{
			return extract<bool>(f(target, boost::ref(lvl4sm_), boost::ref(hsm), object_from_ro_mem(buf, size)));
		});
}

typedef ns::RepeatableTargetSet<ns::IPV4TargetSet> RepeatableIPV4TargetSet;

uint64_t target_hash(ns::Target const& t)
{
	return ((uint64_t) t.ipv4()) | (((uint64_t)(t.port().as_u32()))<<32);
}

typedef ns::RepeatableTargetSet<ns::SimpleTargetSet> RepeatableSimpleTargetSet;

BOOST_PYTHON_MODULE(pynodescan)
{
	class_<NSTargetSetWrap, boost::noncopyable>("TargetSet")
		.def("init", pure_virtual(&ns::TargetSet::init))
		.def("init_shrd", pure_virtual(&ns::TargetSet::init_shrd))
		.def("next_target", pure_virtual(&ns::TargetSet::next_target))
		.def("target_finished", pure_virtual(&ns::TargetSet::target_finished))
		//.def("save_state", &target_set_save_state)
		//.def("restore_state", &target_set_restore_state)
		;

	class_<ns::IPV4TargetSet, bases<ns::TargetSet>>("IPV4TargetSet",
						init<leeloo::ip_list_intervals&, leeloo::port_list_intervals&>(args("ips", "ports")))
		;

	class_<RepeatableIPV4TargetSet, bases<ns::TargetSet>>("RepeatableIPV4TargetSet",
						init<leeloo::ip_list_intervals&, leeloo::port_list_intervals&>(args("ips", "ports")))
		;

	class_<ns::SimpleTargetSet, bases<ns::TargetSet>>("SimpleTargetSet")
		.def("add_target", &ns::SimpleTargetSet::add_target)
		.def("remove_target", &ns::SimpleTargetSet::remove_target)
		;

	class_<RepeatableSimpleTargetSet, bases<ns::TargetSet>>("RepeatableSimpleTargetSet")
		.def("add_target", &RepeatableSimpleTargetSet::add_target)
		.def("remove_target", &RepeatableSimpleTargetSet::remove_target)
		;

	class_<ns::AsyncEngine>("AsyncEngine",
		             init<ns::TargetSet&, size_t, size_t>(args("targets", "nsockets", "timeout"), "Initialize a new AsyncEngine, using a maximum of nsockets. timeouts is in seconds."))
		.def("launch", &ns::AsyncEngine::launch)
		.def("launch_shrd", &ns::AsyncEngine::launch_shrd)
		.def("set_lvl4_connected_callback", &async_engine_set_lvl4_connected_callback)
		.def("set_lvl4_finish_callback", &async_engine_set_lvl4_finish_callback)
		.def("save_state", &ns::AsyncEngine::save_state)
		.def("restore_state", &ns::AsyncEngine::restore_state)
		.def("auto_save_state", &ns::AsyncEngine::auto_save_state)
		;

	class_<ns::Target>("Target",
			init<uint32_t, leeloo::port>(args("ipv4", "port"), "Initialize a target object"))
		.def("ipv4", &ns::Target::ipv4)
		.def("port", &ns::Target::port)
		.def("port_value", &ns::Target::port_value)
		.def("port_protocol", &ns::Target::port_protocol)
		.def("is_end", &ns::Target::is_end)
		.def("__hash__", &target_hash)
		.def("__eq__", &ns::Target::operator==)
		;

	class_<ns::ConnectedTarget>("ConnectedTarget",
		                init<int, ns::Target const&>())
		.def("send", &connected_target_send)
		.def("ipv4", &ns::ConnectedTarget::ipv4)
		.def("port", &ns::ConnectedTarget::port)
		.def("port_value", &ns::ConnectedTarget::port_value)
		.def("port_protocol", &ns::ConnectedTarget::port_protocol)
		.def("target", &ns::ConnectedTarget::target, return_value_policy<return_by_value>())
		;

	class_<ns::Lvl4SM, boost::noncopyable>("Lvl4SM")
		.def("set_reconnect", &ns::Lvl4SM::set_reconnect)
		.def("set_on_connect", lvl4sm_set_on_connect)
		.def("reconnect", &ns::Lvl4SM::reconnect)
		.def("set_char_data_trigger", lvl4sm_set_char_data_trigger)
		.def("set_size_data_trigger", lvl4sm_set_size_data_trigger)
		.def("set_data_trigger", lvl4sm_set_python_data_trigger)
		;

	class_<ns::HostSM, boost::noncopyable>("HostSM")
		;

	class_<ns::Lvl4Buffer, boost::noncopyable>("Lvl4Buffer")
		.def("data", &lvl4buffer_data)
		.def("pop_by", &ns::Lvl4Buffer::pop_by)
		;

	class_<Lvl4PythonPropertiesStorage, boost::shared_ptr<Lvl4PythonPropertiesStorage>>("Lvl4Properties", no_init)
		.def("__init__", make_constructor(python_properties_init))
		.def("remove", &Lvl4PythonPropertiesStorage::remove)
		.def("remove", &python_properties_storage_remove)
		.def("__getitem__", &Lvl4PythonPropertiesStorage::properties_of, return_value_policy<return_by_value>())
		.def("__getitem__", &python_properties_storage_get, return_value_policy<return_by_value>())
		.def("__setitem__", &python_properties_storage_set)
		;

	scope protocols = class_<PythonProtocols>("protocols");

	class_<nsp::SSL>("SSL")
		.def("__call__", &nsp::SSL::operator())
		.def("on_certificate", &ssl_on_certificate, return_internal_reference<>())
		.def("on_cipher", &ssl_on_cipher, return_internal_reference<>())
		.def("on_protocol_failure", &ssl_on_protocol_failure, return_internal_reference<>())
		;

	class_<nsp::SSH>("SSH")
		.def("__call__", &nsp::SSH::operator())
		.def("on_certificate", &ssh_on_certificate, return_internal_reference<>())
		.def("on_invalid_answer", &ssh_on_invalid_answer, return_internal_reference<>())
		;

	class_<nsp::SIP>("SIP")
		.def("__call__", &nsp::SIP::operator())
		.def("on_header", &sip_on_header, return_internal_reference<>())
		.def("on_invalid_header", &sip_on_invalid_header, return_internal_reference<>())
		;
}
