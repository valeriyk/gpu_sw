#include "main.h"
#include "wavefront_obj.h"
#include "geometry.h"
#include "gl.h"
#include "shader_normalmap.h"
#include "shader_phong.h"
#include "bitmap.h"
#include <tga_addon.h>

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

// POSITIVE Z TOWARDS ME

// these are global:
fmat4  UNIFORM_M;
fmat4  UNIFORM_MIT;
Float3 UNIFORM_LIGHT;

// Object transform, aka world transform funciton.  
// Computes matrices that will be used to transform a model's vertices and normals
// from the object space to the world space
void obj_transform (Object *obj, fmat4 *vpv, fmat4 *pv, Float3 *light_dir) {
	
	fmat4_fmat4_mult (vpv, &(obj->model), &(obj->mvpv)); 
    fmat4_fmat4_mult ( pv, &(obj->model), &UNIFORM_M);
	fmat4_inv_transp (&UNIFORM_M, &UNIFORM_MIT);
	
	Float4 light_dir4 = Float3_Float4_vect_conv (light_dir);
	//Float4 light_new = fmat4_Float4_mult  (&UNIFORM_M, &light_dir4);
	Float4 light_new = fmat4_Float4_mult  (pv, &light_dir4);
	UNIFORM_LIGHT = Float4_Float3_vect_conv (&light_new);
    Float3_normalize   (&UNIFORM_LIGHT);
}

