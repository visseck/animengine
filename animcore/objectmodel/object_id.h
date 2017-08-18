#pragma once
#include "animcore/util/namespace.h"
#include <stdint.h>
#include <functional>

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
	uint64_t GetHash() const;
	uint8_t m_Data[16];
};

ANIM_NAMESPACE_END

namespace std
{
	template<>
	struct hash<animengine::ObjectID>
	{
		std::size_t operator()(const animengine::ObjectID& k) const
		{
			return static_cast<size_t>(k.GetHash());
		}
	};
}
