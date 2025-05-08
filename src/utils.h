#pragma once
#include <cstddef>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>

#ifndef FREE_ARR
#define FREE_ARR(x) if (x) { delete[] x; x = nullptr; }
#define FREE_OBJ(x) x.free();
#define FREE_PTR(x) if (x) { x->free(); delete x; x = nullptr; }
#define FREE_VEC(x) x.clear();
#define FREE_OBJ_VEC(x) for (auto& i : x) { i.free(); } x.clear();
#define FREE_PTR_VEC(x) for (auto& i : x) { i->free(); delete i; } x.clear();
#endif

#ifndef sizeOfArray
#define sizeOfArray(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

template <typename T> T memread(char** mem) {
	T result = **((T**)mem);
	*mem = *mem + sizeof(T);
	return result;
}

template <typename T> T memread(T _, char** mem) {
	T result = **((T**)mem);
	*mem = *mem + sizeof(T);
	return result;
}
template <typename T> void memread(T* dst, unsigned int n, char** mem) {
	memmove((char*)dst, *mem, n);
	*mem = *mem + n;
}

#ifndef BYTE
#define BYTE(x) ((uint8_t)((x) & 0xFF))
#endif

// fwrite2 is an fwrite wrapper that swaps endiannes before writing.
// N64 uses big endian while x86_64 uses little endian.
void fwrite2inner(void* ptr, size_t size, size_t count, FILE *file);
template <typename T> void fwrite2(T* ptr, size_t size, size_t count, FILE *file) {
    fwrite2inner((void*)ptr, size, count, file);
}


void replace_end(const char* name, const char* new_ext, const char* old_ext);

void strlwr(const char* s);
int strnicmp(const char* s1, const char* s2, std::size_t n);
int strcmpi(const char* s1, const char* s2);

const char *to_owned(const std::string &str);
bool endswith(const char* str, const char* suffix);
int pack_extension(const std::string& filename);
constexpr int packconst(const char* ext) {
    return ((ext[0] & 0xFF) << 24) | ((ext[1] & 0xFF) << 16) | ((ext[2] & 0xFF) << 8) | (ext[3] & 0xFF);
}