#pragma once

#include "bitmap.h"
#include "geometry.h"
#include "geometry_fixpt.h"
#include "wavefront_obj.h"

#include <platform.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>




#define DEBUG_FIXPT_VARYING 0
#define DEBUG_FIXPT_W       0

#define FLOAT 0



#define MAX_NUM_OF_LIGHTS	1
#define NUM_OF_VARYING_WORDS 28 // must be multiple of 4
#define TILE_WIDTH  32
#define TILE_HEIGHT 16



typedef enum {X = 0, Y, Z, W} axis;
typedef enum {VARYING_FLOAT = 0, VARYING_FIXPT} varying_type;


// If we clip only those triangles which are completely invisible, screenxy_t must be signed
// If we clip all the trianlges which are not completely visible, screenxy_t can be unsigned
typedef int16_t screenxy_t;

typedef uint16_t screenz_t;
//typedef fixpt_t screenz_t;
//typedef uint8_t screenz_t;

typedef struct ScreenPt {
	screenxy_t x;
	screenxy_t y;
	//screenz_t  z;
} ScreenPt;

typedef struct BoundBox {
	ScreenPt min;
	ScreenPt max;
} BoundBox;

typedef struct pixel_color_t {
	uint8_t r, g, b;
} pixel_color_t;

typedef struct Triangle {
	Float4 vtx[3];
} Triangle;


#ifdef DEBUG_FIXPT_VARYING
typedef struct VaryingWord {
	float    as_float;
#else
typedef union VaryingWord {
#endif
	fixpt_t  as_fixpt_t;
} VaryingWord;


typedef VaryingWord VaryingData [NUM_OF_VARYING_WORDS];

typedef struct Varying {
	int32_t num_of_words;
	VaryingData data;
} Varying;

typedef struct Object {
	WaveFrontObj *wfobj;
	Bitmap *texture;
	Bitmap *normalmap;
	Bitmap *specularmap;
	float3 scale;
	float3 rotate;
	float3 tran;
	fmat4  model;
	fmat4  mvp;  // pre-calculated ModelViewProjection matrix
	fmat4  mvit; // inverted and transposed ModelView matrix
	fmat4  shadow_mvp[MAX_NUM_OF_LIGHTS];
} Object;

typedef struct ObjectListNode {
	Object *obj;
	struct ObjectListNode *next;
} ObjectListNode;

typedef struct TrianglePShaderData {
	FixPt3  screen_coords[3];
	fixpt_t w_reciprocal[3];
	Varying varying[3];
	Object  *obj;
} TrianglePShaderData;

typedef struct TriangleListNode {
	struct TrianglePShaderData *tri;
	struct TriangleListNode *next;
} TriangleListNode;


typedef struct Light {
	bool 		enabled;
	Float3		dir;
	Float3		src;
	Float3		eye;
	bool		has_shadow_buf;
	screenz_t	*shadow_buf;
} Light;


extern fmat4 VIEWPORT;
extern Light LIGHTS[MAX_NUM_OF_LIGHTS];
//extern Bitmap TEXTURES[MAX_NUM_OF_TEXTURES];


typedef Float4 (*vertex_shader) (Object *obj, size_t face_idx, size_t vtx_idx, Varying *var);
typedef bool (*pixel_shader)  (Object *obj, Varying *var, pixel_color_t *color);


void init_lights (void);
void new_light (int light_num, Float3 dir, bool add_shadow_buf);
void free_light (int light_num);

void   set_screen_size   (platform_t *p, size_t width, size_t height);
size_t get_screen_width  (void);
size_t get_screen_height (void);
size_t get_screen_depth  (void);

void init_view             (fmat4 *m, Float3 *eye, Float3 *center, Float3 *up);
void init_perspective_proj (fmat4 *m, float left, float right, float top, float bot, float near, float far);
void init_ortho_proj       (fmat4 *m, float left, float right, float top, float bot, float near, float far);
void init_viewport   (int x, int y, int w, int h, int d);

void rotate_coords (fmat4 *in, fmat4 *out, float alpha_deg, axis axis);


pixel_color_t set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a);

