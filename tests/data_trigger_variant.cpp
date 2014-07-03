#include <ns/action.h>
#include <ns/data_trigger_variant.h>
#include <ns/host_state_machine.h>
#include <ns/lvl4_state_machine.h>
#include <ns/target.h>

#include <iostream>
#include <functional>

int g_last_trigger = -1;
typedef std::function<void(int)> myfunc;

bool destructor_called_1 = false;
bool destructor_called_2 = false;

struct FakeDataTrigger1: public ns::DataTrigger
{
public:
	FakeDataTrigger1(myfunc const& f):
		_f(f)
	{ }

	~FakeDataTrigger1()
	{
		destructor_called_1 = true;
	}
public:
	ns::DataTrigger::ProcessReturnCode process_buffer(ns::ConnectedTarget const&, ns::Lvl4SM&, ns::HostSM&, ns::Lvl4Buffer&, uint32_t) override
	{
		g_last_trigger = 1;
		return ns::DataTrigger::NoMoreToProcess;
	}

private:
	myfunc _f;
};

struct FakeDataTrigger2: public ns::DataTrigger
{
public:
	FakeDataTrigger2(myfunc const& f):
		_f(f)
	{ }

	~FakeDataTrigger2()
	{
		destructor_called_2 = true;
	}
public:
	ns::DataTrigger::ProcessReturnCode process_buffer(ns::ConnectedTarget const&, ns::Lvl4SM&, ns::HostSM&, ns::Lvl4Buffer&, uint32_t) override
	{
		g_last_trigger = 2;
		return ns::DataTrigger::NoMoreToProcess;
	}

private:
	myfunc _f;
};

int do_test()
{
	ns::Lvl4Buffer buf;
	ns::Lvl4SM lvl4sm;
	ns::HostSM hsm;
	ns::Target target;
	ns::ConnectedTarget ctarget(-1, target);

	ns::DataTriggerVariant<FakeDataTrigger1, FakeDataTrigger2> variant;
	variant.set<FakeDataTrigger1>([](int i) { std::cout << i << std::endl; });
	variant.process_buffer(ctarget, lvl4sm, hsm, buf, 0);
	if (g_last_trigger != 1) {
		std::cout << "Wrong trigger called!" << std::endl;
		return 1;
	}
	variant.set<FakeDataTrigger2>([](int i) { std::cout << i << std::endl; });
	variant.process_buffer(ctarget, lvl4sm, hsm, buf, 0);
	if (g_last_trigger != 2) {
		std::cout << "Wrong trigger called!" << std::endl;
		return 1;
	}
	return 0;
}

int main()
{
	int ret = do_test();
	if (!destructor_called_1) {
		std::cerr << "Destructor of FakeDataTrigger1 has never been called!" << std::endl;
		ret = 1;
	}
	if (!destructor_called_2) {
		std::cerr << "Destructor of FakeDataTrigger2 has never been called!" << std::endl;
		ret = 1;
	}
	return ret;
}
