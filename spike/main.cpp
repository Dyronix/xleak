#include "hooks.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <dlfcn.h>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// M0 throwaway: the program under test. Exits non-zero when a required route misses the hooks.

extern "C" void* spike_linked_malloc(std::size_t size);
extern "C" void spike_linked_free(void* pointer);
extern "C" void* spike_linked_new(std::size_t size);
extern "C" void spike_linked_delete(void* pointer);

namespace
{
	int g_failures = 0;

	// Runs one route and checks that the hit counters moved.
	template <typename Body>
	void check(const char* name, Body body, bool expect_malloc = true, bool expect_free = true, bool expect_new = false)
	{
		const long malloc_before = spike::get_malloc_hits();
		const long free_before = spike::get_free_hits();
		const long new_before = spike::get_new_hits();
		body();
		const bool malloc_moved = spike::get_malloc_hits() > malloc_before;
		const bool free_moved = spike::get_free_hits() > free_before;
		const bool new_moved = spike::get_new_hits() > new_before;
		const bool passed = (!expect_malloc || malloc_moved) && (!expect_free || free_moved) && (!expect_new || new_moved);

		char line[256];
		std::snprintf(line, sizeof(line), "%s %s: malloc %s, free %s, new %s", passed ? "PASS" : "FAIL", name, malloc_moved ? "seen" : "-", free_moved ? "seen" : "-", new_moved ? "seen" : "-");
		spike::say(line);
		if (!passed)
		{
			++g_failures;
		}
	}

	struct user_global
	{
		user_global()
		{
			spike::say("[global] constructed");
			data = new std::vector<int>(100);
		}
		~user_global()
		{
			spike::say("[global] destroyed");
			delete data;
		}
		std::vector<int>* data;
	};
	user_global g_user;

	struct tls_object
	{
		tls_object() { spike::say("[thread_local] constructed"); }
		~tls_object() { spike::say("[thread_local] destroyed"); }
		std::string text = std::string(200, 'x');
	};

	__attribute__((noinline)) void planted_allocation()
	{
		spike::record_next_stack();
		void* volatile block = std::malloc(40); // planted: this line must appear in the report
		std::free(block);
	}
}

int main(int argc, char** argv)
{
	spike::say(spike::anchor != 0 ? "[main] start" : "");
	spike::say_count("argc", argc);
	if (argc > 1)
	{
		spike::say(argv[1]);
	}

	thread_local tls_object tls;
	(void)tls.text;

	check("direct malloc in exe", [] {
		void* volatile block = std::malloc(64);
		std::free(block);
	});
	check("direct calloc in exe", [] {
		void* volatile block = std::calloc(4, 16);
		std::free(block);
	});
	check("realloc in exe", [] {
		void* volatile block = std::malloc(16);
		block = std::realloc(block, 4096);
		std::free(block);
	});
	check("new in exe", [] {
		int* volatile block = new int(3);
		delete block;
	}, true, true, true);
	check("vector, make_shared, make_unique", [] {
		std::vector<int> numbers(1000);
		auto shared = std::make_shared<std::string>(300, 'y');
		auto unique = std::make_unique<int[]>(50);
		numbers[0] = unique[0];
		// Printed so the optimizer can't remove the allocations
		spike::say_count("  vector first", numbers[0]);
		spike::say_count("  shared size", static_cast<long>(shared->size()));
	}, true, true, true);
	check("strdup inside libc", [] {
		char* const copied = strdup("a string long enough to need the heap");
		std::free(copied);
	});
	check("fopen inside libc", [] {
		std::FILE* const file = std::fopen("/proc/self/exe", "rb");
		if (file != nullptr)
		{
			std::fclose(file);
		}
	});
	check("opendir inside libc", [] {
		DIR* const directory = opendir("/");
		if (directory != nullptr)
		{
			closedir(directory);
		}
	});
	check("libstdc++: ostringstream grows a string inside the .so", [] {
		std::ostringstream stream;
		for (int index = 0; index < 2000; ++index)
		{
			stream << "text ";
		}
		const std::string result = stream.str();
		spike::say_count("  stream size", static_cast<long>(result.size()));
	}, true, true, true);
	check("std::string across the boundary: append, assign, replace", [] {
		std::string text(40, 'a');
		text.append(4000, 'b');
		text.replace(0, 10, 9000, 'c');
		std::string other;
		other = text;
		spike::say_count("  string size", static_cast<long>(other.size()));
	}, true, true, true);
	check("linked .so: malloc and free", [] {
		void* volatile block = spike_linked_malloc(32);
		spike_linked_free(block);
	});
	check("linked .so: new and delete", [] {
		void* volatile block = spike_linked_new(32);
		spike_linked_delete(block);
	}, true, true, true);
	check("alloc in exe, free in .so", [] {
		void* volatile block = std::malloc(32);
		spike_linked_free(block);
	});
	check("alloc in .so, free in exe", [] {
		void* volatile block = spike_linked_malloc(32);
		std::free(block);
	});
	check("dlopen'ed module: malloc and free", [] {
		void* const module = dlopen(SPIKE_LOADED_PATH, RTLD_NOW | RTLD_LOCAL);
		if (module == nullptr)
		{
			spike::say(dlerror());
			++g_failures;
			return;
		}
		auto* const allocate = reinterpret_cast<void* (*)(std::size_t)>(dlsym(module, "spike_loaded_malloc"));
		auto* const release = reinterpret_cast<void (*)(void*)>(dlsym(module, "spike_loaded_free"));
		void* volatile block = allocate(32);
		release(block);
		dlclose(module);
	});
	check("threads", [] {
		std::vector<std::thread> workers;
		for (int index = 0; index < 8; ++index)
		{
			workers.emplace_back([] {
				for (int round = 0; round < 1000; ++round)
				{
					auto* const block = new std::string(100, 'z');
					delete block;
				}
			});
		}
		for (std::thread& worker : workers)
		{
			worker.join();
		}
	}, true, true, true);

	planted_allocation();

	spike::say_count("failures", g_failures);
	spike::say("[main] end");
	return g_failures == 0 ? 7 : 1; // 7: the exit code must come through unchanged
}
