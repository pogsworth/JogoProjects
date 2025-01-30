#pragma once
#include "int_types.h"

struct Arena
{
	size_t Size;
	u8* BaseAddress;
	u8* CurrentLocation;
	size_t Alignment;

	void* Allocate(size_t Request)
	{
		u8* TopLimit = BaseAddress + Size;
		if (CurrentLocation + Request <= TopLimit)
		{
			u8* ReturnValue = CurrentLocation;
			CurrentLocation += (Request + Alignment - 1) & ~(Alignment-1);		// round up to multiple of 8
			return ReturnValue;
		}
		return nullptr;
	}

	void Clear()
	{
		CurrentLocation = BaseAddress;
	}
	void ReleaseMemory();
	static Arena Create(size_t ArenaSize, size_t align = 8);
	static Arena GetScratchArena(u8* memory, size_t size, size_t align = 1)
	{
		if (align == 0 || align > 8 || (align & (align-1)))
			align = 8;
		Arena stack{ size, memory, memory, align};
		return stack;
	}
};
