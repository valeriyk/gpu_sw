#include "gl.h"
//#include "geometry.h"
//#include "geometry_fixpt.h"
#include "shader.h"
#include "wavefront_obj.h"
#include "dynarray.h"

//#include <fixmath.h>

#include <math.h>
#include <stdlib.h>
#include <stdint.h>


size_t SCREEN_WIDTH;
size_t SCREEN_HEIGHT;
size_t SCREEN_DEPTH;

size_t NUM_OF_TILES;

fmat4 VIEWPORT;


int32_t edge_func      (screenxy_t ax, screenxy_t ay, screenxy_t bx, screenxy_t by, screenxy_t cx, screenxy_t cy);
fixpt_t edge_func_fixpt (fixpt_t ax, fixpt_t ay, fixpt_t bx, fixpt_t by, fixpt_t cx, fixpt_t cy);

int32_t min_of_three (int32_t a, int32_t b, int32_t c);
int32_t max_of_three (int32_t a, int32_t b, int32_t c);	



int32_t edge_func(screenxy_t ax, screenxy_t ay, screenxy_t bx, screenxy_t by, screenxy_t cx, screenxy_t cy) {
    int32_t bx_ax = (int32_t) bx - (int32_t) ax;
    int32_t cy_ay = (int32_t) cy - (int32_t) ay;
    int32_t by_ay = (int32_t) by - (int32_t) ay;
    int32_t cx_ax = (int32_t) cx - (int32_t) ax;
    
    int32_t res = bx_ax * cy_ay - by_ay * cx_ax;
    
    bool downwards  = (by_ay  < 0);
    bool horizontal = (by_ay == 0);
    bool leftwards  = (bx_ax  < 0);
    bool topleft    = downwards || (horizontal && leftwards);
    
    return topleft ? ++res : res;
}
	
/*fixpt_t edge_func_fixpt (fixpt_t ax, fixpt_t ay, fixpt_t bx, fixpt_t by, fixpt_t cx, fixpt_t cy) {
    
    fixpt_t bx_ax = fixpt_sub (bx, ax);
    fixpt_t cy_ay = fixpt_sub (cy, ay);
    fixpt_t by_ay = fixpt_sub (by, ay);
    fixpt_t cx_ax = fixpt_sub (cx, ax);

    fixpt_t res = fixpt_sub (fixpt_mul (bx_ax, cy_ay), fixpt_mul (by_ay,cx_ax));
    
    // left-top fill rule:
    bool downwards  = (by_ay  < 0);
    bool horizontal = (by_ay == 0);
    bool leftwards  = (bx_ax  < 0);
    bool topleft    = downwards || (horizontal && leftwards);
    
    return topleft ? fixpt_add (res, 1) : res;
}*/	

