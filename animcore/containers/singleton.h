#pragma once
#include "animcore/util/namespace.h"
#include "animcore/memory/default_allocator.h"

ANIM_NAMESPACE_BEGIN

template<typename T>
class Singleton
{
public:
	Singleton() {}
	static T& Instance() { return *m_Instance; }
	template<typename ...Args>
	static void Initialize(Args... args)
	{
		m_Instance = ANIM_NEW(T, std::forward<Args>(args)...);
	}

	static void Shutdown()
	{
		ANIM_DELETE_SAFE(m_Instance);
	}
private:
	static T* m_Instance;
};

template<typename T>
T* Singleton<T>::m_Instance = nullptr;

ANIM_NAMESPACE_END