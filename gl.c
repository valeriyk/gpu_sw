#include "gl.h"
//#include "geometry.h"
//#include "geometry_fixpt.h"
#include "wavefront_obj.h"
#include "dynarray.h"

//#include <fixmath.h>

#include <math.h>
#include <stdlib.h>
#include <stdint.h>


// POSITIVE Z TOWARDS ME



size_t SCREEN_WIDTH;
size_t SCREEN_HEIGHT;
size_t SCREEN_DEPTH;

size_t NUM_OF_TILES;

fmat4 VIEWPORT;


fixpt_t edge_func_fixpt (fixpt_t ax, fixpt_t ay, fixpt_t bx, fixpt_t by, fixpt_t cx, fixpt_t cy);

int32_t min_of_three (int32_t a, int32_t b, int32_t c);
int32_t max_of_three (int32_t a, int32_t b, int32_t c);	




fixpt_t edge_func_fixpt (fixpt_t ax, fixpt_t ay, fixpt_t bx, fixpt_t by, fixpt_t cx, fixpt_t cy) {
    
    dfixpt_t bx_ax = (dfixpt_t) bx - (dfixpt_t) ax;
    dfixpt_t cy_ay = (dfixpt_t) cy - (dfixpt_t) ay;
    dfixpt_t by_ay = (dfixpt_t) by - (dfixpt_t) ay;
    dfixpt_t cx_ax = (dfixpt_t) cx - (dfixpt_t) ax;

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

FixPt3 get_bar_coords (fixpt_t x[3], fixpt_t y[3], fixpt_t px, fixpt_t py) {
    
    FixPt3 barc;
    
    barc.as_array[0] = edge_func_fixpt (x[1], y[1], x[2], y[2], px, py); // not normalized
	barc.as_array[1] = edge_func_fixpt (x[2], y[2], x[0], y[0], px, py); // not normalized
	barc.as_array[2] = edge_func_fixpt (x[0], y[0], x[1], y[1], px, py); // not normalized
	
	return barc;
}

static inline int32_t min_of_two (int32_t a, int32_t b) {
	return (a < b) ? a : b;
}

static inline int32_t max_of_two (int32_t a, int32_t b) {
	return (a > b) ? a : b;
}

int32_t min_of_three (int32_t a, int32_t b, int32_t c) {
	return min_of_two(a, min_of_two(b, c));
}

int32_t max_of_three (int32_t a, int32_t b, int32_t c) {
	return max_of_two(a, max_of_two(b, c));
}

pixel_color_t set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	pixel_color_t pc;
	pc.r = r;
	pc.g = g;
	pc.b = b;
	//pc.a = a;
	return pc;
}

void init_viewport (int x, int y, int w, int h, int d) {

	// X: map [-1:1] to [0:(SCREEN_WIDTH+HEIGTH)/2]
	// Y: map [-1:1] to [0:SCREEN_HEIGHT]
	// Z: map [-1:1] to [SCREEN_DEPTH:0]				
	// W: leave as is
				
	fmat4_identity (&VIEWPORT);
	fmat4_set (&VIEWPORT, 0, 0, h / 2.0f); //(w/2.0) * (h/w) = h/2.0 - adjust for screen aspect ratio
	fmat4_set (&VIEWPORT, 0, 3, x + w / 2.0f);
	fmat4_set (&VIEWPORT, 1, 1, h / 2.0f);
	fmat4_set (&VIEWPORT, 1, 3, y + h / 2.0f);
	fmat4_set (&VIEWPORT, 2, 2, -d / 2.0f); // minus sign because Z points in opposite directions in NDC and screen/clip
	fmat4_set (&VIEWPORT, 2, 3,  d / 2.0f);
}

void set_screen_size (gpu_cfg_t *cfg, size_t width, size_t height) {
	SCREEN_WIDTH  = width;
	SCREEN_HEIGHT = height;
	SCREEN_DEPTH  = (screenz_t) ~0; // all ones
	
	NUM_OF_TILES = (SCREEN_WIDTH / TILE_WIDTH) * (SCREEN_HEIGHT / TILE_HEIGHT);
	
	cfg->screen_width  = SCREEN_WIDTH;
	cfg->screen_height = SCREEN_HEIGHT;
	cfg->tile_width    = TILE_WIDTH;
	cfg->tile_height   = TILE_HEIGHT;
	cfg->num_of_tiles  = NUM_OF_TILES;
}

size_t get_screen_width (void) {
	return SCREEN_WIDTH;
}

size_t get_screen_height (void) {
	return SCREEN_HEIGHT;
}

