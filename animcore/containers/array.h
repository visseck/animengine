#pragma once
#include <stdint.h>
#include <type_traits>
#include <string.h>
#include <limits>
#include "animcore/util/assert.h"
#include "animcore/memory/default_allocator.h"
#include "animcore/math/utils.h"

ANIM_NAMESPACE_BEGIN

template<typename CountType, typename ObjectType, typename Allocator, bool = std::is_copy_constructible<ObjectType>::value>
class BaseArray
{
private:
	class Iterator_Base
	{
	public:
		bool operator==(const Iterator_Base& other) const
		{
			return m_Array == other.m_Array && m_Index == other.m_Index;
		}

		bool operator!=(const Iterator_Base& other) const
		{
			return !(*this == other);
		}

		bool operator<(const Iterator_Base& other) const
		{
			return (m_Array + m_Index) < (other.m_Array + other.m_Index);
		}

		bool operator>(const Iterator_Base& other) const
		{
			return (m_Array + m_Index) > (other.m_Array + other.m_Index);
		}

		bool operator <=(const Iterator_Base& other) const { return !(*this > other); }
		bool operator >=(const Iterator_Base& other) const { return !(*this < other); }

		void operator++()
		{
			++m_Index;
		}

		void operator--()
		{
			--m_Index;
		}
	protected:
		BaseArray* m_Array;
		uint32_t m_Index;
	};
public:
	class Iterator : public Iterator_Base
	{
		typedef Iterator_Base super;
	public:
		Iterator()
		{
			super::m_Index = 0;
			super::m_Array = nullptr;
		}

		Iterator(BaseArray* pArray, uint32_t index)
		{
			super::m_Index = index;
			super::m_Array = pArray;
		}

		ObjectType& operator->()
		{
			ANIM_ASSERT(super::m_Array);
			return (*super::m_Array)[super::m_Index];
		}

		ObjectType& operator*()
		{
			ANIM_ASSERT(super::m_Array);
			return (*super::m_Array)[super::m_Index];
		}
	};

	class ConstIterator : public Iterator_Base
	{
		typedef Iterator_Base super;
	public:
		ConstIterator()
		{
			super::m_Index = 0;
			super::m_Array = nullptr;
		}

		ConstIterator(const BaseArray* pArray, uint32_t index)
		{
			super::m_Index = index;
			super::m_Array = const_cast<BaseArray*>(pArray);
		}

		const ObjectType& operator->() const
		{
			ANIM_ASSERT(super::m_Array);
			return super::m_Array[super::m_Index];
		}

		const ObjectType& operator*() const
		{
			ANIM_ASSERT(super::m_Array);
			return (*super::m_Array)[super::m_Index];
		}
	};

	BaseArray()
		: m_Capacity(0)
		, m_Size(0)
		, m_Data(nullptr)
		, m_OwnsData(true)
		, m_PreserveOrder(false)
	{
	}
	BaseArray(uint32_t capacity)
		: BaseArray()
	{
		Reallocate(capacity);
	}
	BaseArray(const BaseArray& other)
		: BaseArray()
	{
		Reallocate(other.m_Size);
		CopyFrom(other.m_Data, other.m_Size);
		m_Size = other.m_Size;
		m_PreserveOrder = other.m_PreserveOrder;
	}

	BaseArray(BaseArray&& other)
		: BaseArray()
	{
		if (other.m_OwnsData)
		{
			std::swap(m_Data, other.m_Data);
			std::swap(m_Capacity, other.m_Capacity);
			std::swap(m_Size, other.m_Size);
			std::swap(m_PreserveOrder, other.m_PreserveOrder);
		}
		else
		{
			Reallocate(other.m_Size);
			CopyFrom(other.m_Data, other.m_Size);
			m_PreserveOrder = other.m_PreserveOrder;
		}
	}

	BaseArray(ObjectType* data, uint32_t numElements, uint32_t maxCapacity, bool ownsData)
		: BaseArray()
	{
		m_Data = data;
		m_Size = numElements;
		m_Capacity = maxCapacity;
		m_OwnsData = ownsData;
	}

	virtual ~BaseArray()
	{
		Clear();
		if (m_OwnsData)
			Allocator::Free(m_Data);
		m_Data = nullptr;
	}

	BaseArray& operator=(const BaseArray& other)
	{
		Clear();
		Reserve(other.m_Size);
		CopyFrom(other.m_Data, other.m_Size);
		return *this;
	}

