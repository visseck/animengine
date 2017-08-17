#include "animcore/util/namespace.h"
#include <memory>
#include <atomic>
#include <functional>
#include <type_traits>
#include "animcore/memory/default_allocator.h"


ANIM_NAMESPACE_BEGIN

namespace shared_ptr_detail
{
	struct ControlBlock
	{
		ControlBlock()
			: m_WeakCount(0)
			, m_StrongCount(0)
		{
		}
		std::atomic<uint16_t> m_WeakCount;
		std::atomic<uint16_t> m_StrongCount;
		std::function<void()> m_Deleter;
	};
}

template<typename T>
class WeakPtr;

template<typename T>
class SharedPtr
{
	template<typename U>
	friend class SharedPtr;
public:
	template<typename ...Args>
	static SharedPtr<T> MakeShared(Args... args)
	{
		uint8_t* controlBlockptr = static_cast<uint8_t*>(DefaultAllocator::Allocate(sizeof(T) + sizeof(shared_ptr_detail::ControlBlock)));
		uint8_t* dataPtr = controlBlockptr + sizeof(shared_ptr_detail::ControlBlock);
		T* data = new (dataPtr) T(std::forward<Args>(args)...);
		auto controlBlock = new (controlBlockptr) shared_ptr_detail::ControlBlock;
		return SharedPtr<T>(data, controlBlock, [](T* obj) -> void { obj->~T(); });
	}

	constexpr SharedPtr() 
		: m_Object(nullptr)
		, m_ControlBlock(nullptr)
	{}

	constexpr SharedPtr(nullptr_t)
		: SharedPtr()
	{}

	SharedPtr(T* data)
		: SharedPtr(data, [](T* dataPtr) -> void { DefaultAllocator::Destroy(dataPtr); })
	{}
	
	template<typename U>
	SharedPtr(const SharedPtr<U>& other)
		: m_ControlBlock(other.m_ControlBlock)
		, m_Object(other.m_Object)
	{
		if (m_ControlBlock)
		{
			m_ControlBlock->m_StrongCount++;
		}
	}

	template<typename U>
	SharedPtr(T* data, U && customDeleter)
	{
		if (data != nullptr)
		{
			m_ControlBlock = DefaultAllocator::Create<shared_ptr_detail::ControlBlock>();
			m_Object = data;
			m_ControlBlock->m_StrongCount++;
			m_ControlBlock->m_Deleter = [c = std::forward<U>(customDeleter), data]() -> void { c(data); };
		}
	}

	SharedPtr(SharedPtr&& other)
		: m_ControlBlock(nullptr)
		, m_Object(nullptr)
	{
		std::swap(m_ControlBlock, other.m_ControlBlock);
		std::swap(m_Object, other.m_Object);
	}
	
	SharedPtr& operator=(const SharedPtr& other)
	{
		Release();
		m_ControlBlock = other.m_ControlBlock;
		m_Object = other.m_Object;
		if (m_ControlBlock != nullptr)
		{
			m_ControlBlock->m_StrongCount++;
		}
		return *this;
	}

	template<typename U>
	SharedPtr<T>& operator=(const SharedPtr<U>& other)
	{
		Release();
		m_Object = other.m_Object;
		m_ControlBlock = other.m_ControlBlock;		
		if (m_ControlBlock != nullptr)
		{
			m_ControlBlock->m_StrongCount++;
		}
		return *this;
	}

	template<typename U>
	SharedPtr<T>& operator=(SharedPtr<U>&& other)
	{
		Release();
		m_Object = other.m_Object;
		other.m_Object = nullptr;
		std::swap(m_ControlBlock, other.m_ControlBlock);
		return *this;
	}

	T* operator->()
	{
		return m_Object;
	}

	const T* operator->() const
	{
		return m_Object;
	}

	T* Get()
	{
		return m_Object;
	}

	const T* Get() const
	{
		return m_Object;
	}

	~SharedPtr()
	{
		Release();
	}

