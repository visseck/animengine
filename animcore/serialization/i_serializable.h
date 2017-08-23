#pragma once
#include "animcore/util/namespace.h"
#include "animcore/serialization/reflection.h"

ANIM_NAMESPACE_BEGIN

namespace Serialization
{
	class Serializer;
	class Deserializer;
	class ISerializable : public Reflection::RootReflectedClass
	{
		DECLARE_ROOT_CLASS();
	public:
		virtual void Serialize(Serializer&) const = 0;
		virtual void Deserialize(Deserializer&) = 0;
	};
}

ANIM_NAMESPACE_END

