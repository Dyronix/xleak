#include "hooks.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <link.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

// M0 throwaway. Early start, the exit report, and addr2line.

extern char** environ;

namespace
{
	std::array<void*, 32> g_copy;
	char g_exe_path[4096];

	struct base_query
	{
		std::uintptr_t base = 0;
		bool found = false;
	};

	int find_main_program(dl_phdr_info* info, std::size_t, void* data)
	{
		// The main program comes first, with an empty name
		auto* const query = static_cast<base_query*>(data);
		query->base = info->dlpi_addr;
		query->found = true;
		return 1;
	}

	void report()
	{
		spike::say("[report] atexit handler, should be after every destructor");
		spike::say_count("malloc hits", spike::get_malloc_hits());
		spike::say_count("free hits", spike::get_free_hits());
		spike::say_count("new hits", spike::get_new_hits());
		spike::say_count("delete hits", spike::get_delete_hits());
		spike::say_count("arena uses", spike::get_arena_uses());
		spike::print_recorded_stack(g_exe_path);
	}

	__attribute__((constructor(101))) void early_start()
	{
		const ssize_t length = readlink("/proc/self/exe", g_exe_path, sizeof(g_exe_path) - 1);
		g_exe_path[length > 0 ? length : 0] = '\0';
		spike::say("[constructor 101] before user globals");
		spike::mark_tracking_on();
		std::atexit(report);
	}
}

namespace spike
{
	int anchor = 1;

	void say(const char* text)
	{
		const ssize_t a = write(STDERR_FILENO, text, std::strlen(text));
		const ssize_t b = write(STDERR_FILENO, "\n", 1);
		(void)a;
		(void)b;
	}

	void say_count(const char* label, long value)
	{
		char line[128];
		std::snprintf(line, sizeof(line), "%s = %ld", label, value);
		say(line);
	}

	void print_recorded_stack(const char* executable)
	{
		base_query query;
		dl_iterate_phdr(find_main_program, &query);
		const int count = get_recorded_stack(g_copy.data(), static_cast<int>(g_copy.size()));
		say_count("recorded frames", count);

		for (int index = 0; index < count && index < 6; ++index)
		{
			Dl_info info = {};
			dladdr(g_copy[static_cast<std::size_t>(index)], &info);
			const auto address = reinterpret_cast<std::uintptr_t>(g_copy[static_cast<std::size_t>(index)]);
			const bool in_exe = info.dli_fbase != nullptr && reinterpret_cast<std::uintptr_t>(info.dli_fbase) == query.base;
			if (!in_exe && query.base != 0)
			{
				say(info.dli_fname != nullptr ? info.dli_fname : "(unknown module)");
				continue;
			}

			// One less than the return address, so the line printed is the call
			char offset[32];
			std::snprintf(offset, sizeof(offset), "0x%zx", static_cast<std::size_t>(address - query.base - 1));
			char* const arguments[] = {
				const_cast<char*>("addr2line"), const_cast<char*>("-f"), const_cast<char*>("-i"), const_cast<char*>("-C"),
				const_cast<char*>("-e"), const_cast<char*>(executable), offset, nullptr};
			pid_t child = 0;
			if (posix_spawnp(&child, "addr2line", nullptr, nullptr, arguments, environ) == 0)
			{
				int status = 0;
				waitpid(child, &status, 0);
			}
			else
			{
				say("no addr2line");
			}
		}
	}
}
