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
	
float3 VARYING_U;
float3 VARYING_V;
float3 VARYING_NX;
float3 VARYING_NY;
float3 VARYING_NZ;
float3 VARYING_INTENSITY;

void my_vertex_shader (const WFobj &obj, const int face_idx, const int vtx_idx, const fmat4 &model, const fmat4 &view, const fmat4 &projection, const fmat4 &viewport, float4 &vtx4d) { //ScreenPt &sp) {
	
	int3 vtx_elems;
	for (int k = 0; k < 3; k++) {
		vtx_elems[k] = *((int*) dyn_array_get (obj.face, face_idx*9 + vtx_idx*3 + k));
	}
	
	float3 obj_coords;
	for (int k = 0; k < 3; k++) {
		obj_coords[k] = *((float*) dyn_array_get (obj.vtx, vtx_elems[0]*3 + k));
	}
	
	float4 mc; // model coordinates
    float4 wc; // world coordinates
    float4 vc; // view coordinates
    float4 pc; // projection coordinates
		
	// 0. transform 3d coords to homogenous coords
	float3_float4_conv (obj_coords, mc);
	
	// 1. Model - transform local coords to global
	// 2. View - transform global coords to adjust for camera position
	// 3. Projection - perspective correction
	// 4. Viewport - move to screen coords
	fmat4_float4_mult (model, mc, wc);
	fmat4_float4_mult (view, wc, vc);
	fmat4_float4_mult (projection, vc, pc);
	fmat4_float4_mult (viewport, pc, vtx4d);
	
		
	//t.u[j] = *((float*) dyn_array_get (african_head.text, face[j][1]*2));
	//t.v[j] = *((float*) dyn_array_get (african_head.text, face[j][1]*2 + 1));
	VARYING_U[vtx_idx] = *((float*) dyn_array_get (obj.text, vtx_elems[1]*2));
	VARYING_V[vtx_idx] = *((float*) dyn_array_get (obj.text, vtx_elems[1]*2 + 1));
	
	//t.nx[j] = *((float*) dyn_array_get (african_head.norm, face[j][2]*3));
	//t.ny[j] = *((float*) dyn_array_get (african_head.norm, face[j][2]*3+1));
	//t.nz[j] = *((float*) dyn_array_get (african_head.norm, face[j][2]*3+2));
	VARYING_NX[vtx_idx] = *((float*) dyn_array_get (obj.norm, vtx_elems[2]*3));
	VARYING_NY[vtx_idx] = *((float*) dyn_array_get (obj.norm, vtx_elems[2]*3+1));
	VARYING_NZ[vtx_idx] = *((float*) dyn_array_get (obj.norm, vtx_elems[2]*3+2));

};

