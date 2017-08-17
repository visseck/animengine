#pragma once
#include "animcore/util/namespace.h"
#include <stdint.h>

ANIM_NAMESPACE_BEGIN

class ObjectID
{
public:
	static ObjectID CreateNewGuid();
	ObjectID();
	bool isValid() const;
	bool operator==(const ObjectID& other) const;
	bool operator!=(const ObjectID& other) const;
	bool operator>(const ObjectID& other) const;
	bool operator<(const ObjectID& other) const;
	bool operator>=(const ObjectID& other) const;
	bool operator<=(const ObjectID& other) const;
	uint8_t m_Data[16];
};

ANIM_NAMESPACE_END