#pragma once
#include "animpublic/namespace.h"
#include <cstdio>

ANIM_PUBLIC_NAMESPACE_BEGIN

struct CoreCommands
{
	typedef void*(*AllocateFn)(size_t);
	AllocateFn m_AllocateFn;
	typedef void(*FreeFn)(void*);
	FreeFn m_FreeFn;
};

ANIM_PUBLIC_NAMESPACE_END
