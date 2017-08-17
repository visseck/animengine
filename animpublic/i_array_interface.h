#pragma once
#include <stdint.h>
#include "namespace.h"

ANIM_PUBLIC_NAMESPACE_BEGIN

template<typename T>
class IArrayInterface
{
public:
	virtual size_t Size() const = 0;
	virtual void Reserve(size_t size) = 0;
	virtual void PushBack(const T& value) = 0;
	virtual void PushBack(T&& value) = 0;
	virtual const T& operator[](size_t index) = 0;
	virtual T& operator[](size_t index) = 0;
	virtual void Resize(size_t newSize) = 0;
};

ANIM_PUBLIC_NAMESPACE_END