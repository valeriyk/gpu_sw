#include "inttypes.h"

#include "tga_addon.h"

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
	tga->hdr.vert 	    = 1;
	tga->hdr.horz   	= 0;
	tga->hdr.alpha      = 0;
	
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
	
	return bmp;
}