	BaseArray& operator=(BaseArray&& other)
	{
		Clear();
		if (other.m_OwnsData)
		{
			std::swap(m_Flags, other.m_Flags);
			std::swap(m_Data, other.m_Data);
			std::swap(m_Capacity, other.m_Capacity);
			std::swap(m_Size, other.m_Size);
		}
		else
		{
			Reserve(other.m_Size);
			for (uint32_t i = 0; i < other.m_Size; ++i)
			{
				new (&m_Data[i]) ObjectType(std::move(other.m_Data[i]));
			}
		}
		return *this;
	}

	Iterator begin() { return Iterator(this, 0); }
	Iterator end() { return Iterator(this, m_Size); }
	ConstIterator begin() const { return ConstIterator(this, 0); }
	ConstIterator end() const { return ConstIterator(this, m_Size); }

	uint32_t Size() const { return m_Size; }
	uint32_t Capacity() const { return m_Capacity; }
	void Reserve(uint32_t capacity)
	{
		if (m_Capacity < capacity)
			Reallocate(capacity);
	}
	template<typename U = ObjectType>
	typename std::enable_if<std::is_same<U, ObjectType>::value && std::is_pod<U>::value, void>::type 
		Clear() { m_Size = 0; }

	template<typename U = ObjectType>
	typename std::enable_if<std::is_same<U, ObjectType>::value && !std::is_pod<U>::value, void>::type 
		Clear()
	{
		for (uint32_t i = 0; i < m_Size; ++i)
		{
			auto& obj = m_Data[i];
			obj.~ObjectType();
		}
		m_Size = 0;
	}

	template<typename U = ObjectType>
	typename std::enable_if<!std::is_pod<U>::value, void>::type Resize(uint32_t newSize)
	{
		if (newSize < m_Size)
		{
			for (uint32_t i = newSize; i < m_Size; ++i)
			{
				auto& obj = m_Data[i];
				obj.~ObjectType();
			}
		}
		else if (newSize > m_Size)
		{
			auto oldSize = m_Size;
			Reserve(newSize);
			for (uint32_t i = oldSize; i < newSize; ++i)
			{
				auto& obj = m_Data[i];
				new (&obj) ObjectType();
			}
		}
	}

	template<typename U = ObjectType>
	typename std::enable_if<std::is_pod<U>::value, void>::type Resize(uint32_t newSize)
	{
		Reallocate(newSize);
		m_Size = newSize;
	}

	ObjectType& operator[](uint32_t index) { ANIM_ASSERT(index < m_Size); return m_Data[index]; }
	const ObjectType& operator[](uint32_t index) const { ANIM_ASSERT(index < m_Size); return m_Data[index]; }
	const ObjectType* GetBuffer() const { return m_Data; }
	ObjectType* GetBuffer() { return m_Data; }

	template<typename U = ObjectType>
	typename std::enable_if<std::is_pod<U>::value, void>::type Push(const ObjectType& object)
	{
		if (m_Size + 1 > m_Capacity)
			Reallocate(m_Size + 1);
		memcpy(&m_Data[m_Size], &object, sizeof(ObjectType));
		++m_Size;
	}

	template<typename U = ObjectType>
	typename std::enable_if<!std::is_pod<U>::value, void>::type Push(const ObjectType& object)
	{
		if (m_Size + 1 > m_Capacity)
			Reallocate(m_Size + 1);
		new (&m_Data[m_Size]) ObjectType(object);
	}

	template<typename U = ObjectType>
	typename std::enable_if<!std::is_pod<U>::value, void>::type Push(ObjectType&& object)
	{
		if (m_Size + 1 > m_Capacity)
			Reallocate(m_Size + 1);
		new (&m_Data[m_Size]) ObjectType(std::move(object));
	}

	template<typename U = ObjectType>
	typename std::enable_if<std::is_pod<U>::value, void>::type Pop()
	{
		ANIM_ASSERT(m_Size > 0);
		--m_Size;
	}

	template<typename U = ObjectType>
	typename std::enable_if<!std::is_pod<U>::value, void>::type Pop()
	{
		ANIM_ASSERT(m_Size > 0);
		m_Data[m_Size - 1].~ObjectType();
		--m_Size;
	}

	ObjectType& First()
	{
		ANIM_ASSERT(m_Size > 0);
		return m_Data[0];
	}

