#pragma once

#include "bitmap.h"
#include "geometry.h"
#include "geometry_fixpt.h"
#include "wavefront_obj.h"

#include <gpu_cfg.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>




#define DEBUG_FIXPT_VARYING 0
#define DEBUG_FIXPT_W       0

#define FLOAT 0



#define MAX_NUM_OF_LIGHTS	1
#define NUM_OF_VARYING_WORDS 28 // must be multiple of 4

#define TILE_WIDTH  32
#define TILE_HEIGHT 32


typedef struct gpu_cfg_t gpu_cfg_t;

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


typedef struct VaryingWord {
#ifdef DEBUG_FIXPT_VARYING
	float    as_float;
//#else
//typedef union VaryingWord {
#endif
	fixpt_t  as_fixpt_t;
} VaryingWord;


typedef VaryingWord VaryingData [NUM_OF_VARYING_WORDS];

typedef struct Varying {
	uint16_t num_of_words_written;
	uint16_t num_of_words_read;
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
//} __attribute__ ((aligned (1024))) TrianglePShaderData;
} TrianglePShaderData;
//#pragma align_to(1024,stack)

typedef struct TriangleListNode {
	volatile struct TrianglePShaderData* volatile tri;
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


typedef Float4 (*vertex_shader_fptr) (Object *obj, size_t face_idx, size_t vtx_idx, Varying *var, gpu_cfg_t *cfg);
typedef bool   (*pixel_shader_fptr)  (Object *obj, Varying *var, pixel_color_t *color, gpu_cfg_t *cfg);


struct gpu_cfg_t {

	volatile ObjectListNode* volatile obj_list_ptr;
	
	volatile TrianglePShaderData *volatile *volatile tri_ptr_list[NUM_OF_VSHADERS];
	volatile TrianglePShaderData *tri_for_pshader[NUM_OF_VSHADERS];
	
	volatile pixel_color_t *volatile active_fbuffer;
	
	volatile pixel_color_t *fbuffer_ptr[MAX_NUM_OF_FRAMEBUFFERS];
	volatile screenz_t     *zbuffer_ptr;
	
	volatile vertex_shader_fptr vshader_fptr;
	volatile  pixel_shader_fptr pshader_fptr;
	
	Light lights_arr[MAX_NUM_OF_LIGHTS];
	
	volatile fmat4 viewport;
		
	volatile bool vshaders_run_req;
	volatile bool vshaders_stop_req;
	volatile bool vshader_done[NUM_OF_VSHADERS];
	
	volatile bool pshaders_run_req;
	volatile bool pshaders_stop_req;
	volatile bool pshader_done[NUM_OF_PSHADERS];
	
	uint32_t num_of_vshaders;
	uint32_t num_of_pshaders;
	uint32_t num_of_ushaders;
	uint32_t num_of_tiles;
	uint32_t num_of_fbuffers;
	
	uint32_t screen_width;
	uint32_t screen_height;
	uint32_t screen_depth;
	
	volatile uint32_t tile_width;
	volatile uint32_t tile_height;
	
};

typedef struct shader_cfg_t {
	volatile gpu_cfg_t *common_cfg;
	uint32_t   shader_num;
} shader_cfg_t;

Light light_turn_on  (Float3 dir, bool add_shadow_buf, gpu_cfg_t *cfg);
void  light_turn_off (Light *l);

void   set_screen_size   (gpu_cfg_t *cfg, size_t width, size_t height);
size_t get_screen_width  (gpu_cfg_t *cfg);
size_t get_screen_height (gpu_cfg_t *cfg);
size_t get_screen_depth  (gpu_cfg_t *cfg);
size_t get_tile_width    (gpu_cfg_t *cfg);
size_t get_tile_height   (gpu_cfg_t *cfg);

void init_view             (fmat4 *m, Float3 *eye, Float3 *center, Float3 *up);
void init_perspective_proj (fmat4 *m, float left, float right, float top, float bot, float near, float far);
void init_ortho_proj       (fmat4 *m, float left, float right, float top, float bot, float near, float far);
void init_viewport         (fmat4 *m, int x, int y, int w, int h, int d);

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

pixel_color_t get_pixel_color_from_bitmap (const Bitmap *bmp, const int u, const int v);
Float3        get_norm_Float3_from_bitmap (const Bitmap *bmp, const int u, const int v);
int32_t       get_int32_from_bitmap       (const Bitmap *bmp, const int u, const int v);


BoundBox get_tri_boundbox        (fixpt_t x[3], fixpt_t y[3]);
BoundBox clip_boundbox_to_screen (BoundBox in, gpu_cfg_t *cfg);
BoundBox clip_boundbox_to_tile   (size_t tile_num, BoundBox in, gpu_cfg_t *cfg);

FixPt3 get_bar_coords (fixpt_t x[3], fixpt_t y[3], fixpt_t px, fixpt_t py);
