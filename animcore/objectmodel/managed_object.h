#pragma once
#include "animcore/util/namespace.h"
#include "animcore/objectmodel/object.h"
#include "animcore/objectmodel/object_id.h"

ANIM_NAMESPACE_BEGIN
class ObjectManager;
class ManagedObject : public Object
{
	typedef Object super;
#ifndef EDITOR_AVAILABLE
public:
	const ObjectID& GetObjectID() { return m_ObjectID; }
private:
	void SetObjectID(const ObjectID& obj) { m_ObjectID = obj; }
	ObjectID m_ObjectID;
#else
public:
	const ObjectID& GetObjectID() { return super::GetObjectID(); }
private:
	void SetObjectID(const ObjectID& obj) { super::SetObjectID(obj); }
#endif
	friend class ObjectManager;
	RTTR_ENABLE();
};

ANIM_NAMESPACE_END