#include "main.h"
#include "wavefront_obj.h"
#include "geometry.h"
#include "gl.h"
#include "shader.h"

#include <tga_addon.h>

#include <stdint.h>
#include <stdlib.h>
#include <math.h>


// globals:

float3 light_dir  = { -1.0f,   -0.2f,   -1.1f};
float3 eye        = { 2.0f,   3.0f,   5.0f};
float3 center     = { 0.0f,   0.0f,   0.0f};
float3 up         = { 0.0f,   1.0f,   0.0f};

/*
struct scene {
	float3 light;
	float3 eye;
	float3 center;
	float3 up;
	float3 camera;
}
*/
  
int main(int argc, char** argv) {
       
    size_t screen_size = WIDTH*HEIGHT;//SCREEN_SIZE[0]*SCREEN_SIZE[1];
    screenz_t     *zbuffer = (screenz_t*)     calloc (screen_size, sizeof(screenz_t));
    pixel_color_t *fbuffer0 = (pixel_color_t*) calloc (screen_size, sizeof(pixel_color_t));
    pixel_color_t *fbuffer1 = (pixel_color_t*) calloc (screen_size, sizeof(pixel_color_t));
    pixel_color_t *active_fbuffer = NULL;
    
    WFobj *african_head = wfobj_new ("obj/african_head.obj", "obj/african_head_diffuse.tga");
	WFobj *my_floor     = wfobj_new ("obj/floor.obj"       , "obj/floor_diffuse.tga");
	
	float3 camera;	
	float3_float3_sub(&eye, &center, &camera);
	
	
	
	// 1. Model - transform local coords to global
	// 2. View - transform global coords to adjust for camera position
	// 3. Projection - perspective correction
	// 4. Viewport - move to screen coords
	fmat4 viewport   = FMAT4_IDENTITY;
	fmat4 projection = FMAT4_IDENTITY;
	fmat4 view       = FMAT4_IDENTITY;	
	init_viewport   (&viewport, 0, 0, WIDTH, HEIGHT, DEPTH);//SCREEN_SIZE[0], SCREEN_SIZE[1], SCREEN_SIZE[2]);
    init_projection (&projection, -1.0f/camera[Z]);
    init_view       (&view, &eye, &center, &up);
    
    fmat4 vpv;
	fmat4_fmat4_fmat4_mult (&viewport, &projection, &view, &vpv);
	
	fmat4 projview;
    fmat4_fmat4_mult (&projection, &view, &projview);
    
	
	
    Object *head1  = obj_new (african_head);
    obj_set_translation (head1, 0.f, 0.f, 0.6f);
    obj_build_model     (head1);
    
    Object *floor1 = obj_new (my_floor);
    obj_set_translation (floor1, 0.f, 0.f, 0.75f);
	obj_build_model     (floor1);
	
	Object *floor2 = obj_new (my_floor);
    obj_set_rotation    (floor2, 90.f, 0.f, 0.f);
	obj_set_translation (floor2, 0.f, 0.75f, 0.0f);
	obj_build_model     (floor2);
	
	Object *floor3 = obj_new (my_floor);
    obj_set_rotation    (floor3, 0.f, 0.f, -90.f);
	obj_set_translation (floor3, 0.f, 0.f, 0.75f);
	obj_build_model     (floor3);
					
    //do {
    for (int i = 0; i < 2; i++) {
		active_fbuffer = (active_fbuffer == fbuffer0) ? fbuffer1 : fbuffer0;
		
		for (int i = 0; i < screen_size; i++) zbuffer[i] = 0;
		
		obj_transform       (head1, &vpv, &projview, &light_dir);
		obj_draw            (head1, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer);
			
		obj_transform       (floor1, &vpv, &projview, &light_dir);
		obj_draw            (floor1, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer);
			
		obj_transform       (floor2, &vpv, &projview, &light_dir);
		obj_draw            (floor2, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer);
			
		obj_transform       (floor3, &vpv, &projview, &light_dir);
		obj_draw            (floor3, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer);
	}// while (0);
	
    write_tga_file ("output_fb0.tga", (tbyte *) fbuffer0, 1);
    write_tga_file ("output_fb1.tga", (tbyte *) fbuffer1, 1);
	
	wfobj_free(african_head);
	wfobj_free(my_floor);
	
	free(zbuffer);
	free(fbuffer0);
	free(fbuffer1);
	
    return 0;
}
