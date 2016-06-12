#include "gl.h"
#include "geometry.h"
#include "shader.h"
#include "main.h" // TBD -remove
#include "wavefront_obj.h"

#include <math.h>
#include <stdlib.h>

int edge_func(float4 *a, float4 *b, ScreenPt *c)
{
    return (int) ((*b)[X] - (*a)[X])*(c->y - (*a)[Y]) - ((*b)[Y] - (*a)[Y])*(c->x - (*a)[X]);
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
void draw_triangle (ScreenTriangle *st, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer, WFobj *obj)
{
    // Compute triangle bounding box
    screenxy_t min_x = tri_min_bound (st->vtx_coords[0][X], st->vtx_coords[1][X], st->vtx_coords[2][X], 0);
    screenxy_t max_x = tri_max_bound (st->vtx_coords[0][X], st->vtx_coords[1][X], st->vtx_coords[2][X], WIDTH);
    screenxy_t min_y = tri_min_bound (st->vtx_coords[0][Y], st->vtx_coords[1][Y], st->vtx_coords[2][Y], 0);
    screenxy_t max_y = tri_max_bound (st->vtx_coords[0][Y], st->vtx_coords[1][Y], st->vtx_coords[2][Y], HEIGHT);
    
    if ((min_x == max_x) || (min_y == max_y)) {
		printf ("Degenerate triangle\n");
		return;
    }
    int3 bar;
    ScreenPt p;
    p.z = 0;
    for (p.y = min_y; p.y < max_y; p.y++) {	
        for (p.x = min_x; p.x < max_x; p.x++) {
			bar[0] = edge_func(&st->vtx_coords[1], &st->vtx_coords[2], &p); // not normalized
			bar[1] = edge_func(&st->vtx_coords[2], &st->vtx_coords[0], &p); // not normalized
			bar[2] = edge_func(&st->vtx_coords[0], &st->vtx_coords[1], &p); // not normalized
			
			// If p is on or inside all edges, render pixel.
			if ((bar[0] | bar[1] | bar[2]) > 0) {
				float sum_of_bars = 0.0f;
				float3 bar_clip;
				for (int i = 0; i < 3; i++) {
					bar_clip[i] = (float) bar[i]/st->vtx_coords[i][W]; // not normalized
					sum_of_bars += bar_clip[i];
				}
				if (sum_of_bars == 0) {
					printf ("Im gonna die!!!\n");
					return;
				}				
				for (int i = 0; i < 3; i++) bar_clip[i] /= sum_of_bars;
				p.z = (int) st->vtx_coords[0][Z] + bar_clip[1]*(st->vtx_coords[1][Z] - st->vtx_coords[0][Z]) + bar_clip[2]*(st->vtx_coords[2][Z] - st->vtx_coords[0][Z]);
				if (zbuffer[p.x + p.y*WIDTH] < p.z) {
					zbuffer[p.x + p.y*WIDTH] = p.z;
					pixel_color_t color;
					bool draw = pshader (obj, &bar_clip, &color);
					if (draw) fbuffer[p.x + (719 - p.y)*WIDTH] = color; // TBD remove this p.y hack which avoids flipping the framebuffer
				}
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
}

void init_projection (fmat4 *m, float val) {
	for (int i = 0; i < 4; i++)	fmat4_set (m, i, i, 1.0f);
	fmat4_set (m, 3, 2, val);
}

void init_view (fmat4 *m, float3 *eye, float3 *center, float3 *up) {
	
	float3 z, x, y;
	
	float3_float3_sub(eye, center, &z);
	float3_normalize (&z);
	float3_float3_crossprod(up, &z, &x);
	float3_normalize (&x);
	float3_float3_crossprod(&z, &x, &y);
	float3_normalize (&y);
	
	fmat4 Minv = FMAT4_IDENTITY;
	fmat4 Tr   = FMAT4_IDENTITY;
	
	for (int i = 0; i < 3; i++)	{
		fmat4_set (&Minv, 0, i, x[i]);
		fmat4_set (&Minv, 1, i, y[i]);
		fmat4_set (&Minv, 2, i, z[i]);
		fmat4_set (&Tr, i, 3, -(*center)[i]);
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

void obj_build_model (Object *obj) {
	// 1. scale	
	fmat4 s = FMAT4_IDENTITY;
	fmat4_set (&s, 0, 0, obj->scale[X]);
	fmat4_set (&s, 1, 1, obj->scale[Y]);
	fmat4_set (&s, 2, 2, obj->scale[Z]);
	// 2. rotate
	fmat4 rot_x, rot_xy, rot_xyz;
	rotate_coords (&s,      &rot_x,   obj->rotate[X], X);
	rotate_coords (&rot_x,  &rot_xy,  obj->rotate[Y], Y);
	rotate_coords (&rot_xy, &rot_xyz, obj->rotate[Z], Z);
	// 3. translate	
	fmat4 t = FMAT4_IDENTITY;
	fmat4_set (&t, 0, 3, obj->tran[X]);
	fmat4_set (&t, 1, 3, obj->tran[Y]);
	fmat4_set (&t, 2, 3, obj->tran[Z]);
	
	fmat4_fmat4_mult (&rot_xyz, &t, &(obj->model));
}

void obj_draw (Object *obj, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer) {
	for (int i = 0; i < wfobj_get_num_of_faces(obj->wfobj); i++) {
		ScreenTriangle t;
		for (int j = 0; j < 3; j++) {
			vshader (obj->wfobj, i, j, &(obj->mvpv), &t.vtx_coords[j]);
		}		
		draw_triangle (&t, pshader, zbuffer, fbuffer, obj->wfobj);        
    }
}
