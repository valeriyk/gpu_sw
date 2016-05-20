#include "tgaimage.h"
#include "main.h"
#include "wavefront_obj.h"
#include "geometry.h"
#include "gl.h"
#include "dyn_array.h"
#include <stdint.h>


// globals:

float3 light_dir  = { 0.0f,   -1.0f,   -1.0f};
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
	float4_float3_conv (sc4d, sc3d);
	sp.x = (screenxy_t) sc3d[0];
	sp.y = (screenxy_t) sc3d[1];
	sp.z = (screenz_t)  sc3d[2];
	
	//printf ("[vertex shader] 4Df: %f/%f/%f/%f; 3Df: %f/%f/%f, 3Di: %d/%d/%d\n", sc4d[0], sc4d[1], sc4d[2], sc4d[3], sc3d[0], sc3d[1], sc3d[2], sp.x, sp.y, sp.z);
};

bool my_pixel_shader (const Triangle &t, const WFobj &obj, const int3 &barc, TGAColor &color) {
	
	int barc_sum = 0;
	for (int i = 0; i < 3; i++) barc_sum += barc[i];
	
	int uu = (int) (obj.textw * float3_int3_smult (t.u, barc) / barc_sum);
	int vv = (int) (obj.texth * float3_int3_smult (t.v, barc) / barc_sum);

	TGAColor tmpcolor = obj.texture.get(uu, vv);
	
	//TGAColor tmpcolor = TGAColor (255, 255, 255, 255);
	// interpolate normals
	float intensity = 0;
	
	bool phong = 0;
	bool gouraud = !phong;	
	
	if (phong) {
		float3 interp_norm;
		interp_norm[0] = float3_int3_smult (t.nx, barc) / barc_sum;
		interp_norm[1] = float3_int3_smult (t.ny, barc) / barc_sum;
		interp_norm[2] = float3_int3_smult (t.nz, barc) / barc_sum;
		intensity = -float3_float3_smult (interp_norm, light_dir);
	}
	else if (gouraud) {
		float3 interp_intens;
		for (int i = 0; i < 3; i++) {
			float3 ii = {t.nx[i], t.ny[i], t.nz[i]};
			interp_intens[i] = float3_float3_smult (ii, light_dir);
		}
		intensity = -float3_int3_smult (interp_intens, barc) / barc_sum;
	}	
	//printf("intensity=%f, barc=%d:%d:%d:sum=%d, light=%f:%f:%f \n", intensity, barc[0], barc[1], barc[2], barc_sum, light_dir[0], light_dir[1], light_dir[2]);
	if (intensity > 0) {
		color.r = tmpcolor.r * intensity;
		color.g = tmpcolor.g * intensity;
		color.b = tmpcolor.b * intensity;
		return true;
	}
	return false;
}

    
int main(int argc, char** argv) {
       
    float3 scale  = { 1.f,   1.f,   1.f};
	float3 rotate = { 15.0f, 0.0f,  0.0f};
	float3 tran   = { 0.0f, 0.25f,  0.0f};
	
	WFobj african_head;
    african_head.vtx  = dyn_array_create (sizeof (float), 384);
    african_head.norm = dyn_array_create (sizeof (float), 384);
	african_head.text = dyn_array_create (sizeof (float), 256);
	african_head.face = dyn_array_create (sizeof   (int), 1152);
	read_obj_file ("obj/african_head.obj", african_head);	        
    african_head.texture = TGAImage(1, 1, TGAImage::RGB);
    african_head.texture.read_tga_file("obj/african_head_diffuse.tga");    
    african_head.texture.flip_vertically();
    african_head.textw = african_head.texture.get_width();
    african_head.texth = african_head.texture.get_height();    
    for (int i = 0; i < 3; i++) {
		african_head.scale[i]  = scale[i];
		african_head.rotate[i] = rotate[i];
		african_head.tran[i]   = tran[i];
	}
	
	//////////////////////////////////////////////////////
    /*
     * scale  = { 1.f,   1.f,   1.f};
	rotate = { 15.0f, 0.0f,  0.0f};
	tran   = { 0.0f, 0.25f,  0.0f};
	*/
	
    WFobj my_floor;
    my_floor.vtx  = dyn_array_create (sizeof (float), 48);
    my_floor.norm = dyn_array_create (sizeof (float), 48);
	my_floor.text = dyn_array_create (sizeof (float), 32);
	my_floor.face = dyn_array_create (sizeof   (int), 144);
    read_obj_file ("obj/floor.obj", my_floor);    
    my_floor.texture = TGAImage(1, 1, TGAImage::RGB);
    my_floor.texture.read_tga_file("obj/floor_diffuse.tga");
    my_floor.texture.flip_vertically();
    my_floor.textw = my_floor.texture.get_width();
    my_floor.texth = my_floor.texture.get_height();    
    for (int i = 0; i < 3; i++) {
		my_floor.scale[i]  = scale[i];
		my_floor.rotate[i] = rotate[i];
		my_floor.tran[i]   = tran[i];
	}
	
	    
    
    
    TGAImage image(width, height, TGAImage::RGB);
    
    
    size_t    zbuffer_size = width*height;
    screenz_t zbuffer[zbuffer_size];
    for (int i = 0; i < zbuffer_size; i++)
		zbuffer[i] = 0;
    
    
	float3_normalize (light_dir);
        
	float3 camera;	
	float3_float3_sub(eye, center, camera);
	printf ("camera: x=%f, y=%f, z=%f\n", camera[0], camera[1], camera[2]);
	//float3_normalize (camera); TBD - uncomment?
	printf ("camera norm: x=%f, y=%f, z=%f\n", camera[0], camera[1], camera[2]);
	
	
	fmat4 model      = {0};
	fmat4 view       = {0};
	fmat4 projection = {0};
	fmat4 viewport   = {0};
		
	init_model      (model, african_head.scale, african_head.rotate, african_head.tran);
	init_view       (view, eye, center, up);
	init_projection (projection, -1.0f/camera[Z]);
	init_viewport   (viewport, 0, 0, SCREEN_SIZE[0], SCREEN_SIZE[1], SCREEN_SIZE[2]);
    
    
    int face[3][3]; // 3 vertices and 3 indices for each (coordinate, texture, normal)
    
    for (int i = 0; i < (african_head.face->end) / 9; i++) {
	//for (int i = 13; i < 35; i++) {
        for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				face[j][k] = *((int*) dyn_array_get (african_head.face, i*9 + j*3 + k));
			}
		}
        
        // for each vertex j of a triangle
		Triangle t;     
		for (int j = 0; j < 3; j++) {
			float3 tmp;
			for (int k = 0; k < 3; k++) {
				tmp[k] = *((float*) dyn_array_get (african_head.vtx, face[j][0]*3 + k));
			}
			ScreenPt sp;
			my_vertex_shader (model, view, projection, viewport, tmp, sp);
			t.cx[j] = sp.x;
			t.cy[j] = sp.y;
			t.cz[j] = sp.z;
			t.u[j] = *((float*) dyn_array_get (african_head.text, face[j][1]*2));
			t.v[j] = *((float*) dyn_array_get (african_head.text, face[j][1]*2 + 1));
			t.nx[j] = *((float*) dyn_array_get (african_head.norm, face[j][2]*3));
			t.ny[j] = *((float*) dyn_array_get (african_head.norm, face[j][2]*3+1));
			t.nz[j] = *((float*) dyn_array_get (african_head.norm, face[j][2]*3+2));
		}
		
		float tri_intensity = 0;
		draw_triangle (t, my_pixel_shader, zbuffer, image, african_head, light_dir, tri_intensity);        
    }


	init_model      (model, my_floor.scale, my_floor.rotate, my_floor.tran);
	for (int i = 0; i < (my_floor.face->end) / 9; i++) {
	//for (int i = 13; i < 35; i++) {
        for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				face[j][k] = *((int*) dyn_array_get (my_floor.face, i*9 + j*3 + k));
			}
		}
        
        // for each vertex j of a triangle
		Triangle t;     
		for (int j = 0; j < 3; j++) {
			float3 tmp;
			for (int k = 0; k < 3; k++) {
				tmp[k] = *((float*) dyn_array_get (my_floor.vtx, face[j][0]*3 + k));
			}
			ScreenPt sp;
			my_vertex_shader (model, view, projection, viewport, tmp, sp);
			t.cx[j] = sp.x;
			t.cy[j] = sp.y;
			t.cz[j] = sp.z;
			t.u[j] = *((float*) dyn_array_get (my_floor.text, face[j][1]*2));
			t.v[j] = *((float*) dyn_array_get (my_floor.text, face[j][1]*2 + 1));
			t.nx[j] = *((float*) dyn_array_get (my_floor.norm, face[j][2]*3));
			t.ny[j] = *((float*) dyn_array_get (my_floor.norm, face[j][2]*3+1));
			t.nz[j] = *((float*) dyn_array_get (my_floor.norm, face[j][2]*3+2));
		}
		
		float tri_intensity = 0;
		draw_triangle (t, my_pixel_shader, zbuffer, image, my_floor, light_dir, tri_intensity);        
    }
    
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");

	dyn_array_destroy (african_head.vtx);
	dyn_array_destroy (african_head.norm);
	dyn_array_destroy (african_head.text);
	dyn_array_destroy (african_head.face);
	
    return 0;
}


