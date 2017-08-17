#pragma once
#include <type_traits>
#include "animcore/util/namespace.h"
#include "animcore/objectmodel/managed_object.h"

ANIM_NAMESPACE_BEGIN

template<typename T, typename Enable = void>
struct IsManagedObject
{
	static constexpr bool Value = false;
};

template<typename T>
struct IsManagedObject<T, typename std::enable_if<std::is_base_of<ManagedObject, T>::value>::type>
{
	static constexpr bool Value = true;
};

template<typename T>
struct Reference
{
	static_assert(IsManagedObject<T>::Value, "Reference can only be to a ManagedObject (or derived class)");
public:
	Reference() = default;
	ObjectID m_ObjectID;
	SharedPtr<T> m_ObjPtr;
};

ANIM_NAMESPACE_END
