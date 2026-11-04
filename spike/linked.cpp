#include <cstdlib>
#include <string>

// A shared library the exe links: its own malloc and new must reach the exe's hooks.
extern "C" __attribute__((visibility("default"))) void* spike_linked_malloc(std::size_t size) { return std::malloc(size); }
extern "C" __attribute__((visibility("default"))) void spike_linked_free(void* pointer) { std::free(pointer); }
extern "C" __attribute__((visibility("default"))) void* spike_linked_new(std::size_t size) { return ::operator new(size); }
extern "C" __attribute__((visibility("default"))) void spike_linked_delete(void* pointer) { ::operator delete(pointer); }
