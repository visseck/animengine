#include "object_id.h"
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN
#include <objbase.h>

ANIM_NAMESPACE_BEGIN

ObjectID::ObjectID()
{
	memset(m_Data, 0, sizeof(m_Data));
}

bool ObjectID::isValid() const
{
	for (uint32_t i = 0; i < 16; ++i)
	{
		if (m_Data[i] != 0)
			return false;
	}
	return true;
}

ObjectID ObjectID::CreateNewGuid()
{
	GUID winGuid;
	CoCreateGuid(&winGuid);
	//memcpy()
	ObjectID guid;
	*reinterpret_cast<unsigned long*>(guid.m_Data) = winGuid.Data1;
	*reinterpret_cast<unsigned short*>(&guid.m_Data[4]) = winGuid.Data2;
	*reinterpret_cast<unsigned short*>(&guid.m_Data[6]) = winGuid.Data3;
	memcpy(&guid.m_Data[8], winGuid.Data4, 8);
	return guid;
}

bool ObjectID::operator==(const ObjectID& other) const
{
	return memcmp(m_Data, other.m_Data, sizeof(m_Data)) == 0;
}

bool ObjectID::operator!=(const ObjectID& other) const
{
	return !(*this == other);
}

bool ObjectID::operator>(const ObjectID& other) const
{
	return memcmp(m_Data, other.m_Data, sizeof(m_Data)) > 1;
}

bool ObjectID::operator<(const ObjectID& other) const
{
	return memcmp(m_Data, other.m_Data, sizeof(m_Data)) < 1;
}

bool ObjectID::operator>=(const ObjectID& other) const
{
	return !(*this < other);
}

bool ObjectID::operator<=(const ObjectID& other) const
{
	return !(*this > other);
}

ANIM_NAMESPACE_END

