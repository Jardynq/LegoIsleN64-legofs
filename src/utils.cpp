#include "utils.h"
#include <cstdint>
#include <cstring>


void replace_end(const char* name, const char* new_ext, const char* old_ext) {
    unsigned int len = strlen(name);
    unsigned int len_new = new_ext ? strlen(new_ext) : 0;
    unsigned int len_old = old_ext ? strlen(new_ext) : 0;
    if (len < len_new || len < len_old) {
        return;
    }
    
    if (old_ext != nullptr) {
        char* head = (char*)name + len - len_old;
        if (strcmp(head, old_ext) != 0) {
            return;
        }
    }
    char* head = (char*)name + len - len_new;
    memmove(head, new_ext, len_new);
}


void fwrite2inner(void* ptr, size_t size, size_t count, FILE *file) {
	uint64_t data = 0;
    uint8_t* p = (uint8_t*)&data; 
    if (size == 2) {
		uint16_t d = *(uint16_t *)ptr;
		p[0] = BYTE(d >> 8);
		p[1] = BYTE(d >> 0);
        fwrite((uint16_t*)p, size, count, file);
	} else if (size == 4) {
		uint32_t d = *(uint32_t *)ptr;
		p[0] = BYTE(d >> 24);
		p[1] = BYTE(d >> 16);
		p[2] = BYTE(d >> 8);
		p[3] = BYTE(d >> 0);
        fwrite((uint32_t*)p, size, count, file);
	} else if (size == 8) {
		uint64_t d = *(uint64_t *)ptr;
		p[0] = BYTE(d >> 56);
		p[1] = BYTE(d >> 48);
		p[2] = BYTE(d >> 40);
		p[3] = BYTE(d >> 32);
		p[4] = BYTE(d >> 24);
		p[5] = BYTE(d >> 16);
		p[6] = BYTE(d >> 8);
		p[7] = BYTE(d >> 0);
        fwrite((uint64_t*)p, size, count, file);
	} else {
        fwrite(ptr, size, count, file);
    }
}

void strlwr(const char* s) {
    while (*s) {
        *(char*)s = std::tolower(static_cast<unsigned char>(*s));
        ++s;
    }
}

int strnicmp(const char* s1, const char* s2, std::size_t n) {
    while (n--) {
        unsigned char c1 = tolower((unsigned char)*s1++);
        unsigned char c2 = tolower((unsigned char)*s2++);
        if (c1 != c2) return c1 - c2;
        if (c1 == '\0') break;
    }
    return 0;
}

int strcmpi(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        int c1 = tolower((unsigned char)*s1++);
        int c2 = tolower((unsigned char)*s2++);
        if (c1 != c2) {
            return c1 - c2;
        }
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}


const char *to_owned(const std::string &str) {
	char *result = (char *)malloc(str.length() + 1);
	memmove(result, str.c_str(), str.length());
	result[str.length()] = 0;
	return result;
}

bool endswith(const char* str, const char* suffix) {
    if (!str || !suffix)
        return false;

    size_t lenStr = strlen(str);
    size_t lenSuffix = strlen(suffix);

    if (lenSuffix > lenStr)
        return false;

    return strcmp(str + lenStr - lenSuffix, suffix) == 0;
}

int pack_extension(const std::string& filename) {
    if (filename.size() <= 4) return 0;

    char a = std::tolower(filename[filename.size() - 4]);
    char b = std::tolower(filename[filename.size() - 3]);
    char c = std::tolower(filename[filename.size() - 2]);
    char d = std::tolower(filename[filename.size() - 1]);

    return (a << 24) | (b << 16) | (c << 8) | d;
}
