#include "gl.h"
#include "geometry.h"
#include "main.h" // TBD -remove
#include "tgaimage.h"
#include <math.h>

int orient2d(const ScreenPt &a, const ScreenPt &b, const ScreenPt &c)
{
    return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
}

void  world2screen (const float4 &w, ScreenPt &s) {
	s.x = (screenxy_t) w[X] / w[W];
	s.y = (screenxy_t) w[Y] / w[W];
	s.z = (screenz_t)  w[Z] / w[W];
}


screenxy_t tri_min_bound (const screenxy_t a, const screenxy_t b, const screenxy_t c, const screenxy_t cutoff) {
	int min = a;
	if (b < min) min = b;
	if (c < min) min = c;
	if (min < cutoff) min = cutoff;
	return min;
}

screenxy_t tri_max_bound (const screenxy_t a, const screenxy_t b, const screenxy_t c, const screenxy_t cutoff) {
	int max = a;
	if (b > max) max = b;
	if (c > max) max = c;
	if (max > cutoff) max = cutoff;
	return max;
}

void init_viewport (fmat4 &m, int x, int y, int w, int h, int d) {
	fmat4_set (m, 0, 0, w / 2.0f);
	fmat4_set (m, 0, 3, x + w / 2.0f);
	fmat4_set (m, 1, 1, h / 2.0f);
	fmat4_set (m, 1, 3, y + h / 2.0f);
	fmat4_set (m, 2, 2, d / 2.0f);
	fmat4_set (m, 2, 3, d / 2.0f);
	fmat4_set (m, 3, 3, 1.0f);
}

void init_projection (fmat4 &m, const float val) {
	for (int i = 0; i < 4; i++)	fmat4_set (m, i, i, 1.0f);
	fmat4_set (m, 3, 2, val);
}

void init_view (fmat4 &m, const float3 &eye, const float3 &center, const float3 &up) {
	
	float3 z, x, y;
	
	float3_float3_sub(eye, center, z);
	float3_normalize (z);
	float3_float3_crossprod(up, z, x);
	float3_normalize (x);
	float3_float3_crossprod(z, x, y);
	float3_normalize (y);
	
	fmat4 Minv = {0};
	fmat4 Tr = {0};
	for (int i = 0; i < 4; i++)	{
		fmat4_set (Minv, i, i, 1.0f);
		fmat4_set (Tr, i, i, 1.0f);
	}
	
	for (int i = 0; i < 3; i++)	{
		fmat4_set (Minv, 0, i, x[i]);
		fmat4_set (Minv, 1, i, y[i]);
		fmat4_set (Minv, 2, i, z[i]);
		fmat4_set (Tr, i, 3, -center[i]);
	}
	fmat4_fmat4_mult(Minv, Tr, m);
	//fmat4_fmat4_mult(Tr, Minv, m);
	
	
	//for (int i = 0; i < 4; i++)	
	//	fmat4_set (m, i, i, 1.0f);
	
}

void rotate_coords (const fmat4 &in, fmat4 &out, float alpha_deg, axis axis) {
	float rad = alpha_deg * 0.01745f; // degrees to rad conversion
	float sin_alpha = sin(rad);
	float cos_alpha = cos(rad);
	
	fmat4 r = {0};
	
	fmat4_set (r, 0, 0, (axis == X) ? 1.0f : cos_alpha);
	fmat4_set (r, 1, 1, (axis == Y) ? 1.0f : cos_alpha);
	fmat4_set (r, 2, 2, (axis == Z) ? 1.0f : cos_alpha);
	fmat4_set (r, 3, 3,  1.0f);
	
	fmat4_set (r, 0, 1, (axis == Z) ? -sin_alpha : 0.0f);
	fmat4_set (r, 0, 2, (axis == Y) ? -sin_alpha : 0.0f);
	fmat4_set (r, 1, 2, (axis == X) ? -sin_alpha : 0.0f);
	
	fmat4_set (r, 1, 0, (axis == Z) ?  sin_alpha : 0.0f);
	fmat4_set (r, 2, 0, (axis == Y) ?  sin_alpha : 0.0f);
	fmat4_set (r, 2, 1, (axis == X) ?  sin_alpha : 0.0f);
	
	fmat4_fmat4_mult (in, r, out);
}

