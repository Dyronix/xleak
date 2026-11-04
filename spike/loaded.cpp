#include <cstdlib>

// Loaded with dlopen after startup: late libraries must reach the exe's hooks too.
extern "C" __attribute__((visibility("default"))) void* spike_loaded_malloc(std::size_t size) { return std::malloc(size); }
extern "C" __attribute__((visibility("default"))) void spike_loaded_free(void* pointer) { std::free(pointer); }
