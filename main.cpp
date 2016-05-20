#include "tgaimage.h"
#include "main.h"
#include "wavefront_obj.h"
#include "geometry.h"
#include "gl.h"
#include "dyn_array.h"
#include <stdint.h>


// globals:

TGAImage texture(1, 1, TGAImage::RGB);
//TGAImage texture;
int textw;
int texth;

float3 light_dir  = { 0.0f,   1.0f,   1.0f};
float3 eye        = { 5.0f,   5.0f,   5.0f};
float3 center     = { 0.0f,   0.0f,   0.0f};
float3 up         = { 0.0f,   1.0f,   0.0f};
	


void my_vertex_shader (const fmat4 &model, const fmat4 &view, const fmat4 &projection, const fmat4 &viewport, const float3 &vtx3d, ScreenPt &sp) {
	
	float4 mc; // model coordinates
    float4 wc; // world coordinates
    float4 vc; // view coordinates
    float4 pc; // projection coordinates
	float4 sc4d; // 4d screen coordinates
	float3 sc3d; // 3d screend coordinates
		
	// transform 3d coords to homogenous coords
	float3_float4_conv (vtx3d, mc);
	
	
	// 1. Model - transform local coords to global
	// 2. View - transform global coords to adjust for camera position
	// 3. Projection - perspective correction
	// 4. Viewport - move to screen coords
	fmat4_float4_mult (model, mc, wc);
	fmat4_float4_mult (view, wc, vc);
	fmat4_float4_mult (projection, vc, pc);
	fmat4_float4_mult (viewport, pc, sc4d);
	
	// transform homogenous coords back to 3d
	//float4_float3_conv (sc4d, sc3d);
	float4_float3_conv (sc4d, sc3d);
	sp.x = (screenxy_t) sc3d[0];
	sp.y = (screenxy_t) sc3d[1];
	sp.z = (screenz_t)  sc3d[2];
	
	printf ("[vertex shader] 4Df: %f/%f/%f/%f; 3Df: %f/%f/%f, 3Di: %d/%d/%d\n", sc4d[0], sc4d[1], sc4d[2], sc4d[3], sc3d[0], sc3d[1], sc3d[2], sp.x, sp.y, sp.z);
};

