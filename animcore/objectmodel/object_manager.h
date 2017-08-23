#pragma once
#include "animcore/util/namespace.h"
#include <mutex>

#include "animcore/objectmodel/object_id.h"
#include "animcore/memory/default_allocator.h"
#include "animcore/memory/pointers.h"
#include "animcore/containers/singleton.h"
#include "animcore/containers/unordered_map.h"
#include "animcore/util/assert.h"

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

	template<typename T>
	void UnregisterManagedObject(T* object)
	{
		ANIM_ASSERT_SLOW(m_ObjectMap.find(object->GetObjectID()) != m_ObjectMap.end());
		m_ObjectMap.erase(object->GetObjectID());
		object->~ManagedObject();
		DefaultAllocator::Free(object);
	}
private:
	std::mutex m_ObjectMapMutex;
	UnorderedMap<ObjectID, SharedPtr<ManagedObject>> m_ObjectMap;
};



ANIM_NAMESPACE_END
