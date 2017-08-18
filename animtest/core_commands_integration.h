#pragma once
#include <cstdio>

void* Allocate(std::size_t size);
void Free(void* mem);
