#include "gl.h"
#include "geometry.h"
#include "shader.h"
#include "main.h" // TBD -remove
#include "wavefront_obj.h"

#include <math.h>
#include <stdlib.h>

int edge_func(Float4 *a, Float4 *b, ScreenPt *c) {
    return (int) (b->as_struct.x - a->as_struct.x) * (c->y - a->as_struct.y) - (b->as_struct.y - a->as_struct.y) * (c->x - a->as_struct.x);
}


screenxy_t tri_min_bound (screenxy_t a, screenxy_t b, screenxy_t c, screenxy_t cutoff) {
	int min = a;
	if (b < min) min = b;
	if (c < min) min = c;
	if (min < cutoff) min = cutoff;
	return min;
}

screenxy_t tri_max_bound (screenxy_t a, screenxy_t b, screenxy_t c, screenxy_t cutoff) {
	int max = a;
	if (b > max) max = b;
	if (c > max) max = c;
	if (max > cutoff) max = cutoff;
	return max;
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
	
    // Compute triangle bounding box
    screenxy_t min_x = tri_min_bound (t->vtx[0].as_struct.x, t->vtx[1].as_struct.x, t->vtx[2].as_struct.x, 0);
    screenxy_t max_x = tri_max_bound (t->vtx[0].as_struct.x, t->vtx[1].as_struct.x, t->vtx[2].as_struct.x, WIDTH);
    screenxy_t min_y = tri_min_bound (t->vtx[0].as_struct.y, t->vtx[1].as_struct.y, t->vtx[2].as_struct.y, 0);
    screenxy_t max_y = tri_max_bound (t->vtx[0].as_struct.y, t->vtx[1].as_struct.y, t->vtx[2].as_struct.y, HEIGHT);
    
    if ((min_x == max_x) || (min_y == max_y)) {
		if (GL_DEBUG_0) printf ("Degenerate triangle\n");
		return;
    }
    
    int3 bar;
    ScreenPt p;
    p.z = 0;
    for (p.y = min_y; p.y < max_y; p.y++) {	
		if (GL_DEBUG_1) printf ("for y %d\n", p.y);
        for (p.x = min_x; p.x < max_x; p.x++) {
			if (GL_DEBUG_2) printf ("\tfor x %d\n", p.x);
			bar[0] = edge_func(&(t->vtx[1]), &(t->vtx[2]), &p); // not normalized
			bar[1] = edge_func(&(t->vtx[2]), &(t->vtx[0]), &p); // not normalized
			bar[2] = edge_func(&(t->vtx[0]), &(t->vtx[1]), &p); // not normalized
			
			// If p is on or inside all edges, render pixel.
			if ((bar[0] | bar[1] | bar[2]) > 0) {
				float sum_of_bars = 0.0f;
				Float3 bar_clip;
				for (int i = 0; i < 3; i++) {
					bar_clip.as_array[i] = (float) bar[i] * t->vtx[i].as_struct.w; // W here actually contains 1/W
					sum_of_bars += bar_clip.as_array[i];
				}
				if (sum_of_bars == 0) {
					if (GL_DEBUG_2) printf ("Im gonna die!!!\n");
					return;
				}				
				
				//if (GL_DEBUG_3) printf("checkpoint 2\n");
				//p.z = (screenz_t) t->vtx[0].as_struct.z + bar_clip.as_struct.y * (t->vtx[1].as_struct.z - t->vtx[0].as_struct.z) + bar_clip.as_struct.z * (t->vtx[2].as_struct.z - t->vtx[0].as_struct.z);
				float duck = 0;
				//for (int i = 0; i < 3; i++) duck += t->vtx[i].as_struct.z*bar[i];//bar_clip.as_array[i];
				for (int i = 0; i < 3; i++) duck += t->vtx[i].as_struct.z*bar_clip.as_array[i];
				duck /= (bar_clip.as_array[0] + bar_clip.as_array[1] + bar_clip.as_array[2]);
				p.z = (screenz_t) duck; 
				for (int i = 0; i < 3; i++) {
					bar_clip.as_array[i] /= sum_of_bars;
				}
				if (GL_DEBUG_4) if ((p.x > 45) && (p.x < 55) && (p.y > 30) && (p.y < 60)) printf("\t\t\t[%d, %d] zbuf: %d zpix^ %d\n", p.x, p.y, zbuffer[p.x + p.y*WIDTH], p.z);
				if (p.z > zbuffer[p.x + p.y*WIDTH]) {
					zbuffer[p.x + p.y*WIDTH] = p.z;
					pixel_color_t color;
					//if (GL_DEBUG_3) printf("checkpoint 3\n");
					if (pshader (obj, &bar_clip, &color)) {
						//color = set_color(255, 0, 0, 0);
						if (GL_DEBUG_3) printf("pix [%d, %d] color: r%d g%d b%d\n", p.x, p.y, color.r, color.g, color.b);
						fbuffer[p.x + (HEIGHT - 1 - p.y)*WIDTH] = color; // TBD remove this p.y hack which avoids flipping the framebuffer
					}
					else {
						pixel_color_t color = set_color(255, 255, 255, 0);
						//if (GL_DEBUG_3) printf("pix [%d, %d] color: r%d g%d b%d\n", p.x, p.y, color.r, color.g, color.b);
						//fbuffer[p.x + (HEIGHT - 1 - p.y)*WIDTH] = color; // TBD remove this p.y hack which avoids flipping the framebuffer
					}
				}
				else {
					pixel_color_t color = set_color(255, 0, 0, 0);
					//if (GL_DEBUG_3) printf("pix [%d, %d] color: r%d g%d b%d\n", p.x, p.y, color.r, color.g, color.b);
					//fbuffer[p.x + (HEIGHT - 1 - p.y)*WIDTH] = color; // TBD remove this p.y hack which avoids flipping the framebuffer
				}
				
				/*if ((p.x > 45) && (p.x < 55) && (p.y > 30) && (p.y < 60)) {
					pixel_color_t color = set_color(255, 255, 0, 0);
					//if (GL_DEBUG_3) printf("pix [%d, %d] color: r%d g%d b%d\n", p.x, p.y, color.r, color.g, color.b);
					fbuffer[p.x + (HEIGHT - 1 - p.y)*WIDTH] = color; // TBD remove this p.y hack which avoids flipping the framebuffer
				}*/
				//if (GL_DEBUG_3) printf("checkpoint 10\n");
			}
        }
    }
}

