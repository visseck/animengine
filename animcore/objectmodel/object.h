#pragma once

#include "animcore/util/namespace.h"
#include <rttr/rttr_enable.h>

#ifdef EDITOR_AVAILABLE
#include "animcore/objectmodel/object_id.h"
#endif

ANIM_NAMESPACE_BEGIN

class Object
{
	RTTR_ENABLE();
public:
	virtual void Initialize() {}
#ifdef EDITOR_AVAILABLE
public:
	const ObjectID& GetObjectID() const { return m_ObjectID; }
	void SetObjectID(const ObjectID& objID) { m_ObjectID = objID; }
private:
	ObjectID m_ObjectID;
#endif
	

};

ANIM_NAMESPACE_END