#include "animpublic/interfaces/i_engine_interface.h"
#include "animpublic/commands/core_commands.h"

#include "core_commands_integration.h"
#include <rttr/rttr_enable.h>
#include <rttr/type.h>
#include <rttr/registration.h>
#include "animcore/containers/string.h"
#include "animcore/memory/pointers.h"
#include "animcore/objectmodel/reference.h"

struct Temp
{
	//RTTR_ENABLE();
};
struct Foo : Temp
{
	Foo() {}
	Foo(int y) : x(y) {}
	int x;
	/*RTTR_ENABLE(Temp);*/
};

struct Bar
{
	Foo Stuff() { Foo f; f.x = 5; return f; }
	RTTR_ENABLE();
};
using namespace rttr;
RTTR_REGISTRATION
{
	registration::class_<Temp>("Temp");
	registration::class_<Foo>("Foo")
.property("x", &Foo::x);

registration::class_<Bar>("Bar")
.method("Stuff", &Bar::Stuff);
}


using namespace animengine;
int main()
{
	auto& animController = anim::GetAnimEngineInterfaceController();
	{
		anim::CoreCommands coreCmds;
		coreCmds.m_AllocateFn = &Allocate;
		coreCmds.m_FreeFn = &Free;
		animController.RegisterCoreCommands(coreCmds);
	}
	using namespace rttr;
	type t = type::get<Foo>();
	Foo f;
	for (auto prop : t.get_properties())
	{
		type propType = prop.get_type();
		auto value = prop.get_value(f);
		value.g
	}

	animController.InitializeRuntime();
	animController.FinalizeRuntime();
}