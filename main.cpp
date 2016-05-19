#include "tgaimage.h"
#include "main.h"
#include "wavefront_obj.h"
#include "geometry.h"
#include "gl.h"
#include "dyn_array.h"
#include <stdint.h>


/*
// globals

const int NUM_OF_VERTICES  = 1258;
const int NUM_OF_VTEXTURES = 1339;
const int NUM_OF_FACES     = 2492;
const int NUM_OF_NORMALES  = NUM_OF_VERTICES;
    
float3   obj_vtx  [NUM_OF_VERTICES]  = {0};
float3   obj_norm [NUM_OF_NORMALES]  = {0};
Point2Df obj_text [NUM_OF_VTEXTURES] = {0};
Face     obj_face [NUM_OF_FACES]     = {0};
*/
TGAImage texture(1024, 1024, TGAImage::RGB);


void my_vertex_shader (const fmat4 &model, const fmat4 &view, const fmat4 &projection, const fmat4 &viewport, const float3 &vtx3d, ScreenPt &sp) {
	
	float4 mc; // model coordinates
    float4 wc; // world coordinates
    float4 vc; // view coordinates
    float4 pc; // projection coordinates
	float4 sc; // screen coordinates
		
	// for each coord of vertex j
	for (int k = 0; k < 3; k++) {
		mc[k] = vtx3d[k];	
	}
	mc[W] = 1.0f; // W component
	
	fmat4_float4_mult (model, mc, wc);
	fmat4_float4_mult (view, wc, vc);
	fmat4_float4_mult (projection, vc, pc);
	fmat4_float4_mult (viewport, pc, sc);
	
	world2screen (sc, sp);
};

