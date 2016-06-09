#include "gl.h"
#include "geometry.h"
#include "shader.h"
#include "main.h" // TBD -remove
#include "wavefront_obj.h"

#include <math.h>
#include <stdlib.h>

int orient2d(ScreenPt *a, ScreenPt *b, ScreenPt *c)
{
    return (b->x - a->x)*(c->y - a->y) - (b->y - a->y)*(c->x - a->x);
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

void rotate_coords (fmat4 *in, fmat4 *out, float alpha_deg, axis axis) {
	float rad = alpha_deg * 0.01745f; // degrees to rad conversion
	float sin_alpha = sin(rad);
	float cos_alpha = cos(rad);
	
	fmat4 r = FMAT4_IDENTITY;
	
	fmat4_set (&r, 0, 0, (axis == X) ? 1.0f : cos_alpha);
	fmat4_set (&r, 1, 1, (axis == Y) ? 1.0f : cos_alpha);
	fmat4_set (&r, 2, 2, (axis == Z) ? 1.0f : cos_alpha);
	
	fmat4_set (&r, 0, 1, (axis == Z) ? -sin_alpha : 0.0f);
	fmat4_set (&r, 0, 2, (axis == Y) ? -sin_alpha : 0.0f);
	fmat4_set (&r, 1, 2, (axis == X) ? -sin_alpha : 0.0f);
	
	fmat4_set (&r, 1, 0, (axis == Z) ?  sin_alpha : 0.0f);
	fmat4_set (&r, 2, 0, (axis == Y) ?  sin_alpha : 0.0f);
	fmat4_set (&r, 2, 1, (axis == X) ?  sin_alpha : 0.0f);
	
	fmat4_fmat4_mult (in, &r, out);
}

void init_model (fmat4 *m, float3 *scale, float3 *rotate, float3 *tran) {
	
	// 1. scale	
	fmat4 s = FMAT4_IDENTITY;
	fmat4_set (&s, 0, 0, (*scale)[X]);
	fmat4_set (&s, 1, 1, (*scale)[Y]);
	fmat4_set (&s, 2, 2, (*scale)[Z]);
	// 2. rotate
	fmat4 rot_x, rot_xy, rot_xyz;
	rotate_coords (&s,      &rot_x,   (*rotate)[X], X);
	rotate_coords (&rot_x,  &rot_xy,  (*rotate)[Y], Y);
	rotate_coords (&rot_xy, &rot_xyz, (*rotate)[Z], Z);
	// 3. translate	
	fmat4 t = FMAT4_IDENTITY;
	fmat4_set (&t, 0, 3, (*tran)[X]);
	fmat4_set (&t, 1, 3, (*tran)[Y]);
	fmat4_set (&t, 2, 3, (*tran)[Z]);
	
	fmat4_fmat4_mult (&rot_xyz, &t, m);
}

void draw_triangle (ScreenTriangle *st, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer, WFobj *obj)
{
    
    Triangle t;
    for (int i = 0; i < 3; i++) {
		t.cx[i] = (screenxy_t) st->vtx_coords[i][X];
		t.cy[i] = (screenxy_t) st->vtx_coords[i][Y];
		t.cz[i] = (screenz_t)  st->vtx_coords[i][Z];
		t.cw[i] = st->vtx_coords[i][W];
	}    
    
    // Compute triangle bounding box
    screenxy_t min_x = tri_min_bound (t.cx[0], t.cx[1], t.cx[2], 0);
    screenxy_t max_x = tri_max_bound (t.cx[0], t.cx[1], t.cx[2], WIDTH);
    
    screenxy_t min_y = tri_min_bound (t.cy[0], t.cy[1], t.cy[2], 0);
    screenxy_t max_y = tri_max_bound (t.cy[0], t.cy[1], t.cy[2], HEIGHT);
    
    
    //for (int i = 0; i < 3; i++) printf ("vertex[%d] x:y:z = %d:%d:%d\n", i, t.cx[i], t.cy[i], t.cz[i]);
    //printf ("min/max:  x:%d/%d, y:%d/%d\n", min_x, max_x, min_y, max_y);
    
    if ((min_x == max_x) || (min_y == max_y)) return; // degenerate triangle
    
    // Rasterize:
    // 1. compute barycentric coordinates (bar0,bar1,bar2), don't normalize them
    // 1.a. dumb method: just compute all the values for each pixel
    // 2.a. smart method: compute values once per triangle, then just increment every
    //      pixel and every row
    // 2. interpolate values such as Z for every pixel:
    // 2.a. dumb method: Z = (z0*bar0+z1*bar1+z2*bar2)/sum_of_bar
	//      *divide by sum_of_bar because bar values are not normalized
	// 2.b. smart method: Z = z0 + bar1*(z1-z0)/sum_of_bar + bar2*(z2-z0)/sum_of_bar
	//      *can get rid of bar0
	//      **(z1-z0)/sum_of_bar is constant for a triangle
	//      ***(z2-z0)/sum_of_bar is constant for a triangle
	
	
	ScreenPt test_pt;
	test_pt.x = min_x;
	test_pt.y = min_y;
	test_pt.z = 0;
	
	int A01 = t.cy[0] - t.cy[1];//v[0].coords.y - v[1].coords.y;
	int A12 = t.cy[1] - t.cy[2];//v[1].coords.y - v[2].coords.y;
	int A20 = t.cy[2] - t.cy[0];//v[2].coords.y - v[0].coords.y;
	int B01 = t.cx[1] - t.cx[0];//v[1].coords.x - v[0].coords.x;
	int B12 = t.cx[2] - t.cx[1];//v[2].coords.x - v[1].coords.x;
	int B20 = t.cx[0] - t.cx[2];//v[0].coords.x - v[2].coords.x;
	
	int3 bar_row;
	
	Vertex v[3];
	for (int i = 0; i < 3; i++) {
		v[i].coords.x = t.cx[i];
		v[i].coords.y = t.cy[i];
		v[i].coords.z = t.cz[i];
	}	
	bar_row[0] = orient2d(&v[1].coords, &v[2].coords, &test_pt); // not normalized
    bar_row[1] = orient2d(&v[2].coords, &v[0].coords, &test_pt); // not normalized
    bar_row[2] = orient2d(&v[0].coords, &v[1].coords, &test_pt); // not normalized
          
    float3 bar_clip;
    float sum_of_bars = 0;
    for (int i = 0; i < 3; i++) {
		bar_clip[i] = (float) bar_row[i] / t.cw[i];
		sum_of_bars += bar_clip[i];
	}

	if (sum_of_bars == 0) {
		printf ("Im gonna die!!!\n");
		return;
	}
	   
    ScreenPt p;
    p.z = 0;
    for (p.y = min_y; p.y < max_y; p.y++) {
		
		int3 bar;
		for (int i = 0; i < 3; i++) bar[i] = bar_row[i];	
		
        for (p.x = min_x; p.x < max_x; p.x++) {
			// If p is on or inside all edges, render pixel.
            if ((bar[0] | bar[1] | bar[2]) > 0) {
				sum_of_bars = 0.0f;
				for (int i = 0; i < 3; i++) {
					bar_clip[i] = (float) bar[i]/t.cw[i]; // not normalized
					sum_of_bars += bar_clip[i];
				}
				for (int i = 0; i < 3; i++) bar_clip[i] /= sum_of_bars;
				p.z = (int) t.cz[0] + bar_clip[1]*(t.cz[1]-t.cz[0]) + bar_clip[2]*(t.cz[2] - t.cz[0]); // TBD change to screenz_t or use p.z;
				if (zbuffer[p.x + p.y*WIDTH] < p.z) {
					zbuffer[p.x + p.y*WIDTH] = p.z;
					pixel_color_t color;// = TGAColor (255, 255, 255, 255);
					bool draw = pshader (obj, &bar_clip, &color);
					if (draw) fbuffer[p.x + (719 - p.y)*WIDTH] = color; // TBD remove this p.y hack which avoids flipping the framebuffer
				}
				/*else {
					TGAColor color3 = TGAColor (255, 0, 255, 255);
					image.set (p.x, p.y, color3);
				}*/
			}
            
			bar[0] += A12;
			bar[1] += A20;
			bar[2] += A01;
        }
        
        bar_row[0] += B12;
        bar_row[1] += B20;
        bar_row[2] += B01;
    }
}


//void draw_obj (WFobj *obj, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer, fmat4 *mvpv) {
void obj_draw (Object *obj, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer) {
	for (int i = 0; i < wfobj_get_num_of_faces(obj->wfobj); i++) {
		ScreenTriangle t;
		for (int j = 0; j < 3; j++) {
			vshader (obj->wfobj, i, j, obj->mvpv, &t.vtx_coords[j]);
		}		
		draw_triangle (&t, pshader, zbuffer, fbuffer, obj->wfobj);        
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
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
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

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i=2; i--; ) {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}
*/

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

void obj_transform (Object *obj, fmat4 *vpv, fmat4 *projview, float3 *light_dir) {
	
	init_model       (&(obj->model), &(obj->scale), &(obj->rotate), &(obj->tran));
	fmat4_fmat4_mult (vpv,      &(obj->model), &(obj->mvpv)); 
    fmat4_fmat4_mult (projview, &(obj->model), &UNIFORM_M);
	fmat4_invert     (&UNIFORM_M, &UNIFORM_MI);
	fmat4_transpose  (&UNIFORM_MI, &UNIFORM_MIT);
	
	float4 light_dir4, light_new;
	float3_float4_conv (light_dir, &light_dir4);
	fmat4_float4_mult  (&UNIFORM_M, &light_dir4, &light_new);
	float4_float3_conv (&light_new, &UNIFORM_LIGHT);
    float3_normalize   (&UNIFORM_LIGHT);
}
