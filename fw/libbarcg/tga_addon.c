#include "tga_addon.h"

#include <inttypes.h>
#include <stdbool.h>

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
	tga->hdr.alpha      = 0;// TBD (depth == 32) ? 8 : 0; // bits per pixel for alpha channel
	
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

Bitmap *new_bitmap_from_tga (const char *tga_file) {
    
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
	
	if (bmp->bytespp != 3) {
		return bmp;
	}
	else {
		printf ("\tconverting bitmap from 3 to 4 bytes per pixel\n");
		
		Bitmap *new_bmp = bitmap_new ();
		if (new_bmp == NULL) return NULL;
		
		new_bmp->w = bmp->w;
		new_bmp->h = bmp->h;
		new_bmp->bytespp = 4;
		new_bmp->data = calloc (new_bmp->w * new_bmp->h * new_bmp->bytespp, sizeof (uint8_t));
		
		for (int i = 0; i < new_bmp->h; i++) {
			for (int j = 0; j < new_bmp->w; j++) {
				size_t item_offset = (j + new_bmp->w * i);
				size_t src_byte_offset = item_offset *     bmp->bytespp;
				size_t dst_byte_offset = item_offset * new_bmp->bytespp;
				//pixel_color_t pix;
				for (int k = 0; k < 4; k++) {
					// copy first three bytes and zero the fourth
					if (new_bmp->bytespp > k) {
						*(new_bmp->data + dst_byte_offset + k) = (bmp->bytespp > k) ? (*(bmp->data + src_byte_offset + k)) : 0;
					}
				}
				
				//~ if (new_bmp->bytespp > 0) {
					//~ *(new_bmp->data + dst_byte_offset + 0) = *(bmp->data + src_byte_offset + 0);
				//~ }
				//~ if (new_bmp->bytespp > 1) {
					//~ *(new_bmp->data + dst_byte_offset + 1) = (bmp->bytespp > 1) ? *(bmp->data + src_byte_offset + 1) : 0;
				//~ }
				//~ if (new_bmp->bytespp > ) {
					//~ *(new_bmp->data + dst_byte_offset + 1) = (bmp->bytespp > 1) ? *(bmp->data + src_byte_offset + 1) : 0;
				//~ }
				//~ if (new_bmp->bytespp > 1) {
					//~ *(new_bmp->data + dst_byte_offset + 1) = (bmp->bytespp > 1) ? *(bmp->data + src_byte_offset + 1) : 0;
				//~ }
				//~ *(new_bmp->data + dst_byte_offset + 2) = *(bmp->data + src_byte_offset + 2);
				//~ *(new_bmp->data + dst_byte_offset + 3) = 0; // TBD extend 
			}
		}
		
		bitmap_free (bmp);
			
		return new_bmp;
	}
}
