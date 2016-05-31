#include "tgaimage.h"
#include "main.h"
#include "wavefront_obj.h"
#include "geometry.h"
#include "gl.h"

#include "shader.h"
#include <stdint.h>
#include <stdlib.h>
#include <math.h>


// globals:

float3 light_dir  = { -1.0f,   -0.2f,   -1.1f};
float3 eye        = { 2.0f,   3.0f,   5.0f};
float3 center     = { 0.0f,   0.0f,   0.0f};
float3 up         = { 0.0f,   1.0f,   0.0f};
	
    
int main(int argc, char** argv) {
       
    size_t buffer_size = SCREEN_SIZE[0]*SCREEN_SIZE[1];
    
    screenz_t     *zbuffer = (screenz_t*)     calloc (buffer_size, sizeof(screenz_t));
    pixel_color_t *fbuffer = (pixel_color_t*) calloc (buffer_size, sizeof(pixel_color_t));
    
    for (int i = 0; i < buffer_size; i++) {
		zbuffer[i] = INT32_MIN;	
		fbuffer[i] = set_color(0, 0, 0, 0);
	}
	
	
    
    float3 default_scale  = { 1.f,   1.f,   1.f};
	float3 default_rotate = { 0.0f, 0.0f,  0.0f};
	float3 default_tran   = { 0.0f, 0.0f,  0.0f};
	float3 scale;
	float3 rotate;
	float3 tran;
	
	/*
	WFobj african_head;
	WFobj my_floor;
    init_obj (african_head, "obj/african_head.obj", "obj/african_head_diffuse.tga");
    //init_obj (african_head, "obj/african_head.obj", "obj/floor_diffuse.tga");
    init_obj (my_floor,     "obj/floor.obj",        "obj/floor_diffuse.tga");
    */
    WFobj *african_head = wfobj_new ("obj/african_head.obj");
	TGAImage *head_diffuse = new TGAImage (1, 1, TGAImage::RGB);
	head_diffuse->read_tga_file ("obj/african_head_diffuse.tga");
	african_head->texture = head_diffuse;
	
	//wfobj_load_texture (african_head, "obj/african_head_diffuse.tga");
	//WFobj *african_head = wfobj_new ("obj/african_head.obj", "obj/african_head_diffuse.tga");
	//WFobj *my_floor = wfobj_new ("obj/floor.obj", "obj/floor_diffuse.tga");
	WFobj *my_floor = wfobj_new ("obj/floor.obj");
	TGAImage *floor_diffuse = new TGAImage (1, 1, TGAImage::RGB);
	floor_diffuse->read_tga_file ("obj/floor_diffuse.tga");
	my_floor->texture = floor_diffuse;
	
	
	float4 light_dir4, light_new;
	
	float3 camera;	
	float3_float3_sub(&eye, &center, &camera);
	//printf ("camera: x=%f, y=%f, z=%f\n", camera[0], camera[1], camera[2]);
	//float3_normalize (camera); //TBD - uncomment?
	//printf ("camera norm: x=%f, y=%f, z=%f\n", camera[0], camera[1], camera[2]);
	
	
	fmat4 model      = {0};
	fmat4 view       = {0};
	fmat4 projection = {0};
	fmat4 viewport   = {0};
		
	init_view       (&view, &eye, &center, &up);
	init_projection (&projection, -1.0f/camera[Z]);
	init_viewport   (&viewport, 0, 0, SCREEN_SIZE[0], SCREEN_SIZE[1], SCREEN_SIZE[2]);
    fmat4 projview;
    fmat4_fmat4_mult (&projection, &view, &projview);
    
    for (int i = 0; i < 3; i++) {
		scale[i]  = default_scale[i];
		rotate[i] = default_rotate[i];
		tran[i]   = default_tran[i];
	}
	//rotate[0] = 0.0f;
	tran[2]   = 0.6f;
	
    init_model      (&model, scale, rotate, tran);
    fmat4_fmat4_mult (&projview, &model, &UNIFORM_M);
	fmat4_invert (&UNIFORM_M, &UNIFORM_MI);
	fmat4_transpose (&UNIFORM_MI, &UNIFORM_MIT);
	
	float3_float4_conv (&light_dir, &light_dir4);
	fmat4_float4_mult (&UNIFORM_M, &light_dir4, &light_new);
	float4_float3_conv (&light_new, &UNIFORM_LIGHT);
    float3_normalize (&UNIFORM_LIGHT);
    
    
	// 1. Model - transform local coords to global
	// 2. View - transform global coords to adjust for camera position
	// 3. Projection - perspective correction
	// 4. Viewport - move to screen coords
	// Doing everyhting in reverse order:
	fmat4 tmp1, tmp2, mvpv;
	fmat4_fmat4_mult (&viewport, &projection, &tmp1);
	fmat4_fmat4_mult (&tmp1, &view, &tmp2);
	fmat4_fmat4_mult (&tmp2, &model, &mvpv); 
	draw_obj (african_head, my_vertex_shader, my_pixel_shader, zbuffer, fbuffer, &mvpv);
	
	
	for (int i = 0; i < 3; i++) {
		scale[i]  = default_scale[i];
		rotate[i] = default_rotate[i];
		tran[i]   = default_tran[i];
	}
	rotate[0] = 0.0f;
	tran[2]   = 0.75f;
	
	init_model       (&model, scale, rotate, tran);	
	fmat4_fmat4_mult (&projview, &model, &UNIFORM_M);
	
	fmat4_invert (&UNIFORM_M, &UNIFORM_MI);
	fmat4_transpose (&UNIFORM_MI, &UNIFORM_MIT);
	
	float3_float4_conv (&light_dir, &light_dir4);
	fmat4_float4_mult (&UNIFORM_M, &light_dir4, &light_new);
	float4_float3_conv (&light_new, &UNIFORM_LIGHT);
    float3_normalize (&UNIFORM_LIGHT);
    
	fmat4_fmat4_mult (&tmp2, &model, &mvpv); 
	draw_obj (my_floor, my_vertex_shader, my_pixel_shader, zbuffer, fbuffer, &mvpv);
	
    for (int i = 0; i < 3; i++) {
		scale[i]  = default_scale[i];
		rotate[i] = default_rotate[i];
		tran[i]   = default_tran[i];
	}
	rotate[0] = 90.0f;
	tran[1]   = 0.75f;
	
	init_model (&model, scale, rotate, tran);	
	fmat4_fmat4_mult (&projview, &model, &UNIFORM_M);
	fmat4_invert (&UNIFORM_M, &UNIFORM_MI);
	fmat4_transpose (&UNIFORM_MI, &UNIFORM_MIT);
	
	float3_float4_conv (&light_dir, &light_dir4);
	fmat4_float4_mult (&UNIFORM_M, &light_dir4, &light_new);
	float4_float3_conv (&light_new, &UNIFORM_LIGHT);
    float3_normalize (&UNIFORM_LIGHT);
    
    fmat4_fmat4_mult (&tmp2, &model, &mvpv); 
	draw_obj (my_floor, my_vertex_shader, my_pixel_shader, zbuffer, fbuffer, &mvpv);
	
    for (int i = 0; i < 3; i++) {
		scale[i]  = default_scale[i];
		rotate[i] = default_rotate[i];
		tran[i]   = default_tran[i];
	}
	rotate[2] = -90.0f;
	tran[0]   = 0;//-0.75f;
	tran[1]   = 0;//0.75f;
	tran[2]   = 0.75f;
	
	init_model (&model, scale, rotate, tran);	
	fmat4_fmat4_mult (&projview, &model, &UNIFORM_M);
	fmat4_invert (&UNIFORM_M, &UNIFORM_MI);
	fmat4_transpose (&UNIFORM_MI, &UNIFORM_MIT);
	
	float3_float4_conv (&light_dir, &light_dir4);
	fmat4_float4_mult (&UNIFORM_M, &light_dir4, &light_new);
	float4_float3_conv (&light_new, &UNIFORM_LIGHT);
    float3_normalize (&UNIFORM_LIGHT);
    
    fmat4_fmat4_mult (&tmp2, &model, &mvpv); 
	draw_obj (my_floor, my_vertex_shader, my_pixel_shader, zbuffer, fbuffer, &mvpv);
	
    
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

	wfobj_free(african_head);
	wfobj_free(my_floor);
	
	free(zbuffer);
	free(fbuffer);
	
	delete head_diffuse;
	delete floor_diffuse;
	
    return 0;
}
