#pragma once
#include "util/namespace.h"

#include <unordered_map>
#include <mutex>

#include "objectmodel/managed_object.h"
#include "objectmodel/object_id.h"
#include "memory/default_allocator.h"
#include "memory/pointers.h"
#include "containers/singleton.h"
#include "util/assert.h"
#include "animcore/objectmodel/reference.h"

ANIM_NAMESPACE_BEGIN

class ObjectManager : public Singleton<ObjectManager>
{
public:	
	template<typename T>
	SharedPtr<T> GetManagedObject(const ObjectID& objectID);

	template<typename T>
	SharedPtr<T> LoadManagedObject(const ObjectID& objectID)
	{
		auto ret = GetManagedObject<T>(objectID);
		if (ret != nullptr)
			return ret;
		// Loading kickoff here
		T* data = nullptr;

		auto ptr = SharedPtr<T>(data, [](T* obj) -> void { ObjectManager::Instance().UnregisterManagedObject(obj); });
		m_ObjectMap[objectID] = ptr;
	}

	void UnregisterManagedObject(ManagedObject* object)
	{
		ANIM_ASSERT_SLOW(m_ObjectMap.find(object->GetObjectID()) != m_ObjectMap.end());
		m_ObjectMap.erase(object->GetObjectID());
		object->~ManagedObject();
		DefaultAllocator::Free(object);
	}
private:
	std::mutex m_ObjectMapMutex;
	std::unordered_map<ObjectID, SharedPtr<ManagedObject>> m_ObjectMap;
};



ANIM_NAMESPACE_END