int main(int argc, char** argv) {
       
    size_t screen_size = WIDTH*HEIGHT;
    
    Float3 light_dir = Float3_set (-0.2f,  -0.3f,  -1.0f);
    //Float3 eye       = Float3_set ( 3.0f,   2.0f,   5.0f);
    Float3 eye       = Float3_set ( 0.0f,   0.0f,   5.0f);
	Float3 center    = Float3_set ( 0.0f,   0.0f,   0.0f);
	Float3 up        = Float3_set ( 0.0f,   1.0f,   0.0f);

    screenz_t     *zbuffer  = (screenz_t*)     calloc (screen_size, sizeof(screenz_t)    );
    pixel_color_t *fbuffer0 = (pixel_color_t*) calloc (screen_size, sizeof(pixel_color_t));
    pixel_color_t *fbuffer1 = (pixel_color_t*) calloc (screen_size, sizeof(pixel_color_t));
    
    pixel_color_t *active_fbuffer = NULL;
    
    /*
    Bitmap *african_head_diffuse      = new_bitmap_from_tga ("obj/african_head_diffuse.tga");
    Bitmap *african_head_normal_map   = new_bitmap_from_tga ("obj/african_head_nm.tga");
    Bitmap *african_head_specular_map = new_bitmap_from_tga ("obj/african_head_spec.tga");
    WFobj *african_head = wfobj_new ("obj/african_head.obj", african_head_diffuse, african_head_normal_map, african_head_specular_map);
	*/
	
	Bitmap *cube_diffuse      = new_bitmap_from_tga ("obj/floor_diffuse.tga");
	Bitmap *cube_normal_map   = new_bitmap_from_tga ("obj/floor_nm_tangent.tga");
	Bitmap *cube_specular_map = new_bitmap_from_tga ("obj/floor_nm_tangent.tga");
	WFobj *my_cube = wfobj_new ("obj/cube.obj", cube_diffuse, cube_normal_map, cube_specular_map);
	
	/*
	WFobj *my_floor = wfobj_new ("obj/floor.obj");
	wfobj_load_texture      (my_floor, "obj/floor_diffuse.tga");
	wfobj_load_normal_map   (my_floor, "obj/floor_nm_tangent.tga");
	*/
	
	Float3 camera = Float3_Float3_sub(&eye, &center);	
	
	
	// 1. Model - transform local coords to global
	// 2. View - transform global coords to adjust for camera position
	// 3. Projection - perspective correction
	// 4. Viewport - move to screen coords
	fmat4 viewport   = FMAT4_IDENTITY;
	fmat4 proj       = FMAT4_IDENTITY;
	fmat4 view       = FMAT4_IDENTITY;	
	init_viewport   (&viewport, 0, 0, WIDTH, HEIGHT, DEPTH);
    init_projection (&proj, -1.0f/camera.as_struct.z);
    init_view       (&view, &eye, &center, &up);
    
    fmat4 viewport_proj_view;
	fmat4_fmat4_fmat4_mult (&viewport, &proj, &view, &viewport_proj_view);
	
	fmat4 proj_view;
    fmat4_fmat4_mult (&proj, &view, &proj_view);	
	
	/*
    Object *head1  = obj_new (african_head);
    obj_set_translation (head1, 1.f, 0.f, 0.6f);
    obj_set_scale       (head1, 0.6, 0.6, 0.6);
    obj_set_rotation    (head1, 0.f, 0.f, 0.f);
    obj_init_model      (head1);
    
    Object *head2  = obj_new (african_head);
    obj_set_translation (head2, -1.f, 0.f, 0.6f);
    obj_init_model      (head2);
    */
    
   /* 
    Object *floor1 = obj_new (my_floor);
    obj_set_translation (floor1, 0.f, 0.f, 0.75f);
	obj_build_model     (floor1);
	*/
	Object *cube1 = obj_new (my_cube);
	obj_set_translation (cube1, 0.f, 0.f, 0.0f);
	obj_set_rotation (cube1, 45, 45, 0);
	//obj_set_scale    (cube1, 0.5, 0.5, 0.5);
	obj_init_model (cube1);
	
	
	/*
	Object *floor2 = obj_new (my_floor);
    obj_set_rotation    (floor2, 90.f, 0.f, 0.f);
	obj_set_translation (floor2, 0.f, 0.75f, 0.0f);
	obj_build_model     (floor2);
	
	Object *floor3 = obj_new (my_floor);
    obj_set_rotation    (floor3, 0.f, 0.f, -90.f);
	obj_set_translation (floor3, 0.f, 0.f, 0.75f);
	obj_build_model     (floor3);
	*/
					
    //do {
    for (int i = 0; i < 1; i++) {
		active_fbuffer = (active_fbuffer == fbuffer0) ? fbuffer1 : fbuffer0;
		
		for (int i = 0; i < screen_size; i++) zbuffer[i] = 0;
		/*
		obj_transform           (head1, &viewport_proj_view, &proj_view, &light_dir);
		//obj_draw            (head1, phong_vertex_shader, phong_pixel_shader, zbuffer, active_fbuffer);
		obj_draw            (head1, nm_vertex_shader, nm_pixel_shader, zbuffer, active_fbuffer);
			
		obj_transform           (head2, &viewport_proj_view, &proj_view, &light_dir);
		obj_draw            (head2, nm_vertex_shader, nm_pixel_shader, zbuffer, active_fbuffer);
		
		//transform       (floor1, &vpv, &projview, &light_dir);
		//obj_draw            (floor1, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer);
		
		light_dir.as_struct.z = -light_dir.as_struct.z;
		*/
		
		obj_transform	       (cube1, &viewport_proj_view, &proj_view, &light_dir);
		//obj_draw           (cube1, nm_vertex_shader, nm_pixel_shader, zbuffer, active_fbuffer);
		obj_draw           (cube1, phong_vertex_shader, phong_pixel_shader, zbuffer, active_fbuffer);
		
		/*
		obj_transform       (floor2, &vpv, &projview, &light_dir);
		obj_draw            (floor2, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer);
			
		obj_transform       (floor3, &vpv, &projview, &light_dir);
		obj_draw            (floor3, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer);
		*/
	}// while (0);
	
    write_tga_file ("output_fb0.tga", (tbyte *) fbuffer0, WIDTH, HEIGHT, 1);
    write_tga_file ("output_fb1.tga", (tbyte *) fbuffer1, WIDTH, HEIGHT, 1);
	
	//wfobj_free(african_head);
	//wfobj_free(my_floor);
	
	free(zbuffer);
	free(fbuffer0);
	free(fbuffer1);
	
    return 0;
}
