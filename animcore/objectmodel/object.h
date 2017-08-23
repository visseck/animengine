#pragma once

#include "animcore/serialization/i_serializable.h"
#include "animcore/util/namespace.h"
#include <rttr/rttr_enable.h>

#ifdef EDITOR_AVAILABLE
#include "animcore/objectmodel/object_id.h"
#endif

ANIM_NAMESPACE_BEGIN

namespace Serialization
{
	class Serializer;
	class Deserializer;
}

class Object : public Serialization::ISerializable
{
	DECLARE_DERIVED_CLASS();
public:
	virtual void Initialize() {}
#ifdef EDITOR_AVAILABLE
public:
	const ObjectID& GetObjectID() const { return m_ObjectID; }
	void SetObjectID(const ObjectID& objID) { m_ObjectID = objID; }
	virtual void Serialize(Serialization::Serializer& res) const override;
	virtual void Deserialize(Serialization::Deserializer& res) override;
private:
	ObjectID m_ObjectID;
#endif
	

};

ANIM_NAMESPACE_END