#include "object_serializer.h"
#include "serialization.h"

ANIM_NAMESPACE_BEGIN

namespace Serialization
{
	namespace ImplDetails
	{
		using namespace rttr;
		bool SerializeAtomicType(const type& t, const variant& var, Serializer& res)
		{
			if (t.is_arithmetic())
			{
				if (t == type::get<bool>())
				{
					SerializeHelper<bool>::Apply(var.get_value<bool>(), res);
				}
				else if (t == type::get<char>())
				{
					SerializeHelper<char>::Apply(var.get_value<char>(), res);
				}
				else if (t == type::get<int8_t>())
				{
					SerializeHelper<int8_t>::Apply(var.get_value<int8_t>(), res);
				}
				else if (t == type::get<int16_t>())
				{
					SerializeHelper<int16_t>::Apply(var.get_value<int16_t>(), res);
				}
				else if (t == type::get<int32_t>())
				{
					SerializeHelper<int32_t>::Apply(var.get_value<int32_t>(), res);
				}
				else if (t == type::get<int64_t>())
				{
					SerializeHelper<int64_t>::Apply(var.get_value<int64_t>(), res);
				}
				else if (t == type::get<uint8_t>())
				{
					SerializeHelper<uint8_t>::Apply(var.get_value<uint8_t>(), res);
				}
				else if (t == type::get<uint16_t>())
				{
					SerializeHelper<uint16_t>::Apply(var.get_value<uint16_t>(), res);
				}
				else if (t == type::get<uint32_t>())
				{
					SerializeHelper<uint32_t>::Apply(var.get_value<uint32_t>(), res);
				}
				else if (t == type::get<uint64_t>())
				{
					SerializeHelper<uint64_t>::Apply(var.get_value<uint64_t>(), res);
				}
				else if (t == type::get<float>())
				{
					SerializeHelper<float>::Apply(var.get_value<float>(), res);
				}
				else if (t == type::get<double>())
				{
					SerializeHelper<double>::Apply(var.get_value<double>(), res);
				}
				return true;
			}
			else if (t.is_enumeration())
			{
				bool ok = false;
				auto result = var.to_string(&ok);
				SerializeHelper<decltype(result)>::Apply(result, res);
				return true;
			}
			else if (t == type::get<SimpleString>())
			{
				SerializeHelper<SimpleString>::Apply(var.get_value<SimpleString>(), res);
				return true;
			}
			return false;
		}
		void SerializeArray(const rttr::variant_array_view& view, Serializer& res);
		void SerializeAssociativeContainer(const rttr::variant_associative_view& view, Serializer& res);
		void SerializeVariant(const  rttr::variant& var, Serializer& res);
	}
}

ANIM_NAMESPACE_END