#pragma once

//#include "bitmap.h"
#include <geometry.h>
#include <wavefront_obj.h>

//#include <gpu_cfg.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>






#define GPU_MAX_LIGHTS	1
#define GPU_MAX_VARYINGS 28 // must be multiple of 4
#define GPU_MAX_TRIANGLES_PER_TILE_LOG2 11
#define GPU_MAX_TRIANGLES_PER_TILE (1 << GPU_MAX_TRIANGLES_PER_TILE_LOG2)
#define GPU_TILE_WIDTH_LOG2 5
#define GPU_TILE_HEIGHT_LOG2 5
#define GPU_TILE_WIDTH  (1 << GPU_TILE_WIDTH_LOG2)
#define GPU_TILE_HEIGHT (1 << GPU_TILE_HEIGHT_LOG2)
#define GPU_MAX_SCREEN_WIDTH  1024
#define GPU_MAX_SCREEN_HEIGHT 1024
#define GPU_MAX_TILES ((GPU_MAX_SCREEN_WIDTH / GPU_TILE_WIDTH) * (GPU_MAX_SCREEN_HEIGHT / GPU_TILE_HEIGHT))
#define GPU_MAX_USHADERS 4
#define GPU_MAX_FRAMEBUFFERS	64
#define GPU_CFG_ABS_ADDRESS 0xFFFE0000



#define DEBUG_FIXPT_VARYING 0
#define DEBUG_FIXPT_W       0


///////////////////////////////////////////////////////////////////////////////////
typedef  int32_t  fixpt_t;
typedef  int64_t dfixpt_t;

#define  FIXPT_BITS (sizeof( fixpt_t) * 8)
#define DFIXPT_BITS (sizeof(dfixpt_t) * 8)

#define       XY_FRACT_BITS 4
#define        Z_FRACT_BITS 4
#define     BARC_FRACT_BITS (XY_FRACT_BITS*2)
#define W_RECIPR_FRACT_BITS 29
#define     OOWI_FRACT_BITS 16
#define  VARYING_FRACT_BITS 14




/*
typedef fixpt_t fixpt_t1[1];
typedef float   float_t1[1];

typedef struct FixPtValue {
	fixpt_t value;
} FixPtValue;
typedef struct FloatValue {
	float value;
} FloatValue;

typedef union FixPt1 {
	fixpt_t1   as_array;
	FixPtValue as_struct;
} FixPt1;
*/

typedef fixpt_t fixpt_t2[2];
typedef struct FixPtUV {
	fixpt_t u, v;
} FixPtUV;
typedef union FixPt2 {
	fixpt_t2 as_array;
	FixPtUV as_struct;
} FixPt2;


typedef fixpt_t fixpt_t3[3];
typedef struct FixPtXYZ {
	fixpt_t x, y, z;
} FixPtXYZ;
typedef union FixPt3 {
	fixpt_t3 as_array;
	FixPtXYZ as_struct;
} FixPt3;


typedef fixpt_t fixpt_t4[4];
typedef struct FixPtXYZW {
	fixpt_t x, y, z, w;
} FixPtXYZW;
typedef union FixPt4 {
	fixpt_t4 as_array;
	FixPtXYZW as_struct;
} FixPt4;


static inline fixpt_t fixpt_get_min (void) {
	return 0x80000000;
}

static inline fixpt_t fixpt_get_max (void) {
	return 0x7FFFFFFF;
}

static inline fixpt_t fixpt_from_float (float a, uint8_t fract_bits) {
	//~ //fixpt_t c = (fixpt_t) roundf (a * (1 << fract_bits));
	//~ fixpt_t c = (fixpt_t) (roundf (a) * (1 << fract_bits)); // TBD - this is too slow
	fixpt_t c = (fixpt_t) (a * (1 << fract_bits)); // TBD
	return c;
}
	
	
//~ static inline fixpt_t fixpt_from_float_no_rnd (float a, uint8_t fract_bits) {
	//~ fixpt_t c = (fixpt_t) (a * (1 << fract_bits));
	//~ return c;
//~ }

static inline float fixpt_to_float (fixpt_t a, uint8_t fract_bits) {
	assert (fract_bits < 32);
	float tmp = (float) (1 << fract_bits);
	assert (tmp != 0);
	return ((float) a) / tmp;
}

