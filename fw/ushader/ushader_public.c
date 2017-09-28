#include "gl.h"
#include "wavefront_obj.h"

#include <math.h>
#include <stdlib.h>
#include <stdint.h>


// POSITIVE Z TOWARDS ME



fixpt_t edge_func_fixpt (hfixpt_t ax, hfixpt_t ay, hfixpt_t bx, hfixpt_t by, hfixpt_t cx, hfixpt_t cy);
//void set_tile_size (gpu_cfg_t *cfg, size_t width, size_t height);
	
	
	
hfixpt_t min_of_three (hfixpt_t a, hfixpt_t b, hfixpt_t c);
hfixpt_t max_of_three (hfixpt_t a, hfixpt_t b, hfixpt_t c);	



fixpt_t edge_func_fixpt (hfixpt_t ax, hfixpt_t ay, hfixpt_t bx, hfixpt_t by, hfixpt_t cx, hfixpt_t cy) {
    
    hfixpt_t bx_ax = bx - ax;
    hfixpt_t cy_ay = cy - ay;
    hfixpt_t by_ay = by - ay;
    hfixpt_t cx_ax = cx - ax;

    fixpt_t mul_0 = bx_ax * cy_ay;
    fixpt_t mul_1 = by_ay * cx_ax;
    fixpt_t diff  = mul_0 - mul_1;
    fixpt_t res = (diff >> (2*XY_FRACT_BITS - BARC_FRACT_BITS));
    
    // left-top fill rule:
    bool downwards  = (by_ay  < 0);
    bool horizontal = (by_ay == 0);
    bool leftwards  = (bx_ax  < 0);
    bool topleft    = downwards || (horizontal && leftwards);
    
    return topleft ? res + 1 : res;
}


fixpt_t edge_func_fixpt2 (hfixpt_t ax, hfixpt_t ay, hfixpt_t bx, hfixpt_t by, hfixpt_t cx, hfixpt_t cy) {
    
    fixpt_t bx_ax = (fixpt_t) bx - (fixpt_t) ax;
    fixpt_t cy_ay = (fixpt_t) cy - (fixpt_t) ay;
    fixpt_t by_ay = (fixpt_t) by - (fixpt_t) ay;
    fixpt_t cx_ax = (fixpt_t) cx - (fixpt_t) ax;

    dfixpt_t mul_0 = bx_ax * cy_ay;
    dfixpt_t mul_1 = by_ay * cx_ax;
    dfixpt_t diff  = mul_0 - mul_1;
    fixpt_t res = (fixpt_t) (diff >> (2*XY_FRACT_BITS - BARC_FRACT_BITS));
    
    // left-top fill rule:
    bool downwards  = (by_ay  < 0);
    bool horizontal = (by_ay == 0);
    bool leftwards  = (bx_ax  < 0);
    bool topleft    = downwards || (horizontal && leftwards);
    
    return topleft ? res + 1 : res;
}

FixPt3 get_bar_coords (hfixpt_t x[3], hfixpt_t y[3], hfixpt_t px, hfixpt_t py) {
    
    FixPt3 barc;
    
    barc.as_array[0] = edge_func_fixpt (x[1], y[1], x[2], y[2], px, py); // not normalized
	barc.as_array[1] = edge_func_fixpt (x[2], y[2], x[0], y[0], px, py); // not normalized
	barc.as_array[2] = edge_func_fixpt (x[0], y[0], x[1], y[1], px, py); // not normalized
	
	return barc;
}

static inline hfixpt_t min_of_two (hfixpt_t a, hfixpt_t b) {
	return (a < b) ? a : b;
}

static inline hfixpt_t max_of_two (hfixpt_t a, hfixpt_t b) {
	return (a > b) ? a : b;
}

hfixpt_t min_of_three (hfixpt_t a, hfixpt_t b, hfixpt_t c) {
	return min_of_two(a, min_of_two(b, c));
}

hfixpt_t max_of_three (hfixpt_t a, hfixpt_t b, hfixpt_t c) {
	return max_of_two(a, max_of_two(b, c));
}

pixel_color_t set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	pixel_color_t pc;
	pc.as_byte.r = r;
	pc.as_byte.g = g;
	pc.as_byte.b = b;
	pc.as_byte.a = a;
	return pc;
}