	void Release()
	{
		if (m_ControlBlock != nullptr)
		{
			if (std::atomic_fetch_sub<uint16_t>(&m_ControlBlock->m_StrongCount, 1) == 1)
			{
				m_ControlBlock->m_Deleter();
				if (m_ControlBlock->m_WeakCount == 0)
				{
					DefaultAllocator::Destroy(m_ControlBlock);
				}
			}
			m_ControlBlock = nullptr;
		}
		m_Object = nullptr;
	}

	WeakPtr<T> GetWeakPtr() const
	{
		return WeakPtr<T>(*this);
	}
private:
	template<typename U>
	SharedPtr(T* data, shared_ptr_detail::ControlBlock* block, U&& customDeleter)
		: m_ControlBlock(block)
		, m_Object(data)
	{
		m_ControlBlock->m_StrongCount++;
		m_ControlBlock->m_Deleter = [c = std::forward<U>(customDeleter), data]() -> void { c(data); };
	}
	explicit SharedPtr(const WeakPtr<T>& ptr)
		: m_ControlBlock(ptr.m_ControlBlock)
	{
		if (m_ControlBlock != nullptr)
		{
			if (std::atomic_fetch_add<uint16_t>(&m_ControlBlock->m_StrongCount, 1) == 0)
			{
				m_ControlBlock = nullptr;
			}
		}
	}

	shared_ptr_detail::ControlBlock* m_ControlBlock;
	T* m_Object;
	friend class WeakPtr<T>;
};

template<typename T, typename U>
bool operator==(const SharedPtr<T>& a, const SharedPtr<U>& b)
{
	return a.Get() == b.Get();
}

template<typename T, typename U>
bool operator!=(const SharedPtr<T>& a, const SharedPtr<U>& b)
{
	return a.Get() != b.Get();
}

template<typename T>
class WeakPtr
{
	template<typename U>
	friend class WeakPtr;
public:
	WeakPtr() {}
	WeakPtr(const WeakPtr& other)
		: m_ControlBlock(other.m_ControlBlock)
		, m_Object(other.m_Object)
	{
		if (m_ControlBlock != nullptr)
		{
			m_ControlBlock->m_WeakCount++;
		}
	}

	WeakPtr(WeakPtr&& other)
		: m_ControlBlock(nullptr)
		, m_Object(nullptr)
	{
		std::swap(m_ControlBlock, other.m_ControlBlock);
		std::swap(m_Object, other.m_Object);
	}

	template<typename U>
	WeakPtr(const WeakPtr<U>& other)
		: m_ControlBlock(other.m_ControlBlock)
		, m_Object(other.m_Object)
	{
		if (m_ControlBlock != nullptr)
		{
			m_ControlBlock->m_WeakCount++;
		}
	}

	template<typename U>
	WeakPtr(WeakPtr<U>&& other)
		: m_ControlBlock(nullptr)
		, m_Object(nullptr)
	{
		std::swap(m_ControlBlock, other.m_ControlBlock);
		m_Object = other.m_Object;
		other.m_Object = nullptr;
	}

	WeakPtr& operator=(const WeakPtr& other)
	{
		Release();
		m_ControlBlock = other.m_ControlBlock;
		m_Object = other.m_Object;
		if (m_ControlBlock != nullptr)
		{
			m_ControlBlock->m_WeakCount++;
		}
		return *this;
	}

	template<typename U>
	WeakPtr<T>& operator=(const WeakPtr<U>& other)
	{
		Release();
		m_ControlBlock = other.m_ControlBlock;
		m_Object = other.m_Object;
		if (m_ControlBlock != nullptr)
		{
			m_ControlBlock->m_WeakCount++;
		}
		return *this;
	}

	template<typename U>
	WeakPtr<T>& operator=(WeakPtr<U>&& other)
	{
		Release();
		std::swap(m_ControlBlock, other.m_ControlBlock);
		m_Object = other.m_Object;
		other.m_Object = nullptr;
		return *this;
	}

	WeakPtr operator=(WeakPtr&& other)
	{
		Release();
		std::swap(m_ControlBlock, other.m_ControlBlock);
		std::swap(m_Object, other.m_Object);
		return *this;
	}

	SharedPtr<T> Lock()
	{
		return SharedPtr<T>(*this);
	}

	~WeakPtr()
	{
		Release();
	}

