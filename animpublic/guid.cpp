#include "guid.h"
#define WIN32_LEAN_AND_MEAN
#include <objbase.h>

ANIM_PUBLIC_NAMESPACE_BEGIN

Guid::Guid()
{
	memset(m_Data, 0, sizeof(m_Data));
}

bool Guid::isEmpty() const
{
	for (uint32_t i = 0; i < 16; ++i)
	{
		if (m_Data[i] != 0)
			return false;
	}
	return true;
}

Guid Guid::CreateNewGuid()
{
	GUID winGuid;
	CoCreateGuid(&winGuid);
	//memcpy()
	Guid guid;
	*reinterpret_cast<unsigned long*>(guid.m_Data) = winGuid.Data1;
	*reinterpret_cast<unsigned short*>(&guid.m_Data[4]) = winGuid.Data2;
	*reinterpret_cast<unsigned short*>(&guid.m_Data[6]) = winGuid.Data3;
	memcpy(&guid.m_Data[8], winGuid.Data4, 8);
	return guid;
}

ANIM_PUBLIC_NAMESPACE_END