size_t get_screen_depth (void) {
	return SCREEN_DEPTH;
}


void init_lights (void) {
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		LIGHTS[i].enabled        = false;
		LIGHTS[i].has_shadow_buf = false;
		LIGHTS[i].shadow_buf     = NULL;
	}
}

void new_light (int light_num, Float3 dir, bool add_shadow_buf) { //TBD add light_src,
	LIGHTS[light_num].enabled = true;
	LIGHTS[light_num].dir = dir;
	LIGHTS[light_num].src = Float3_set (-dir.as_struct.x, -dir.as_struct.y, -dir.as_struct.z);
	
	if (add_shadow_buf) {
		LIGHTS[light_num].has_shadow_buf = true;
		LIGHTS[light_num].shadow_buf = (screenz_t*) calloc (SCREEN_WIDTH*SCREEN_HEIGHT, sizeof(screenz_t));
	}
}

void free_light (int light_num) {
	LIGHTS[light_num].enabled        = false;
	LIGHTS[light_num].has_shadow_buf = false;
	
    if (LIGHTS[light_num].shadow_buf != NULL) {
		free (LIGHTS[light_num].shadow_buf);
	}
}

void init_perspective_proj (fmat4 *m, float left, float right, float top, float bot, float near, float far) {
	fmat4_identity (m);
	fmat4_set (m, 0, 0,       ( 2.0f * near) / (right - left));
	fmat4_set (m, 0, 2,       (right + left) / (right - left));
	fmat4_set (m, 1, 1,       ( 2.0f * near) / (  top -  bot));
	fmat4_set (m, 1, 2,       (  top +  bot) / (  top -  bot));
	fmat4_set (m, 2, 2,      -(  far + near) / (  far - near));
	fmat4_set (m, 2, 3, (-2.0f * far * near) / (  far - near));
	fmat4_set (m, 3, 2,  -1.0f);
	fmat4_set (m, 3, 3,   0.0f);
}

void init_ortho_proj (fmat4 *m, float left, float right, float top, float bot, float near, float far) {
	fmat4_identity (m);
	fmat4_set (m, 0, 0,            2.0f / (right - left));
	fmat4_set (m, 0, 3, -(right + left) / (right - left));
	fmat4_set (m, 1, 1,            2.0f / (  top -  bot));
	fmat4_set (m, 1, 3, -(  top +  bot) / (  top -  bot));
	fmat4_set (m, 2, 2,           -2.0f / (  far - near));
	fmat4_set (m, 2, 3, -(  far + near) / (  far - near));
	fmat4_set (m, 3, 3, 1.0f);
}

void init_view (fmat4 *m, Float3 *eye, Float3 *center, Float3 *up) {
	
	Float3 z = Float3_Float3_sub(eye, center);
	Float3_normalize (&z);
	Float3 x = Float3_Float3_crossprod(up, &z);
	Float3_normalize (&x);
	Float3 y = Float3_Float3_crossprod(&z, &x);
	Float3_normalize (&y);
	
	fmat4 Orient = FMAT4_IDENTITY;
	fmat4 Transl = FMAT4_IDENTITY;
	
	for (int i = 0; i < 3; i++)	{
		fmat4_set (&Orient, 0, i, x.as_array[i]);
		fmat4_set (&Orient, 1, i, y.as_array[i]);
		fmat4_set (&Orient, 2, i, z.as_array[i]);

		fmat4_set (&Transl, i, 3, -(eye->as_array[i]));
	}
	fmat4_fmat4_mult(&Orient, &Transl, m);
}

void rotate_coords (fmat4 *in, fmat4 *out, float alpha_deg, axis a) {
	float rad = alpha_deg * 0.01745f; // degrees to rad conversion
	float sin_alpha = sinf(rad);
	float cos_alpha = cosf(rad);
	
	fmat4 r = FMAT4_IDENTITY;
	
	fmat4_set (&r, 0, 0, (a == X) ? 1.0f : cos_alpha);
	fmat4_set (&r, 1, 1, (a == Y) ? 1.0f : cos_alpha);
	fmat4_set (&r, 2, 2, (a == Z) ? 1.0f : cos_alpha);
	
	fmat4_set (&r, 0, 1, (a == Z) ? -sin_alpha : 0.0f);
	fmat4_set (&r, 0, 2, (a == Y) ? -sin_alpha : 0.0f);
	fmat4_set (&r, 1, 2, (a == X) ? -sin_alpha : 0.0f);
	
	fmat4_set (&r, 1, 0, (a == Z) ?  sin_alpha : 0.0f);
	fmat4_set (&r, 2, 0, (a == Y) ?  sin_alpha : 0.0f);
	fmat4_set (&r, 2, 1, (a == X) ?  sin_alpha : 0.0f);
	
	fmat4_fmat4_mult (in, &r, out);
}

