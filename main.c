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
    pixel_color_t *fbuffer = (pixel_color_t*) calloc (screen_size, sizeof(pixel_color_t));
    
    WFobj *african_head = wfobj_new ("obj/african_head.obj", "obj/african_head_diffuse.tga");
	WFobj *my_floor     = wfobj_new ("obj/floor.obj"       , "obj/floor_diffuse.tga");
	
	float3 default_scale  = { 1.f,   1.f,   1.f};
	float3 default_rotate = { 0.0f, 0.0f,  0.0f};
	float3 default_tran   = { 0.0f, 0.0f,  0.0f};
	float3 scale;
	float3 rotate;
	float3 tran;
	
	
	
	float3 camera;	
	float3_float3_sub(&eye, &center, &camera);
	fmat4 view       = FMAT4_IDENTITY;
	init_view       (&view, &eye, &center, &up);
	
	fmat4 projection = FMAT4_IDENTITY;
	fmat4 viewport   = FMAT4_IDENTITY;
		
	fmat4 model      = FMAT4_IDENTITY;
	
	init_projection (&projection, -1.0f/camera[Z]);
	init_viewport   (&viewport, 0, 0, WIDTH, HEIGHT, DEPTH);//SCREEN_SIZE[0], SCREEN_SIZE[1], SCREEN_SIZE[2]);
    fmat4 projview;
    fmat4_fmat4_mult (&projection, &view, &projview);
    
    Object *head1 = obj_new (african_head);
	//obj_set_scale       (head1, 1.f, 1.f, 1.f);
    //obj_set_rotation    (head1, 0.f, 0.f, 0.f);
    obj_set_translation (head1, 0.f, 0.f, 0.6f);
    obj_transform       (head1, &projview, &light_dir);
    
	// 1. Model - transform local coords to global
	// 2. View - transform global coords to adjust for camera position
	// 3. Projection - perspective correction
	// 4. Viewport - move to screen coords
	// Doing everyhting in reverse order:
	fmat4 tmp1, tmp2, mvpv;
	fmat4_fmat4_mult (&viewport, &projection, &tmp1);
	fmat4_fmat4_mult (&tmp1, &view, &tmp2);
	fmat4_fmat4_mult (&tmp2, &(head1->model), &mvpv); 
	draw_obj (african_head, my_vertex_shader, my_pixel_shader, zbuffer, fbuffer, &mvpv);
	
    Object *floor1 = obj_new (my_floor);
	//obj_set_scale       (floor1, 1.f, 1.f, 1.f);
    //obj_set_rotation    (floor1, 0.f, 0.f, 0.f);
    obj_set_translation (floor1, 0.f, 0.f, 0.75f);
    obj_transform       (floor1, &projview, &light_dir);
	fmat4_fmat4_mult (&tmp2, &(floor1->model), &mvpv); 
	draw_obj (my_floor, my_vertex_shader, my_pixel_shader, zbuffer, fbuffer, &mvpv);
    
    Object *floor2 = obj_new (my_floor);
	//obj_set_scale       (floor2, 1.f, 1.f, 1.f);
    obj_set_rotation    (floor2, 90.f, 0.f, 0.f);
    obj_set_translation (floor2, 0.f, 0.75f, 0.0f);
    obj_transform       (floor2, &projview, &light_dir);
    fmat4_fmat4_mult (&tmp2, &(floor2->model), &mvpv); 
	draw_obj (my_floor, my_vertex_shader, my_pixel_shader, zbuffer, fbuffer, &mvpv);
	
	Object *floor3 = obj_new (my_floor);
	//obj_set_scale       (floor3, 1.f, 1.f, 1.f);
    obj_set_rotation    (floor3, 0.f, 0.f, -90.f);
    obj_set_translation (floor3, 0.f, 0.f, 0.75f);
    obj_transform       (floor3, &projview, &light_dir);
    fmat4_fmat4_mult (&tmp2, &(floor3->model), &mvpv); 
	draw_obj (my_floor, my_vertex_shader, my_pixel_shader, zbuffer, fbuffer, &mvpv);
	
    // write down the framebuffer
	TGAData frame_data;	
	TGA *tga = TGAOpen ("output2.tga", "w");
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
	frame_data.img_data = (tbyte*) fbuffer;
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
	free(fbuffer);
	
    return 0;
}