	const ObjectType First() const
	{
		ANIM_ASSERT(m_Size > 0);
		return m_Data[0];
	}

	ObjectType& Last()
	{
		ANIM_ASSERT(m_Size > 0);
		return m_Data[m_Size - 1];
	}

	const ObjectType& Last() const
	{
		ANIM_ASSERT(m_Size > 0);
		return m_Data[m_Size - 1];
	}

	ObjectType& Grow()
	{
		if (m_Size + 1 > m_Capacity)
			Reallocate(m_Size + 1);
		new (&m_Data[m_Size]) ObjectType();
		return m_Data[m_Size++];
	}

	void RemoveAt(uint32_t index)
	{
		ANIM_ASSERT(index < m_Size);
		if (!m_PreserveOrder)
		{
			std::swap(m_Data[index], m_Data[--m_Size]);
		}
		else
		{
			m_Data[index].~ObjectType();
			for (uint32_t i = index; i < m_Size - 1; ++i)
			{
				m_Data[index] = std::move(m_Data[index + 1]);
			}
			--m_Size;
		}
	}

private:
	void Reallocate(uint32_t newSize)
	{
		ANIM_ASSERT(newSize < std::numeric_limits<CountType>::max());
		auto newData = Allocator::Allocate(newSize);// : nullptr;
		if (m_Data != nullptr)
		{
			if (newData != nullptr)
				memcpy(newData, m_Data, (MAX(m_Size, newSize) * sizeof(ObjectType)));
			if (m_OwnsData)
				Allocator::Free(m_Data);
		}
		m_Data = newData;
		m_OwnsData = 1;
		m_Capacity = newSize;
	}

	template<typename U = ObjectType>
	typename std::enable_if<std::is_same<U, ObjectType>::value && std::is_pod<U>::value && std::is_copy_constructible<U>::value, void>::type CopyFrom(const U* values, uint32_t numItems)
	{
		memcpy(m_Data, values, numItems * sizeof(U));
	}

	template<typename U = ObjectType>
	typename std::enable_if<std::is_same<U, ObjectType>::value && !std::is_pod<U>::value && std::is_copy_constructible<U>::value, void>::type CopyFrom(const U* values, uint32_t numItems)
	{
		for (uint32_t i = 0; i < numItems; ++i)
		{
			new (&m_Data[i]) U(values[i]);
		}
	}

	ObjectType* m_Data;
	CountType m_Size;
	CountType m_Capacity;
	union
	{
		uint8_t m_Flags;
		struct
		{
			uint8_t m_OwnsData : 1;
			uint8_t m_PreserveOrder : 1;
		};
	};
};

template<typename CountType, typename ObjectType, typename Allocator>
class BaseArray<CountType, ObjectType, Allocator, false> : public BaseArray<CountType, ObjectType, Allocator, true>
{
	using BaseArray<CountType, ObjectType, Allocator, true>::BaseArray;
public:
	BaseArray() = default;
	BaseArray(const BaseArray&) = delete;
	BaseArray(BaseArray&&) = default;
	BaseArray& operator=(const BaseArray&) = delete;
	BaseArray& operator=(BaseArray&&) = default;
};

template<typename ObjectType, typename Allocator = DefaultAllocatorT<ObjectType>>
using Array = BaseArray<uint16_t, ObjectType, Allocator>;

template<typename ObjectType, typename Allocator = DefaultAllocatorT<ObjectType>>
using BigArray = BaseArray<uint32_t, ObjectType, Allocator>;

template<typename ObjectType, uint32_t FIXED_SIZE, typename Allocator = DefaultAllocatorT<ObjectType>>
class InplaceArray : public BaseArray<uint16_t, ObjectType, Allocator>
{
	typedef BaseArray<uint16_t, ObjectType, Allocator> super;
public:
	InplaceArray() : super(reinterpret_cast<float*>(m_FixedBuffer), 0, FIXED_SIZE, false) {}
	InplaceArray(const InplaceArray&) = default;
	InplaceArray(InplaceArray&& other) = default;
	~InplaceArray() = default;
	InplaceArray& operator=(const InplaceArray&) = default;
	InplaceArray& operator=(InplaceArray&& other) = default;
private:
	uint8_t m_FixedBuffer[sizeof(ObjectType)*FIXED_SIZE];
};

ANIM_NAMESPACE_END