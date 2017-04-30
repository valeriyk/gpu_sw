#pragma once

#include <stdint.h>

typedef struct Bitmap {
	uint8_t *data;
	uint32_t w;
	uint32_t h;
	uint32_t bytespp;
} Bitmap;

Bitmap * bitmap_new  ();
void     bitmap_free (Bitmap *bmp);