static inline float dfixpt_to_float (dfixpt_t a, uint8_t dfract_bits) {
	assert (dfract_bits < 64);
	float tmp = (float) (1L << dfract_bits);
	assert (tmp != 0);
	return ((float) a) / tmp;
}

static inline FixPt2 Float2_FixPt2_conv (Float2 *in) {
	FixPt2 fx;
	for (int k = 0; k < 2; k++) {
		fx.as_array[k] = fixpt_from_float (in->as_array[k], VARYING_FRACT_BITS);
	}
	return fx;
}

static inline Float2 FixPt2_Float2_conv (FixPt2 *in) {
	Float2 f;
	for (int k = 0; k < 2; k++) {
		f.as_array[k] = fixpt_to_float (in->as_array[k], VARYING_FRACT_BITS);
	}
	return f;
}

static inline FixPt3 Float3_FixPt3_conv (Float3 *in) {
	FixPt3 fx;
	for (int k = 0; k < 3; k++) {
		fx.as_array[k] = fixpt_from_float (in->as_array[k], VARYING_FRACT_BITS);
	}
	return fx;
}

static inline Float3 FixPt3_Float3_conv (FixPt3 *in) {
	Float3 f;
	for (int k = 0; k < 3; k++) {
		f.as_array[k] = fixpt_to_float (in->as_array[k], VARYING_FRACT_BITS);
	}
	return f;
}

static inline FixPt4 Float4_FixPt4_conv (Float4 *in) {
	FixPt4 fx;
	for (int k = 0; k < 4; k++) {
		fx.as_array[k] = fixpt_from_float (in->as_array[k], VARYING_FRACT_BITS);
	}
	return fx;
}

static inline Float4 FixPt4_Float4_conv (FixPt4 *in) {
	Float4 f;
	for (int k = 0; k < 4; k++) {
		f.as_array[k] = fixpt_to_float (in->as_array[k], VARYING_FRACT_BITS);
	}
	return f;
}

static inline fixpt_t fixpt_add (fixpt_t a, fixpt_t b) {
	fixpt_t c = a + b;
	assert (((a > 0) && (b > 0)) ? (c > 0) : 1);
	assert (((a < 0) && (b < 0)) ? (c < 0) : 1);
	return c;
}

static inline fixpt_t fixpt_sub (fixpt_t a, fixpt_t b) {
	fixpt_t c = a - b;
	assert (((a > 0) && (b < 0)) ? (c > 0) : 1);
	assert (((a < 0) && (b > 0)) ? (c < 0) : 1);
	return c;
}

static inline FixPt3 FixPt3_FixPt3_add (FixPt3 a, FixPt3 b) {
	FixPt3 c;
	for (int i = 0; i < 3; i++) {
		c.as_array[i] = a.as_array[i] + b.as_array[i];
	}
	return c;
}

///////////////////////////////////////////////////////////////////////////////////

typedef struct gpu_cfg_t gpu_cfg_t;

typedef enum {X = 0, Y, Z, W} axis;
typedef enum {VARYING_FLOAT = 0, VARYING_FIXPT} varying_type;


// If we clip only those triangles which are completely invisible, screenxy_t must be signed
// If we clip all the trianlges which are not completely visible, screenxy_t can be unsigned
typedef int16_t screenxy_t;

typedef uint16_t screenz_t;
//typedef fixpt_t screenz_t;
//typedef uint8_t screenz_t;


typedef struct Bitmap {
	uint8_t *data;
	uint32_t w;
	uint32_t h;
	uint32_t bytespp;
} Bitmap;

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


typedef VaryingWord VaryingData [GPU_MAX_VARYINGS];

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
	fmat4  shadow_mvp[GPU_MAX_LIGHTS];
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
} __attribute__ ((aligned (1024))) TrianglePShaderData;
//#pragma align_to(1024,stack)



typedef struct Light {
	bool 		enabled;
	Float3		dir;
	Float3		src;
	Float3		eye;
	bool		has_shadow_buf;
	screenz_t	*shadow_buf;
} Light;