pixel_color_t set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	pixel_color_t pc;
	pc.r = r;
	pc.g = g;
	pc.b = b;
	//pc.a = a;
	return pc;
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

void init_viewport (fmat4 *m, int x, int y, int w, int h, int d) {
	fmat4_set (m, 0, 0, h / 2.0f); //(w/2.0) * (h/w) = h/2.0 - adjust for screen aspect ratio
	fmat4_set (m, 0, 3, x + w / 2.0f);
	fmat4_set (m, 1, 1, h / 2.0f);
	fmat4_set (m, 1, 3, y + h / 2.0f);
	fmat4_set (m, 2, 2, d / 2.0f);
	fmat4_set (m, 2, 3, d / 2.0f);
	fmat4_set (m, 3, 3, 1.0f);
	print_fmat4 (m, "viewport matrix");
}

/*void init_projection (fmat4 *m, float val) {
	for (int i = 0; i < 4; i++)	fmat4_set (m, i, i, 1.0f);
	fmat4_set (m, 3, 2, val);
}*/

void init_projection (fmat4 *m, float left, float right, float top, float bot, float near, float far) {
	fmat4_set (m, 0, 0,       ( 2.0f * near) / (right - left));
	fmat4_set (m, 0, 2,       (right + left) / (right - left));
	fmat4_set (m, 1, 1,       ( 2.0f * near) / (  top -  bot));
	fmat4_set (m, 1, 2,       (  top +  bot) / (  top -  bot));
	fmat4_set (m, 2, 2,      -(  far + near) / (  far - near));
	fmat4_set (m, 2, 3, (-2.0f * far * near) / (  far - near));
	fmat4_set (m, 3, 2,  -1.0f);
	fmat4_set (m, 3, 3,   0.0f);
}


