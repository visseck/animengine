#include "core_commands_integration.h"
#include <malloc.h>


void* Allocate(size_t size)
{
	return malloc(size);
}

void Free(void* mem)
{
	free(mem);
}
