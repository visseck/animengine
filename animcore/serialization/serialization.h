#pragma once
#include <stdint.h>
#include <type_traits>
#include <unordered_map>
#include <functional>
#include "animcore/util/namespace.h"
#include "animcore/containers/array.h"
#include "animcore/containers/string.h"
#include "animcore/memory/pointers.h"
#include "animcore/objectmodel/object_id.h"
#include "animcore/objectmodel/reference.h"
#include "animcore/objectmodel/object_manager.h"
#include "animcore/serialization/reflection.h"


ANIM_NAMESPACE_BEGIN

namespace Serialization
{
	class Serializer;
	class Deserializer;
	static constexpr uint32_t SerializationFormatVersion = 1;
	
	class IReadStream
	{
	public:
		virtual void Read(void * dst, uint32_t numBytes) = 0;
		virtual void Reset() = 0;
		virtual uint32_t GetNumBytesRead() const = 0;
	};

	class IWriteStream
	{
	public:
		virtual void Write(const void * data, uint32_t numBytes) = 0;
		virtual void Reserve(uint32_t numBytes) = 0;
		virtual void Reset() = 0;
		virtual uint32_t GetNumBytesWritten() const = 0;
	};

	namespace ImplDetails
	{
		class SizeAccumulator : public IWriteStream
		{
		public:
			SizeAccumulator() : m_CurSize(0) {}
			virtual void Write(const void *, uint32_t numBytes) override { m_CurSize += numBytes; }
			virtual void Reserve(uint32_t) override {}
			virtual void Reset() override { m_CurSize = 0; }
			virtual uint32_t GetNumBytesWritten() const override { return m_CurSize; }
		private:
			uint32_t m_CurSize;
		};
	}


	namespace ImplDetails
	{
		template<typename T, class Enable = void>
		struct SerializeHelper;
		template<typename T, class Enable = void>
		struct DeserializeHelper;
	}

	class Serializer
	{
	public:
		Serializer(IWriteStream & stream, uint32_t version)
			: m_Stream(stream), m_RecursiveCount(0), m_Version(version)
		{
			m_Stream.Write(
				reinterpret_cast<const uint8_t *>(&SerializationFormatVersion), 
				sizeof(SerializationFormatVersion));
			m_Stream.Write(
				reinterpret_cast<uint8_t *>(&version), 
				sizeof(version));
		}
		Serializer(const Serializer &) = delete;
		Serializer & operator=(const Serializer &) = delete;

		template <typename T>
		typename std::enable_if<!std::is_pointer<T>::value, void>::type Serialize(const T & obj);

		template <typename T>
		void Serialize(T * obj);

		inline void Serialize(const void * src, uint32_t numBytes);
		uint32_t GetVersion() const { return m_Version; }
	private:
		uint32_t m_Version;
		template <typename T>
		inline typename std::enable_if<!std::is_pointer<T>::value, uint32_t>::type GetSize(const T & obj) const;
		template <typename T>
		inline uint32_t GetSize(T * obj) const;

	private:
		template <typename T, class Enable>
		friend struct ImplDetails::SerializeHelper;
		IWriteStream & m_Stream;
		uint16_t m_RecursiveCount;
	};

	class Deserializer
	{
	public:
		Deserializer(IReadStream & stream) 
			: m_Stream(stream)
			, m_IsValid(false)
		{
			uint32_t serializationVersion;
			m_Stream.Read(reinterpret_cast<uint8_t *>(&serializationVersion), sizeof(serializationVersion));
			if (serializationVersion == SerializationFormatVersion)
			{
				m_IsValid = true;
				m_Stream.Read(reinterpret_cast<uint8_t *>(&m_Version), sizeof(m_Version));
			}
		}
		Deserializer(const Deserializer &) = delete;
		Deserializer & operator=(const Deserializer &) = delete;


		inline bool IsValid() const { return m_IsValid; }
		template <typename T>
		inline void Deserialize(T & obj);
		inline void Deserialize(void * dst, uint32_t numBytes);

		uint32_t GetDataVersion() const { return m_Version; }
	private:
		template <typename T>
		inline T * FactoryObject();