bool my_pixel_shader (const Triangle &t, const WFobj &obj, const float3 &barw, pixel_color_t &color) {
	
	/*
	int uu = (int) (obj.textw * float3_float3_smult (t.u, barw));
	int vv = (int) (obj.texth * float3_float3_smult (t.v, barw));
	*/
	int uu = (int) (obj.textw * float3_float3_smult (VARYING_U, barw));
	int vv = (int) (obj.texth * float3_float3_smult (VARYING_V, barw));
	
	TGAColor tmpcolor = obj.texture.get(uu, obj.texth-vv-1);
	
	
	//TGAColor tmpcolor = TGAColor (255, 255, 255, 255);
	// interpolate normals
	float intensity = 0;
	
	bool phong = 0;
	bool gouraud = !phong;	
	
	if (phong) {
		float3 interp_norm;
		interp_norm[0] = float3_float3_smult (VARYING_NX, barw);
		interp_norm[1] = float3_float3_smult (VARYING_NY, barw);
		interp_norm[2] = float3_float3_smult (VARYING_NZ, barw);
		intensity = -float3_float3_smult (interp_norm, light_dir);
	}
	else if (gouraud) {
		float3 interp_intens;
		for (int i = 0; i < 3; i++) {
			float3 ii = {VARYING_NX[i], VARYING_NY[i], VARYING_NZ[i]};
			interp_intens[i] = float3_float3_smult (ii, light_dir);
		}
		intensity = -float3_float3_smult (interp_intens, barw);
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
	float3 rotate = { 0.0f, 0.0f,  0.0f};
	float3 tran   = { 0.0f, 0.0f,  0.0f};
	
	WFobj african_head;
    init_obj (african_head, "obj/african_head.obj", "obj/african_head_diffuse.tga");
    for (int i = 0; i < 3; i++) {
		african_head.scale[i]  = scale[i];
		african_head.rotate[i] = rotate[i];
		african_head.tran[i]   = tran[i];
	}
	
    WFobj my_floor;
    init_obj (my_floor, "obj/floor.obj", "obj/floor_diffuse.tga");
    for (int i = 0; i < 3; i++) {
		my_floor.scale[i]  = 2.0f;//scale[i];
		my_floor.rotate[i] = rotate[i];
		my_floor.tran[i]   = tran[i];
	}
	my_floor.tran[2]   = -0.5f;
	
    size_t    buffer_size = width*height;
    //screenz_t     zbuffer[buffer_size];
    //pixel_color_t fbuffer[buffer_size];    
    screenz_t *zbuffer = (screenz_t*) calloc (buffer_size, sizeof(screenz_t));
    pixel_color_t *fbuffer = (pixel_color_t*) calloc (buffer_size, sizeof(pixel_color_t));
    for (int i = 0; i < buffer_size; i++) {
		zbuffer[i] = INT32_MIN;
		
		fbuffer[i].r = 0;
		fbuffer[i].g = 0;
		fbuffer[i].b = 0;
		fbuffer[i].a = 0;
	}
    
    
	float3_normalize (light_dir);
        
	float3 camera;	
	float3_float3_sub(eye, center, camera);
	//printf ("camera: x=%f, y=%f, z=%f\n", camera[0], camera[1], camera[2]);
	//float3_normalize (camera); TBD - uncomment?
	//printf ("camera norm: x=%f, y=%f, z=%f\n", camera[0], camera[1], camera[2]);
	
	
	fmat4 model      = {0};
	fmat4 view       = {0};
	fmat4 projection = {0};
	fmat4 viewport   = {0};
		
	init_model      (model, african_head.scale, african_head.rotate, african_head.tran);
	init_view       (view, eye, center, up);
	init_projection (projection, -1.0f/camera[Z]);
	init_viewport   (viewport, 0, 0, SCREEN_SIZE[0], SCREEN_SIZE[1], SCREEN_SIZE[2]);
    
    for (int i = 0; i < (african_head.face->end) / 9; i++) {
	//for (int i = 13; i < 35; i++) {
    	// for each vertex j of a triangle
		Triangle t;
		for (int j = 0; j < 3; j++) {
			float4 scr_coords;
			my_vertex_shader (african_head, i, j, model, view, projection, viewport, scr_coords);
			t.cx[j] = (screenxy_t) scr_coords[0]/scr_coords[3];
			t.cy[j] = (screenxy_t) scr_coords[1]/scr_coords[3];
			t.cz[j] = (screenz_t)  scr_coords[2]/scr_coords[3];
			t.cw[j] = scr_coords[3];	
		}		
		draw_triangle (t, my_pixel_shader, zbuffer, fbuffer, african_head, light_dir, 0.0f);        
    }
	
	init_model      (model, my_floor.scale, my_floor.rotate, my_floor.tran);
	for (int i = 0; i < (my_floor.face->end) / 9; i++) {
	//for (int i = 13; i < 35; i++) {
        // for each vertex j of a triangle
		Triangle t;     
		for (int j = 0; j < 3; j++) {
			float4 scr_coords;
			my_vertex_shader (my_floor, i, j, model, view, projection, viewport, scr_coords);
			t.cx[j] = (screenxy_t) scr_coords[0]/scr_coords[3];
			t.cy[j] = (screenxy_t) scr_coords[1]/scr_coords[3];
			t.cz[j] = (screenz_t)  scr_coords[2]/scr_coords[3];
			t.cw[j] = scr_coords[3];	
		}		
		draw_triangle (t, my_pixel_shader, zbuffer, fbuffer, my_floor, light_dir, 0.0f);             
    }

    // write down the framebuffer
    TGAImage image(width, height, TGAImage::RGB);
    for (int i = 0; i < width; i++) {
		TGAColor color;
		for (int j = 0; j < height; j++) {
			color.r = fbuffer[i + j*width].r;
			color.g = fbuffer[i + j*width].g;
			color.b = fbuffer[i + j*width].b;
			image.set(i, j, color);
		}
	}
	image.write_tga_file("output.tga");

	dyn_array_destroy (african_head.vtx);
	dyn_array_destroy (african_head.norm);
	dyn_array_destroy (african_head.text);
	dyn_array_destroy (african_head.face);
	
    return 0;
}