void init_model (fmat4 &m, const float3 &scale, const float3 &rotate, const float3 &tran) {
	
	// scale - rotate - translate
	
	// 1. scale	
	fmat4 s = {0};
	fmat4_set (s, 0, 0, scale[X]);
	fmat4_set (s, 1, 1, scale[Y]);
	fmat4_set (s, 2, 2, scale[Z]);
	fmat4_set (s, 3, 3, 1.0f);
	
	// 2. rotate
	fmat4 tmp1, tmp2, r;
	rotate_coords (   s, tmp1, rotate[X], X);
	rotate_coords (tmp1, tmp2, rotate[Y], Y);
	rotate_coords (tmp2,    r, rotate[Z], Z);
		
	// 3. translate	
	fmat4 t = {0};
	for (int i = 0; i < 4; i++)	
		fmat4_set (t, i, i, 1.0f);
	fmat4_set (t, 0, 3, tran[X]);
	fmat4_set (t, 1, 3, tran[Y]);
	fmat4_set (t, 2, 3, tran[Z]);
	
	fmat4_fmat4_mult (r, t, m);
}

void draw_triangle (const Vertex *v, pixel_shader shader, screenz_t *zbuffer, TGAImage &image, TGAImage &texture, float3 light_dir, float tri_intensity)
{
    // Compute triangle bounding box
    screenxy_t min_x = tri_min_bound (v[0].coords.x, v[1].coords.x, v[2].coords.x, 0);
    screenxy_t max_x = tri_max_bound (v[0].coords.x, v[1].coords.x, v[2].coords.x, SCREEN_SIZE[0]);
    
    screenxy_t min_y = tri_min_bound (v[0].coords.y, v[1].coords.y, v[2].coords.y, 0);
    screenxy_t max_y = tri_max_bound (v[0].coords.y, v[1].coords.y, v[2].coords.y, SCREEN_SIZE[1]);
    
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
	
	int A01 = v[0].coords.y - v[1].coords.y;
	int A12 = v[1].coords.y - v[2].coords.y;
	int A20 = v[2].coords.y - v[0].coords.y;
	int B01 = v[1].coords.x - v[0].coords.x;
	int B12 = v[2].coords.x - v[1].coords.x;
	int B20 = v[0].coords.x - v[2].coords.x;
	
	int3 bar_row;
	bar_row[0] = orient2d(v[1].coords, v[2].coords, test_pt); // not normalized
    bar_row[1] = orient2d(v[2].coords, v[0].coords, test_pt); // not normalized
    bar_row[2] = orient2d(v[0].coords, v[1].coords, test_pt); // not normalized
            
	int sum_of_bars = bar_row[0] + bar_row[1] + bar_row[2];
	if (sum_of_bars == 0) {
		printf ("Im gonna die!!!\n");
		return;
	}
	int z1z0_over_sob = (v[1].coords.z - v[0].coords.z) / sum_of_bars;
	int z2z0_over_sob = (v[2].coords.z - v[0].coords.z) / sum_of_bars;
	
    ScreenPt p;
    p.z = 0;
    for (p.y = min_y; p.y < max_y; p.y++) {
		
		int3 bar;		
		for (int i = 0; i < 3; i++) bar[i] = bar_row[i];
		
        for (p.x = min_x; p.x < max_x; p.x++) {
            // Determine whether a point is to the left,
            // to the right, or on the edge of a triangle.
            // Repeat for all edges.
            //int3 barc;
            /*
             * bar[0] = orient2d(v[1].coords, v[2].coords, p); // not normalized TBD remove
            bar[1] = orient2d(v[2].coords, v[0].coords, p); // not normalized
            bar[2] = orient2d(v[0].coords, v[1].coords, p); // not normalized
            */
            // If p is on or inside all edges, render pixel.
            if ((bar[0] | bar[1] | bar[2]) > 0) {
								
				int z = v[0].coords.z + bar[1]*z1z0_over_sob + bar[2]*z2z0_over_sob; // TBD change to screenz_t or use p.z;
				
				if (zbuffer[p.x + p.y*width] < z) {
					zbuffer[p.x + p.y*width] = (screenz_t) z;
					
					TGAColor color;
					bool draw = shader (v, bar, color);
					if (draw) image.set (p.x, p.y, color);
				}
			}
            /*int barc_sum = 0;
            for (int i = 0; i < 3; i++) barc_sum += barc[i];
            // If p is on or inside all edges, render pixel.
            //if ((barc[0] == 0) && (barc[1] == 0) && (barc[2] == 0)) continue;
			if (barc_sum == 0) continue;
			else if (barc[0] >= 0 && barc[1] >= 0 && barc[2] >= 0) {
                int z = 0; // TBD change to screenz_t or use p.z;
                for (int i = 0; i < 3; i++) z += v[i].coords.z*barc[i];
                z /= barc_sum;
                
                if (zbuffer[p.x + p.y*width] < z) {
					zbuffer[p.x + p.y*width] = (screenz_t) z;
					
					TGAColor color;
					bool draw = shader (v, barc, color);
					if (draw) image.set (p.x, p.y, color);
				}
			}
			*/
			bar[0] += A12;
			bar[1] += A20;
			bar[2] += A01;
        }
        
        bar_row[0] += B12;
        bar_row[1] += B20;
        bar_row[2] += B01;
    }
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