pixel_color_t color_mult (pixel_color_t pix, float intensity) {
	pixel_color_t pc;
	uint8_t int_int = (uint8_t) (intensity * 255);
	uint16_t tmp_r = ((uint16_t) pix.as_byte.r) * ((uint16_t) int_int);
	uint16_t tmp_g = ((uint16_t) pix.as_byte.g) * ((uint16_t) int_int);
	uint16_t tmp_b = ((uint16_t) pix.as_byte.b) * ((uint16_t) int_int);
	pc.as_byte.r = (uint8_t) (tmp_r >> 8);
	pc.as_byte.g = (uint8_t) (tmp_g >> 8);
	pc.as_byte.b = (uint8_t) (tmp_b >> 8);
	pc.as_byte.a = 0;
	
	//~ int r = pix.r * intensity + 5;
	//~ int g = pix.g * intensity + 5;
	//~ int b = pix.b * intensity + 5;
		
	//~ if (r > 255) r = 255;
	//~ if (g > 255) g = 255;
	//~ if (b > 255) b = 255;
	
	//~ pc = set_color (r, g, b, 0);
	
	return pc;
}


size_t get_screen_width  (gpu_cfg_t *cfg) {
	return cfg->screen_width;
}

size_t get_screen_height (gpu_cfg_t *cfg) {
	return cfg->screen_height;
}

size_t get_screen_depth  (gpu_cfg_t *cfg) {
	return cfg->screen_depth;
}


BoundBox get_tri_boundbox (hfixpt_t x[3], hfixpt_t y[3]) {
	
	BoundBox bb;
    
    bb.min.x = hfixpt_to_screenxy (min_of_three (x[0], x[1], x[2]));
    bb.max.x = hfixpt_to_screenxy (max_of_three (x[0], x[1], x[2]));
    bb.min.y = hfixpt_to_screenxy (min_of_three (y[0], y[1], y[2]));
    bb.max.y = hfixpt_to_screenxy (max_of_three (y[0], y[1], y[2]));
    
    return bb;
}

BoundBox clip_boundbox_to_screen (BoundBox in, gpu_cfg_t *cfg) {
	
	BoundBox out;
	
	out.min.x = max_of_two (in.min.x, 0);
    out.max.x = min_of_two (in.max.x, get_screen_width(cfg) - 1);
    out.min.y = max_of_two (in.min.y, 0);
    out.max.y = min_of_two (in.max.y, get_screen_height(cfg) - 1);

	return out;
}

BoundBox clip_boundbox_to_tile (size_t tile_num, BoundBox in, gpu_cfg_t *cfg) {
	
	BoundBox tile;
				
	tile.min.x = (tile_num % (get_screen_width(cfg) >> GPU_TILE_WIDTH_LOG2)) << GPU_TILE_WIDTH_LOG2;
    tile.min.y = (tile_num / (get_screen_width(cfg) >> GPU_TILE_WIDTH_LOG2)) << GPU_TILE_HEIGHT_LOG2;
    tile.max.x = tile.min.x + GPU_TILE_WIDTH  - 1; 
    tile.max.y = tile.min.y + GPU_TILE_HEIGHT - 1;
    
    BoundBox out;
	
	out.min.x = max_of_two (tile.min.x, in.min.x);
    out.max.x = min_of_two (tile.max.x, in.max.x);
    out.min.y = max_of_two (tile.min.y, in.min.y);
    out.max.y = min_of_two (tile.max.y, in.max.y);
        
    return out;
}




/*
void draw_line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x=x0; x<=x1; x++) {
        float t = (x-x0)/(float)(x1-x0);
        int y = y0*(1.-t) + y1*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}
*/






void varying_fifo_push_float  (Varying *vry, float data) {
	size_t idx = vry->num_of_words_written;

	vry->data[idx].as_fixpt_t = fixpt_from_float (data, VARYING_FRACT_BITS); // no rounding
	if (DEBUG_FIXPT_VARYING) {
		vry->data[idx].as_float = data;
	}


	vry->num_of_words_written++;
}

void varying_fifo_push_Float2 (Varying *vry, Float2 *data) {
	for (int i = 0; i < 2; i++) {
		varying_fifo_push_float (vry, data->as_array[i]);
	}
}

void varying_fifo_push_Float3 (Varying *vry, Float3 *data) {
	for (int i = 0; i < 3; i++) {
		varying_fifo_push_float (vry, data->as_array[i]);
	}
}

void varying_fifo_push_Float4 (Varying *vry, Float4 *data) {
	for (int i = 0; i < 4; i++) {
		varying_fifo_push_float (vry, data->as_array[i]);
	}
}

static inline VaryingWord varying_fifo_pop (Varying *vry, varying_type type) {
	
	assert ((type == VARYING_FIXPT) || (type == VARYING_FLOAT));
	assert (vry->num_of_words_written > 0);
	
	size_t idx = vry->num_of_words_read;
	VaryingWord data;
	if (type == VARYING_FIXPT) {
		data.as_fixpt_t = vry->data[idx].as_fixpt_t;
	}
	else if (type == VARYING_FLOAT) {
		data.as_float = fixpt_to_float (vry->data[idx].as_fixpt_t, VARYING_FRACT_BITS);
	}
	
	vry->num_of_words_read++;
	return data;
}

