#include "tgaimage.h"
#include "main.h"
#include "wavefront_obj.h"

#include <math.h>
#include <stdint.h>

const int width  = 800;//1280;
const int height = 800;//720;
const int depth  = 1024;
const int SCREEN_SIZE[3] = {width, height, depth};

#define TEXTURE_U_SIZE 1024
#define TEXTURE_V_SIZE 1024


void  world2screen2 (const float4 &w, Screen_coords &s) {
	s.x = (screenx_t) w[X] / w[W];
	s.y = (screeny_t) w[Y] / w[W];
	s.z = (screenz_t) w[Z] / w[W];
}

void fmat4_set (fmat4& mat, int row, int col, float val) {
	mat[row][col] = val;
}

float fmat4_get (fmat4& mat, int row, int col) {
	return mat[row][col];
}

void  fmat4_fmat4_mult (const fmat4& a, const fmat4& b, fmat4& c) {
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			c[i][j] = 0;
			for (int k = 0; k < 4; k++)
				c[i][j] += a[i][k]*b[k][j];
		}
}

void  fmat4_float4_mult (const fmat4 &a, const float4 &b, float4 &c) {
	for (int i = 0; i < 4; i++) {
		c[i] = 0;
		for (int j = 0; j < 4; j++)
			c[i] += a[i][j]*b[j];
	}
}

/*
void  float4_float3_conv (const float4& in, float3& out) {
	for (int i = 0; i < 3; i++) {
		out[i] = in[i]/in[W];
	}
}

void  float4_int3_conv (const float4 &in, int3 &out) {
	for (int i = 0; i < 3; i++) {
		out[i] = (int) (in[i]/in[W]);
	}
}
*/



int orient2d(const Screen_coords &a, const Screen_coords &b, const Screen_coords &c)
{
    return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
}

//void ()



//void min (max)

