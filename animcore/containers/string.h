#pragma once
#include "animcore/util/namespace.h"
#include "animcore/memory/default_allocator.h"
#include <string>

ANIM_NAMESPACE_BEGIN

using SimpleString = std::basic_string<char, std::char_traits<char>, DefaultSTDAllocator<char>>;

ANIM_NAMESPACE_END