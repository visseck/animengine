#include "managed_object.h"
#include <rttr/registration.h>

ANIM_NAMESPACE_BEGIN

RTTR_REGISTRATION
{
	using namespace rttr;
	registration::class_<ManagedObject>("ManagedObject")
	.constructor()
	.property_readonly("ObjectID", &ManagedObject::GetObjectID);
}

ANIM_NAMESPACE_END
