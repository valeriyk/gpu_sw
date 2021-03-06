#pragma once

#ifdef ARC_APEX
	#include <apexextensions.h>
#endif
#include <assert.h>
#include <geometry.h>
#include <hasha.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wavefront_obj.h>



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

#define GPU_CFG_ABS_ADDRESS      0xFFFE0000


#define DEBUG_FIXPT_VARYING 0
#define DEBUG_FIXPT_W       0

enum {
	HASHA_HOST_TO_USHADER_MST = 0,
	HASHA_HOST_TO_VIDEOCTRL_MST
};
enum {
	HASHA_HOST_TO_USHADER_SLV,
};
enum {
	HASHA_USHADER_UPSTREAM_MST = 0,
	HASHA_USHADER_DOWNSTREAM_MST
};
enum {
	HASHA_USHADER_UPSTREAM_SLV = 0,
	HASHA_USHADER_DOWNSTREAM_SLV
};

enum {
	HASHA_VIDEOCTRL_SLV = 0
};



///////////////////////////////////////////////////////////////////////////////////
typedef   int16_t  hfixpt_t;
typedef  uint16_t uhfixpt_t;
typedef   int32_t   fixpt_t;
typedef  uint32_t  ufixpt_t;
typedef   int64_t  dfixpt_t;

#define HFIXPT_BITS (sizeof(hfixpt_t) * 8)
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

typedef hfixpt_t hfixpt_t3[3];
typedef struct hFixPtXYZ {
	hfixpt_t x, y, z;
} hFixPtXYZ;
typedef union hFixPt3 {
	hfixpt_t3 as_array;
	hFixPtXYZ as_struct;
} hFixPt3;

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
static inline fixpt_t fixpt_from_float_fast (float *a, uint8_t fract_bits) {
	//~ //fixpt_t c = (fixpt_t) roundf (a * (1 << fract_bits));
	//~ fixpt_t c = (fixpt_t) (roundf (a) * (1 << fract_bits)); // TBD - this is too slow
	fixpt_t c = (fixpt_t) (*a * (1 << fract_bits)); // TBD
	return c;
}

