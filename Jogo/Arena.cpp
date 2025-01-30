#include "Jogo.h"
#include "Arena.h"

void Arena::ReleaseMemory()
{
	Jogo::Free(BaseAddress);
}

Arena Arena::Create(size_t ArenaSize, size_t align)
{
	Arena NewArena = { ArenaSize };

	NewArena.BaseAddress = (u8*)Jogo::Allocate(ArenaSize);
	NewArena.CurrentLocation = NewArena.BaseAddress;
	if (align == 0 || (align & (align-1)))
	{
		align = 8;
	}
	if (align > 4096)
		align = 4096;

	NewArena.Alignment = align;

	return NewArena;
}