bool my_pixel_shader (const Vertex *v, const int3 &barc, TGAColor &color) {
	
	int barc_sum = 0;
	for (int i = 0; i < 3; i++) barc_sum += barc[i];
	
	int uu = (int) (TEXTURE_U_SIZE * (v[0].txt_uv.u*barc[0] + v[1].txt_uv.u*barc[1] + v[2].txt_uv.u*barc[2]) / barc_sum);
	int vv = (int) (TEXTURE_V_SIZE * (v[0].txt_uv.v*barc[0] + v[1].txt_uv.v*barc[1] + v[2].txt_uv.v*barc[2]) / barc_sum);

	TGAColor tmpcolor = texture.get(uu, vv);
	
	//TGAColor tmpcolor = TGAColor (255, 255, 255, 255);
	// interpolate normals
	float intensity = 0;
	
	bool phong = 0;
	bool gouraud = !phong;
	bool flat = 0;
	
	float3 intnorm = {0, 0, 0};
	float3 light_dir  = { 0.0f,   0.0f,  -1.0f}; // TBD - remove
	
	if (phong) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++)
				intnorm[i] += v[j].norm[i]*barc[j];
			intnorm[i] /= (barc[0] + barc[1] + barc[2]);
			intensity += (intnorm[i]*light_dir[i]);
		}			
	}
	else if (gouraud) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++)
				intnorm[i] += v[i].norm[j]*light_dir[j];
			intensity += intnorm[i]*barc[i];
		}
		intensity /= -(barc[0] + barc[1] + barc[2]);
	}
	//else if (flat) intensity = tri_intensity;
	
	if (intensity > 0) {
		color.r = tmpcolor.r * intensity;
		color.g = tmpcolor.g * intensity;
		color.b = tmpcolor.b * intensity;
		return true;
	}
	return false;
}

    
int main(int argc, char** argv) {
    
    
    const int NUM_OF_VERTICES  = 1258;
    const int NUM_OF_VTEXTURES = 1339;
    const int NUM_OF_FACES     = 2492;
    const int NUM_OF_NORMALES  = NUM_OF_VERTICES;
   
    //float3   obj_vtx  [NUM_OF_VERTICES]  = {0};
    float3   obj_norm [NUM_OF_NORMALES]  = {0};
    Point2Df obj_text [NUM_OF_VTEXTURES] = {0};
    Face     obj_face [NUM_OF_FACES]     = {0};
    
    
	DynArray *obj_vtx  = dyn_array_create (sizeof(float), 384);
	//DynArray obj_norm = dyn_array_create (sizeof(float),   256);
	//DynArray obj_text = dyn_array_create (sizeof(Point2Df), 256);
	//DynArray obj_face = dyn_array_create (sizeof(Face),     256);
	
	read_obj_file ("obj/african_head.obj", obj_vtx, obj_norm, obj_text, obj_face);
	//read_obj_file ("obj/cube.obj", obj_vtx, obj_norm, obj_text, obj_face);
    TGAImage image(width, height, TGAImage::RGB);
    
    //TGAImage texture(1024, 1024, TGAImage::RGB);
    texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.flip_vertically();
    
    size_t    zbuffer_size = width*height;
    screenz_t zbuffer[zbuffer_size];
    for (int i = 0; i < zbuffer_size; i++)
		zbuffer[i] = 0;
    
    float3 light_dir  = { 0.0f,   0.0f,  -1.0f};
    float3 eye        = { 0.0f,   0.0f,   5.0f};
    float3 center     = { 0.0f,   0.0f,   0.0f};
    float3 up         = { 0.0f,  -0.2f,   0.0f};
	float3 obj_scale  = { 0.5f,   0.5f,   0.5f};
	float3 obj_rotate = {15.0f,  15.0f,  15.0f};
	float3 obj_tran   = { 0.25f, -0.25f,  0.0f};
	
	float3_normalize (light_dir);
        
	float3 camera;	
	float3_float3_sub(eye, center, camera);
	printf ("camera: x=%f, y=%f, z=%f\n", camera[0], camera[1], camera[2]);
	float3_normalize (camera);
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
		
	init_model      (model, obj_scale, obj_rotate, obj_tran);
	init_viewport   (viewport, 0, 0, SCREEN_SIZE[0], SCREEN_SIZE[1], SCREEN_SIZE[2]);
	init_projection (projection, -1.0f/camera[Z]);
	init_view       (view, eye, center, up);
    
    
    for (int i = 0; i < NUM_OF_FACES; i++) {
	//for (int i = 13; i < 35; i++) {
        Face face = obj_face[i];
        
        // for each vertex j of a triangle
        Vertex vtx[3];     
		for (int j = 0; j < 3; j++) {
			float3 tmp;
			//tmp = (float3) dyn_array_get(obj_vtx, face.vtx_idx[j]*3);
			for (int k = 0; k < 3; k++) {
				//tmp = (float*) dyn_array_get(obj_vtx, face.vtx_idx[j]*3 + k);
				//tmp2[k] = *tmp;
				//tmp[k] = *(float**) dyn_array_get(obj_vtx, face.vtx_idx[j]*3 + k);
				tmp[k] = *((float*) obj_vtx->data[face.vtx_idx[j]*3 + k]);
				//printf ("face %d, vertex %d, coord %d = %f, face.vtx_idx[j] = %d\n", i, j, k, tmp2, face.vtx_idx[j]);
			}
			//printf ("world coords for vtx shader: x = %f, y = %f, z = %f\n", tmp[0], tmp[1], tmp[2]);
			my_vertex_shader (model, view, projection, viewport, tmp, vtx[j].coords);
			vtx[j].txt_uv = obj_text[face.txt_idx[j]];
			for (int k = 0; k < 3; k++) {
				vtx[j].norm[k]   = obj_norm[face.vtx_idx[j]][k];
			}
		}
		
		float tri_intensity = 0;
		/*bool flat = 1;
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
		}*/
		draw_triangle (vtx, my_pixel_shader, zbuffer, image, texture, light_dir, tri_intensity);        
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");

	dyn_array_destroy (obj_vtx);
	
    return 0;
}


