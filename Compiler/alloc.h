#ifndef CC_ALLOCA_H
#define CC_ALLOCA_H

#include "config.h"

#ifdef CC_USE_CUSTOM_HEADER
#include "cheaders.h"
#else
#include <stdlib.h>
#endif // 

inline void* allocate(size_t size)
{
	return malloc(size);
}

inline void* deallocate(void* ptr)
{
	free(ptr);
	return NULL;
}


#endif // !CC_ALLOCA_H_




