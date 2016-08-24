#include "gl.h"
#include "geometry.h"
#include "shader.h"
//#include "main.h" // TBD -remove
#include "wavefront_obj.h"

#include <math.h>
#include <stdlib.h>
#include <stdint.h>


screenxy_t SCREEN_WIDTH;
screenxy_t SCREEN_HEIGHT;
screenz_t  SCREEN_DEPTH;


int32_t edge_func(screenxy_t ax, screenxy_t ay, screenxy_t bx, screenxy_t by, screenxy_t cx, screenxy_t cy) {
    return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
}

static inline int32_t min_of_two (int32_t a, int32_t b) {
	return (a < b) ? a : b;
}

static inline int32_t max_of_two (int32_t a, int32_t b) {
	return (a > b) ? a : b;
}

int32_t min_of_three (int32_t a, int32_t b, int32_t c) {
	return min_of_two (a, min_of_two (b, c));
}

int32_t max_of_three (int32_t a, int32_t b, int32_t c) {
	return max_of_two (a, max_of_two (b, c));
}

pixel_color_t set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	pixel_color_t pc;
	pc.r = r;
	pc.g = g;
	pc.b = b;
	//pc.a = a;
	return pc;
}

/*void init_viewport (fmat4 *m, int x, int y, int w, int h, int d) {
	fmat4_set (m, 0, 0, h / 2.0f); //(w/2.0) * (h/w) = h/2.0 - adjust for screen aspect ratio
	fmat4_set (m, 0, 3, x + w / 2.0f);
	fmat4_set (m, 1, 1, h / 2.0f);
	fmat4_set (m, 1, 3, y + h / 2.0f);
	fmat4_set (m, 2, 2, d / 2.0f);
	fmat4_set (m, 2, 3, d / 2.0f);
	fmat4_set (m, 3, 3, 1.0f);
	print_fmat4 (m, "viewport matrix");
}*/

