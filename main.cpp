#include "tgaimage.h"
#include "main.h"
#include "wavefront_obj.h"
#include <math.h>
#include <stdint.h>

const int width  = 800;//1280;
const int height = 800;//720;
const int depth  = 65536;
const int SCREEN_SIZE[3] = {width, height, depth};

#define TEXTURE_U_SIZE 1024
#define TEXTURE_V_SIZE 1024


void  world2screen (const float4 &w, ScreenPt &s) {
	s.x = (screenxy_t) w[X] / w[W];
	s.y = (screenxy_t) w[Y] / w[W];
	s.z = (screenz_t)  w[Z] / w[W];
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



int orient2d(const ScreenPt &a, const ScreenPt &b, const ScreenPt &c)
{
    return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
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

void draw_triangle (const Vertex *v, screenz_t *zbuffer, TGAImage &image, TGAImage &texture, float3 light_dir, float tri_intensity)
{
    // Compute triangle bounding box
    screenxy_t min_x = tri_min_bound (v[0].coords.x, v[1].coords.x, v[2].coords.x, 0);
    screenxy_t max_x = tri_max_bound (v[0].coords.x, v[1].coords.x, v[2].coords.x, SCREEN_SIZE[0]);
    
    screenxy_t min_y = tri_min_bound (v[0].coords.y, v[1].coords.y, v[2].coords.y, 0);
    screenxy_t max_y = tri_max_bound (v[0].coords.y, v[1].coords.y, v[2].coords.y, SCREEN_SIZE[1]);
    
    // Rasterize
    ScreenPt p;
    p.z = 0;
    for (p.y = min_y; p.y < max_y; p.y++) {
        for (p.x = min_x; p.x < max_x; p.x++) {
            // Determine whether a point is to the left,
            // to the right, or on an edge of a triangle.
            // Repeat for all edges.
            int w0 = orient2d(v[1].coords, v[2].coords, p);
            int w1 = orient2d(v[2].coords, v[0].coords, p);
            int w2 = orient2d(v[0].coords, v[1].coords, p);
			// If p is on or inside all edges, render pixel.
            if ((w0 == 0) && (w1 == 0) && (w2 == 0)) continue;
			else if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                int z = (v[0].coords.z*w0 + v[1].coords.z*w1 + v[2].coords.z*w2) / (w0 + w1 + w2); // TBD change to screenz_t or use p.z;
                if (zbuffer[p.x + p.y*width] < z) {
					zbuffer[p.x + p.y*width] = (screenz_t) z;
					
					int uu = (int) (TEXTURE_U_SIZE * (v[0].txt_uv.u*w0 + v[1].txt_uv.u*w1 + v[2].txt_uv.u*w2) / (w0 + w1 + w2));
					int vv = (int) (TEXTURE_V_SIZE * (v[0].txt_uv.v*w0 + v[1].txt_uv.v*w1 + v[2].txt_uv.v*w2) / (w0 + w1 + w2));
			
					if (uu < 0 || vv < 0 ) printf ("A");
					else if (uu >= TEXTURE_U_SIZE || vv >= TEXTURE_V_SIZE) printf ("B");
			
					TGAColor color = texture.get(uu, vv);
					
					// interpolate normals
					float pix_intensity = 0.0f;
					bool phong = 0;
					bool gouraud = 0;
					bool flat = 1;
					
					if (phong) {
						float3 intnorm;
						for (int i = 0; i < 3; i++)
							intnorm[i] = (v[0].norm[i]*w0 + v[1].norm[i]*w1 + v[2].norm[i]*w2) / (w0 + w1 + w2);
						//intnorm.y = (v[0].norm.y*w0 + v[1].norm.y*w1 + v[2].norm.y*w2) / (w0 + w1 + w2);
						//intnorm.z = (v[0].norm.z*w0 + v[1].norm.z*w1 + v[2].norm.z*w2) / (w0 + w1 + w2);
						
						for (int i = 0; i < 3; i++)
							pix_intensity += (intnorm[i]*light_dir[i]);
						//intensity /= (light_dir[0] + light_dir[1] + light_dir[2]);
					}
					else if (gouraud) {
						float3 intnorm;
						for (int i = 0; i < 3; i++)
							intnorm[i] = v[i].norm[0]*light_dir[0] + v[i].norm[1]*light_dir[1] + v[0].norm[2]*light_dir[2];
						//float v1int = v[1].norm.x*light_dir.x + v[1].norm.y*light_dir.y + v[1].norm.z*light_dir.z;
						//float v2int = v[2].norm.x*light_dir.x + v[2].norm.y*light_dir.y + v[2].norm.z*light_dir.z;
						pix_intensity = -(intnorm[0]*w0 + intnorm[1]*w1 + intnorm[2]*w2) / (w0 + w1 + w2);
					}
					float intensity;
					if (flat) intensity = tri_intensity;
					else intensity = pix_intensity;
					
					if (intensity > 0) {
						color.r = color.r * intensity;
						color.g = color.g * intensity;
						color.b = color.b * intensity;
						image.set(p.x, p.y, color);
					}
				}
			}
        }
    }
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
	
	fmat4 Minv;
	fmat4 Tr;
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
	
	//for (int i = 0; i < 4; i++)	
	//	fmat4_set (m, i, i, 1.0f);
	
}


void init_model (fmat4 &m, const float3 &scale, const float3 &tran) {
	
	// scale - rotate - translate
	
	// 1. scale	
	fmat4_set (m, 0, 0, scale[X]);
	fmat4_set (m, 1, 1, scale[Y]);
	fmat4_set (m, 2, 2, scale[Z]);
	fmat4_set (m, 3, 3, 1.0f);
	
	// 3. translate	
	fmat4_set (m, 0, 3, tran[X]);
	fmat4_set (m, 1, 3, tran[Y]);
	fmat4_set (m, 2, 3, tran[Z]);
	fmat4_set (m, 3, 3, 1.0f);
}
	
void float3_float3_sub (const float3 &a, const float3 &b, float3 &c) {
	for (int i = 0; i < 3; i++) c[i] = a[i] - b[i];
}
float float3_float3_smult (const float3 &a, const float3 &b) {
	float smult = 0;
	for (int i = 0; i < 3; i++ ) smult += a[i]*b[i];
	return smult;
}

void float3_normalize (float3 &v) {
	float length = (float) sqrt(pow(v[0], 2) + pow(v[1], 2) + pow(v[2], 2));
	for (int i = 0; i < 3; i++) v[i] = v[i] / length;
}

void float3_float3_crossprod(const float3 &a, const float3 &b, float3 &c) {
	c[0] = a[1]*b[2] - a[2]*b[1];
	c[1] = a[2]*b[0] - a[0]*b[2]; 
	c[2] = a[0]*b[1] - a[1]*b[0];
}

int main(int argc, char** argv) {
    
    
    const int NUM_OF_VERTICES  = 1258;
    const int NUM_OF_VTEXTURES = 1339;
    const int NUM_OF_FACES     = 2492;
    const int NUM_OF_NORMALES  = NUM_OF_VERTICES;
   
    
    float3   obj_vtx  [NUM_OF_VERTICES]  = {0};
    float3   obj_norm [NUM_OF_NORMALES]  = {0};
    Point2Df obj_text [NUM_OF_VTEXTURES] = {0};
    Face     obj_face [NUM_OF_FACES]     = {0};
        
	read_obj_file ("obj/african_head.obj", obj_vtx, obj_norm, obj_text, obj_face);
	//read_obj_file ("obj/cube.obj", obj_vtx, obj_norm, obj_text, obj_face);
    TGAImage image(width, height, TGAImage::RGB);
    
    TGAImage texture(1024, 1024, TGAImage::RGB);
    texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.flip_vertically();
    
    size_t    zbuffer_size = width*height;
    screenz_t zbuffer[zbuffer_size];
    for (int i = 0; i < zbuffer_size; i++)
		zbuffer[i] = 0;
    
    float3 light_dir = { 0.0f,  0.0f, -1.0f};
    float3 eye       = { 0.0f,  0.0f,  5.0f};
    float3 center    = { 0.0f,  0.0f,  0.0f};
    float3 up        = { 0.0f, -1.0f,  0.0f};
	
	float3_normalize (light_dir);
        
	float3 camera;	
	float3_float3_sub(eye, center, camera);
	printf ("camera: x=%f, y=%f, z=%f\n", camera[0], camera[1], camera[2]);
	//float3_normalize (camera);
	printf ("camera norm: x=%f, y=%f, z=%f\n", camera[0], camera[1], camera[2]);
	
	
	
	// 0. Read local vertex coords
	// 1. Model - transform local coords to global
	// 2. View - transform global coords to adjust for camera position
	// 3. Projection - perspective correction
	// 4. Viewport - move to screen coords
	
	
	
	fmat4 model      = {0};
	fmat4 view       = {0};
	fmat4 projection = {0};
	fmat4 viewport   = {0};
	fmat4 tmp        = {0};
	fmat4 tmp2       = {0};
	
	float3 scale = {0.5f , 0.5f, 0.5f};
	float3 tran  = {0.25f, 0.0f, 0.0f};
	
	init_model      (model, scale, tran);
	init_viewport   (viewport, 0, 0, SCREEN_SIZE[0], SCREEN_SIZE[1], SCREEN_SIZE[2]);
	init_projection (projection, -1.0f/camera[Z]);
	init_view       (view, eye, center, up);
    
    for (int i = 0; i < NUM_OF_FACES; i++) {
	//for (int i = 13; i < 35; i++) {
        Face face = obj_face[i];
        
        // Fetch coords of vertices into three float[4] arrays
        Vertex vtx[3];
        float4 mc[3]; // model coordinates
        float4 wc[3]; // world coordinates
        float4 vc[3]; // view coordinates
        float4 pc[3]; // projection coordinates
		float4 sc[3]; // screen coordinates
		
        
		// for each vertex j of a triangle
		for (int j = 0; j < 3; j++) {
			// for each coord of vertex j
			for (int k = 0; k < 3; k++) {
				mc[j][k] = obj_vtx[face.vtx_idx[j]][k];	
			}
			mc[j][W] = 1.0f; // W component
			
			fmat4_float4_mult (model, mc[j], wc[j]);
			fmat4_float4_mult (view, wc[j], vc[j]);
			fmat4_float4_mult (projection, wc[j], pc[j]);
			fmat4_float4_mult (viewport, pc[j], sc[j]);
			
			world2screen (sc[j], vtx[j].coords);
			vtx[j].txt_uv = obj_text[face.txt_idx[j]];
			for (int k = 0; k < 3; k++) {
				vtx[j].norm[k]   = obj_norm[face.vtx_idx[j]][k];
			}
			
		}
		
		float tri_intensity = 0;
		bool flat = 1;
		if (flat) {
			// flat shading
			 
			// move two sides of the triangle to (0,0,0) each
			
			float3 w[3];
			for (int k = 0; k < 3; k++)
				for (int m = 0; m < 3; m++)
					w[k][m] = wc[k][m];
				
			float3 f0;
			float3 f1;
			float3_float3_sub (w[2], w[0], f0); 
			float3_float3_sub (w[1], w[0], f1); 
			
			// cross product of two sides
			float3 tri_normal;
			float3_float3_crossprod (f0, f1, tri_normal);
			
			// normalize the cross product: divide each vector coordinate by the vector length
			float3_normalize (tri_normal);
		
			// scalar product, gives zero when normal is perpendicular to light vector
			//tri_intensity = float3_float3_smult (tri_normal, light_dir);
			tri_intensity = float3_float3_smult (tri_normal, light_dir);
		}
		draw_triangle (vtx, zbuffer, image, texture, light_dir, tri_intensity);        
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");

    return 0;
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