		IReadStream & m_Stream;
		template <typename T, class Enable>
		friend struct ImplDetails::DeserializeHelper;
		uint32_t m_Version;
		bool m_IsValid;
	};

	namespace ImplDetails
	{
#pragma region defaultfail
		template <typename T, class Enable>
		struct SerializeHelper
		{
			static void Apply(const T & obj, Serializer & res)
			{
				static_assert(std::is_same<T, T>::value, "TRYING TO SERIALIZE AN OBJECT OF UNKNOWN TYPE(DERIVE FROM Object)");
			}
		};
		template <typename T, typename Enable>
		struct DeserializeHelper
		{
			static void Apply(Deserializer & res, T & obj)
			{
				static_assert(std::is_same<T, T>::value, "TRYING TO SERIALIZE AN OBJECT OF UNKNOWN TYPE(DERIVE FROM Object)");
			}
		};
#pragma endregion defaultfail

#pragma region integraltypes
		template <typename T>
		struct SerializeHelper<T,
			typename std::enable_if<(std::is_integral<T>::value || std::is_floating_point<T>::value) &&
			!std::is_pointer<T>::value>::type>
		{
			static void Apply(const T & obj, Serializer & res)
			{
				res.Serialize(&obj, sizeof(T));
			}
		};
		template <typename T>
		struct DeserializeHelper<T,
			typename std::enable_if<(std::is_integral<T>::value || std::is_floating_point<T>::value) &&
			!std::is_pointer<T>::value>::type>
		{
			static void Apply(Deserializer & res, T & obj) { res.Deserialize(reinterpret_cast<uint8_t *>(&obj), sizeof(T)); }
		};
#pragma endregion integraltypes

#pragma region enumtypes
		template <typename T>
		struct SerializeHelper<T, typename std::enable_if<std::is_enum<T>::value>::type>
		{
			static void Apply(const T & obj, Serializer & res)
			{
				uint64_t value = static_cast<uint64_t>(obj);
				SerializeHelper<uint64_t>::Apply(value, res);
			}
		};

		template <typename T>
		struct DeserializeHelper<T, typename std::enable_if<std::is_enum<T>::value>::type>
		{
			static void Apply(Deserializer & res, T & obj)
			{
				uint64_t value;
				DeserializeHelper<uint64_t>::Apply(res, value);
				obj = static_cast<T>(value);
			}
		};
#pragma endregion enumtypes

#pragma region Array
		template<typename CountType, typename T, typename Allocator>
		struct SerializeHelper<BaseArray<CountType, T, Allocator>>
		{
			static void Apply(const BaseArray<CountType, T, Allocator>& obj, Serializer& res)
			{
				SerializeHelper<CountType>::Apply(obj.Size(), res);
				for (const auto& cur : obj)
				{
					SerializeHelper<T>::Apply(cur, res);
				}
			}
		};

		template<typename CountType, typename T, typename Allocator>
		struct DeserializeHelper<BaseArray<CountType, T, Allocator>>
		{
			static void Apply(Deserializer & res, BaseArray<CountType, T, Allocator> & obj)
			{
				ANIM_ASSERT(obj.Size() == 0);
				CountType numItems;
				DeserializeHelper<CountType>::Apply(res, numItems);
				obj.Resize(numItems);
				for (CountType i = 0; i < numItems; ++i)
				{
					DeserializeHelper<T>::Apply(res, obj[i]);
				}
			}
		};
#pragma endregion Array

#pragma region voidstar
		template <>
		struct SerializeHelper<void *>
		{
			static void Apply(const void *& obj, Serializer & res)
			{
				uint64_t addr = (uint64_t)obj;
				res.Serialize(addr);
			}
		};
		template <>
		struct DeserializeHelper<void *>
		{
			static void Apply(Deserializer & res, void *& obj)
			{
				uint64_t addr = 0;
				DeserializeHelper<uint64_t>::Apply(res, addr);
				obj = (void *)addr;
			}
		};
#pragma endregion voidstar

#pragma region ISerializable
		template<typename T>
		struct SerializeHelper<T, typename std::enable_if<std::is_base_of<ISerializable, T>::value>::type>
		{
			static void Apply(const ISerializable& obj, Serializer& res)
			{
				obj.Serialize(res);
			}
		};

