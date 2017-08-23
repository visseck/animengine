#pragma once
#include "animcore/util/namespace.h"
#include "animcore/objectmodel/object.h"

ANIM_NAMESPACE_BEGIN

class Message : public Object
{
	DECLARE_DERIVED_CLASS();
public:
	virtual ~Message() {}
};

ANIM_NAMESPACE_END