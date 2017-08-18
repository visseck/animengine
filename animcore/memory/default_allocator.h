#pragma once
#include "animcore/util/namespace.h"
#include "animcore/interface/engine_interface.h"
#include "animpublic/commands/core_commands.h"

ANIM_NAMESPACE_BEGIN

class DefaultAllocator
{
public:
	static void Free(void* data)
	{
		if (data == nullptr)
			return;
		EngineInterface::GetCoreCommands().m_FreeFn(data);
	}

	static void* Allocate(size_t size)
	{
		return EngineInterface::GetCoreCommands().m_AllocateFn(size);
	}

	template<typename T>
	static T* Allocate(size_t numElements = 1)
	{
		return static_cast<T*>(Allocate(numElements * sizeof(T)));
	}

	template<typename T, typename ...Args>
	static T* Create(Args... args)
	{
		return new (Allocate<T>()) T(std::forward<Args>(args)...);
	}

	template<typename T>
	static void Destroy(T* data)
	{
		if (data == nullptr)
			return;
		data->~T();
		Free(data);
	}
};

template<typename T>
class DefaultAllocatorT
{
	public:
	static T* Allocate(size_t numElements = 1)
	{
		return DefaultAllocator::Allocate<T>(numElements);
	}

	static void Free(void* data)
	{
		DefaultAllocator::Free(data);
	}

};

template<typename T>
class DefaultSTDAllocator
{
public:
	typedef T value_type;
	DefaultSTDAllocator() = default;
	template<typename U>
	DefaultSTDAllocator(const DefaultSTDAllocator<U>& other) {}

	T* allocate(std::size_t n)
	{
		return DefaultAllocator::Allocate<T>(n);
	}

	void deallocate(T* p, std::size_t)
	{
		return DefaultAllocator::Free(p);
	}
};

template<typename T, typename U>
bool operator==(const DefaultSTDAllocator<T>&, const DefaultSTDAllocator<U>&) { return true; }
template<typename T, typename U>
bool operator!=(const DefaultSTDAllocator<T>&, const DefaultSTDAllocator<U>&) { return false; }

#define ANIM_NEW(type, ...) DefaultAllocator::Create<type>(__VA_ARGS__)
#define ANIM_DELETE(ptr) DefaultAllocator::Destroy(ptr)
#define ANIM_DELETE_SAFE(ptr) do { DefaultAllocator::Destroy(ptr); ptr = nullptr; } while(false);

ANIM_NAMESPACE_END