int set_screen_size (screenxy_t width, screenxy_t height) {
	SCREEN_WIDTH  = width;
	SCREEN_HEIGHT = height;
	
	switch (sizeof(screenz_t)) {
		case 1: SCREEN_DEPTH = UINT8_MAX; break;
		case 2: SCREEN_DEPTH = UINT16_MAX; break;
		case 4: SCREEN_DEPTH = UINT32_MAX; break;
		case 8: SCREEN_DEPTH = (uint64_t) UINT64_MAX; break;
		default: return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

screenxy_t get_screen_width (void) {
	return SCREEN_WIDTH;
}

screenxy_t get_screen_height (void) {
	return SCREEN_HEIGHT;
}

screenz_t get_screen_depth (void) {
	return SCREEN_DEPTH;
}

void init_perspective_proj (fmat4 *m, float left, float right, float top, float bot, float near, float far) {
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
	float sin_alpha = sin(rad);
	float cos_alpha = cos(rad);
	
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
void draw_triangle (Triangle *t, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer, WFobj *obj)
{    
	if (GL_DEBUG_0) {
		printf ("\tcall draw_triangle()\n");
		for (int i = 0; i < 3; i++) {
			printf ("\t\tvertex %d: x=%f, y=%f, z=%f, w=%f\n", i, t->vtx[i].as_struct.x, t->vtx[i].as_struct.y, t->vtx[i].as_struct.z, t->vtx[i].as_struct.w);
		}
	}
	
	// fixed point coordinates with subpixel precision:
	// forum.devmaster.net/t/advanced-rasterization/6145
	screenxy_t x[3];
	screenxy_t y[3];
	for (int i = 0; i < 3; i++) {
		x[i] = (screenxy_t) t->vtx[i].as_struct.x * (1 << FIX_PT_PRECISION);
		y[i] = (screenxy_t) t->vtx[i].as_struct.y * (1 << FIX_PT_PRECISION);
	}
	
    // Compute triangle bounding box.
    screenxy_t min_x = max_of_two (       0, min_of_three (x[0], x[1], x[2]) >> FIX_PT_PRECISION);
    screenxy_t max_x = min_of_two ( SCREEN_WIDTH-1, max_of_three (x[0], x[1], x[2]) >> FIX_PT_PRECISION);
    screenxy_t min_y = max_of_two (       0, min_of_three (y[0], y[1], y[2]) >> FIX_PT_PRECISION);
    screenxy_t max_y = min_of_two (SCREEN_HEIGHT-1, max_of_three (y[0], y[1], y[2]) >> FIX_PT_PRECISION);
    
    ScreenPt p;
    for (p.y = min_y; p.y < max_y; p.y++) {	
		for (p.x = min_x; p.x < max_x; p.x++) {
			int3 bar;
			bar[0] = edge_func(x[1], y[1], x[2], y[2], p.x << FIX_PT_PRECISION, p.y << FIX_PT_PRECISION); // not normalized
			bar[1] = edge_func(x[2], y[2], x[0], y[0], p.x << FIX_PT_PRECISION, p.y << FIX_PT_PRECISION); // not normalized
			bar[2] = edge_func(x[0], y[0], x[1], y[1], p.x << FIX_PT_PRECISION, p.y << FIX_PT_PRECISION); // not normalized
			
			// If p is on or inside all edges, render pixel.
			if ((bar[0] | bar[1] | bar[2]) > 0) {
				float sum_of_bars      = 0.0f;
				float sum_of_bar_clips = 0.0f;
				Float3 bar_clip;
				for (int i = 0; i < 3; i++) {
					bar_clip.as_array[i] = (float) bar[i] * t->vtx[i].as_struct.w; // W here actually contains 1/W
					sum_of_bar_clips += bar_clip.as_array[i];
					sum_of_bars += (bar[i]);
				}
				if (sum_of_bars == 0) {
					if (GL_DEBUG_2) printf ("Im gonna die!!!\n");
					return;
				}				
				
				for (int i = 0; i < 3; i++) {
					bar_clip.as_array[i] /= sum_of_bar_clips;
				}
				
				
				//p.z = (screenz_t) t->vtx[0].as_struct.z + bar_clip.as_struct.y * (t->vtx[1].as_struct.z - t->vtx[0].as_struct.z) + bar_clip.as_struct.z * (t->vtx[2].as_struct.z - t->vtx[0].as_struct.z);
				float duck = 0;
				
				for (int i = 0; i < 3; i++) duck += t->vtx[i].as_struct.z*bar_clip.as_array[i];
				
				//for (int i = 0; i < 3; i++) duck += t->vtx[i].as_struct.z*bar[i];
				//duck /= sum_of_bars;
				
				p.z = (screenz_t) duck; 
				
				
				//printf ("draw_triangle: x=%d y=%d z=%d\t", p.x, p.y, p.z);
				
				uint32_t pix_num = p.x + p.y*SCREEN_WIDTH;
				if (p.z > zbuffer[pix_num]) {
					zbuffer[pix_num] = p.z;
					pixel_color_t color;
					if (pshader (obj, &bar_clip, &color) && (fbuffer != NULL)) {
						fbuffer[p.x + (SCREEN_HEIGHT-p.y-1)*SCREEN_WIDTH] = color;
					}
				}
			}
        }
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

void obj_draw (Object *obj, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer) {
	
	for (int i = 0; i < wfobj_get_num_of_faces(obj->wfobj); i++) {
		
		if (GL_DEBUG_0) {
			printf("call obj_draw()\n");
		}
		
		Triangle clip;
		Triangle ndc;
		Triangle screen;
		
		bool is_clipped = false;//true; // sticky bit
		
		for (int j = 0; j < 3; j++) {
			
			clip.vtx[j] = vshader (obj->wfobj, i, j, &(obj->mvp));
			
			// clip & normalize (clip -> NDC):
			if (clip.vtx[j].as_struct.w > 0) {
				float reciprocal_w = 1.0 / clip.vtx[j].as_struct.w; // we checked above that it's not zero
				for (int k = 0; k < 3; k++) {
					ndc.vtx[j].as_array[k] = clip.vtx[j].as_array[k] * reciprocal_w; // normalize
					//if ((ndc.vtx[j].as_array[k] <= 1.0f) && (ndc.vtx[j].as_array[k] >= -1.0f)) {
					if ((ndc.vtx[j].as_array[k] > 1.0f) || (ndc.vtx[j].as_array[k] < -1.0f)) {
						//is_clipped = false;
						is_clipped = true;
					}
				}
				ndc.vtx[j].as_struct.w = reciprocal_w;	
			}
			
			// NDC -> screen
			if (!is_clipped) {
				//screen.vtx[j] = fmat4_Float4_mult (viewport, &(ndc.vtx[j]));
				screen.vtx[j].as_struct.x = SCREEN_WIDTH / 2.0 + ndc.vtx[j].as_struct.x * SCREEN_HEIGHT / 2.0; // map [-1:1] to [0:(SCREEN_WIDTH+HEIGTH)/2]
				//screen.vtx[j].as_struct.x = (ndc.vtx[j].as_struct.x + 1.0) *  SCREEN_WIDTH / 2.0; // map [-1:1] to [0:SCREEN_WIDTH]
				screen.vtx[j].as_struct.y = (ndc.vtx[j].as_struct.y + 1.0) * SCREEN_HEIGHT / 2.0; // map [-1:1] to [0:SCREEN_HEIGHT]
				screen.vtx[j].as_struct.z =  SCREEN_DEPTH * (1.0 - ndc.vtx[j].as_struct.z) / 2.0; // map [-1:1] to [SCREEN_DEPTH:0]				
				screen.vtx[j].as_struct.w =  ndc.vtx[j].as_struct.w;
				
				if (GL_DEBUG_0) {
					printf ("\t\tNDC coord:    %f, %f, %f\n",    ndc.vtx[j].as_struct.x,    ndc.vtx[j].as_struct.y,    ndc.vtx[j].as_struct.z);
					printf ("\t\tscreen coord: %f, %f, %f\n", screen.vtx[j].as_struct.x, screen.vtx[j].as_struct.y, screen.vtx[j].as_struct.z);
				}
			}
		}
		
		if (!is_clipped) {
			draw_triangle (&screen, pshader, zbuffer, fbuffer, obj->wfobj);
		}
    }
}
