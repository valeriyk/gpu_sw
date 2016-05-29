#include "tgaimage.h"
#include "main.h"
#include "wavefront_obj.h"
#include "geometry.h"
#include "gl.h"

#include "shader.h"
#include <stdint.h>
#include <math.h>


// globals:

float3 light_dir  = { -1.0f,   -0.0f,   -0.0f};
float3 eye        = { 5.0f,   5.0f,   5.0f};
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
	wfobj_load_texture (african_head, "obj/african_head_diffuse.tga");
	
	float4 light_dir4, light_new;
	
	float3 camera;	
	float3_float3_sub(eye, center, camera);
	//printf ("camera: x=%f, y=%f, z=%f\n", camera[0], camera[1], camera[2]);
	//float3_normalize (camera); TBD - uncomment?
	//printf ("camera norm: x=%f, y=%f, z=%f\n", camera[0], camera[1], camera[2]);
	
	
	fmat4 model      = {0};
	fmat4 view       = {0};
	fmat4 projection = {0};
	fmat4 viewport   = {0};
		
	init_view       (&view, eye, center, up);
	init_projection (projection, -1.0f/camera[Z]);
	init_viewport   (viewport, 0, 0, SCREEN_SIZE[0], SCREEN_SIZE[1], SCREEN_SIZE[2]);
    fmat4 projview;
    fmat4_fmat4_mult (&projection, &view, &projview);
    
    for (int i = 0; i < 3; i++) {
		scale[i]  = default_scale[i];
		rotate[i] = default_rotate[i];
		tran[i]   = default_tran[i];
	}
	//rotate[0] = 0.0f;
	//tran[2]   = 0.7f;
	
    init_model      (&model, scale, rotate, tran);
    fmat4_fmat4_mult (&projview, &model, &UNIFORM_M);
	fmat4_invert (&UNIFORM_M, &UNIFORM_MI);
	fmat4_transpose (&UNIFORM_MI, &UNIFORM_MIT);
	
	float3_float4_conv (light_dir, light_dir4);
	fmat4_float4_mult (UNIFORM_M, light_dir4, light_new);
	float4_float3_conv (light_new, UNIFORM_LIGHT);
    float3_normalize (UNIFORM_LIGHT);
    
    for (int i = 0; i < (african_head->face->end) / 9; i++) {
	//for (int i = 13; i < 35; i++) {
    	// for each vertex j of a triangle
		ScreenTriangle t;
		for (int j = 0; j < 3; j++) {
			my_vertex_shader (african_head, i, j, model, view, projection, viewport, t.vtx_coords[j]);
		}		
		draw_triangle (t, my_pixel_shader, zbuffer, fbuffer, african_head, light_dir);        
    }
	
	/*
	for (int i = 0; i < 3; i++) {
		scale[i]  = default_scale[i];
		rotate[i] = default_rotate[i];
		tran[i]   = default_tran[i];
	}
	rotate[0] = 0.0f;
	tran[2]   = 0.75f;
	
	init_model       (&model, scale, rotate, tran);	
	fmat4_fmat4_mult (&projview, &model, &UNIFORM_M);
	
	
    //print_fmat4 (&viewport, "Viewport: ");
    //print_fmat4 (&projection, "Projection: ");
	//print_fmat4 (&view, "View: ");
	//print_fmat4 (&projview, "Projview: ");
	//print_fmat4 (&model, "Model: ");
	//print_fmat4 (&UNIFORM_M, "M: ");
	
	fmat4_invert (&UNIFORM_M, &UNIFORM_MI);
	//print_fmat4 (&UNIFORM_MI, "MI: ");
	//fmat4 check_inv;
	//fmat4_fmat4_mult (&UNIFORM_M, &UNIFORM_MI, &check_inv);
	//print_fmat4 (&check_inv, "check inv: ");
	//for (int i = 0; i < 4; i++) {
	//	for (int j = 0; j < 4; j++) {
	//		if ((i == j) && (check_inv[i][j] != 1.0f)) printf ("Matrix inversion fault!\n");
	//		if ((i != j) && (fabs(check_inv[i][j]) > 0.0001f)) printf ("Matrix inversion fault!\n");
	//	}
	//}
	fmat4_transpose (&UNIFORM_MI, &UNIFORM_MIT);
	//print_fmat4 (&UNIFORM_MIT, "MIT: ");
	for (int i = 0; i < (my_floor.face->end) / 9; i++) {
	//for (int i = 13; i < 35; i++) {
        // for each vertex j of a triangle
		ScreenTriangle t;     
		for (int j = 0; j < 3; j++) {
			my_vertex_shader (my_floor, i, j, model, view, projection, viewport, t.vtx_coords[j]);
		}		
		draw_triangle (t, my_pixel_shader, zbuffer, fbuffer, my_floor, light_dir);             
    }
    
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
	for (int i = 0; i < (my_floor.face->end) / 9; i++) {
		ScreenTriangle t;     
		for (int j = 0; j < 3; j++) {
			my_vertex_shader (my_floor, i, j, model, view, projection, viewport, t.vtx_coords[j]);
		}		
		draw_triangle (t, my_pixel_shader, zbuffer, fbuffer, my_floor, light_dir);             
    }
    
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
	for (int i = 0; i < (my_floor.face->end) / 9; i++) {
	    // for each vertex j of a triangle
		ScreenTriangle t;     
		for (int j = 0; j < 3; j++) {
			my_vertex_shader (my_floor, i, j, model, view, projection, viewport, t.vtx_coords[j]);
		}		
		draw_triangle (t, my_pixel_shader, zbuffer, fbuffer, my_floor, light_dir);             
    }
	*/
    
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

/*
	dyn_array_destroy (african_head->vtx);
	dyn_array_destroy (african_head->norm);
	dyn_array_destroy (african_head->text);
	dyn_array_destroy (african_head->face);
*/
	wfobj_free(african_head);
	
	free(zbuffer);
	free(fbuffer);
	
    return 0;
}