		template<typename T>
		struct DeserializeHelper<T, typename std::enable_if<std::is_base_of<ISerializable, T>::value>::type>
		{
			static void Apply(Serializer& res, ISerializable& obj)
			{
				obj.Deserialize(res);
			}
		};
#pragma endregion ISerializable

#pragma region ISerializablePtr
		template <typename T>
		struct SerializeHelper<T *, typename std::enable_if<std::is_base_of<ISerializable, T>::value>::type>
		{
			static void Apply(const ISerializable * obj, Serializer & res)
			{
				SerializeHelper<bool>::Apply(obj != nullptr, res);
				if (obj != nullptr)
				{
					const auto & classInfo = obj->GetReflectedClassInfo();
					ANIM_ASSERT(classInfo.GetTypeID() != 0);
					SerializeHelper<uint64_t>::Apply(classInfo.GetTypeID(), res);
					SerializeHelper<T>::Apply(*obj, res);
				}
			}
		};
		template <typename T>
		struct DeserializeHelper<T *, typename std::enable_if<std::is_base_of<ISerializable, T>::value>::type>
		{
			static void Apply(Deserializer & res, T *& obj)
			{
				ANIM_ASSERT(obj == nullptr);
				bool hasObject = false;
				DeserializeHelper<bool>::Apply(res, hasObject);
				if (hasObject)
				{
					uint64_t typeID = 0;
					DeserializeHelper<uint64_t>::Apply(res, typeID);
					ANIM_ASSERT(typeID != 0);
					obj = Reflection::TypeRegistry::FactoryClass<T>(typeID);
					ANIM_ASSERT(obj != nullptr);
					DeserializeHelper<T>::Apply(res, *obj);
				}
			}
		};
#pragma endregion ISerializablePtr

#pragma region TPtr
		template<typename T>
		struct SerializeHelper<T*, typename std::enable_if<!std::is_base_of<ISerializable, T>::value>::type>
		{
			static void Apply(const T* obj, Serializer& res)
			{
				SerializeHelper<bool>::Apply(obj != nullptr, res);
				if (obj != nullptr)
				{
					SerializeHelper<T>::Apply(*obj, res);
				}
			}
		};

		template<typename T>
		struct DeserializeHelper<T*, typename std::enable_if<!std::is_base_of<ISerializable, T>::value>::type>
		{
			static void Apply(Deserializer& res, T*& obj)
			{
				ANIM_ASSERT(obj == nullptr);
				bool hasObject;
				DeserializeHelper<bool>::Apply(res, hasObject);
				if (hasObject)
				{
					obj = res.FactoryObject<T>();
					ANIM_ASSERT(obj != nullptr);
					DeserializeHelper<T>::Apply(res, *obj);
				}
			}
		};
#pragma endregion TPtr

#pragma region SimpleString
		template<>
		struct SerializeHelper<SimpleString>
		{
			static void Apply(const SimpleString & obj, Serializer & res)
			{
				uint32_t strLen = static_cast<uint32_t>(obj.length());
				SerializeHelper<uint32_t>::Apply(strLen, res);
				res.Serialize(reinterpret_cast<const uint8_t*>(obj.c_str()), strLen);
			}
		};

		template<>
		struct DeserializeHelper<SimpleString>
		{
			static void Apply(Deserializer& res, SimpleString& obj)
			{
				ANIM_ASSERT(obj.empty());
				uint32_t strLen;
				DeserializeHelper<uint32_t>::Apply(res, strLen);
				char* buffer = static_cast<char*>(alloca(strLen));
				res.Deserialize(buffer, strLen);
				obj.resize(strLen);
				char* next = buffer;
				for (uint32_t i = 0; i < strLen; ++i, ++buffer)
				{
					obj.at(i) = *buffer;
				}
			}
		};
#pragma endregion SimpleString

#pragma region UniquePtr
		template<typename T>
		struct SerializeHelper<UniquePtr<T>>
		{
			static void Apply(const UniquePtr<T> & obj, Serializer & res)
			{
				SerializeHelper<T*>::Apply(obj.get(), res);
			}
		};

