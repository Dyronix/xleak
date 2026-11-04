#include "hooks.h"

#include <atomic>
#include <cstdlib>
#include <new>

// M0 throwaway. Replaces operator new and delete in the executable.

namespace
{
	std::atomic<long> g_new_hits{0};
	std::atomic<long> g_delete_hits{0};
}

void* operator new(std::size_t size)
{
	++g_new_hits;
	void* const block = std::malloc(size != 0 ? size : 1);
	if (block == nullptr)
	{
		throw std::bad_alloc();
	}
	return block;
}

void* operator new[](std::size_t size) { return ::operator new(size); }

void operator delete(void* pointer) noexcept
{
	if (pointer != nullptr)
	{
		++g_delete_hits;
	}
	std::free(pointer);
}

void operator delete[](void* pointer) noexcept { ::operator delete(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { ::operator delete(pointer); }
void operator delete[](void* pointer, std::size_t) noexcept { ::operator delete(pointer); }

namespace spike
{
	long get_new_hits() { return g_new_hits; }
	long get_delete_hits() { return g_delete_hits; }
}
