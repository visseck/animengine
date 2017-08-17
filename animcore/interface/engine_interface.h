#pragma once
#include "animcore/util/namespace.h"
#include "animpublic/interfaces/i_engine_interface.h"

namespace anim
{
	struct CoreCommands;
}
ANIM_NAMESPACE_BEGIN

class EngineInterface : public anim::IEngineInterface
{
public:
	static anim::CoreCommands& GetCoreCommands();
};

ANIM_NAMESPACE_END

