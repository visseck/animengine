#include "object.h"
#include "animcore/serialization/serialization.h"

ANIM_NAMESPACE_BEGIN

IMPLEMENT_CONCRETE_DERIVED_CLASS(Object, Serialization::ISerializable);

void Object::Serialize(Serialization::Serializer& res) const
{
	res.Serialize(m_ObjectID);
}

void Object::Deserialize(Serialization::Deserializer& res)
{
	res.Deserialize(m_ObjectID);
}



ANIM_NAMESPACE_END
