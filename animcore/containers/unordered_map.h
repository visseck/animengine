#pragma once
#include "animcore/util/namespace.h"
#include "animcore/memory/default_allocator.h"
#include <unordered_map>

ANIM_NAMESPACE_BEGIN

template<
	typename Key,
	typename T,
	typename Hash = std::hash<Key>,
	typename KeyEqual = std::equal_to<Key>>
	using UnorderedMap = std::unordered_map<Key, T, Hash, KeyEqual, DefaultSTDAllocator<std::pair<const Key, T>>>;

ANIM_NAMESPACE_END