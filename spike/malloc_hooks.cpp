#include "hooks.h"

#include <array>
#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <execinfo.h>
#include <malloc.h>

// M0 throwaway. Defines the malloc family in the executable and forwards to glibc.

namespace
{
	using malloc_fn = void* (*)(std::size_t);
	using calloc_fn = void* (*)(std::size_t, std::size_t);
	using realloc_fn = void* (*)(void*, std::size_t);
	using free_fn = void (*)(void*);

	malloc_fn g_real_malloc = nullptr;
	calloc_fn g_real_calloc = nullptr;
	realloc_fn g_real_realloc = nullptr;
	free_fn g_real_free = nullptr;

	std::atomic<long> g_malloc_hits{0};
	std::atomic<long> g_free_hits{0};
	std::atomic<long> g_arena_uses{0};
	std::atomic<bool> g_tracking{false};

	// dlsym may call calloc before the real one is known
	alignas(std::max_align_t) std::array<char, 64 * 1024> g_arena;
	std::atomic<std::size_t> g_arena_used{0};

	__thread bool t_guard __attribute__((tls_model("initial-exec"))) = false;
	__thread bool t_record_next __attribute__((tls_model("initial-exec"))) = false;

	std::array<void*, 32> g_stack;
	int g_stack_size = 0;

	void* arena_allocate(std::size_t size)
	{
		const std::size_t rounded = (size + 15u) & ~std::size_t{15u};
		const std::size_t offset = g_arena_used.fetch_add(rounded);
		++g_arena_uses;
		if (offset + rounded > g_arena.size())
		{
			std::abort();
		}
		return g_arena.data() + offset;
	}

	bool in_arena(const void* pointer)
	{
		const char* const byte = static_cast<const char*>(pointer);
		return byte >= g_arena.data() && byte < g_arena.data() + g_arena.size();
	}

	void resolve()
	{
		if (g_real_malloc != nullptr)
		{
			return;
		}
		t_guard = true;
		g_real_calloc = reinterpret_cast<calloc_fn>(dlsym(RTLD_NEXT, "calloc"));
		g_real_realloc = reinterpret_cast<realloc_fn>(dlsym(RTLD_NEXT, "realloc"));
		g_real_free = reinterpret_cast<free_fn>(dlsym(RTLD_NEXT, "free"));
		g_real_malloc = reinterpret_cast<malloc_fn>(dlsym(RTLD_NEXT, "malloc"));
		t_guard = false;
	}

	void on_allocation()
	{
		if (t_guard || !g_tracking.load(std::memory_order_relaxed))
		{
			return;
		}
		t_guard = true;
		++g_malloc_hits;
		if (t_record_next)
		{
			t_record_next = false;
			g_stack_size = backtrace(g_stack.data(), static_cast<int>(g_stack.size()));
		}
		t_guard = false;
	}
}

extern "C"
{
	void* malloc(std::size_t size) noexcept
	{
		if (g_real_malloc == nullptr)
		{
			if (t_guard)
			{
				return arena_allocate(size);
			}
			resolve();
		}
		void* const result = g_real_malloc(size);
		on_allocation();
		return result;
	}

	void* calloc(std::size_t count, std::size_t size) noexcept
	{
		if (g_real_calloc == nullptr)
		{
			if (t_guard)
			{
				void* const block = arena_allocate(count * size);
				std::memset(block, 0, count * size);
				return block;
			}
			resolve();
		}
		void* const result = g_real_calloc(count, size);
		on_allocation();
		return result;
	}

	void* realloc(void* pointer, std::size_t size) noexcept
	{
		if (g_real_realloc == nullptr)
		{
			if (t_guard)
			{
				return arena_allocate(size);
			}
			resolve();
		}
		if (in_arena(pointer))
		{
			void* const moved = g_real_malloc(size);
			std::memcpy(moved, pointer, size);
			return moved;
		}
		void* const result = g_real_realloc(pointer, size);
		on_allocation();
		return result;
	}

	void free(void* pointer) noexcept
	{
		if (pointer == nullptr || in_arena(pointer))
		{
			return;
		}
		if (g_real_free == nullptr)
		{
			resolve();
		}
		// The manual asks a replacement free to keep errno
		const int saved_errno = errno;
		if (!t_guard && g_tracking.load(std::memory_order_relaxed))
		{
			++g_free_hits;
		}
		g_real_free(pointer);
		errno = saved_errno;
	}
}

namespace spike
{
	long get_malloc_hits() { return g_malloc_hits; }
	long get_free_hits() { return g_free_hits; }
	long get_arena_uses() { return g_arena_uses; }

	void mark_tracking_on()
	{
		// Loads libgcc now, outside any hook, as backtrace(3) warns it allocates on first use
		std::array<void*, 4> frames;
		backtrace(frames.data(), static_cast<int>(frames.size()));
		g_tracking = true;
	}

	void record_next_stack() { t_record_next = true; }

	int get_recorded_stack(void** frames, int capacity)
	{
		const int count = g_stack_size < capacity ? g_stack_size : capacity;
		std::memcpy(frames, g_stack.data(), static_cast<std::size_t>(count) * sizeof(void*));
		return count;
	}
}
