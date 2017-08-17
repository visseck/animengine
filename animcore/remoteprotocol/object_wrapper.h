#pragma once
#include "animcore/util/namespace.h"
#include "animcore/objectmodel/object.h"

#ifdef EDITOR_AVAILABLE


ANIM_NAMESPACE_BEGIN

class ObjectWrapper
{
public:
	ObjectWrapper(Object& obj);
	ObjectID m_ObjectID;
};

ANIM_NAMESPACE_END

#endif