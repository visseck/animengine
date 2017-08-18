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


ANIM_NAMESPACE_BEGIN

namespace Serialization
{
	class Serializer;
	class Deserializer;
	static constexpr uint32_t SerializationFormatVersion = 1;

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
		template<typename T>
		void Serialize(const T& obj);
		void Serialize(uint32_t size, const void* data);
	private:
		template <typename T, class Enable>
		friend struct ImplDetails::SerializeHelper;
	};

	class Deserializer
	{
	public:
		template<typename T>
		inline T* FactoryObject();

		template<typename T>
		void Deserialize(T& obj);
		void Deserialize(void* data, uint32_t size);
	private:
		template<typename T, class Enable>
		friend struct ImplDetails::DeserializeHelper;
	};

	namespace ImplDetails
	{
#pragma region defaultfail
		template <typename T, class Enable>
		struct SerializeHelper
		{
			static void Apply(const T & obj, Serializer & res)
			{
				using namespace rttr;
				static_assert(std::is_base_of<Object, T>::value, "TRYING TO SERIALIZE AN OBJECT OF UNKNOWN TYPE(DERIVE FROM Object)");
				type objType = type::get<T>(); 
				auto props = objType.get_properties();
				for (auto& prop : props)
				{
//					prop.
				}
			}
		};
		template <typename T, typename Enable>
		struct DeserializeHelper
		{
			static void Apply(Deserializer & res, T & obj)
			{
				static_assert(std::is_base_of<Object, T>::value, "TRYING TO DESERIALIZE AN UNKNOWN OBJECT.DERIVE FROM Object");
				DeserializeHelper<Object>::Apply(obj, res);
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
				res.Serialize(sizeof(T), reinterpret_cast<const uint8_t *>(&obj));
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

#pragma region TPtr
		template<typename T>
		struct SerializeHelper<T*>
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
		struct DeserializeHelper<T*>
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
				res.Serialize(strLen, obj.c_str());

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
				res.Serialize(sizeof(obj.m_Data), obj.m_Data);
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

}

ANIM_NAMESPACE_END