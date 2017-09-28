#include "tga_addon.h"

#include <inttypes.h>

#include <stdlib.h>

void write_tga_file (char *tga_file, tbyte *buffer, int width, int height, int depth, bool lre) {
	TGAData frame_data;	
	TGA *tga = TGAOpen (tga_file, "w");
	tga->hdr.id_len 	= 0;
	tga->hdr.map_t		= 0;
	tga->hdr.img_t 		= (depth == 8) ? 3 : 2; // 8-bit format is black&white, others are rgb
	tga->hdr.map_first 	= 0;
	tga->hdr.map_entry 	= 0;
	tga->hdr.map_len	= 0;
	tga->hdr.x 			= 0;
	tga->hdr.y 			= 0;
	tga->hdr.width 		= width;
	tga->hdr.height 	= height;
	tga->hdr.depth 		= depth;
	tga->hdr.vert 	    = 0;
	tga->hdr.horz   	= 0;
	tga->hdr.alpha      = (depth == 32) ? 8 : 0; // bits per pixel for alpha channel
	
	frame_data.flags = TGA_IMAGE_DATA | TGA_IMAGE_ID | TGA_RGB;
	
	if (lre) frame_data.flags |= TGA_RLE_ENCODE;
	
	frame_data.img_data = buffer;
	frame_data.cmap = NULL;
	frame_data.img_id = NULL;
	int status = TGAWriteImage (tga, &frame_data);
	if (status != TGA_OK) {
		//printf ("%s\n", tga_error_strings[status]);
		return;
	}
	TGAClose(tga);
}

Bitmap * new_bitmap_from_tga (const char *tga_file) {
    
    Bitmap *bmp = bitmap_new ();
    if (bmp == NULL) return NULL;
    
    TGA *tga = TGAOpen ((char*) tga_file, "r");
	if (!tga || tga->last != TGA_OK) return NULL;
	
	TGAData tgadata;
	tgadata.flags = TGA_IMAGE_DATA | TGA_IMAGE_ID | TGA_RGB;
	if (TGAReadImage (tga, &tgadata) != TGA_OK) return NULL;	
	bmp->data    = tgadata.img_data;
	bmp->w       = tga->hdr.width;
	bmp->h       = tga->hdr.height;
	bmp->bytespp = tga->hdr.depth / 8;
	printf ("tga: w=%"PRIu32", h=%"PRIu32", bpp=%"PRIu32"\n", bmp->w, bmp->h, bmp->bytespp);
	TGAClose(tga);
	
	Bitmap *rgb32 = bitmap_new ();
	if (rgb32 == NULL) return NULL;
	
	rgb32->data = calloc (bmp->w * bmp->h * 4, sizeof (uint8_t));
	
	for (int i = 0; i < bmp->h; i++) {
		for (int j = 0; j < bmp->w; j++) {
			size_t word_offset = (j + bmp->w * i);
			size_t byte_offset24 = word_offset * bmp->bytespp;
			size_t byte_offset32 = word_offset * 4;
			//pixel_color_t pix;
			*(rgb32->data + byte_offset32 + 0) = *(bmp->data + byte_offset24 + 0);
			*(rgb32->data + byte_offset32 + 1) = *(bmp->data + byte_offset24 + 1);
			*(rgb32->data + byte_offset32 + 2) = *(bmp->data + byte_offset24 + 2);
			*(rgb32->data + byte_offset32 + 3) = 0; // TBD extend 
		}
	}
	
	rgb32->w = bmp->w;
	rgb32->h = bmp->h;
	rgb32->bytespp = 4;
	
	bitmap_free (bmp);
		
	return rgb32;
}