	void Release()
	{
		if (m_ControlBlock != nullptr)
		{
			if (m_ControlBlock->m_StrongCount == 0)
			{
				if (std::atomic_fetch_sub<uint16_t>(&m_ControlBlock->m_WeakCount, 1) == 1)
				{
					DefaultAllocator::Destroy(m_ControlBlock);
				}
			}
			m_ControlBlock = nullptr;
		}
	}
private:
	explicit WeakPtr(const SharedPtr<T>& sharedPtr)
	{
		m_ControlBlock = sharedPtr.m_ControlBlock;
		m_Object = sharedPtr.m_Object;
		if (m_ControlBlock != nullptr)
		{
			m_ControlBlock->m_WeakCount++;
		}
	}

	friend class SharedPtr<T>;
	T* m_Object;
	mutable shared_ptr_detail::ControlBlock* m_ControlBlock;
};

template<typename T>
class UniquePtr
{
	template<typename U>
	friend class UniquePtr;
public:
	template<typename ...Args>
	static UniquePtr MakeUnique(Args... args)
	{
		return UniquePtr(DefaultAllocator::Create(std::forward<Args>(args)...));
	}

	constexpr UniquePtr()
		: m_Object(nullptr)
	{
	}

	UniquePtr(T* data)
		: m_Object(data)
	{
	}

	constexpr UniquePtr(nullptr_t)
		: UniquePtr()
	{
	}

	UniquePtr(const UniquePtr&) = delete;

	UniquePtr(UniquePtr&& other)
		: m_Object(nullptr)
	{
		std::swap(m_Object, other.m_Object);
	}

	template<typename U>
	UniquePtr(UniquePtr<U>&& other)
	{
		m_Object = other.m_Object;
		other.m_Object = nullptr;
	}

	UniquePtr& operator=(UniquePtr&& other)
	{
		Reset();
		std::swap(m_Object, other.m_Object);
	}

	template<typename U>
	UniquePtr& operator=(UniquePtr<U>&& other)
	{
		Reset();
		m_Object = other.m_Object;
		other.m_Object = nullptr;
	}

	void Reset(T* data = nullptr)
	{
		if (m_Object != nullptr)
		{
			DefaultAllocator::Destroy(m_Object);			
		}		
		m_Object = data;
	}

	~UniquePtr()
	{
		Reset();
	}

	T* operator->() { return m_Object; }
	const T* operator->() const { return m_Object; }
	T* Get() { return m_Object; }
	const T* Get() { return m_Object; }

	T* Release()
	{
		auto tmp = m_Object;
		m_Object = nullptr;
		return m_Object;
	}
private:
	T* m_Object;
};


//template<typename T, typename Allocator = DefaultAllocator>
//class shared_ptr
//{	
//public:
//	constexpr shared_ptr()
//		: super()
//	{
//	}
//
//	explicit shared_ptr(T* ptr)
//		: super(ptr, DefaultAllocator::Destroy, DefaultSTDAllocator<T>{})
//	{
//	}
//};
//using shared_ptr = std::shared_ptr<T>;
//
//template<typename T>
//class unique_ptr : public std::unique_ptr<T, void(*)(void*)>
//{
//	typedef std::unique_ptr<T, void(*)(void*)> super;
//public:
//	unique_ptr()
//		: super(nullptr, DefaultAllocator::Free)
//	{
//	}
//
//	unique_ptr(T* data)
//		: super(data, DefaultAllocator::Free)
//	{
//	}
//
//	unique_ptr(unique_ptr&& other)
//		: super(std::move(other))
//	{
//	}
//};
//
//template<typename T>
//using weak_ptr = std::weak_ptr<T>;
//
//template<typename T, typename ...Args>
//unique_ptr<T> Make_Unique(Args... args)
//{
//	T* data = DefaultAllocator::Allocate<T>();
//	new (data) T(std::forward<Args>(args)...);
//	return unique_ptr<T>(data, DefaultAllocator::Free);
//}
//
//template<typename T, typename ...Args>
//shared_ptr<T> Make_Shared(Args... args)
//{
//	return std::allocate_shared(DefaultSTDAllocator{}, std::forward<Args>(args)...);
//}

ANIM_NAMESPACE_END