typedef Float4 (*vertex_shader_fptr) (Object *obj, size_t face_idx, size_t vtx_idx, Varying *var, gpu_cfg_t *cfg);
typedef bool   (*pixel_shader_fptr)  (Object *obj, Varying *var, Light *lights_arr, uint32_t screen_width, uint32_t screen_height, pixel_color_t *color);


struct gpu_cfg_t {

	volatile ObjectListNode* volatile obj_list_ptr;
	
	volatile TrianglePShaderData *volatile *tri_ptr_list[GPU_MAX_USHADERS];
	volatile TrianglePShaderData *volatile tri_for_pshader[GPU_MAX_USHADERS];
	
	volatile pixel_color_t *volatile active_fbuffer;
	
	volatile pixel_color_t *fbuffer_ptr[GPU_MAX_FRAMEBUFFERS];
	volatile screenz_t     *zbuffer_ptr;
	
	volatile vertex_shader_fptr vshader_fptr;
	volatile  pixel_shader_fptr pshader_fptr;
	
	volatile Light lights_arr[GPU_MAX_LIGHTS];
	
	volatile fmat4 viewport;
		
	volatile bool vshaders_run_req;
	volatile bool vshaders_stop_req;
	volatile bool vshader_done[GPU_MAX_USHADERS];
	
	volatile bool pshaders_run_req;
	volatile bool pshaders_stop_req;
	volatile bool pshader_done[GPU_MAX_USHADERS];
	
//	volatile uint32_t num_of_vshaders;
//	volatile uint32_t num_of_pshaders;
	volatile uint32_t num_of_ushaders;
	volatile uint32_t num_of_tiles;
	volatile uint32_t num_of_fbuffers;
	
	volatile uint32_t screen_width;
	volatile uint32_t screen_height;
	volatile uint32_t screen_depth;
	
	//volatile uint32_t tile_width;
	//volatile uint32_t tile_height;
	
};

typedef struct shader_cfg_t {
	gpu_cfg_t *common_cfg;
	uint32_t   shader_num;
} shader_cfg_t;

size_t get_screen_width  (gpu_cfg_t *cfg);
size_t get_screen_height (gpu_cfg_t *cfg);
size_t get_screen_depth  (gpu_cfg_t *cfg);

pixel_color_t set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a);

extern void varying_fifo_push_float  (Varying *vry, float   data);
extern void varying_fifo_push_Float2 (Varying *vry, Float2 *data);
extern void varying_fifo_push_Float3 (Varying *vry, Float3 *data);
extern void varying_fifo_push_Float4 (Varying *vry, Float4 *data);

extern fixpt_t varying_fifo_pop_fixpt  (Varying *vry);
extern FixPt2  varying_fifo_pop_FixPt2 (Varying *vry);
extern FixPt3  varying_fifo_pop_FixPt3 (Varying *vry);
extern FixPt4  varying_fifo_pop_FixPt4 (Varying *vry);

extern float   varying_fifo_pop_float  (Varying *vry);
extern Float2  varying_fifo_pop_Float2 (Varying *vry);
extern Float3  varying_fifo_pop_Float3 (Varying *vry);
extern Float4  varying_fifo_pop_Float4 (Varying *vry);

extern pixel_color_t get_pixel_color_from_bitmap (const Bitmap *bmp, const int u, const int v);
extern Float3        get_norm_Float3_from_bitmap (const Bitmap *bmp, const int u, const int v);
extern int32_t       get_int32_from_bitmap       (const Bitmap *bmp, const int u, const int v);




///////// PRIVATE

typedef struct ScreenPt {
	screenxy_t x;
	screenxy_t y;
	//screenz_t  z;
} ScreenPt;

typedef struct BoundBox {
	ScreenPt min;
	ScreenPt max;
} BoundBox;


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

BoundBox get_tri_boundbox        (fixpt_t x[3], fixpt_t y[3]);
BoundBox clip_boundbox_to_screen (BoundBox in, gpu_cfg_t *cfg);
BoundBox clip_boundbox_to_tile   (size_t tile_num, BoundBox in, gpu_cfg_t *cfg);

FixPt3 get_bar_coords (fixpt_t x[3], fixpt_t y[3], fixpt_t px, fixpt_t py);
