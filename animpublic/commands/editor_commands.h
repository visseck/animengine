#pragma once
#include <tuple>
#include "namespace.h"
#include "types.h"
#include "i_array_interface.h"

ANIM_PUBLIC_NAMESPACE_BEGIN

struct EditorCommandInterface
{
	typedef void(*EnumerateAnimationsCommand)(IArrayInterface<std::tuple<const char*, AnimResId>>& animations);
	EnumerateAnimationsCommand m_EnumerateAnimationsCommand;

	typedef void(*QueryAnimMetaData)(const AnimResId& animID, float& animLengthOut);
	QueryAnimMetaData m_QueryAnimMetaDataCommand;
};


ANIM_PUBLIC_NAMESPACE_END