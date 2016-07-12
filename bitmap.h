#pragma once

#include <stdint.h>

typedef struct Bitmap {
	uint8_t *data;
	int w;
	int h;
	int bytespp;
} Bitmap;

Bitmap * bitmap_new  ();
void     bitmap_free (Bitmap *bmp);
