#pragma once
#include <stdint.h>
#include "animcore/util/namespace.h"
#include "animcore/containers/unordered_map.h"

ANIM_NAMESPACE_BEGIN

namespace Reflection
{
	class RootReflectedClass;
	constexpr uint64_t Fnv1(uint64_t h, const char * s)
	{
		return (*s == 0) ? h : Fnv1((h * 1099511628211ull) ^ static_cast<uint64_t>(*s), s + 1);
	}

	constexpr uint64_t Fnv1(const char * s)
	{
		return true ? Fnv1(14695981039346656037ull, s) : throw "Hashing at runtime!";
	}

	struct ClassInfo
	{
		typedef RootReflectedClass * (*FactoryFunction)();
		const char * m_TypeName;
		const ClassInfo * m_SuperClass;
		FactoryFunction Construct;
		inline uint64_t GetTypeID() const { return Fnv1(m_TypeName); }
		bool DerivesFrom(uint64_t typeIDOther) const
		{
			const ClassInfo * curInfo = this;
			while (curInfo != nullptr)
			{
				if (curInfo->GetTypeID() == typeIDOther)
					return true;
				curInfo = curInfo->m_SuperClass;
			}
			return false;
		}

		bool DerivesFrom(const ClassInfo & otherInfo) const { return DerivesFrom(otherInfo.GetTypeID()); }
	};

	struct RegistrationProxy
	{
		RegistrationProxy(bool value) { (void)(value); }
	};

	class RootReflectedClass
	{
	public:
		virtual const Reflection::ClassInfo & GetReflectedClassInfo() const = 0;
	};

	class TypeRegistry
	{
	public:
		static TypeRegistry & GetRegistry()
		{
			static TypeRegistry registry;
			return registry;
		}

		static bool RegisterType(const ClassInfo * info)
		{
			auto & registry = GetRegistry();
			auto iter = registry._allTypes.find(info->GetTypeID());
			if (iter == registry._allTypes.end())
			{
				registry._allTypes[info->GetTypeID()] = info;
			}
			return true;
		}

		template <typename T>
		static T * FactoryClass(uint64_t typeID)
		{
			auto classInfo = GetReflectedClassInfo(typeID);
			if (classInfo == nullptr)
				return nullptr;
			return static_cast<T *>(classInfo->Construct());
		}

		static const ClassInfo * GetReflectedClassInfo(uint64_t typeID)
		{
			auto & registryTypes = GetRegistry()._allTypes;
			auto iter = registryTypes.find(typeID);
			if (iter != registryTypes.end())
				return iter->second;
			return nullptr;
		}

	private:
		UnorderedMap<uint64_t, const ClassInfo *> _allTypes;
	};
}

ANIM_NAMESPACE_END

#define DECLARE_ROOT_CLASS()                                                                                           \
	\
public:                                                                                                                \
	virtual const Reflection::ClassInfo & GetReflectedClassInfo() const override;                                      \
	\
static inline Reflection::ClassInfo &                                                                                  \
	GetStaticClassInfo()                                                                                               \
	{                                                                                                                  \
		return s_ClassInfo;                                                                                            \
	}                                                                                                                  \
	\
protected:                                                                                                             \
	static Reflection::ClassInfo s_ClassInfo;                                                                          \
	static Reflection::RegistrationProxy s_RegistrationProxy


#define DECLARE_DERIVED_CLASS()                                                                                        \
	\
public:                                                                                                                \
	virtual const Reflection::ClassInfo & GetReflectedClassInfo() const override;                                      \
	static inline Reflection::ClassInfo & GetStaticClassInfo() { return s_ClassInfo; }                                 \
	\
protected:                                                                                                             \
	static Reflection::ClassInfo s_ClassInfo;                                                                          \
	static Reflection::RegistrationProxy s_RegistrationProxy

#define IMPLEMENT_ABSTRACT_ROOT_CLASS(a)                                                                               \
	const Reflection::ClassInfo & a## ::GetReflectedClassInfo() const { return s_ClassInfo; }                          \
	\
Reflection::ClassInfo a## ::s_ClassInfo = {#a, nullptr, nullptr};                                                      \
	\
Reflection::RegistrationProxy a## ::s_RegistrationProxy = Reflection::TypeRegistry::RegisterType(&##a## ::s_ClassInfo)

#define IMPLEMENT_CONCRETE_ROOT_CLASS(a)                                                                               \
	const Reflection::ClassInfo & a## ::GetReflectedClassInfo() const { return s_ClassInfo; }                          \
	\
Reflection::ClassInfo a## ::s_ClassInfo = {#a, nullptr, []() -> Reflection::RootReflectedClass * { return new a(); }}; \
	\
Reflection::RegistrationProxy a## ::s_RegistrationProxy = Reflection::TypeRegistry::RegisterType(&##a## ::s_ClassInfo)

#define IMPLEMENT_ABSTRACT_DERIVED_CLASS(a, b)                                                                         \
	const Reflection::ClassInfo & a## ::GetReflectedClassInfo() const { return s_ClassInfo; }                          \
	\
Reflection::ClassInfo a## ::s_ClassInfo = {#a, &b## ::s_ClassInfo, nullptr};                                           \
	\
Reflection::RegistrationProxy a## ::s_RegistrationProxy = Reflection::TypeRegistry::RegisterType(&##a## ::s_ClassInfo)

#define IMPLEMENT_CONCRETE_DERIVED_CLASS(a, b)                                                                         \
	const Reflection::ClassInfo & a## ::GetReflectedClassInfo() const { return s_ClassInfo; }                          \
	\
Reflection::ClassInfo a## ::s_ClassInfo = {                                                                            \
		#a, &b## ::s_ClassInfo, []() -> Reflection::RootReflectedClass * { return new a(); }};                         \
	\
Reflection::RegistrationProxy a## ::s_RegistrationProxy = Reflection::TypeRegistry::RegisterType(&##a## ::s_ClassInfo)