float  varying_fifo_pop_float (Varying *vry) {
	VaryingWord v = varying_fifo_pop (vry, VARYING_FLOAT);
	return v.as_float;
}

Float2  varying_fifo_pop_Float2 (Varying *vry) {
	Float2 data;
	for (int i = 0; i < 2; i++) {
		VaryingWord v = varying_fifo_pop (vry, VARYING_FLOAT);
		data.as_array[i] = v.as_float;
	}
	return data;
}

Float3  varying_fifo_pop_Float3 (Varying *vry) {
	Float3 data;
	for (int i = 0; i < 3; i++) {
		VaryingWord v = varying_fifo_pop (vry, VARYING_FLOAT);
		data.as_array[i] = v.as_float;
	}
	return data;
}

Float4  varying_fifo_pop_Float4 (Varying *vry) {
	Float4 data;
	for (int i = 0; i < 4; i++) {
		VaryingWord v = varying_fifo_pop (vry, VARYING_FLOAT);
		data.as_array[i] = v.as_float;
	}
	return data;
}


fixpt_t  varying_fifo_pop_fixpt (Varying *vry) {
	VaryingWord v = varying_fifo_pop (vry, VARYING_FIXPT);
	return v.as_fixpt_t;
}

FixPt2  varying_fifo_pop_FixPt2 (Varying *vry) {
	FixPt2 data;
	for (int i = 0; i < 2; i++) {
		VaryingWord v = varying_fifo_pop (vry, VARYING_FIXPT);
		data.as_array[i] = v.as_fixpt_t;
	}
	return data;
}

FixPt3  varying_fifo_pop_FixPt3 (Varying *vry) {
	FixPt3 data;
	for (int i = 0; i < 3; i++) {
		VaryingWord v = varying_fifo_pop (vry, VARYING_FIXPT);
		data.as_array[i] = v.as_fixpt_t;
	}
	return data;
}

FixPt4  varying_fifo_pop_FixPt4 (Varying *vry) {
	FixPt4 data;
	for (int i = 0; i < 4; i++) {
		VaryingWord v = varying_fifo_pop (vry, VARYING_FIXPT);
		data.as_array[i] = v.as_fixpt_t;
	}
	return data;
}


pixel_color_t get_pixel_color_from_bitmap (const Bitmap *bmp, const int u, const int v) {
	pixel_color_t pix;
	if (bmp->data != NULL) {
		pixel_color_t *p = (pixel_color_t *) bmp->data;
		//~ size_t offset = (u + bmp->w * v) * (bmp->bytespp);
		//~ pix.r = *(bmp->data + offset + 0);
		//~ pix.g = *(bmp->data + offset + 1);
		//~ pix.b = *(bmp->data + offset + 2);
		pix = p[u + bmp->w * v];
	}
	else {
		pix.as_word = 0;
	}	
	
	return pix;
}


//~ pixel_color_t get_pixel_color_from_rgb32_bitmap (const Bitmap *bmp, const int u, const int v) {
	//~ pixel_color_t pix;
	//~ if (bmp->data != NULL) {
		//~ size_t offset = (u + bmp->w * v) * (bmp->bytespp);
		//~ pix.r = *(bmp->data + offset + 0);
		//~ pix.g = *(bmp->data + offset + 1);
		//~ pix.b = *(bmp->data + offset + 2);
		//~ //pix.a = *(bmp->data + offset + 3);
	//~ }
	//~ else {
		//~ pix.r = 0;
		//~ pix.g = 0;
		//~ pix.b = 0;
		//~ //pix.a = 0;
	//~ }
	//~ return pix;
//~ }

Float3 get_norm_Float3_from_bitmap (const Bitmap *bmp, const int u, const int v) {
	Float3 val;
	if (bmp->data != NULL) {
		size_t offset = (u + bmp->w * v) * bmp->bytespp;
		val.as_struct.x = *(bmp->data + offset + 0) / 255.f * 2.f - 1.f; // map [0:255] to [-1:1]
		val.as_struct.y = *(bmp->data + offset + 1) / 255.f * 2.f - 1.f; // map [0:255] to [-1:1]
		val.as_struct.z = *(bmp->data + offset + 2) / 255.f * 2.f - 1.f; // map [0:255] to [-1:1]
	}
	else {
		val.as_struct.x = 0.f;
		val.as_struct.y = 0.f;
		val.as_struct.z = 0.f;
	}
	return val;
}

int32_t get_int32_from_bitmap (const Bitmap *bmp, const int u, const int v) {
	if (bmp->data != NULL) {
		size_t offset = (u + bmp->w * v) * bmp->bytespp;
		return (int32_t) *(bmp->data + offset);
	}
	else {
		return 0;
	}
}

/*
void draw_line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x=x0; x<=x1; x++) {
        float t = (x-x0)/(float)(x1-x0);
        int y = y0*(1.-t) + y1*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}
*/
