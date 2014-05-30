#include <ns/action.h>
#include <ns/data_trigger_variant.h>
#include <ns/host_state_machine.h>
#include <ns/lvl4_state_machine.h>
#include <ns/target.h>

#include <iostream>

int g_last_trigger = -1;

struct FakeDataTrigger1: public ns::DataTrigger
{
public:
	ns::DataTrigger::ProcessReturnCode process_buffer(ns::ConnectedTarget const&, ns::Lvl4SM&, ns::HostSM&, ns::Lvl4Buffer&, uint32_t) override
	{
		g_last_trigger = 1;
		return ns::DataTrigger::NoMoreToProcess;
	}
};

struct FakeDataTrigger2: public ns::DataTrigger
{
public:
	ns::DataTrigger::ProcessReturnCode process_buffer(ns::ConnectedTarget const&, ns::Lvl4SM&, ns::HostSM&, ns::Lvl4Buffer&, uint32_t) override
	{
		g_last_trigger = 2;
		return ns::DataTrigger::NoMoreToProcess;
	}
};

int main()
{
	ns::Lvl4Buffer buf;
	ns::Lvl4SM lvl4sm;
	ns::HostSM hsm;
	ns::Target target;
	ns::ConnectedTarget ctarget(-1, target);

	ns::DataTriggerVariant<FakeDataTrigger1, FakeDataTrigger2> variant;
	variant.set<FakeDataTrigger1>();
	variant.process_buffer(ctarget, lvl4sm, hsm, buf, 0);
	if (g_last_trigger != 1) {
		std::cout << "Wrong trigger called!" << std::endl;
		return 1;
	}
	variant.set<FakeDataTrigger2>();
	variant.process_buffer(ctarget, lvl4sm, hsm, buf, 0);
	if (g_last_trigger != 2) {
		std::cout << "Wrong trigger called!" << std::endl;
		return 1;
	}

	return 0;
}