//bool my_pixel_shader (const Vertex *v, const int3 &barc, TGAColor &color) {
bool my_pixel_shader (const Triangle &t, const int3 &barc, TGAColor &color) {
	
	int barc_sum = 0;
	for (int i = 0; i < 3; i++) barc_sum += barc[i];
	
	//int uu = (int) (TEXTURE_U_SIZE * (v[0].text[0]*barc[0] + v[1].text[0]*barc[1] + v[2].text[0]*barc[2]) / barc_sum);
	int uu = (int) (textw * float3_int3_smult (t.u, barc) / barc_sum);
	//int vv = (int) (TEXTURE_V_SIZE * (v[0].text[1]*barc[0] + v[1].text[1]*barc[1] + v[2].text[1]*barc[2]) / barc_sum);
	int vv = (int) (texth * float3_int3_smult (t.v, barc) / barc_sum);

	TGAColor tmpcolor = texture.get(uu, vv);
	
	//TGAColor tmpcolor = TGAColor (255, 255, 255, 255);
	// interpolate normals
	float intensity = 0;
	
	bool phong = 0;
	bool gouraud = !phong;
	bool flat = 0;
	
	
	//intensity = float3_float3_smult (vtx_int);
	if (phong) {
		float3 interp_norm;
		interp_norm[0] = float3_int3_smult (t.nx, barc) / barc_sum;
		interp_norm[1] = float3_int3_smult (t.ny, barc) / barc_sum;
		interp_norm[2] = float3_int3_smult (t.nz, barc) / barc_sum;
		intensity = float3_float3_smult (interp_norm, light_dir);
	}
	else if (gouraud) {
		float3 interp_intens;
		for (int i = 0; i < 3; i++) {
			float3 ii = {t.nx[i], t.ny[i], t.nz[i]};
			interp_intens[i] = float3_float3_smult (ii, light_dir);
			/*intnorm[i] = 0;
			for (int j = 0; j < 3; j++)
				intnorm[i] += v[i].norm[j]*light_dir[j];
			intensity += intnorm[i]*barc[i];*/
		}
		//intensity = -intensity / barc_sum;
		intensity = float3_int3_smult (interp_intens, barc) / barc_sum;
		//printf("%f ", intensity);
	}
	//else if (flat) intensity = tri_intensity;
	
	//printf("intensity=%f, barc=%d:%d:%d:sum=%d, light=%f:%f:%f \n", intensity, barc[0], barc[1], barc[2], barc_sum, light_dir[0], light_dir[1], light_dir[2]);
	//intensity = 1; // tbd remove
	if (intensity > 0) {
		color.r = tmpcolor.r * intensity;
		color.g = tmpcolor.g * intensity;
		color.b = tmpcolor.b * intensity;
		return true;
	}
	return false;
}

    
int main(int argc, char** argv) {
       
	DynArray *obj_vtx  = dyn_array_create (sizeof (float), 384);
	DynArray *obj_norm = dyn_array_create (sizeof (float), 384);
	DynArray *obj_text = dyn_array_create (sizeof (float), 256);
	DynArray *obj_face = dyn_array_create (sizeof   (int), 768);
	
	//read_obj_file ("obj/african_head.obj", obj_vtx, obj_norm, obj_text, obj_face);
	read_obj_file ("obj/floor.obj", obj_vtx, obj_norm, obj_text, obj_face);
    TGAImage image(width, height, TGAImage::RGB);
    
    
    //texture.read_tga_file("obj/african_head_diffuse.tga");
    texture.read_tga_file("obj/floor_diffuse.tga");
    texture.flip_vertically();
    textw = texture.get_width();
    texth = texture.get_height();
    
    size_t    zbuffer_size = width*height;
    screenz_t zbuffer[zbuffer_size];
    for (int i = 0; i < zbuffer_size; i++)
		zbuffer[i] = 0;
    
    float3 obj_scale  = { 1.f,   1.f,   1.f};
	float3 obj_rotate = { 15.0f, 0.0f,  0.0f};
	float3 obj_tran   = { 0.0f, 0.25f,  0.0f};
	
	float3_normalize (light_dir);
        
	float3 camera;	
	float3_float3_sub(eye, center, camera);
	printf ("camera: x=%f, y=%f, z=%f\n", camera[0], camera[1], camera[2]);
	//float3_normalize (camera);
	printf ("camera norm: x=%f, y=%f, z=%f\n", camera[0], camera[1], camera[2]);
	
	
	fmat4 model      = {0};
	fmat4 view       = {0};
	fmat4 projection = {0};
	fmat4 viewport   = {0};
		
	init_model      (model, obj_scale, obj_rotate, obj_tran);
	init_view       (view, eye, center, up);
	init_projection (projection, -1.0f/camera[Z]);
	init_viewport   (viewport, 0, 0, SCREEN_SIZE[0], SCREEN_SIZE[1], SCREEN_SIZE[2]);
    
    
    int face[3][3]; // 3 vertices and 3 indices for each (coordinate, texture, normal)
    //for (int i = 0; i < NUM_OF_FACES; i++) {
    for (int i = 0; i < (obj_face->end) / 9; i++) {
	//for (int i = 13; i < 35; i++) {
        //Face face = obj_face[i];
        for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				face[j][k] = *((int*) dyn_array_get (obj_face, i*9 + j*3 + k));
			}
		}
		        
        
        //Vertex vtx[3];
        Triangle t;     
		// for each vertex j of a triangle
		for (int j = 0; j < 3; j++) {
			float3 tmp;
			for (int k = 0; k < 3; k++) {
				tmp[k] = *((float*) dyn_array_get (obj_vtx, face[j][0]*3 + k));
			}
			ScreenPt sp;
			my_vertex_shader (model, view, projection, viewport, tmp, sp);
			t.cx[j] = sp.x;
			t.cy[j] = sp.y;
			t.cz[j] = sp.z;
			
			//vtx[j].txt_uv = obj_text[face.txt_idx[j]];
			/*for (int k = 0; k < 2; k++) {
				vtx[j].text[k] = *((float*) dyn_array_get (obj_text, face[j][1]*2 + k));//obj_norm[face.vtx_idx[j]][k];
			}*/
			t.u[j] = *((float*) dyn_array_get (obj_text, face[j][1]*2));
			t.v[j] = *((float*) dyn_array_get (obj_text, face[j][1]*2 + 1));
			
			/*for (int k = 0; k < 3; k++) {
				vtx[j].norm[k] = *((float*) dyn_array_get (obj_norm, face[j][2]*3 + k));//obj_norm[face.vtx_idx[j]][k];
			}*/
			t.nx[j] = *((float*) dyn_array_get (obj_norm, face[j][2]*3));
			t.ny[j] = *((float*) dyn_array_get (obj_norm, face[j][2]*3+1));
			t.nz[j] = *((float*) dyn_array_get (obj_norm, face[j][2]*3+2));
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
		draw_triangle (t, my_pixel_shader, zbuffer, image, texture, light_dir, tri_intensity);        
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");

	dyn_array_destroy (obj_vtx);
	
    return 0;
}