		template<typename T>
		struct DeserializeHelper<UniquePtr<T>>
		{
			static void Apply(Deserializer & res, UniquePtr<T> & obj)
			{
				ANIM_ASSERT(obj == nullptr);
				T* tmp = nullptr;
				DeserializeHelper<T*>::Apply(res, tmp);
				obj.Reset(tmp);
			}
		};
#pragma endregion UniquePtr

#pragma region SharedPtr
		template<typename T>
		struct SerializeHelper<SharedPtr<T>>
		{
			static void Apply(const SharedPtr<T>& obj, Serializer& res)
			{
				static_assert(!std::is_same<T, T>::value, "Do not support serialization of SharedPtr");
			}
		};

		template<typename T>
		struct DeserializeHelper<SharedPtr<T>>
		{
			static void Apply(Deserializer& res, SharedPtr<T>& obj)
			{
				static_assert(!std::is_same<T, T>::value, "Do not support serialization of SharedPtr");
			}
		};
#pragma endregion SharedPtr

#pragma region ObjectID
		template<>
		struct SerializeHelper<ObjectID>
		{
			static void Apply(const ObjectID & obj, Serializer & res)
			{
				res.Serialize(obj.m_Data, sizeof(obj.m_Data));
			}
		};

		template<>
		struct DeserializeHelper<ObjectID>
		{
			static void Apply(Deserializer& res, ObjectID& obj)
			{
				res.Deserialize(obj.m_Data, sizeof(obj.m_Data));
			}
		};
#pragma endregion ObjectID

#pragma region Reference
		template<typename T>
		struct SerializeHelper<Reference<T>>
		{
			static void Apply(const Reference<T>& obj, Serializer& res)
			{
				SerializeHelper<ObjectID>::Apply(obj.m_ObjectID, res);
			}
		};

		template<typename T>
		struct DeserializeHelper<Reference<T>>
		{
			static void Apply(const Reference<T>& obj, Serializer& res)
			{
				DeserializeHelper<ObjectID>::Apply(res, obj.m_ObjectID);
				obj.m_ObjPtr = ObjectManager::Instance().LoadManagedObject(obj.m_ObjectID);
			}
		};
#pragma endregion Reference
	}

	template <typename T>
	inline uint32_t Serializer::GetSize(T * obj) const
	{
		ImplDetails::SizeAccumulator acc;
		Serializer res(acc, m_Version);
		res.Serialize(obj);
		return acc.GetNumBytesWritten();
	}


	template <typename T>
	inline typename std::enable_if<!std::is_pointer<T>::value, uint32_t>::type Serializer::GetSize(const T & obj) const
	{
		ImplDetails::SizeAccumulator acc;
		Serializer res(acc, m_Version);
		res.Serialize(obj);
		return acc.GetNumBytesWritten();
	}

	template <typename T>
	typename std::enable_if<!std::is_pointer<T>::value, void>::type Serializer::Serialize(const T & obj)
	{
		if (m_RecursiveCount++ == 0)
		{
			uint32_t size = GetSize(obj);
			m_Stream.Reserve(size);
		}
		ImplDetails::SerializeHelper<T>::Apply(obj, *this);
		--m_RecursiveCount;
	}

	template <typename T>
	void Serializer::Serialize(T * obj)
	{
		if (m_RecursiveCount++ == 0)
		{
			uint32_t size = GetSize<T>(obj);
			m_Stream.Reserve(size);
		}
		ImplDetails::SerializeHelper<T *>::Apply(obj, *this);
		--m_RecursiveCount;
	}

	inline void Serializer::Serialize(const void * src, uint32_t numBytes)
	{
		m_Stream.Write(src, numBytes);
	}

	template <typename T>
	inline void Deserializer::Deserialize(T & obj)
	{
		if (!m_IsValid)
			return;
		ImplDetails::DeserializeHelper<T>::Apply(*this, obj);
	}

	inline void Deserializer::Deserialize(void * dst, uint32_t numBytes)
	{
		if (!m_IsValid)
			return;
		m_Stream.Read(dst, numBytes);
	}

	template <typename T>
	inline T * Deserializer::FactoryObject()
	{
		return DefaultAllocator::Create<T>();
	}
}

ANIM_NAMESPACE_END