Object* obj_new (WaveFrontObj *wfobj, Bitmap *texture, Bitmap *normalmap, Bitmap *specularmap);
void obj_free            (Object *obj);
void obj_set_scale       (Object *obj, float x,     float y,     float z);
void obj_set_rotation    (Object *obj, float x_deg, float y_deg, float z_deg);
void obj_set_translation (Object *obj, float x,     float y,     float z);
void obj_init_model      (Object *obj);
//void obj_transform       (Object *obj, fmat4 *vpv, fmat4 *projview, float3 *light_dir);




void varying_fifo_push_float  (Varying *vry, float   data);
void varying_fifo_push_Float2 (Varying *vry, Float2 *data);
void varying_fifo_push_Float3 (Varying *vry, Float3 *data);
void varying_fifo_push_Float4 (Varying *vry, Float4 *data);

fixpt_t varying_fifo_pop_fixpt  (Varying *vry);
FixPt2  varying_fifo_pop_FixPt2 (Varying *vry);
FixPt3  varying_fifo_pop_FixPt3 (Varying *vry);
FixPt4  varying_fifo_pop_FixPt4 (Varying *vry);

float   varying_fifo_pop_float  (Varying *vry);
Float2  varying_fifo_pop_Float2 (Varying *vry);
Float3  varying_fifo_pop_Float3 (Varying *vry);
Float4  varying_fifo_pop_Float4 (Varying *vry);




static inline fixpt_t fixpt_from_screenxy (screenxy_t a) {
	fixpt_t c = ((fixpt_t) a) << XY_FRACT_BITS;
	return c;
}

static inline screenxy_t fixpt_to_screenxy (fixpt_t a) {
	/*bool round_up = (a >> (FRACT_BITS-1)) & 1;
	
	screenz_t c = a >> FRACT_BITS;
	return (round_up) ? ++c : c;*/
	return (screenxy_t) (a >> XY_FRACT_BITS);
}


static inline fixpt_t fixpt_from_screenz (screenz_t a) {
	fixpt_t c = ((fixpt_t) a) << Z_FRACT_BITS;
	return c;
}

static inline screenz_t fixpt_to_screenz (fixpt_t a) {
	//bool round_up = (a >> (FRACT_BITS-1)) & 1;
	
	//screenz_t c = a >> FRACT_BITS;
	//return (round_up) ? ++c : c;
	return (screenz_t) (a >> Z_FRACT_BITS);
}

/*
pixel_color_t get_rgb_from_texture       (const Object *obj, const int u, const int v);
Float3        get_normal_from_map        (const Object *obj, const int u, const int v);
int           get_specularity_from_map   (const Object *obj, const int u, const int v);
*/

pixel_color_t get_pixel_color_from_bitmap   (const Bitmap *bmp, const int u, const int v);
Float3        get_norm_Float3_from_bitmap (const Bitmap *bmp, const int u, const int v);
int32_t       get_int32_from_bitmap       (const Bitmap *bmp, const int u, const int v);


BoundBox get_tri_boundbox (fixpt_t x[3], fixpt_t y[3]);
BoundBox clip_boundbox_to_screen (BoundBox in);
BoundBox clip_boundbox_to_tile (BoundBox in, size_t tile_num);

FixPt3 get_bar_coords (fixpt_t x[3], fixpt_t y[3], fixpt_t px, fixpt_t py);

Varying interpolate_varying (Varying vry[3], fixpt_t w_reciprocal[3], FixPt3 *bar);

screenz_t interpolate_z (fixpt_t z[3], FixPt3 *bar);

void tiler (TrianglePShaderData *tri, TriangleListNode *tri_ptr[]);
