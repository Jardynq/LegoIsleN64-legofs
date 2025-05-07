#pragma once

#include <cstdio>
#include <cstring>
#include "worlds.h"

bool dump_texture(const Texture& texture, const char* path);
bool dump_lod(const Lod& lod, const char* filepath);
