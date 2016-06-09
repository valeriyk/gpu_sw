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
    
	fmat4 mvpv; // computed for every object
	
	
    Object *head1  = obj_new (african_head);
    Object *floor1 = obj_new (my_floor);
    Object *floor2 = obj_new (my_floor);
    Object *floor3 = obj_new (my_floor);
    
    //do {
    for (int i = 0; i < 2; i++) {
		if (active_fbuffer == fbuffer0) active_fbuffer = fbuffer1;
		else active_fbuffer = fbuffer0;
		
		for (int i = 0; i < screen_size; i++) zbuffer[i] = 0;
		
		obj_set_translation (head1, 0.f, 0.f, 0.6f);
		obj_transform       (head1, &projview, &light_dir);
		fmat4_fmat4_mult    (&vpv, &(head1->model), &mvpv); 
		obj_draw            (head1, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer, &mvpv);
		
		
		obj_set_translation (floor1, 0.f, 0.f, 0.75f);
		obj_transform       (floor1, &projview, &light_dir);
		fmat4_fmat4_mult    (&vpv, &(floor1->model), &mvpv); 
		obj_draw            (floor1, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer, &mvpv);
		
		
		obj_set_rotation    (floor2, 90.f, 0.f, 0.f);
		obj_set_translation (floor2, 0.f, 0.75f, 0.0f);
		obj_transform       (floor2, &projview, &light_dir);
		fmat4_fmat4_mult    (&vpv, &(floor2->model), &mvpv); 
		obj_draw            (floor2, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer, &mvpv);
		
		
		obj_set_rotation    (floor3, 0.f, 0.f, -90.f);
		obj_set_translation (floor3, 0.f, 0.f, 0.75f);
		obj_transform       (floor3, &projview, &light_dir);
		fmat4_fmat4_mult    (&vpv, &(floor3->model), &mvpv); 
		obj_draw            (floor3, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer, &mvpv);
	}// while (0);
	
    
	TGAData frame_data;	
	TGA *tga;
	
	// write down the framebuffer0
	tga = TGAOpen ("output_fb0.tga", "w");
	tga->hdr.id_len 	= 0;
	tga->hdr.map_t		= 0;
	tga->hdr.img_t 		= 2;
	tga->hdr.map_first 	= 0;
	tga->hdr.map_entry 	= 0;
	tga->hdr.map_len	= 0;
	tga->hdr.x 			= 0;
	tga->hdr.y 			= 0;
	tga->hdr.width 		= WIDTH;//SCREEN_SIZE[0];
	tga->hdr.height 	= HEIGHT;//SCREEN_SIZE[1];
	tga->hdr.depth 		= 24;
	tga->hdr.vert 	    = 1;
	tga->hdr.horz   	= 0;
	tga->hdr.alpha      = 0;
	
	//frame_data.flags = TGA_IMAGE_DATA | TGA_IMAGE_ID | TGA_RGB;
	//frame_data.flags = TGA_IMAGE_DATA | TGA_RGB | TGA_RLE_ENCODE;
	frame_data.flags = TGA_IMAGE_DATA | TGA_RGB;
	frame_data.img_data = (tbyte*) fbuffer0;
	frame_data.cmap = NULL;
	frame_data.img_id = NULL;
	if (TGAWriteImage (tga, &frame_data) != TGA_OK) {
		printf ("TGA error code 2!\n");
		return 1;
	}
	TGAClose(tga);
	
	// write down the framebuffer1
	tga = TGAOpen ("output_fb1.tga", "w");
	tga->hdr.id_len 	= 0;
	tga->hdr.map_t		= 0;
	tga->hdr.img_t 		= 2;
	tga->hdr.map_first 	= 0;
	tga->hdr.map_entry 	= 0;
	tga->hdr.map_len	= 0;
	tga->hdr.x 			= 0;
	tga->hdr.y 			= 0;
	tga->hdr.width 		= WIDTH;//SCREEN_SIZE[0];
	tga->hdr.height 	= HEIGHT;//SCREEN_SIZE[1];
	tga->hdr.depth 		= 24;
	tga->hdr.vert 	    = 1;
	tga->hdr.horz   	= 0;
	tga->hdr.alpha      = 0;
	
	//frame_data.flags = TGA_IMAGE_DATA | TGA_IMAGE_ID | TGA_RGB;
	//frame_data.flags = TGA_IMAGE_DATA | TGA_RGB | TGA_RLE_ENCODE;
	frame_data.flags = TGA_IMAGE_DATA | TGA_RGB;
	frame_data.img_data = (tbyte*) fbuffer1;
	frame_data.cmap = NULL;
	frame_data.img_id = NULL;
	if (TGAWriteImage (tga, &frame_data) != TGA_OK) {
		printf ("TGA error code 2!\n");
		return 1;
	}
	TGAClose(tga);
	
	wfobj_free(african_head);
	wfobj_free(my_floor);
	
	free(zbuffer);
	free(fbuffer0);
	free(fbuffer1);
	
    return 0;
}