fixpt_t edge_func_fixpt (fixpt_t ax, fixpt_t ay, fixpt_t bx, fixpt_t by, fixpt_t cx, fixpt_t cy) {
    
    dfixpt_t bx_ax = (dfixpt_t) bx - (dfixpt_t) ax;
    dfixpt_t cy_ay = (dfixpt_t) cy - (dfixpt_t) ay;
    dfixpt_t by_ay = (dfixpt_t) by - (dfixpt_t) ay;
    dfixpt_t cx_ax = (dfixpt_t) cx - (dfixpt_t) ax;

    //fixpt_t res = fixpt_sub (fixpt_mul (bx_ax, cy_ay), fixpt_mul (by_ay,cx_ax));
    dfixpt_t mul_0 = bx_ax * cy_ay;
    dfixpt_t mul_1 = by_ay * cx_ax;
    dfixpt_t diff  = mul_0 - mul_1;
    fixpt_t res = (fixpt_t) (diff >> FRACT_BITS);
    
    // left-top fill rule:
    bool downwards  = (by_ay  < 0);
    bool horizontal = (by_ay == 0);
    bool leftwards  = (bx_ax  < 0);
    bool topleft    = downwards || (horizontal && leftwards);
    
    return topleft ? res + 1 : res;
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
	//fmat4_set (&VIEWPORT, 2, 2, d / 2.0f);
	//fmat4_set (&VIEWPORT, 2, 3, 0);
	fmat4_set (&VIEWPORT, 2, 2, -d / 2.0f); // minus sign because Z points in opposite directions in NDC and screen/clip
	fmat4_set (&VIEWPORT, 2, 3,  d / 2.0f);
	//print_fmat4 (&VIEWPORT, "viewport matrix");
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


void new_light (int light_num, Float3 dir) { //TBD add light_src,
	LIGHTS[light_num].enabled = true;
	LIGHTS[light_num].dir = dir;
	LIGHTS[light_num].src = Float3_set (-dir.as_struct.x, -dir.as_struct.y, -dir.as_struct.z);
	LIGHTS[light_num].shadow_buf = (screenz_t*) calloc (SCREEN_WIDTH*SCREEN_HEIGHT, sizeof(screenz_t));
}

void free_light (int light_num) {
	LIGHTS[light_num].enabled = false;
    free (LIGHTS[light_num].shadow_buf);
}

void init_scene (void) {
	//Scene.num_of_lights = 0;
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		LIGHTS[i].enabled = false;
		LIGHTS[i].shadow_buf = NULL;
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

Object* obj_new (WFobj *wfobj) {
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
	return obj;
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

fixpt_t interpolate_z (fixpt_t z0, fixpt_t z1z0, fixpt_t z2z0, FixPt3 *bar) {
	//fixpt_add (z_fixp[0], fixpt_add (fixpt_mul (z1z0, bar_fixp.as_array[1]), fixpt_mul (z2z0, bar_fixp.as_array[2])));
	dfixpt_t mul_1 = z1z0 * bar->as_array[1];
	dfixpt_t mul_2 = z2z0 * bar->as_array[2];
	dfixpt_t z0_1  = ((dfixpt_t) z0) << FRACT_BITS;
	dfixpt_t acc = z0_1 + mul_1 + mul_2;
	return (fixpt_t) (acc >> FRACT_BITS);
}

screenz_t interpolate2_z (fixpt_t z[3], FixPt3 *bar) {
	//fixpt_add (z_fixp[0], fixpt_add (fixpt_mul (z1z0, bar_fixp.as_array[1]), fixpt_mul (z2z0, bar_fixp.as_array[2])));
	dfixpt_t mul_0 = z[0] * bar->as_array[0];
	dfixpt_t mul_1 = z[1] * bar->as_array[1];
	dfixpt_t mul_2 = z[2] * bar->as_array[2];
	//dfixpt_t z0_1  = ((dfixpt_t) z0) << FRACT_BITS;
	dfixpt_t acc = mul_0 + mul_1 + mul_2;
	
	assert ((mul_0 > 0) && (mul_1 > 0) && (mul_2 > 0) && (acc > 0));
	assert ((mul_0 < 0) && (mul_1 < 0) && (mul_2 < 0) && (acc < 0));
	dfixpt_t sum_of_bars = bar->as_array[0] + bar->as_array[1] + bar->as_array[2];
	dfixpt_t dres = acc / sum_of_bars;
	fixpt_t   res = (fixpt_t) dres;
	
	assert ((res < z[0]) || (res < z[1]) || (res < z[2]));
	assert (res >= 0);
	
	if (res < 0) res = 0;
	if (res >= (get_screen_depth() << FRACT_BITS)) res = get_screen_depth();
	//if (res < 0) printf ("neg Z! acc=%llx, sumob=%llx, dres=%llx, res=%x\n", acc, sum_of_bars, dres, res);
	return fixpt_to_screenz (res);
}

Varying interpolate_varying (Varying *vry, FixPt3 *bar, FixPt3 *one_over_w) {

	Varying vry_interp;
	
	vry_interp.num_of_words = vry[0].num_of_words;

	if (vry_interp.num_of_words > 0) {
		// Interpolate Z with perspective correction
		fixpt_t sum_of_bars = 0;
		FixPt3 bar_clip;
		for (int i = 0; i < 3; i++) {
			bar_clip.as_array[i] = fixpt_mul (bar->as_array[i], one_over_w->as_array[i]); // multiply by 1/W instead of dividing by W
			sum_of_bars = fixpt_add (sum_of_bars, bar_clip.as_array[i]);
		}
		
		// Normalize Z
		for (int i = 0; i < 3; i++) {
			bar_clip.as_array[i] = (sum_of_bars == 0) ? 0 : fixpt_div (bar_clip.as_array[i], sum_of_bars);
		}
		
		for (int i = 0; i < vry_interp.num_of_words; i++) {
			fixpt_t vtx0_norm = vry[0].data.as_fixpt_t[i];
			fixpt_t vtx1_norm = vry[1].data.as_fixpt_t[i];
			fixpt_t vtx2_norm = vry[2].data.as_fixpt_t[i];
			
			fixpt_t mpy0 = fixpt_mul (vtx0_norm, bar_clip.as_array[0]);
			fixpt_t mpy1 = fixpt_mul (vtx1_norm, bar_clip.as_array[1]);
			fixpt_t mpy2 = fixpt_mul (vtx2_norm, bar_clip.as_array[2]);
			
			vry_interp.data.as_fixpt_t[i] = fixpt_add (mpy0, fixpt_add (mpy1, mpy2));
		}
	}
	return vry_interp;
}

TriBoundBox get_bounding_box (size_t tile_num, fixpt_t x[3], fixpt_t y[3]) {
	
	TriBoundBox bb;
	
	// Compute tile bounding box.
    screenxy_t tile_min_x = (tile_num % (SCREEN_WIDTH/TILE_WIDTH)) * TILE_WIDTH;
    screenxy_t tile_max_x = tile_min_x + TILE_WIDTH - 1; 
    screenxy_t tile_min_y = (tile_num / (SCREEN_WIDTH/TILE_WIDTH)) * TILE_HEIGHT;
    screenxy_t tile_max_y = tile_min_y + TILE_HEIGHT - 1;
    
    if (GL_DEBUG_1) {
		printf("\t\ttile bounding box: x %d;%d\ty %d;%d\n", tile_min_x, tile_max_x, tile_min_y, tile_max_y);
	}
   
    // Compute triangle bounding box.
    screenxy_t tri_min_x = fixpt_to_screenxy (min_of_three (x[0], x[1], x[2]));
    screenxy_t tri_max_x = fixpt_to_screenxy (max_of_three (x[0], x[1], x[2]));
    screenxy_t tri_min_y = fixpt_to_screenxy (min_of_three (y[0], y[1], y[2]));
    screenxy_t tri_max_y = fixpt_to_screenxy (max_of_three (y[0], y[1], y[2]));
    if (GL_DEBUG_1) {
		 printf("\t\ttriangle bounding box: x %d;%d\ty %d;%d\n", tri_min_x, tri_max_x, tri_min_y, tri_max_y);
    }
    
    bb.min.x = max_of_two (tile_min_x, tri_min_x);
    bb.max.x = min_of_two (tile_max_x, tri_max_x);
    bb.min.y = max_of_two (tile_min_y, tri_min_y);
    bb.max.y = min_of_two (tile_max_y, tri_max_y);

	return bb;
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
void draw_triangle (TriangleVtxListNode *tri, size_t tile_num, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer) {    
	if (GL_DEBUG_0) {
		printf("\tcall draw_triangle()\n");
	}
	
	fixpt_t    x_fixp[3];
	fixpt_t    y_fixp[3];
	fixpt_t    z_fixp[3];
	//fixpt_t    w_fixp[3];
	
	// re-pack X, Y, Z, W coords of the three vertices
	for (int i = 0; i < 3; i++) {
		x_fixp[i] = tri->screen_coords[i].as_struct.x;
		y_fixp[i] = tri->screen_coords[i].as_struct.y;
		z_fixp[i] = tri->screen_coords[i].as_struct.z;
		//w_fixp[i] = tri->screen_coords[i].as_struct.w;
		
		if (DEBUG_Z) {
			if (z_fixp[i] < 0) printf ("Fixp Z < 0: %x\n", z_fixp[i]);
		}
	}
	
	TriBoundBox bb = get_bounding_box (tile_num, x_fixp, y_fixp);
	
	fixpt_t px_fixp = fixpt_from_screenxy (bb.min.x);
	fixpt_t py_fixp = fixpt_from_screenxy (bb.min.y);
	
	FixPt3 bar_initial;
	bar_initial.as_array[0] = edge_func_fixpt (x_fixp[1], y_fixp[1], x_fixp[2], y_fixp[2], px_fixp, py_fixp); // not normalized
	bar_initial.as_array[1] = edge_func_fixpt (x_fixp[2], y_fixp[2], x_fixp[0], y_fixp[0], px_fixp, py_fixp); // not normalized
	bar_initial.as_array[2] = edge_func_fixpt (x_fixp[0], y_fixp[0], x_fixp[1], y_fixp[1], px_fixp, py_fixp); // not normalized
	
	fixpt_t sum_of_2bars = fixpt_add (bar_initial.as_array[0], bar_initial.as_array[1]);
	fixpt_t sum_of_bars = fixpt_add (bar_initial.as_array[2], sum_of_2bars);
	if (sum_of_bars == 0) return;
	/*if ((bar_initial.as_array[0] > 0) && (bar_initial.as_array[1] > 0))
		if (sum_of_2bars < 0)
			printf ("sum_of_2bars: %x + %x = %x\n", bar_initial.as_array[0], bar_initial.as_array[1], sum_of_2bars);
	if ((bar_initial.as_array[2] > 0) && (sum_of_2bars > 0))
		if (sum_of_bars < 0)
			printf ("sum_of_3bars: %x + %x = %x\n", bar_initial.as_array[2], sum_of_2bars, sum_of_bars);
	*/
	
	FixPt3 bar_row_incr;
	bar_row_incr.as_array[0] = fixpt_sub (x_fixp[2], x_fixp[1]);
	bar_row_incr.as_array[1] = fixpt_sub (x_fixp[0], x_fixp[2]);
	bar_row_incr.as_array[2] = fixpt_sub (x_fixp[1], x_fixp[0]);
	
	FixPt3 bar_col_incr;
    bar_col_incr.as_array[0] = fixpt_sub (y_fixp[1], y_fixp[2]);
	bar_col_incr.as_array[1] = fixpt_sub (y_fixp[2], y_fixp[0]);
	bar_col_incr.as_array[2] = fixpt_sub (y_fixp[0], y_fixp[1]);
	
	
	fixpt_t z1z0 = fixpt_div (fixpt_sub (z_fixp[1], z_fixp[0]), sum_of_bars);
	fixpt_t z2z0 = fixpt_div (fixpt_sub (z_fixp[2], z_fixp[0]), sum_of_bars);
	//fixpt_t w1w0 = fixpt_div (fixpt_sub (w_fixp[1], w_fixp[0]), sum_of_bars);
	//fixpt_t w2w0 = fixpt_div (fixpt_sub (w_fixp[2], w_fixp[0]), sum_of_bars);
		
	FixPt3   bar;
	FixPt3   bar_row = bar_initial;
	ScreenPt p;
    for (p.y = bb.min.y; p.y <= bb.max.y; p.y++) {	
		
		bar = bar_row;
		
		for (p.x = bb.min.x; p.x <= bb.max.x; p.x++) {
			
			// If p is on or inside all edges, render pixel.
			if ((bar.as_array[0] > 0) && (bar.as_array[1] > 0) && (bar.as_array[2] > 0)) { // left-top fill rule
				
				// Interpolate and normalize Z
				//fixpt_t z = fixpt_add (z_fixp[0], fixpt_add (fixpt_mul (z1z0, bar_fixp.as_array[1]), fixpt_mul (z2z0, bar_fixp.as_array[2])));
				fixpt_t z = interpolate_z (z_fixp[0], z1z0, z2z0, &bar);
				//screenz_t z = interpolate2_z (z_fixp, &bar_fixp);
				
				size_t pix_num = p.x + p.y * SCREEN_WIDTH;
				
				// saturating subtraction:
				//int32_t z_diff = z - zbuffer[pix_num];
				fixpt_t z_diff = fixpt_sub (z, fixpt_from_screenz(zbuffer[pix_num]));
				if ((z > 0) && (zbuffer[pix_num] < 0) && (z_diff < 0)) z_diff = fixpt_get_max();
				else if ((z < 0) && (zbuffer[pix_num] > 0) && (z_diff > 0)) z_diff = fixpt_get_min();
								
				//if (p.z > zbuffer[pix_num]) {
				if (z_diff > 0) {
					//zbuffer[pix_num] = z;
					zbuffer[pix_num] = fixpt_to_screenz (z);

					// Interpolation of Varying values:

					Varying vry_interp;					
					vry_interp.num_of_words = tri->varying[0].num_of_words;
					if (vry_interp.num_of_words > 0) {
						// Interpolate Z with perspective correction
						/*
						fixpt_t w_interp = fixpt_add (fixpt_mul (w_fixp[0], bar.as_array[0]), fixpt_add (fixpt_mul (w_fixp[1], bar.as_array[1]), fixpt_mul (w_fixp[2], bar.as_array[2])));
						
						for (int i = 0; i < vry_interp.num_of_words; i++) {
							fixpt_t vtx0_norm = fixpt_mul (tri->varying[0].data.as_fixpt_t[i], w_fixp[0]);
							fixpt_t vtx1_norm = fixpt_mul (tri->varying[1].data.as_fixpt_t[i], w_fixp[1]);
							fixpt_t vtx2_norm = fixpt_mul (tri->varying[2].data.as_fixpt_t[i], w_fixp[2]);
							
							fixpt_t mpy0 = fixpt_mul (vtx0_norm, bar.as_array[0]);
							fixpt_t mpy1 = fixpt_mul (vtx1_norm, bar.as_array[1]);
							fixpt_t mpy2 = fixpt_mul (vtx2_norm, bar.as_array[2]);
							
							vry_interp.data.as_fixpt_t[i] = fixpt_add (mpy0, fixpt_add (mpy1, mpy2));
							vry_interp.data.as_fixpt_t[i] = fixpt_div (vry_interp.data.as_fixpt_t[i], w_interp);
						}
						*/
						float w_interp = 0;
						for (int i = 0; i < 3; i++) w_interp += tri->w_reciprocal[i] * fixpt_to_float (bar.as_array[i]);
						
						//fixpt_t w_interp = fixpt_add (fixpt_mul (w_fixp[0], bar.as_array[0]), fixpt_add (fixpt_mul (w_fixp[1], bar.as_array[1]), fixpt_mul (w_fixp[2], bar.as_array[2])));
						
						for (int i = 0; i < vry_interp.num_of_words; i++) {
							float vtx0_norm = tri->varying[0].data.as_float[i] * tri->w_reciprocal[0];
							float vtx1_norm = tri->varying[1].data.as_float[i] * tri->w_reciprocal[1];
							float vtx2_norm = tri->varying[2].data.as_float[i] * tri->w_reciprocal[2];
							
							float mpy0 = vtx0_norm * fixpt_to_float (bar.as_array[0]);
							float mpy1 = vtx1_norm * fixpt_to_float (bar.as_array[1]);
							float mpy2 = vtx2_norm * fixpt_to_float (bar.as_array[2]);
							
							vry_interp.data.as_float[i] = (mpy0 + mpy1 + mpy2) / w_interp;
						}
					}					
					
					if (GL_DEBUG_0) {
						printf("\t\tcall pshader()\n");
					}
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


void tiler (TriangleVtxListNode *tri_node, TrianglePtrListNode *tri_ptr[]) {
	
	if (GL_DEBUG_0) {
		printf("\tcall tiler()\n");
	}
	
	// fixed point coordinates with subpixel precision:
	// forum.devmaster.net/t/advanced-rasterization/6145
	screenxy_t x[3];
	screenxy_t y[3];
	for (int i = 0; i < 3; i++) {
		Float3 coords = FixPt3_Float3_conv(&(tri_node->screen_coords[i]));
		//x[i] = (screenxy_t) fix16_to_int (coords.as_struct.x);// * (1 << FIX_PT_PRECISION);
		//y[i] = (screenxy_t) fix16_to_int (coords.as_struct.y);// * (1 << FIX_PT_PRECISION);
		x[i] = (screenxy_t) coords.as_struct.x * (1 << FIX_PT_PRECISION);
		y[i] = (screenxy_t) coords.as_struct.y * (1 << FIX_PT_PRECISION);
	}
	
    // Compute triangle bounding box.
    screenxy_t min_x = max_of_two (              0, (min_of_three (x[0], x[1], x[2]) >> FIX_PT_PRECISION));
    screenxy_t max_x = min_of_two ( SCREEN_WIDTH-1, (max_of_three (x[0], x[1], x[2]) >> FIX_PT_PRECISION));
    screenxy_t min_y = max_of_two (              0, (min_of_three (y[0], y[1], y[2]) >> FIX_PT_PRECISION));
    screenxy_t max_y = min_of_two (SCREEN_HEIGHT-1, (max_of_three (y[0], y[1], y[2]) >> FIX_PT_PRECISION));
    
    min_x &= ~(TILE_WIDTH-1);
    min_y &= ~(TILE_HEIGHT-1);
    
    ScreenPt p;
    for (p.y = min_y; p.y < max_y; p.y += TILE_HEIGHT) {	
		for (p.x = min_x; p.x < max_x; p.x += TILE_WIDTH) {
			// Evaluate barycentric coords in the corners of the tile:
			
			int x0 = p.x << FIX_PT_PRECISION;
			int x1 = (p.x + TILE_WIDTH - 1) << FIX_PT_PRECISION;
			int y0 = p.y << FIX_PT_PRECISION;
			int y1 = (p.y + TILE_HEIGHT - 1) << FIX_PT_PRECISION;
			
			//printf ("\tTile %d: x=%d y=%d\n", ((p.y/TILE_HEIGHT)*(SCREEN_WIDTH/TILE_WIDTH)+(p.x/TILE_WIDTH)), p.x/TILE_WIDTH, p.y/TILE_HEIGHT);
			
			bool edge0_corner0_outside = (edge_func (x[1], y[1], x[2], y[2], x0, y0) < 0); // not normalized
			bool edge0_corner1_outside = (edge_func (x[1], y[1], x[2], y[2], x0, y1) < 0); // not normalized
			bool edge0_corner2_outside = (edge_func (x[1], y[1], x[2], y[2], x1, y0) < 0); // not normalized
			bool edge0_corner3_outside = (edge_func (x[1], y[1], x[2], y[2], x1, y1) < 0); // not normalized
			if (edge0_corner0_outside && edge0_corner1_outside && edge0_corner2_outside && edge0_corner3_outside) continue;
			
			bool edge1_corner0_outside = (edge_func (x[2], y[2], x[0], y[0], x0, y0) < 0); // not normalized
			bool edge1_corner1_outside = (edge_func (x[2], y[2], x[0], y[0], x0, y1) < 0); // not normalized
			bool edge1_corner2_outside = (edge_func (x[2], y[2], x[0], y[0], x1, y0) < 0); // not normalized
			bool edge1_corner3_outside = (edge_func (x[2], y[2], x[0], y[0], x1, y1) < 0); // not normalized
			if (edge1_corner0_outside && edge1_corner1_outside && edge1_corner2_outside && edge1_corner3_outside) continue;
			
			bool edge2_corner0_outside = (edge_func (x[0], y[0], x[1], y[1], x0, y0) < 0); // not normalized
			bool edge2_corner1_outside = (edge_func (x[0], y[0], x[1], y[1], x0, y1) < 0); // not normalized
			bool edge2_corner2_outside = (edge_func (x[0], y[0], x[1], y[1], x1, y0) < 0); // not normalized
			bool edge2_corner3_outside = (edge_func (x[0], y[0], x[1], y[1], x1, y1) < 0); // not normalized
			if (edge2_corner0_outside && edge2_corner1_outside && edge2_corner2_outside && edge2_corner3_outside) continue;
			
			size_t tile_num = (p.y >> (int) log2f(TILE_HEIGHT)) * (SCREEN_WIDTH / TILE_WIDTH) + (p.x >> (int) log2f(TILE_WIDTH));
			
			TrianglePtrListNode *node = tri_ptr[tile_num];
			if (node == NULL) {
				node = calloc (1, sizeof (TrianglePtrListNode));
				node->tri  = tri_node;
				node->next = NULL;	
				
				tri_ptr[tile_num] = node;
			}
			else {
				while (node->next != NULL) {
					node = node->next;
				}
				node->next = calloc (1, sizeof (TrianglePtrListNode));
				node->next->tri  = tri_node;
				node->next->next = NULL;	
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
void draw_frame (ObjectNode *obj_list_head, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer) {
	
	if (GL_DEBUG_0)
	{
		printf("call draw_frame()\n");
	}
			
	TrianglePtrListNode *tile_idx_table [NUM_OF_TILES];
	for (int i = 0; i < NUM_OF_TILES; i++)
	{
		tile_idx_table[i] = NULL;
	}
			
	ObjectNode *obj_list_node;
	
	obj_list_node = obj_list_head;		
	int num_of_faces = 0;
	while (obj_list_node != NULL)
	{
		num_of_faces += wfobj_get_num_of_faces(obj_list_node->obj->wfobj);
		obj_list_node = obj_list_node->next;
	}
	TriangleVtxListNode *vtx_list = calloc (num_of_faces, sizeof (TriangleVtxListNode));
	
	obj_list_node = obj_list_head;
	
	int tri_num = 0;
	
	while (obj_list_node != NULL) {
		for (size_t i = 0; i < wfobj_get_num_of_faces(obj_list_node->obj->wfobj); i++) {
						
			Triangle clip;
			Triangle ndc;
			Triangle screen;
			
			vtx_list[tri_num].obj = obj_list_node->obj;
			
			bool is_clipped = true; // sticky bit
			for (size_t j = 0; j < 3; j++) {
				
				// // First four floats of Varying contain XYZW of a vertex in clip space
				// clip.vtx[j] = FixPt4_Float4_conv (&(vtx_list[tri_num].varying[j].data.as_FixPt4[0]));
				clip.vtx[j] = vshader (vtx_list[tri_num].obj, i, j, &(vtx_list[tri_num].varying[j]) ); // CALL VERTEX SHADER
				
				// Clip & normalize (clip -> NDC):
				if (clip.vtx[j].as_struct.w > 0) {
					
					// This division is done once here to avoid three deivisions below
					// No div by zero because we checked above that it's > 0
					float reciprocal_w = 1.0f / clip.vtx[j].as_struct.w; 
					
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
						
						if (DEBUG_Z) {
							if (screen.vtx[j].as_struct.z < 0) printf ("Z < 0: %f\n", screen.vtx[j].as_struct.z);
							if (screen.vtx[j].as_struct.z > get_screen_depth()) printf ("Z > depth: %f\n", screen.vtx[j].as_struct.z);
						}

						// We don't need W anymore, but we will need 1/W later, so replacing the former with the latter
						// because we have it for free here
						screen.vtx[j].as_struct.w = reciprocal_w;

						/*
						if (GL_DEBUG_0) {
							printf ("\t\tNDC coord:    %f, %f, %f\n",    ndc.vtx[j].as_struct.x,    ndc.vtx[j].as_struct.y,    ndc.vtx[j].as_struct.z);
							printf ("\t\tscreen coord: %f, %f, %f\n", screen.vtx[j].as_struct.x, screen.vtx[j].as_struct.y, screen.vtx[j].as_struct.z);
						}
						*/
						
						// // Replace clip coords with screen coords within the Varying struct
						// // before passing it on to draw_triangle()
						// vtx_list[tri_num].varying[j].data.as_FixPt4[0] = Float4_FixPt4_conv(&(screen.vtx[j]));
						
						//vtx_list[tri_num].screen_coords[j] = Float4_FixPt4_conv(&(screen.vtx[j]));
						for (int k = 0; k < 3; k++) {
							vtx_list[tri_num].screen_coords[j].as_array[k] = fixpt_from_float (screen.vtx[j].as_array[k]);
						}
						vtx_list[tri_num].w_reciprocal[j] = screen.vtx[j].as_struct.w;
					}		
				}
			}
			
			if (!is_clipped) {
				tiler(&(vtx_list[tri_num]), tile_idx_table);
				tri_num++;
			}
		}
		
		obj_list_node = obj_list_node->next;
	}
	
	for (int i = 0; i < NUM_OF_TILES; i++) {
		TrianglePtrListNode *node = tile_idx_table[i];
		while (node != NULL) {
			TriangleVtxListNode *tri = node->tri;
			draw_triangle (tri, i, pshader, zbuffer, fbuffer);	
			node = node->next;
		}		
	}
}
