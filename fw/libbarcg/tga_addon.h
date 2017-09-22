#pragma once

#include "gl.h"

#include <tga.h>

#include <stdbool.h>

void write_tga_file (char *tga_file, tbyte *buffer, int width, int height, int depth, bool lre);

Bitmap * new_bitmap_from_tga (const char *tga_file);