void draw_triangle (const Vertex *v, screenz_t *zbuffer, TGAImage &image, TGAImage &texture, Point3Df light_dir)
{
    // Compute triangle bounding box
    /*
    int2 min; // [0] = X, [1] = Y;
    int2 max; // [0] = X, [1] = Y;
    for (int i = 0; i < 2; i++) {
		min[i] = v[0].coords[i];
		max[i] = v[0].coords[i];
		for (int j = 1; j < 3; j++) {
			if (v[j].coords[i] > max[i]) max[i] = v[j].coords[i];
			if (v[j].coords[i] < min[i]) min[i] = v[j].coords[i];
		}
		if (max[i] >= SCREEN_SIZE[i])  max[i] = SCREEN_SIZE[i] - 1;
		if (min[i] < 0) min[i] = 0;
	}*/
	
	int2 min, max;
	min[0] = 0;
	min[1] = 0;
	max[0] = width;
	max[1] = height;
	// Rasterize
    Screen_coords p;
    p.z = 0;
    for (p.y = min[Y]; p.y < max[Y]; p.y++) {
        //printf("L\n");
        for (p.x = min[X]; p.x < max[X]; p.x++) {
            //printf("M\n");
            // Determine whether a point is to the left,
            // to the right, or on an edge of a triangle.
            // Repeat for all edges.
            int w0 = orient2d(v[1].coords, v[2].coords, p);
            int w1 = orient2d(v[2].coords, v[0].coords, p);
            int w2 = orient2d(v[0].coords, v[1].coords, p);
			// If p is on or inside all edges, render pixel.
            if ((w0 == 0) && (w1 == 0) && (w2 == 0)) continue;
			else if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                //printf ("orient2d: w0=%d, w1=%d, w2=%d, Z0=%d, Z1=%d, Z2=%d\n", w0, w1, w2, v[0].coords.z, v[1].coords.z, v[2].coords.z);
                int z = (v[0].coords.z*w0 + v[1].coords.z*w1 + v[2].coords.z*w2) / (w0 + w1 + w2);
                //if (z != 0) printf ("new Z = %d\n", z);
                //printf("N\n");
                if (zbuffer[p.x + p.y*width] < z) {
					zbuffer[p.x + p.y*width] = (screenz_t) z;
					
					int uu = (int) (TEXTURE_U_SIZE * (v[0].txt_uv.u*w0 + v[1].txt_uv.u*w1 + v[2].txt_uv.u*w2) / (w0 + w1 + w2));
					int vv = (int) (TEXTURE_V_SIZE * (v[0].txt_uv.v*w0 + v[1].txt_uv.v*w1 + v[2].txt_uv.v*w2) / (w0 + w1 + w2));
			
					if (uu < 0 || vv < 0 ) printf ("A");
					else if (uu >= TEXTURE_U_SIZE || vv >= TEXTURE_V_SIZE) printf ("B");
			
					TGAColor color = texture.get(uu, vv);
					
					// interpolate normals
					float intensity;
					bool phong = 0;
					bool gouraud = !phong;
					
					if (phong) {
						Point3Df intnorm;
						intnorm.x = (v[0].norm.x*w0 + v[1].norm.x*w1 + v[2].norm.x*w2) / (w0 + w1 + w2);
						intnorm.y = (v[0].norm.y*w0 + v[1].norm.y*w1 + v[2].norm.y*w2) / (w0 + w1 + w2);
						intnorm.z = (v[0].norm.z*w0 + v[1].norm.z*w1 + v[2].norm.z*w2) / (w0 + w1 + w2);
						intensity = (intnorm.x*light_dir.x + intnorm.y*light_dir.y + intnorm.z*light_dir.z) / (light_dir.x + light_dir.y + light_dir.z);
					}
					else if (gouraud) {
						float v0int = v[0].norm.x*light_dir.x + v[0].norm.y*light_dir.y + v[0].norm.z*light_dir.z;
						float v1int = v[1].norm.x*light_dir.x + v[1].norm.y*light_dir.y + v[1].norm.z*light_dir.z;
						float v2int = v[2].norm.x*light_dir.x + v[2].norm.y*light_dir.y + v[2].norm.z*light_dir.z;
						intensity = (v0int*w0 + v1int*w1 + v2int*w2) / (w0 + w1 + w2);
						intensity = -intensity;
					}
					if (intensity > 0) {
						color.r = color.r * intensity;
						color.g = color.g * intensity;
						color.b = color.b * intensity;
						image.set(p.x, p.y, color);
					}
					//image.set(p.x, p.y, TGAColor(255,255,255,255));
				}
			}
        }
    }
}




void init_viewport (fmat4 &vp, int x, int y, int w, int h, int d) {
	vp[0][0] = w / 2.0f;
	vp[0][3] = x + w / 2.0f;
	vp[1][1] = h / 2.0f;
	vp[1][3] = y + h / 2.0f;
	vp[2][2] = d / 2.0f;
	vp[2][3] = d / 2.0f;
	vp[3][3] = 1.0f;
}