static inline hfixpt_t hfixpt_from_float (float a, uint8_t fract_bits) {
	hfixpt_t c = (hfixpt_t) (a * (1 << fract_bits)); // TBD
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
typedef uint16_t screenxy_t;

typedef struct xy_uhfixpt_t {
	uhfixpt_t x;
	uhfixpt_t y;
} xy_uhfixpt_t;

typedef union xy_uhfixpt_pck_t {
	uint32_t     as_word;
	xy_uhfixpt_t as_coord;
} xy_uhfixpt_pck_t;

typedef uint16_t screenz_t;
//typedef fixpt_t screenz_t;
//typedef uint8_t screenz_t;


typedef struct Bitmap {
	uint8_t *data;
	uint32_t w;
	uint32_t h;
	uint32_t bytespp;
} Bitmap;

/*
typedef struct Bitmap32 {
	uint32_t *data;
	uint32_t w;
	uint32_t h;
} Bitmap32;
*/

/*
typedef struct rgb24_t {
	uint8_t r, g, b;
} rgb24_t;

typedef struct rgb32_t {
	uint8_t r, g, b, a;
} rgb32_t;
*/
//typedef rgb32_t pixel_color_t;

typedef struct rgba8888 {
	uint8_t r, g, b, a;
	//uint8_t a, r, g, b;
	//uint8_t a, b, g, r;
} rgba8888;

typedef union pixel_color_t {
	rgba8888 as_byte;
	uint32_t as_word;
}  __attribute__ ((aligned (4))) pixel_color_t;

/*typedef struct pixel_color32_t {
	uint8_t r, g, b, a;
} pixel_color32_t;
*/
typedef struct Triangle {
	Float4 vtx[3];
} Triangle;


typedef struct VaryingWord {
#ifdef DEBUG_FIXPT_VARYING
	float    as_float;
#else
typedef union VaryingWord {
	float    as_float;
#endif
	fixpt_t  as_fixpt_t;
} VaryingWord;


typedef VaryingWord VaryingData [GPU_MAX_VARYINGS];

typedef struct Varying {
	VaryingData data;
	uint8_t num_of_words_written;
	uint8_t num_of_words_read;
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
	fixpt_t w_reciprocal[3];
	Varying varying[3];
	Object  *obj;
	//hFixPt3  screen_coords[3];
	//hfixpt_t screen_x[3];
	//hfixpt_t screen_y[3];
	xy_uhfixpt_pck_t vtx_a;
	xy_uhfixpt_pck_t vtx_b;
	xy_uhfixpt_pck_t vtx_c;
	//fixpt_t screen_z[3];
	fixpt_t z0;
	fixpt_t z1z0_over_sob;
	fixpt_t z2z0_over_sob;
} __attribute__ ((aligned (1024))) TrianglePShaderData;
//#pragma align_to(1024,stack)

typedef struct TriangleTileData {
	volatile TrianglePShaderData *volatile data;
	//volatile FixPt3 bar;
	//volatile fixpt_t z0;
	//volatile fixpt_t z1z0_over_sob;
	//volatile fixpt_t z2z0_over_sob;
} TriangleTileData;


typedef struct Light {
	bool 		enabled;
	Float3		dir;
	Float3		src;
	Float3		eye;
	bool		has_shadow_buf;
	screenz_t	*shadow_buf;
} Light;


typedef Float4 (*vertex_shader_fptr) (Object *obj, VtxAttr *attribs, Varying *var, gpu_cfg_t *cfg);
typedef bool   (*pixel_shader_fptr)  (Object *obj, Varying *var, gpu_cfg_t *cfg, pixel_color_t *color);


struct gpu_cfg_t {

	ObjectListNode* volatile obj_list_ptr;
	
	//TrianglePShaderData *volatile *tri_ptr_list[GPU_MAX_USHADERS];
	TriangleTileData *tri_ptr_list[GPU_MAX_USHADERS];
	TrianglePShaderData *volatile tri_for_pshader[GPU_MAX_USHADERS];
	
	pixel_color_t *volatile   active_fbuffer;
	pixel_color_t *volatile inactive_fbuffer;
	
	pixel_color_t *fbuffer_ptr[GPU_MAX_FRAMEBUFFERS];
	screenz_t     *zbuffer_ptr;
	
	vertex_shader_fptr vshader_fptr;
	pixel_shader_fptr pshader_fptr;
	
	Light lights_arr[GPU_MAX_LIGHTS];
	
	fmat4 viewport;
	
//	volatile uint32_t num_of_vshaders;
//	volatile uint32_t num_of_pshaders;
	uint32_t num_of_ushaders;
	uint32_t num_of_tiles;
	uint32_t num_of_fbuffers;
	
	uint32_t screen_width;
	uint32_t screen_height;
	uint32_t screen_depth;
	
	//volatile uint32_t tile_width;
	//volatile uint32_t tile_height;
	
};

typedef struct pthread_cfg_t {
	gpu_cfg_t      *common_cfg;
	hasha_block_t  *hasha_block_ptr;
	uint32_t        core_num;
} pthread_cfg_t;

size_t get_screen_width  (gpu_cfg_t *cfg);
size_t get_screen_height (gpu_cfg_t *cfg);
size_t get_screen_depth  (gpu_cfg_t *cfg);

pixel_color_t set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a);
pixel_color_t color_mult (pixel_color_t pix, fixpt_t intensity);

extern void varying_fifo_push_float  (Varying *vry, float   data);
extern void varying_fifo_push_Float2 (Varying *vry, Float2 *data);
extern void varying_fifo_push_Float3 (Varying *vry, Float3 *data);
extern void varying_fifo_push_Float4 (Varying *vry, Float4 *data);

static inline void varying_fifo_push_float_fast  (Varying *vry, float *data) {
	size_t idx = vry->num_of_words_written;

	vry->data[idx].as_fixpt_t = fixpt_from_float_fast (data, VARYING_FRACT_BITS); // no rounding
	if (DEBUG_FIXPT_VARYING) {
		vry->data[idx].as_float = *data;
	}


	vry->num_of_words_written++;
}

static inline void varying_fifo_push_Float2_fast (Varying *vry, Float2 *data) {
	for (int i = 0; i < 2; i++) {
		varying_fifo_push_float_fast (vry, &data->as_array[i]);
	}
}

static inline void varying_fifo_push_Float3_fast (Varying *vry, Float3 *data) {
	for (int i = 0; i < 3; i++) {
		varying_fifo_push_float_fast (vry, &data->as_array[i]);
	}
}

static inline void varying_fifo_push_Float4_fast (Varying *vry, Float4 *data) {
	for (int i = 0; i < 4; i++) {
		varying_fifo_push_float_fast (vry, &data->as_array[i]);
	}
}

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

typedef struct bbox_uhfixpt_t {
	xy_uhfixpt_pck_t min;
	xy_uhfixpt_pck_t max;
} bbox_uhfixpt_t;


static inline fixpt_t fixpt_from_screenxy (screenxy_t a) {
	fixpt_t c = ((fixpt_t) a) << XY_FRACT_BITS;
	return c;
}

static inline hfixpt_t hfixpt_from_screenxy (screenxy_t a) {
	hfixpt_t c = ((hfixpt_t) a) << XY_FRACT_BITS;
	return c;
}

static inline screenxy_t fixpt_to_screenxy (fixpt_t a) {
	/*bool round_up = (a >> (FRACT_BITS-1)) & 1;
	
	screenz_t c = a >> FRACT_BITS;
	return (round_up) ? ++c : c;*/
	return (screenxy_t) (a >> XY_FRACT_BITS);
}

static inline screenxy_t hfixpt_to_screenxy (hfixpt_t a) {
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


//~ static inline hfixpt_t min_of_two (hfixpt_t a, hfixpt_t b) {
	//~ return (a < b) ? a : b;
//~ }

//~ static inline hfixpt_t max_of_two (hfixpt_t a, hfixpt_t b) {
	//~ return (a > b) ? a : b;
//~ }

//~ static inline hfixpt_t min_of_three (hfixpt_t a, hfixpt_t b, hfixpt_t c) {
	//~ return min_of_two(a, min_of_two(b, c));
//~ }

//~ static hfixpt_t max_of_three (hfixpt_t a, hfixpt_t b, hfixpt_t c) {
	//~ return max_of_two(a, max_of_two(b, c));
//~ }



static inline uhfixpt_t min_of_two_uh (uhfixpt_t a, uhfixpt_t b) {
	return (a < b) ? a : b;
}

static inline uhfixpt_t max_of_two_uh (uhfixpt_t a, uhfixpt_t b) {
	return (a > b) ? a : b;
}


static inline xy_uhfixpt_pck_t get_min_xy (xy_uhfixpt_pck_t a, xy_uhfixpt_pck_t b) {
	
	xy_uhfixpt_pck_t min;

#ifdef ARC_APEX    
	min.as_word = min_xy (a.as_word, b.as_word);
#else
    min.as_coord.x = min_of_two_uh (a.as_coord.x, b.as_coord.x) & 0xfff0;
    min.as_coord.y = min_of_two_uh (a.as_coord.y, b.as_coord.y) & 0xfff0;
#endif

    return min;
}

static inline xy_uhfixpt_pck_t get_max_xy (xy_uhfixpt_pck_t a, xy_uhfixpt_pck_t b) {
	
	xy_uhfixpt_pck_t max;

#ifdef ARC_APEX 
	max.as_word = max_xy (a.as_word, b.as_word);
#else    
    max.as_coord.x = max_of_two_uh (a.as_coord.x, b.as_coord.x);
    max.as_coord.y = max_of_two_uh (a.as_coord.y, b.as_coord.y);
#endif

    return max;
}


//~ BoundBox get_tri_boundbox        (hfixpt_t x[3], hfixpt_t y[3]);
//~ //bbox_uhfixpt_t get_tri_bbox  (xy_uhfixpt_pck_t a, xy_uhfixpt_pck_t b, xy_uhfixpt_pck_t c);
//~ static inline bbox_uhfixpt_t get_tri_bbox (xy_uhfixpt_pck_t a, xy_uhfixpt_pck_t b, xy_uhfixpt_pck_t c) {
	
	//~ bbox_uhfixpt_t bb;
    
    //~ bb.min.as_coord.x = min_of_three (a.as_coord.x, b.as_coord.x, c.as_coord.x);// & 0xfff0;
    //~ bb.max.as_coord.x = max_of_three (a.as_coord.x, b.as_coord.x, c.as_coord.x);// & 0xfff0;
    //~ bb.min.as_coord.y = min_of_three (a.as_coord.y, b.as_coord.y, c.as_coord.y);// & 0xfff0;
    //~ bb.max.as_coord.y = max_of_three (a.as_coord.y, b.as_coord.y, c.as_coord.y);// & 0xfff0;
    
    //~ return bb;
//~ }


static inline bbox_uhfixpt_t get_tile_bbox (size_t tile_num, gpu_cfg_t *cfg) {
	
	bbox_uhfixpt_t tile;
	
	uint16_t llx = (tile_num % (get_screen_width(cfg) >> GPU_TILE_WIDTH_LOG2)) << GPU_TILE_WIDTH_LOG2;
	uint16_t lly = (tile_num / (get_screen_width(cfg) >> GPU_TILE_WIDTH_LOG2)) << GPU_TILE_HEIGHT_LOG2;			
	tile.min.as_coord.x = llx << XY_FRACT_BITS;
    tile.min.as_coord.y = lly << XY_FRACT_BITS;
    tile.max.as_coord.x = (llx + GPU_TILE_WIDTH  - 1) << XY_FRACT_BITS; 
    tile.max.as_coord.y = (lly + GPU_TILE_HEIGHT - 1) << XY_FRACT_BITS;
    
    return tile;
}

static inline bbox_uhfixpt_t get_screen_bbox (gpu_cfg_t *cfg) {
	
	bbox_uhfixpt_t screen;
	
	screen.min.as_coord.x = 0;
    screen.min.as_coord.y = 0;
    screen.max.as_coord.x = (get_screen_width (cfg) - 1) << XY_FRACT_BITS;
    screen.max.as_coord.y = (get_screen_height(cfg) - 1) << XY_FRACT_BITS;
    
    return screen;
}

//~ BoundBox clip_boundbox_to_screen (BoundBox in, gpu_cfg_t *cfg);
//~ //BoundBox clip_boundbox_to_tile   (size_t tile_num, BoundBox in, gpu_cfg_t *cfg);
//~ //bbox_uhfixpt_t clip_bbox_to_tile   (size_t tile_num, bbox_uhfixpt_t in, gpu_cfg_t *cfg);
//~ static inline bbox_uhfixpt_t clip_tri_bbox_to_tile (bbox_uhfixpt_t *tri, bbox_uhfixpt_t *tile) {
	
	//~ bbox_uhfixpt_t out;
	
	//~ out.min.as_coord.x = max_of_two (tile->min.as_coord.x, tri->min.as_coord.x) & 0xfff0;
    //~ out.max.as_coord.x = min_of_two (tile->max.as_coord.x, tri->max.as_coord.x);
    //~ out.min.as_coord.y = max_of_two (tile->min.as_coord.y, tri->min.as_coord.y) & 0xfff0;
    //~ out.max.as_coord.y = min_of_two (tile->max.as_coord.y, tri->max.as_coord.y);
        
    //~ return out;
//~ }

static inline bbox_uhfixpt_t clip_tri_to_bbox (xy_uhfixpt_pck_t a, xy_uhfixpt_pck_t b, xy_uhfixpt_pck_t c, bbox_uhfixpt_t *tile) {
	
	xy_uhfixpt_pck_t min_ab      = get_min_xy (a, b);
	xy_uhfixpt_pck_t min_abc     = get_min_xy (min_ab, c);
	xy_uhfixpt_pck_t min_clipped = get_max_xy (min_abc, tile->min);
	
	xy_uhfixpt_pck_t max_ab      = get_max_xy (a, b);
	xy_uhfixpt_pck_t max_abc     = get_max_xy (max_ab, c);
	xy_uhfixpt_pck_t max_clipped = get_min_xy (max_abc, tile->max);
	
	bbox_uhfixpt_t out;
	
	out.min = min_clipped;
    out.max = max_clipped;
        
    return out;
}


FixPt3 get_bar_coords (hfixpt_t x[3], hfixpt_t y[3], hfixpt_t px, hfixpt_t py);
fixpt_t edge_func_fixpt2 (xy_uhfixpt_pck_t a, xy_uhfixpt_pck_t b, xy_uhfixpt_pck_t c);
FixPt3 get_bar_coords2 (xy_uhfixpt_pck_t a, xy_uhfixpt_pck_t b, xy_uhfixpt_pck_t c, xy_uhfixpt_pck_t p);

#ifdef ARC_APEX
	
	typedef union apex_reg_32b {
		uint32_t u;
		 int32_t s;
	} apex_reg_32b; 

	static inline int32_t _core_read_bar0 () {
		apex_reg_32b r;
		r.u = _core_read (CR_BAR0);
		return r.s;
	}

	static inline void _core_write_bar0 (int32_t val) {
		apex_reg_32b r;
		r.s = val;
		_core_write (r.u, CR_BAR0);
	}

	static inline int32_t _core_read_bar1 () {
		apex_reg_32b r;
		r.u = _core_read (CR_BAR1);
		return r.s;
	}

	static inline void _core_write_bar1 (int32_t val) {
		apex_reg_32b r;
		r.s = val;
		_core_write (r.u, CR_BAR1);
	}

	static inline int32_t _core_read_bar2 () {
		apex_reg_32b r;
		r.u = _core_read (CR_BAR2);
		return r.s;
	}

	static inline void _core_write_bar2 (int32_t val) {
		apex_reg_32b r;
		r.s = val;
		_core_write (r.u, CR_BAR2);
	}
	
#endif

void    wfobj_get_attribs              (const WaveFrontObj *wfobj, const int face_idx, const int vtx_idx, VtxAttr *attribs);


