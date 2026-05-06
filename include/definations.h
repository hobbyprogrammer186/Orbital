#pragma once

/*#ifdef _WIN32
#include <windows.h>
#define RTLD_LAZY 0
inline void* dlopen(const char* filename, int) { return (void*)LoadLibraryA(filename); }
inline void* dlsym(void* handle, const char* name) { return (void*)GetProcAddress((HMODULE)handle, name); }
inline int dlclose(void* handle) { return FreeLibrary((HMODULE)handle) ? 0 : -1; }
#else
#include <dlfcn.h>
#endif*/