void init_view (fmat4 *m, Float3 *eye, Float3 *center, Float3 *up) {
	
	Float3 z = Float3_Float3_sub(eye, center);
	Float3_normalize (&z);
	Float3 x = Float3_Float3_crossprod(up, &z);
	Float3_normalize (&x);
	Float3 y = Float3_Float3_crossprod(&z, &x);
	Float3_normalize (&y);
	
	fmat4 Minv = FMAT4_IDENTITY;
	fmat4 Tr   = FMAT4_IDENTITY;
	
	for (int i = 0; i < 3; i++)	{
		fmat4_set (&Minv, 0, i, x.as_array[i]);
		fmat4_set (&Minv, 1, i, y.as_array[i]);
		fmat4_set (&Minv, 2, i, z.as_array[i]);
		fmat4_set (&Tr, i, 3, -(center->as_array[i]));
	}
	fmat4_fmat4_mult(&Minv, &Tr, m);
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

void obj_set_rotation (Object *obj, float x, float y, float z) {
	obj->rotate[0] = x;
	obj->rotate[1] = y;
	obj->rotate[2] = z;
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

void obj_draw (Object *obj, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer, fmat4 *viewport) {
	
	for (int i = 0; i < wfobj_get_num_of_faces(obj->wfobj); i++) {
	//for (int i = 0; i < 4; i++) {
		
		if (GL_DEBUG_0) {
			printf("call obj_draw()\n");
		}
		
		Triangle clip;
		Triangle ndc;
		Triangle screen;
		
		bool is_clipped = false; // sticky bit
		
		for (int j = 0; j < 3; j++) {
			
			clip.vtx[j] = vshader (obj->wfobj, i, j, &(obj->mvpv));
			
			// clip & normalize (clip -> NDC):
			if (clip.vtx[j].as_struct.w <= 0) {
				is_clipped = true;
				break;
			}			
			float reciprocal_w = 1.0 / clip.vtx[j].as_struct.w; // we checked above that it's not zero
			for (int k = 0; k < 3; k++) {
				ndc.vtx[j].as_array[k] = clip.vtx[j].as_array[k] * reciprocal_w; // normalize
				if ((ndc.vtx[j].as_array[k] > 1.0f) || (ndc.vtx[j].as_array[k] < -1.0f)) {
					is_clipped = true; // clip
					break;
				}
			}
			ndc.vtx[j].as_struct.w = reciprocal_w;	
			
			// NDC -> screen
			if (!is_clipped) {
				//screen.vtx[j] = fmat4_Float4_mult (viewport, &(ndc.vtx[j]));
				screen.vtx[j].as_struct.x = ndc.vtx[j].as_struct.x * HEIGHT/2 + WIDTH/2;
				screen.vtx[j].as_struct.y = ndc.vtx[j].as_struct.y * WIDTH/2 + WIDTH/2;
				screen.vtx[j].as_struct.z = -(ndc.vtx[j].as_struct.z * DEPTH/2 - DEPTH/2); // TBD - remove magic numbers
				screen.vtx[j].as_struct.w = ndc.vtx[j].as_struct.w;
				
				if (GL_DEBUG_0) {
					printf ("\t\tNDC coord:    %f, %f, %f\n",        ndc.vtx[j].as_struct.x,    ndc.vtx[j].as_struct.y,    ndc.vtx[j].as_struct.z);
					printf ("\t\tscreen coord: %f, %f, %f, %f\n", screen.vtx[j].as_struct.x, screen.vtx[j].as_struct.y, screen.vtx[j].as_struct.z);
				}
			}
			else break;
		}
		
		if (!is_clipped) {
			draw_triangle (&screen, pshader, zbuffer, fbuffer, obj->wfobj);
		}
    }
}
