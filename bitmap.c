#include "bitmap.h"
#include <tga.h>

#include <stdlib.h>

Bitmap * bitmap_new () {
	Bitmap *bmp = (Bitmap*) malloc (sizeof(Bitmap));
	return bmp;
}

void bitmap_free (Bitmap *bmp) {
	if (bmp->data != NULL) free (bmp->data);
	if (bmp       != NULL) free (bmp);
}
