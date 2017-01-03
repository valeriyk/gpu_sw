#pragma once

#include "bitmap.h"
#include "geometry.h"
#include "geometry_fixpt.h"
#include "wavefront_obj.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>




#define FLOAT 1



#define MAX_NUM_OF_LIGHTS	1
#define NUM_OF_VARYING_WORDS 28 // must be multiple of 4
#define TILE_WIDTH  32
#define TILE_HEIGHT 16



typedef enum {X = 0, Y, Z, W} axis;


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


typedef float  varying_float  [NUM_OF_VARYING_WORDS];
//typedef Float2 varying_Float2 [NUM_OF_VARYING_WORDS/2];
//typedef Float3 varying_Float3 [NUM_OF_VARYING_WORDS/4];
//typedef Float4 varying_Float4 [NUM_OF_VARYING_WORDS/4];
/*
typedef union VaryingFloat {
	varying_float  as_float;
	varying_Float2 as_Float2;
	varying_Float4 as_Float4;
} VaryingFloat;
*/
typedef fixpt_t varying_fixpt_t [NUM_OF_VARYING_WORDS];
//typedef FixPt2  varying_FixPt2  [NUM_OF_VARYING_WORDS/2];
//typedef Float3 varying_Float3 [NUM_OF_VARYING_WORDS/4];
//typedef FixPt4  varying_FixPt4  [NUM_OF_VARYING_WORDS/4];

typedef union VaryingData {
	varying_fixpt_t  as_fixpt_t;
//	varying_FixPt2   as_FixPt2;
//	varying_FixPt4   as_FixPt4;
	varying_float    as_float;
//	varying_Float2   as_Float2;
//	varying_Float4   as_Float4;
} VaryingData;

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

typedef struct TriangleVtxListNode {
	FixPt3  screen_coords[3];
	nfixpt_t w_reciprocal[3];
	Varying varying[3];
	Object  *obj;
	struct TriangleVtxListNode *next;
} TriangleVtxListNode;

typedef struct TrianglePtrListNode {
	struct TriangleVtxListNode *tri;
	struct TrianglePtrListNode *next;
} TrianglePtrListNode;


typedef struct Light {
	bool 		enabled;
	Float3		dir;
	Float3		src;
	Float3		eye;
	screenz_t	*shadow_buf;
} Light;


extern fmat4 VIEWPORT;
extern Light LIGHTS[MAX_NUM_OF_LIGHTS];
//extern Bitmap TEXTURES[MAX_NUM_OF_TEXTURES];


typedef Float4 (*vertex_shader) (Object *obj, size_t face_idx, size_t vtx_idx, Varying *var);
typedef bool (*pixel_shader)  (Object *obj, Varying *var, pixel_color_t *color);



void new_light  (int light_num, Float3 dir);
void free_light (int light_num);
void init_scene (void);


//Light light[MAX_NUM_OF_LIGHTS];


void create_light (Float3 dir, screenz_t *shadow_buf, int light_num);

void   set_screen_size   (size_t width, size_t height);
size_t get_screen_width  (void);
size_t get_screen_height (void);
size_t get_screen_depth  (void);

//int   orient2d (ScreenPt *a, ScreenPt *b, ScreenPt *c);

//void  world2screen (float4 &w, ScreenPt &s);

//screenxy_t tri_min_bound (screenxy_t a, screenxy_t b, screenxy_t c, screenxy_t cutoff);
//screenxy_t tri_max_bound (screenxy_t a, screenxy_t b, screenxy_t c, screenxy_t cutoff);

void init_view             (fmat4 *m, Float3 *eye, Float3 *center, Float3 *up);
void init_perspective_proj (fmat4 *m, float left, float right, float top, float bot, float near, float far);
void init_ortho_proj       (fmat4 *m, float left, float right, float top, float bot, float near, float far);
void init_viewport   (int x, int y, int w, int h, int d);

void rotate_coords (fmat4 *in, fmat4 *out, float alpha_deg, axis axis);

void draw_triangle (TriangleVtxListNode *tri, size_t tile_num, pixel_shader shader, screenz_t *zbuffer, pixel_color_t *fbuffer);

pixel_color_t set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a);

//void write_fbuffer (pixel_color_t *fbuffer, int x, int y, pixel_color_t *val);
//void write_zbuffer (screenz_t     *zbuffer, int x, int y, screenz_t     *val);



Object* obj_new (WaveFrontObj *wfobj, Bitmap *texture, Bitmap *normalmap, Bitmap *specularmap);
void obj_set_scale       (Object *obj, float x,     float y,     float z);
void obj_set_rotation    (Object *obj, float x_deg, float y_deg, float z_deg);
void obj_set_translation (Object *obj, float x,     float y,     float z);
void obj_init_model      (Object *obj);
//void obj_transform       (Object *obj, fmat4 *vpv, fmat4 *projview, float3 *light_dir);
void draw_frame          (ObjectListNode *obj_list, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer);




void varying_fifo_push_float  (Varying *vry, float   data);
void varying_fifo_push_Float2 (Varying *vry, Float2 *data);
void varying_fifo_push_Float3 (Varying *vry, Float3 *data);
void varying_fifo_push_Float4 (Varying *vry, Float4 *data);

/*fixpt_t varying_pop_fixpt  (Varying *vry);
FixPt2  varying_pop_FixPt2 (Varying *vry);
FixPt3  varying_pop_FixPt3 (Varying *vry);
FixPt4  varying_pop_FixPt4 (Varying *vry);
*/

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
