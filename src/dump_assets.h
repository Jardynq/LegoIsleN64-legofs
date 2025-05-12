#pragma once

#include <cstdio>
#include <cstring>
#include "worlds.h"

bool dump_texture(const Texture& texture, const char* path);
void dump_components(Model* model, const std::string& dest, std::unordered_map<std::string, std::mutex*>* mutexes);
void dump_lod(const Lod& lod, float center[3], const std::string& dest, const char* roiname, int index, std::mutex* mutex);
