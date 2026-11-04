#pragma once

#include <cstddef>

// M0 throwaway. Counters read by main, and the one recorded stack.
namespace spike
{
	void say(const char* text);
	void say_count(const char* label, long value);

	long get_malloc_hits();
	long get_free_hits();
	long get_new_hits();
	long get_delete_hits();
	long get_arena_uses();

	void mark_tracking_on();

	// The next malloc on this thread records its stack
	void record_next_stack();
	int get_recorded_stack(void** frames, int capacity);

	// Run from the atexit report, so nothing else needs the hooks by then
	void print_recorded_stack(const char* executable);

	extern int anchor;
}
