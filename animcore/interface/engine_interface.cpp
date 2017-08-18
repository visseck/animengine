#include "engine_interface.h"
#include "animcore/memory/pointers.h"
#include "animpublic/commands/core_commands.h"
#include "animcore/containers/array.h"

ANIM_NAMESPACE_BEGIN
static anim::CoreCommands s_CoreCommands;

class EngineInterfaceImpl : public EngineInterface
{
public:
	virtual void InitializeRuntime() override;
	virtual void FinalizeRuntime() override;
	virtual void RegisterCoreCommands(anim::CoreCommands& commandStruct) override
	{
		s_CoreCommands = commandStruct;
	}
};
static EngineInterfaceImpl s_EngineInterface;


void EngineInterfaceImpl::InitializeRuntime()
{
	Array<int> stuff;
	stuff.Push(5);
}

void EngineInterfaceImpl::FinalizeRuntime()
{
}

anim::CoreCommands& EngineInterface::GetCoreCommands()
{
	return s_CoreCommands;
}

ANIM_NAMESPACE_END

ANIM_PUBLIC_NAMESPACE_BEGIN

IEngineInterface& GetAnimEngineInterfaceController()
{
	return animengine::s_EngineInterface;
}

ANIM_PUBLIC_NAMESPACE_END

