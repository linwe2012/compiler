#pragma once
#include <stdlib.h>

inline void* allocate(size_t size)
{
	return malloc(size);
}

inline void* deallocate(void* ptr)
{
	free(ptr);
	return NULL;
}