#pragma once
#include "util/namespace.h"
#include "memory/default_allocator.h"

ANIM_NAMESPACE_BEGIN

template<typename T>
class Singleton
{
public:
	Singleton() : m_Instance(nullptr) {}
	static T& Instance() { return *m_Instance; }
	template<typename ...Args>
	static void Initialize(Args... args)
	{
		m_Instance = ANIM_NEW(T, std::forward<Args>(args...));
	}

	static void Shutdown()
	{
		ANIM_DELETE_SAFE(m_Instance);
	}
private:
	static T* m_Instance;
};

ANIM_NAMESPACE_END