#pragma once
#include <stdint.h>
#include "namespace.h"

ANIM_PUBLIC_NAMESPACE_BEGIN

class Guid
{
public:
	static Guid CreateNewGuid();
	Guid();
	bool isEmpty() const;
	uint8_t m_Data[16];
};

ANIM_PUBLIC_NAMESPACE_END