Object* obj_new (WaveFrontObj *wfobj, Bitmap *texture, Bitmap *normalmap, Bitmap *specularmap) {
	Object *obj = (Object*) malloc (sizeof(Object));
	obj->wfobj = wfobj;
	for (int i = 0; i < 3; i++) {
		obj->scale[i]  = 1.f;
		obj->rotate[i] = 0.f;
		obj->tran[i]   = 0.f;
	}
	fmat4_identity (&(obj->mvp));
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		fmat4_identity (&(obj->shadow_mvp[i]));
	}
	
	obj->texture     = texture;
	obj->normalmap   = normalmap;
	obj->specularmap = specularmap;
	
	return obj;
}

void obj_free (Object *obj) {
	free (obj);
}


void obj_set_scale (Object *obj, float x, float y, float z) {
	obj->scale[0] = x;
	obj->scale[1] = y;
	obj->scale[2] = z;
}

void obj_set_rotation (Object *obj, float x_deg, float y_deg, float z_deg) {
	obj->rotate[0] = x_deg;
	obj->rotate[1] = y_deg;
	obj->rotate[2] = z_deg;
}

void obj_set_translation (Object *obj, float x, float y, float z) {
	obj->tran[0] = x;
	obj->tran[1] = y;
	obj->tran[2] = z;
}

void obj_init_model (Object *obj) {
	
	// 1. translate	
	fmat4 t = FMAT4_IDENTITY;
	fmat4_set (&t, 0, 3, obj->tran[X]);
	fmat4_set (&t, 1, 3, obj->tran[Y]);
	fmat4_set (&t, 2, 3, obj->tran[Z]);
	
	// 2. rotate
	fmat4 rot_x, rot_xy, rot_xyz;
	rotate_coords (&t,      &rot_x,   obj->rotate[X], X);
	rotate_coords (&rot_x,  &rot_xy,  obj->rotate[Y], Y);
	rotate_coords (&rot_xy, &rot_xyz, obj->rotate[Z], Z);
	
	// 3. scale	
	fmat4 s = FMAT4_IDENTITY;
	fmat4_set (&s, 0, 0, obj->scale[X]);
	fmat4_set (&s, 1, 1, obj->scale[Y]);
	fmat4_set (&s, 2, 2, obj->scale[Z]);
	fmat4_fmat4_mult (&rot_xyz, &s, &(obj->model));
}

BoundBox get_tri_boundbox (fixpt_t x[3], fixpt_t y[3]) {
	
	BoundBox bb;
    
    bb.min.x = fixpt_to_screenxy (min_of_three (x[0], x[1], x[2]));
    bb.max.x = fixpt_to_screenxy (max_of_three (x[0], x[1], x[2]));
    bb.min.y = fixpt_to_screenxy (min_of_three (y[0], y[1], y[2]));
    bb.max.y = fixpt_to_screenxy (max_of_three (y[0], y[1], y[2]));
    
    return bb;
}

BoundBox clip_boundbox_to_screen (BoundBox in) {
	
	BoundBox out;
	
	out.min.x = max_of_two (in.min.x, 0);
    out.max.x = min_of_two (in.max.x, SCREEN_WIDTH - 1);
    out.min.y = max_of_two (in.min.y, 0);
    out.max.y = min_of_two (in.max.y, SCREEN_HEIGHT - 1);

	return out;
}

