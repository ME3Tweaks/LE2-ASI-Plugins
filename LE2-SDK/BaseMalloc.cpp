#pragma once
#include "BaseMalloc.h"

// Setup of this object must be done before using it. You should not call these directly!
void* UnrealMalloc::Malloc(int count, int alignment)
{
	if (GMalloc.appMalloc == nullptr)
	{
		CreateMalloc((void**)(&GMalloc.appMalloc));
	}
	return GMalloc.appMalloc(count, alignment, 0);
}

void UnrealMalloc::Free(void* allocation)
{
	if (GMalloc.appFree == nullptr)
	{
		CreateFree((void**)(&GMalloc.appFree));
	}
	GMalloc.appFree(allocation, 0);
}

void* UnrealMalloc::Realloc(void* original, int count, int alignment)
{
	if (GMalloc.appRealloc == nullptr)
	{
		CreateRealloc((void**)(&GMalloc.appRealloc));
	}
	return GMalloc.appRealloc(original, count, alignment, 0);
}

UnrealMalloc UnrealMalloc::GMalloc;