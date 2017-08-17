#pragma once
#include <stdint.h>
#include "namespace.h"
#include "opaque_handle.h"
#include "guid.h"

ANIM_PUBLIC_NAMESPACE_BEGIN
namespace Detail
{
enum HandleTypes
{
	ANIMATION_HANDLE,
	DISPLACEMENT_HANDLE
};

enum ResourceIdentifierTypes
{
	ANIM_RESOURCE
};
}

using AnimHandle = OpaqueHandle<void*, ANIMATION_HANDLE>;
using AnimResId = OpaqueHandle<Guid, ANIM_RESOURCE>;
using DisplacementHandle = OpaqueHandle<void*, DISPLACEMENT_HANDLE>;

ANIM_PUBLIC_NAMESPACE_END