BoundBox clip_boundbox_to_tile (BoundBox in, size_t tile_num) {
	
	BoundBox tile;
				
	tile.min.x = (tile_num % (SCREEN_WIDTH/TILE_WIDTH)) * TILE_WIDTH;
    tile.min.y = (tile_num / (SCREEN_WIDTH/TILE_WIDTH)) * TILE_HEIGHT;
    tile.max.x = tile.min.x + TILE_WIDTH  - 1; 
    tile.max.y = tile.min.y + TILE_HEIGHT - 1;
    
    BoundBox out;
	
	out.min.x = max_of_two (tile.min.x, in.min.x);
    out.max.x = min_of_two (tile.max.x, in.max.x);
    out.min.y = max_of_two (tile.min.y, in.min.y);
    out.max.y = min_of_two (tile.max.y, in.max.y);
    
    
    /*
    out.min.x = max_of_two (              0, in.min.x - tile.min.x);
    out.max.x = min_of_two (TILE_WIDTH  - 1, in.max.x - tile.min.x);
    out.min.y = max_of_two (              0, in.min.y - tile.min.y);
    out.max.y = min_of_two (TILE_HEIGHT - 1, in.max.y - tile.min.y);
    */
    
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


void tiler (TrianglePShaderData *tri, TriangleListNode *tri_ptr[]) {
	
	fixpt_t x[3];
	fixpt_t y[3];
	
	// re-pack X, Y, Z, W coords of the three vertices
	for (int i = 0; i < 3; i++) {
		x[i] = tri->screen_coords[i].as_struct.x;
		y[i] = tri->screen_coords[i].as_struct.y;
	}
	
	BoundBox bb = clip_boundbox_to_screen (get_tri_boundbox (x, y));
    bb.min.x &= ~(TILE_WIDTH-1);
    bb.min.y &= ~(TILE_HEIGHT-1);
    
    ScreenPt p;
    
    // Evaluate barycentric coords in all the four corners of each tile
    for (p.y = bb.min.y; p.y <= bb.max.y; p.y += TILE_HEIGHT) {	
		for (p.x = bb.min.x; p.x <= bb.max.x; p.x += TILE_WIDTH) {

			// find corners of the tile:
			fixpt_t x0 = fixpt_from_screenxy (p.x);
			fixpt_t x1 = fixpt_from_screenxy (p.x + TILE_WIDTH - 1);
			fixpt_t y0 = fixpt_from_screenxy (p.y);
			fixpt_t y1 = fixpt_from_screenxy (p.y + TILE_HEIGHT - 1);
			
			// get barycentric coords in each corner:
			FixPt3 b0 = get_bar_coords (x, y, x0, y0);
			FixPt3 b1 = get_bar_coords (x, y, x0, y1);
			FixPt3 b2 = get_bar_coords (x, y, x1, y0);
			FixPt3 b3 = get_bar_coords (x, y, x1, y1);
			
			bool tri_inside_tile = true; // sticky bit
			for (int i = 0; i < 3; i++) {
				// If barycentric coord "i" is negative in all four corners, triangle is outside the tile
				// See here: http://forum.devmaster.net/t/advanced-rasterization/6145
				if ((b0.as_array[i] & b1.as_array[i] & b2.as_array[i] & b3.as_array[i]) < 0) {
					tri_inside_tile = false;
					break;
				}
			}
			
			if (tri_inside_tile) {
				
				size_t tile_num = (p.y >> (int) log2f(TILE_HEIGHT)) * (SCREEN_WIDTH / TILE_WIDTH) + (p.x >> (int) log2f(TILE_WIDTH));
				
				TriangleListNode *node = tri_ptr[tile_num];
				if (node == NULL) {
					node = calloc (1, sizeof (TriangleListNode));
					node->tri  = tri;
					node->next = NULL;	
					
					tri_ptr[tile_num] = node;
				}
				else {
					while (node->next != NULL) {
						node = node->next;
					}
					node->next = calloc (1, sizeof (TriangleListNode));
					node->next->tri  = tri;
					node->next->next = NULL;	
				}
			}
		}
	}
}




void varying_fifo_push_float  (Varying *vry, float data) {
	size_t idx = vry->num_of_words;

	vry->data[idx].as_fixpt_t = fixpt_from_float_no_rnd (data, VARYING_FRACT_BITS);
	if (DEBUG_FIXPT_VARYING) {
		vry->data[idx].as_float = data;
	}


	vry->num_of_words++;
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
	
	size_t num_of_words = vry->num_of_words;
	
	assert ((type == VARYING_FIXPT) || (type == VARYING_FLOAT));
	assert (num_of_words > 0);
	
	static size_t idx = 0;
	
	VaryingWord data;
	if (type == VARYING_FIXPT) {
		data.as_fixpt_t = vry->data[idx].as_fixpt_t;
	}
	else if (type == VARYING_FLOAT) {
		data.as_float = fixpt_to_float (vry->data[idx].as_fixpt_t, VARYING_FRACT_BITS);
	}
	idx++;
	if (idx == num_of_words) idx = 0;
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
		size_t offset = (u + bmp->w * v) * (bmp->bytespp);
		pix.r = *(bmp->data + offset + 0);
		pix.g = *(bmp->data + offset + 1);
		pix.b = *(bmp->data + offset + 2);
	}
	else {
		pix.r = 0;
		pix.g = 0;
		pix.b = 0;
	}
	return pix;
}

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