int main(int argc, char** argv) {
    
    const int NUM_OF_VERTICES = 1258;
    const int NUM_OF_VTEXTURES = 1339;
    const int NUM_OF_FACES = 2492;
    const int NUM_OF_NORMALES = NUM_OF_VERTICES;
    
    float3   obj_vtx  [NUM_OF_VERTICES];
    Point3Df obj_norm [NUM_OF_NORMALES];
    Point2Df obj_text [NUM_OF_VTEXTURES];
    Face     obj_face [NUM_OF_FACES];
        
	read_obj_file ("obj/african_head.obj", obj_vtx, obj_norm, obj_text, obj_face);
    TGAImage image(width, height, TGAImage::RGB);
    
    TGAImage texture(1024, 1024, TGAImage::RGB);
    texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.flip_vertically();
    
    size_t    zbuffer_size = width*height;
    screenz_t zbuffer[zbuffer_size];
    for (int i = 0; i < zbuffer_size; i++)
		zbuffer[i] = 0;
    
    Point3Df light_dir = { 0.0f,  0.0f, -1.0f};
    float3   camera    = { 0.0f,  0.0f,  5.0f};
	
	// 0. Read local vertex coords
	// 1. Model - transform local coords to global
	// 2. View - transform global coords to adjust for camera position
	// 3. Projection - perspective correction
	// 4. Viewport - move to screen coords
	
	
	
	fmat4 mod      = {0};
	fmat4 view     = {0};
	fmat4 proj     = {0};
	fmat4 viewport = {0};
	fmat4 tmp      = {0};
	fmat4 tmp2     = {0};
	
	float4 tmp3 = {0};
	float4 tmp4 = {0};
	for (int i = 0; i < 4; i++)	{
		//fmat4_set (mod,      1.0f, i, i);
		//fmat4_set (view,     i, i, 2.0f);
		fmat4_set (proj,     i, i, 1.0f);
		//fmat4_set (viewport, i, i, 1.0f);
	}
	
	
	fmat4_set (proj, 3, 2, -1.0f / camera[Z]);
	init_viewport (viewport, 0, 0, SCREEN_SIZE[0], SCREEN_SIZE[1], SCREEN_SIZE[2]);
    
    for (int i = 0; i < NUM_OF_FACES; i++) {
	//for (int i = 13; i < 35; i++) {
        Face face = obj_face[i];
        
        // Fetch coords of vertices into three float[4] arrays
        Vertex vtx[3];
        float4 wc[3];
		float4 sc[3];
        
		// for each vertex j of a triangle
		for (int j = 0; j < 3; j++) {
			// for each coord of vertex j
			for (int k = 0; k < 3; k++) {
				wc[j][k] = obj_vtx[face.vtx_idx[j]][k];	
			}
			wc[j][W] = 1.0f; // W component
			
			fmat4_fmat4_mult  (viewport, proj, tmp);
			fmat4_float4_mult (tmp, wc[j], sc[j]);
			//fmat4_float4_mult (viewport, wc[j], sc[j]);
			
			world2screen2 (sc[j], vtx[j].coords);
			//vtx[j].coords = sc[j];
			vtx[j].txt_uv = obj_text[face.txt_idx[j]];
			vtx[j].norm   = obj_norm[face.vtx_idx[j]];
		}
		draw_triangle (vtx, zbuffer, image, texture, light_dir);        
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");

    return 0;
}



       /*
        if (0) {
			//vect3d_scal_prod(tri_normal, light_dir);
			
			// move two sides of the triangle to (0,0,0) each
        
			float3 f0;
			float3 f1;
			float3 tri_norm;
			for (int j = 0; j < 3; j++) {
				tri_norm[j] = f0[j] * f1[]
			}
			* 
			f0.x = w2.x - w0.x;
			f0.y = w2.y - w0.y;
			f0.z = w2.z - w0.z;
			Point3Df f1;
			f1.x = w1.x - w0.x;
			f1.y = w1.y - w0.y;
			f1.z = w1.z - w0.z;
			
			// cross product of two sides
			Point3Df tri_normal;
			tri_normal.x = f0.y*f1.z - f0.z*f1.y;
			tri_normal.y = f0.z*f1.x - f0.x*f1.z; 
			tri_normal.z = f0.x*f1.y - f0.y*f1.x;
			
			// normalize the cross product: divide each vector coordinate by the vector length
			float tri_normal_length = (float) sqrt(tri_normal.x*tri_normal.x + tri_normal.y*tri_normal.y + tri_normal.z*tri_normal.z);
			tri_normal.x = tri_normal.x/tri_normal_length;
			tri_normal.y = tri_normal.y/tri_normal_length;
			tri_normal.z = tri_normal.z/tri_normal_length;
		
			// scalar product, gives zero when normal is perpendicular to light vector
			float intensity = tri_normal.x*light_dir.x + tri_normal.y*light_dir.y + tri_normal.z*light_dir.z;
			//draw_triangle (p0, p1, p2, image, TGAColor(i%255, i%255, i%255, 255));
			//if (intensity > 0) draw_triangle (s0, s1, s2, zbuffer, image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
			if (intensity > 0) draw_triangle (s0, s1, s2, zbuffer, image, texture, intensity);
		}
		*/


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
