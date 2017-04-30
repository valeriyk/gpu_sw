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

void set_screen_size (size_t width, size_t height) {
	SCREEN_WIDTH  = width;
	SCREEN_HEIGHT = height;
	SCREEN_DEPTH  = (screenz_t) ~0; // all ones
	
	NUM_OF_TILES = (SCREEN_WIDTH / TILE_WIDTH) * (SCREEN_HEIGHT / TILE_HEIGHT);
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

screenz_t interpolate_z (fixpt_t z[3], FixPt3 *bar) {
	
	dfixpt_t mul_0 = (int64_t) z[0] * (int64_t) bar->as_array[0]; // ZM.ZF * BM.BF = (ZM+BM).(ZF+BF)
	dfixpt_t mul_1 = (int64_t) z[1] * (int64_t) bar->as_array[1]; // ZM.ZF * BM.BF = (ZM+BM).(ZF+BF)
	dfixpt_t mul_2 = (int64_t) z[2] * (int64_t) bar->as_array[2]; // ZM.ZF * BM.BF = (ZM+BM).(ZF+BF)
	dfixpt_t acc = mul_0 + mul_1 + mul_2; // (ZM+BM).(ZF+BF) + (ZM+BM).(ZF+BF) + (ZM+BM).(ZF+BF) = (ZM+BM+1).(ZF+BF)
	dfixpt_t sum_of_bars = bar->as_array[0] + bar->as_array[1] + bar->as_array[2]; // BM.BF + BM.BF + BM.BF = (BM+1).BF
	dfixpt_t res = acc / sum_of_bars; // (ZM+BM+1).(ZF+BF) / (BM+1).BF = ZM.ZF
	
	return fixpt_to_screenz ((fixpt_t) res); // ZM
}

dfixpt_t interpolate_w (fixpt_t w_reciprocal[3], FixPt3 *bar) {
    	
	dfixpt_t mul_0 = (dfixpt_t) bar->as_array[0] * (dfixpt_t) w_reciprocal[0]; // BM.BF * WM.WF = (BM+WM).(BF+WF)
	dfixpt_t mul_1 = (dfixpt_t) bar->as_array[1] * (dfixpt_t) w_reciprocal[1]; // BM.BF * WM.WF = (BM+WM).(BF+WF)
	dfixpt_t mul_2 = (dfixpt_t) bar->as_array[2] * (dfixpt_t) w_reciprocal[2]; // BM.BF * WM.WF = (BM+WM).(BF+WF)
	dfixpt_t acc   = mul_0 + mul_1 + mul_2; // (BM+WM).(BF+WF) + (BM+WM).(BF+WF) + (BM+WM).(BF+WF) = (BM+WM+1).(BF+WF)
	assert (acc != 0);
	dfixpt_t one = (1L << (OOWI_FRACT_BITS + BARC_FRACT_BITS + W_RECIPR_FRACT_BITS)); // 1.(WI+BF+WF)
	dfixpt_t res = one / acc; // 1.(WI+BF+WF) / (BM+WM+1).(BF+WF) = (64-WI).WI

	if (DEBUG_FIXPT_W) {
		float   mul_0f = fixpt_to_float (bar->as_array[0], BARC_FRACT_BITS) * fixpt_to_float (w_reciprocal[0], W_RECIPR_FRACT_BITS);
		float   mul_1f = fixpt_to_float (bar->as_array[1], BARC_FRACT_BITS) * fixpt_to_float (w_reciprocal[1], W_RECIPR_FRACT_BITS);
		float   mul_2f = fixpt_to_float (bar->as_array[2], BARC_FRACT_BITS) * fixpt_to_float (w_reciprocal[2], W_RECIPR_FRACT_BITS);
		float   accf  = mul_0f + mul_1f + mul_2f;
		float   resf = 1.0f / accf;
		
		printf ("mul_0 %f/%f\t", mul_0f, dfixpt_to_float (mul_0, BARC_FRACT_BITS + W_RECIPR_FRACT_BITS));
		printf ("mul_1 %f/%f\t", mul_1f, dfixpt_to_float (mul_1, BARC_FRACT_BITS + W_RECIPR_FRACT_BITS));
		printf ("mul_2 %f/%f\t", mul_2f, dfixpt_to_float (mul_2, BARC_FRACT_BITS + W_RECIPR_FRACT_BITS));
		printf ("acc %f/%f\t", accf, dfixpt_to_float (acc, BARC_FRACT_BITS + W_RECIPR_FRACT_BITS));
		printf ("res %f/%f\n", resf, dfixpt_to_float (res, OOWI_FRACT_BITS));
	}
	
	return (dfixpt_t) res;		
}


Varying interpolate_varying (Varying vry[3], fixpt_t w_reciprocal[3], FixPt3 *bar) {

	assert (vry[0].num_of_words == vry[1].num_of_words);
	assert (vry[0].num_of_words == vry[2].num_of_words);
					
	Varying vry_interp;					
	vry_interp.num_of_words = vry[0].num_of_words;
	
	if (vry_interp.num_of_words > 0) {
		
		dfixpt_t one_over_wi = interpolate_w (w_reciprocal, bar);
		
		for (int i = 0; i < vry_interp.num_of_words; i++) {
			
			#define NNN 20
						
			dfixpt_t vtx0_norm_fixpt = (dfixpt_t) vry[0].data[i].as_fixpt_t * (dfixpt_t) w_reciprocal[0];  // 18.14 * 3.29 = 21.43
			dfixpt_t vtx1_norm_fixpt = (dfixpt_t) vry[1].data[i].as_fixpt_t * (dfixpt_t) w_reciprocal[1];  // 
			dfixpt_t vtx2_norm_fixpt = (dfixpt_t) vry[2].data[i].as_fixpt_t * (dfixpt_t) w_reciprocal[2];  // 

			dfixpt_t mpy0_fixpt = (vtx0_norm_fixpt >> NNN) * (dfixpt_t) bar->as_array[0]; // (21.43>>16) * 24.8 = 21.27 * 24.8 = 29.35
			dfixpt_t mpy1_fixpt = (vtx1_norm_fixpt >> NNN) * (dfixpt_t) bar->as_array[1]; // 
			dfixpt_t mpy2_fixpt = (vtx2_norm_fixpt >> NNN) * (dfixpt_t) bar->as_array[2]; // 

			dfixpt_t acc_fixpt = mpy0_fixpt + mpy1_fixpt + mpy2_fixpt; // 29.35

			vry_interp.data[i].as_fixpt_t = (fixpt_t) ((acc_fixpt * (dfixpt_t) one_over_wi) >> (W_RECIPR_FRACT_BITS - NNN + BARC_FRACT_BITS + OOWI_FRACT_BITS)); // (29.35 * 16.16 = 13.51)>>43 = 56.8
			
			if (DEBUG_FIXPT_VARYING) {
				float vtx0_norm = vry[0].data[i].as_float * fixpt_to_float (w_reciprocal[0], W_RECIPR_FRACT_BITS);
				float vtx1_norm = vry[1].data[i].as_float * fixpt_to_float (w_reciprocal[1], W_RECIPR_FRACT_BITS);
				float vtx2_norm = vry[2].data[i].as_float * fixpt_to_float (w_reciprocal[2], W_RECIPR_FRACT_BITS);
				float mpy0 = vtx0_norm * fixpt_to_float (bar->as_array[0], BARC_FRACT_BITS);
				float mpy1 = vtx1_norm * fixpt_to_float (bar->as_array[1], BARC_FRACT_BITS);
				float mpy2 = vtx2_norm * fixpt_to_float (bar->as_array[2], BARC_FRACT_BITS);
				float acc = mpy0 + mpy1 + mpy2;
				vry_interp.data[i].as_float = acc * dfixpt_to_float (one_over_wi, OOWI_FRACT_BITS);
				
				printf ("vtx0_norm: %f/%f\t", vtx0_norm, dfixpt_to_float(vtx0_norm_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS));
				printf ("vtx1_norm: %f/%f\t", vtx1_norm, dfixpt_to_float(vtx1_norm_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS));
				printf ("vtx2_norm: %f/%f\n", vtx2_norm, dfixpt_to_float(vtx2_norm_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS));
				printf ("mpy0: %f/%f\t", mpy0, dfixpt_to_float(mpy0_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS + BARC_FRACT_BITS - NNN));
				printf ("mpy1: %f/%f\t", mpy1, dfixpt_to_float(mpy1_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS + BARC_FRACT_BITS - NNN));
				printf ("mpy2: %f/%f\n", mpy2, dfixpt_to_float(mpy2_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS + BARC_FRACT_BITS - NNN));
				printf ("acc: %f/%f\n", acc, dfixpt_to_float (acc_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS + BARC_FRACT_BITS - NNN));
				printf ("vry interp: %f/%f\n", vry_interp.data[i].as_float, dfixpt_to_float (vry_interp.data[i].as_fixpt_t, VARYING_FRACT_BITS));
			}
		}
	}
	
	return vry_interp;					
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
    
	return out;
}


// Rasterize:
// 1. compute barycentric coordinates (bar0,bar1,bar2), don't normalize them
// 1.a. dumb method: just compute all the values for each pixel
// 1.b. smart method: compute values once per triangle, then just increment every
//      pixel and every row
// 2. interpolate values such as Z for every pixel:
// 2.a. dumb method: Z = (z0*bar0+z1*bar1+z2*bar2)/sum_of_bar
//      *divide by sum_of_bar because bar values are not normalized
// 2.b. smart method: Z = z0 + bar1*(z1-z0)/sum_of_bar + bar2*(z2-z0)/sum_of_bar
//      *can get rid of bar0
//      **(z1-z0)/sum_of_bar is constant for a triangle
//      ***(z2-z0)/sum_of_bar is constant for a triangle
void draw_triangle (TrianglePShaderData *tri, size_t tile_num, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer) {    
	if (GL_DEBUG_0) {
		printf("\tcall draw_triangle()\n");
	}
	
	fixpt_t    x[3];
	fixpt_t    y[3];
	fixpt_t    z[3];
		
	// re-pack X, Y, Z coords of the three vertices
	for (int i = 0; i < 3; i++) {
		x[i] = tri->screen_coords[i].as_struct.x;
		y[i] = tri->screen_coords[i].as_struct.y;
		z[i] = tri->screen_coords[i].as_struct.z;
		
		assert (z[i] >= 0);
	}
	
	BoundBox bb = clip_boundbox_to_tile (get_tri_boundbox (x, y), tile_num);
	
	fixpt_t px = fixpt_from_screenxy (bb.min.x);
	fixpt_t py = fixpt_from_screenxy (bb.min.y);
	
	FixPt3 bar_initial = get_bar_coords (x, y, px, py);
	
	fixpt_t sum_of_2bars = fixpt_add (bar_initial.as_array[0], bar_initial.as_array[1]);
	fixpt_t sum_of_bars = fixpt_add (bar_initial.as_array[2], sum_of_2bars);
	if (sum_of_bars == 0) return;
	
	FixPt3 bar_row_incr;
	bar_row_incr.as_array[0] = fixpt_sub (x[2], x[1]) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	bar_row_incr.as_array[1] = fixpt_sub (x[0], x[2]) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	bar_row_incr.as_array[2] = fixpt_sub (x[1], x[0]) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	
	FixPt3 bar_col_incr;
    bar_col_incr.as_array[0] = fixpt_sub (y[1], y[2]) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	bar_col_incr.as_array[1] = fixpt_sub (y[2], y[0]) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	bar_col_incr.as_array[2] = fixpt_sub (y[0], y[1]) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	
	
	FixPt3   bar;
	FixPt3   bar_row = bar_initial;
	ScreenPt p;
    for (p.y = bb.min.y; p.y <= bb.max.y; p.y++) {	
		
		bar = bar_row;
		
		for (p.x = bb.min.x; p.x <= bb.max.x; p.x++) {
			
			// If p is on or inside all edges, render pixel.
			if ((bar.as_array[0] > 0) && (bar.as_array[1] > 0) && (bar.as_array[2] > 0)) { // left-top fill rule
				
				screenz_t zi = interpolate_z (z, &bar);
				size_t pix_num = p.x + p.y * SCREEN_WIDTH;
				
				if (zi > zbuffer[pix_num]) {
					zbuffer[pix_num] = zi;

					Varying vry_interp = interpolate_varying (tri->varying, tri->w_reciprocal, &bar);
					
					pixel_color_t color;
					if (pshader (tri->obj, &vry_interp, &color) && (fbuffer != NULL)) {
						fbuffer[p.x + (SCREEN_HEIGHT-p.y-1) * SCREEN_WIDTH] = color;
					}
				}
			}
			bar = FixPt3_FixPt3_add (bar, bar_col_incr);
        }
        bar_row = FixPt3_FixPt3_add (bar_row, bar_row_incr);
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

//
// obj_draw: Draw 3D Object
//
// Arguments:
//  *obj     - object to be drawn (object coordinates, rotation, scale, translation)
//   vshader - pointer to vertex shader function
//   pshader - pointer to pixel shader function
//  *zbuffer - pointer to Z-buffer
//  *fbuffer - pointer to framebuffer
//
// - For each face (triangle) of the object:
//    - For each vertex of the face (triangle):
//       - call vshader() which returns a Varying union containing vertex
//         coords in clip space
//       - transform vertex coords from clip space to NDC
//       - check that NDC belongs to [-1:1], otherwise vertex is clipped
//       - if the vertex is not clipped then transform its coords from
//         NDC to screen space
//    - If at least one vertex is not clipped, call draw_triangle()
//
void draw_frame (ObjectListNode *obj_list_head, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer) {
	
	if (GL_DEBUG_0)
	{
		printf("call draw_frame()\n");
	}
			
	TriangleListNode *tile_idx_table [NUM_OF_TILES];
	for (int i = 0; i < NUM_OF_TILES; i++) {
		tile_idx_table[i] = NULL;
	}
			
	ObjectListNode *obj_list_node;
	obj_list_node = obj_list_head;		
	int num_of_faces = 0;
	while (obj_list_node != NULL) {
		num_of_faces += wfobj_get_num_of_faces(obj_list_node->obj->wfobj);
		obj_list_node = obj_list_node->next;
	}
	TrianglePShaderData *tri_data_array = calloc (num_of_faces, sizeof (TrianglePShaderData));
	
	obj_list_node = obj_list_head;
	
	int tri_num = 0;
	
	
	// This is for DEBUG_FIXPT_W
	float max_w = 0;
	float max_w_recip = 0;
	float min_w = 10000;
	float min_w_recip = 10000;
	
	
	while (obj_list_node != NULL) {
		for (size_t i = 0; i < wfobj_get_num_of_faces(obj_list_node->obj->wfobj); i++) {
						
			Triangle clip;
			Triangle ndc;
			Triangle screen;
			
			tri_data_array[tri_num].obj = obj_list_node->obj;
			
			bool is_clipped = true; // sticky bit
			for (size_t j = 0; j < 3; j++) {
				
				// // First four floats of Varying contain XYZW of a vertex in clip space
				tri_data_array[tri_num].varying[j].num_of_words = 0;
				clip.vtx[j] = vshader (tri_data_array[tri_num].obj, i, j, &(tri_data_array[tri_num].varying[j]) ); // CALL VERTEX SHADER
				
				// Clip & normalize (clip -> NDC):
				if (clip.vtx[j].as_struct.w > 0) {
					
					// This division is done once here to avoid three deivisions below
					// No div by zero because we checked above that it's > 0
					float reciprocal_w = 1.0f / clip.vtx[j].as_struct.w; 
					
					if (DEBUG_FIXPT_W) {
						if (clip.vtx[j].as_struct.w > max_w) max_w = clip.vtx[j].as_struct.w;
						if (reciprocal_w > max_w_recip) max_w_recip = reciprocal_w;
						
						if (clip.vtx[j].as_struct.w < min_w) min_w = clip.vtx[j].as_struct.w;
						if (reciprocal_w < min_w_recip) min_w_recip = reciprocal_w;
					}
					
					// Compute XYZ in NDC by dividing XYZ in clip space by W (i.e. multiplying by 1/W)
					// If at least one coord belongs to [-1:1] then the vertex is not clipped
					for (int k = 0; k < 4; k++) {
						ndc.vtx[j].as_array[k] = clip.vtx[j].as_array[k] * reciprocal_w; // normalize
						if ((ndc.vtx[j].as_array[k] <= 1.0f) && (ndc.vtx[j].as_array[k] >= -1.0f)) {
							is_clipped = false;
						}
					}

					if (!is_clipped) {
						screen.vtx[j] = fmat4_Float4_mult (&VIEWPORT, &(ndc.vtx[j]));
					
						// We don't need W anymore, but we will need 1/W later, so replacing the former with the latter
						// because we have it for free here
						screen.vtx[j].as_struct.w = reciprocal_w;

						// Replace clip coords with screen coords within the Varying struct
						// before passing it on to draw_triangle()
						tri_data_array[tri_num].screen_coords[j].as_struct.x =  fixpt_from_float        (screen.vtx[j].as_struct.x, XY_FRACT_BITS);
						tri_data_array[tri_num].screen_coords[j].as_struct.y =  fixpt_from_float        (screen.vtx[j].as_struct.y, XY_FRACT_BITS);
						tri_data_array[tri_num].screen_coords[j].as_struct.z =  fixpt_from_float        (screen.vtx[j].as_struct.z, Z_FRACT_BITS);
						tri_data_array[tri_num].w_reciprocal[j]              =  fixpt_from_float_no_rnd (screen.vtx[j].as_struct.w, W_RECIPR_FRACT_BITS);
					}		
				}
			}
			
			if (!is_clipped) {
				tiler(&(tri_data_array[tri_num]), tile_idx_table);
				tri_num++;
			}
		}
		
		obj_list_node = obj_list_node->next;
	}
	
	if (DEBUG_FIXPT_W) {
		printf ("max w: %f, max 1/w: %f\t\tmin w: %f, min 1/w: %f\n", max_w, max_w_recip, min_w, min_w_recip);
	}
					
	for (int i = 0; i < NUM_OF_TILES; i++) {
		TriangleListNode *node = tile_idx_table[i];
		while (node != NULL) {
			TrianglePShaderData *tri = node->tri;
			draw_triangle (tri, i, pshader, zbuffer, fbuffer);	
			node = node->next;
		}		
	}
	
	for (int i = 0; i < NUM_OF_TILES; i++) {
		TriangleListNode *node = tile_idx_table[i];
		TriangleListNode *tmp;
		while (node != NULL) {
			tmp = node->next;
			free (node);
			node = tmp;
		}		
	}	
	free (tri_data_array);
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
