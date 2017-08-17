#pragma once
#include "animpublic/namespace.h"

ANIM_PUBLIC_NAMESPACE_BEGIN
struct CoreCommands;

class IEngineInterface
{
public:
	IEngineInterface() {}
	virtual void InitializeRuntime() = 0;
	virtual void FinalizeRuntime() = 0;
	virtual void RegisterCoreCommands(CoreCommands& commandStruct) = 0;
	virtual ~IEngineInterface() {}
private:
	IEngineInterface(const IEngineInterface&) = delete;
	IEngineInterface& operator=(const IEngineInterface&) = delete;
};
extern IEngineInterface& GetAnimEngineInterfaceController();

ANIM_PUBLIC_NAMESPACE_END
