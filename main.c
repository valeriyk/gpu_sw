#include "main.h"
#include "wavefront_obj.h"
#include "geometry.h"
#include "gl.h"
//#include "shader_normalmap.h"
//#include "shader_phong.h"
#include "shader_depth.h"
#include "bitmap.h"
#include <tga_addon.h>

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

// POSITIVE Z TOWARDS ME

// these are global:
fmat4  UNIFORM_M;
fmat4  UNIFORM_MIT;
fmat4  UNIFORM_MSHADOW;
Float3 UNIFORM_LIGHT;
screenz_t *UNIFORM_SHADOWBUF;


// Object transform, aka world transform function.  
// Computes matrices that will be used to transform a model's vertices and normals
// from the object space to the world space
void obj_transform (Object *obj, fmat4 *proj, fmat4 *view, Float3 *light_dir) {
	
	//fmat4_fmat4_fmat4_mult ( proj, view, &(obj->model), &(obj->mvp)); 
    //fmat4_fmat4_mult ( view, &(obj->model), &UNIFORM_M);
    fmat4_fmat4_mult ( view, &(obj->model), &UNIFORM_M);
    fmat4_fmat4_mult ( proj, &UNIFORM_M, &(obj->mvp));
	fmat4_inv_transp (&UNIFORM_M, &UNIFORM_MIT);
	
	/*print_fmat4 (&(obj->model), "model matrix");
	print_fmat4 (view, "view matrix");
	print_fmat4 (proj, "projection matrix");
	print_fmat4 (&UNIFORM_M, "UNIFORM_M");
	print_fmat4 (&UNIFORM_MIT, "UNIFORM_MIT");
	*/
	Float4 light_dir4 = Float3_Float4_vect_conv (light_dir);
	// Light vector changes after View and Projection transformations only,
	// it does not depend on Model transformation
	Float4 light_new = fmat4_Float4_mult  (view, &light_dir4);
	UNIFORM_LIGHT = Float4_Float3_vect_conv (&light_new);
    Float3_normalize   (&UNIFORM_LIGHT);
}

int main(int argc, char** argv) {
       
    size_t screen_size = WIDTH*HEIGHT;
    
    screenz_t     *zbuffer  = (screenz_t*)     calloc (screen_size, sizeof(screenz_t));
    pixel_color_t *fbuffer0 = (pixel_color_t*) calloc (screen_size, sizeof(pixel_color_t));
    pixel_color_t *fbuffer1 = (pixel_color_t*) calloc (screen_size, sizeof(pixel_color_t));
    
    screenz_t     *depth_buffer  = (screenz_t*)     calloc (screen_size, sizeof(screenz_t));
    //pixel_color_t *shadow_buffer = (pixel_color_t*) calloc (screen_size, sizeof(pixel_color_t));
    UNIFORM_SHADOWBUF = depth_buffer;
    
    pixel_color_t *active_fbuffer = NULL;
    
    
    Bitmap *african_head_diff = new_bitmap_from_tga ("obj/african_head_diffuse.tga");
    Bitmap *african_head_nmap = new_bitmap_from_tga ("obj/african_head_nm.tga");
    Bitmap *african_head_spec = new_bitmap_from_tga ("obj/african_head_spec.tga");
    WFobj *african_head = wfobj_new ("obj/african_head.obj", african_head_diff, african_head_nmap, african_head_spec);
	
	Bitmap *diablo_diff = new_bitmap_from_tga ("obj/diablo/diablo3_pose_diffuse.tga");
	Bitmap *diablo_nmap = new_bitmap_from_tga ("obj/diablo/diablo3_pose_nm.tga");
	Bitmap *diablo_spec = new_bitmap_from_tga ("obj/diablo/diablo3_pose_spec.tga");
	WFobj  *diablo = wfobj_new ("obj/diablo/diablo3_pose.obj", diablo_diff, diablo_nmap, diablo_spec);
	
	Bitmap *cube_diff = new_bitmap_from_tga ("obj/floor_diffuse.tga");
	Bitmap *cube_nmap = new_bitmap_from_tga ("obj/floor_nm_tangent.tga");
	WFobj  *my_cube = wfobj_new ("obj/cube.obj", cube_diff, cube_nmap, NULL);
	
	/*
	WFobj *my_floor = wfobj_new ("obj/floor.obj");
	wfobj_load_texture      (my_floor, "obj/floor_diffuse.tga");
	wfobj_load_normal_map   (my_floor, "obj/floor_nm_tangent.tga");
	*/
	
	//Float3 camera = Float3_Float3_sub(&eye, &center);	
	
	
	// 1. Model Matrix - transform local coords to global
	
    Object *head1  = obj_new (african_head);
    obj_set_translation (head1, -1.f, 0.f, -0.f);
    //obj_set_scale       (head1, 3, 3, 3);
    obj_set_rotation    (head1, 0.f, 20.f, 0.f);
    obj_init_model      (head1);
    
    Object *head2  = obj_new (african_head);
    obj_set_translation (head2, 1.f, 0.f, -0.0f);
    obj_set_rotation    (head2, 0.f, 20.f, 0.f);
    obj_init_model      (head2);
    
    
    Object *diablo1  = obj_new (diablo);
    obj_set_translation (diablo1, 0.f, 0.f, -3.f);
    //obj_set_scale       (diablo1, 3, 3, 3);
    //obj_set_rotation    (diablo1, 0.f, 20.f, 0.f);
    obj_init_model      (diablo1);
    
    /* 
    Object *floor1 = obj_new (my_floor);
    obj_set_translation (floor1, 0.f, 0.f, 0.75f);
	obj_build_model     (floor1);
	*/
	Object *cube1 = obj_new (my_cube);
	obj_set_scale    (cube1, 10, 10, 10);
	//obj_set_rotation (cube1, 45, 45, 0);
	obj_set_translation (cube1, -5.5f, 5.5f, -5.5f);
	obj_init_model (cube1);
	
	Object *cube2 = obj_new (my_cube);
	//obj_set_scale    (cube1, 0.5, 0.5, 0.5);
	obj_set_scale    (cube2, 2, 2, 2);
	//obj_set_rotation (cube2, 45, 45, 0);
	obj_set_translation (cube2, 1.5f, -1.5f, 1.5f);
	obj_init_model (cube2);
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
	
	// 2. View Matrix - transform global coords to camera coords
	//Float3 eye       = Float3_set ( 3.0f,   2.0f,   5.0f);
    Float3 eye    = Float3_set ( 0.0f,   0.0f,   10.000f);
	Float3 center = Float3_set ( 0.0f,   0.0f,   0.0f);
	Float3 up     = Float3_set ( 0.0f,   1.0f,   0.0f);
	fmat4 view    = FMAT4_IDENTITY;	
	init_view (&view, &eye, &center, &up);
    
    // 3. Projection Matrix - perspective correction
	float aspect_ratio = 16/9;
	float top   = 0.6;
	float bot   = -top;
	float right = top * aspect_ratio;
	float left  = -right;
	float near  = 1;
	float far   = 100;
	
	fmat4 persp_proj  = FMAT4_IDENTITY;
	fmat4 ortho_proj  = FMAT4_IDENTITY;
	init_perspective_proj (&persp_proj, left, right, top, bot, near, far);
	init_ortho_proj       (&ortho_proj, left*20, right*20, top*20, bot*20, near, far);
	
	// 4. Viewport Matrix - move to screen coords
	//fmat4 viewport = FMAT4_IDENTITY;
	//init_viewport (&viewport, 0, 0, WIDTH, HEIGHT, DEPTH);
    
    
	
    
    
    //Float3 light_dir = Float3_set (-1.0f,  -0.3f,  -1.0f);
    Float3 light_dir = Float3_set ( -1.0f,  1.0f,  -1.25f);
    Float3 light_src = Float3_set ( 40.0f,   -40.0f,   50.0f);
    					
    //do {
    bool heads = 0;
    for (int i = 0; i < 1; i++) {
		active_fbuffer = (active_fbuffer == fbuffer0) ? fbuffer1 : fbuffer0;
		
		for (int i = 0; i < screen_size; i++) {
			zbuffer[i] = 0;
			depth_buffer[i] = 0;
		}
		
		init_view (&view, &light_src, &center, &up);
		
		fmat4 shadow_mvp;
		fmat4 mvp_inv;
		if (heads) {
			obj_transform       (head1, &ortho_proj, &view, &light_dir);
			obj_draw            (head1, depth_vshader_pass1, depth_pshader_pass1, depth_buffer, NULL);
			
			obj_transform       (head2, &ortho_proj, &view, &light_dir);
			obj_draw            (head2, depth_vshader_pass1, depth_pshader_pass1, depth_buffer, NULL);
		}
		else {
			init_view     (&view, &light_src, &center, &up);
			obj_transform (cube1, &ortho_proj, &view, &light_dir);
			obj_draw      (cube1, depth_vshader_pass1, depth_pshader_pass1, depth_buffer, NULL);
			fmat4_copy    (&(cube1->mvp), &shadow_mvp);
			
			init_view        (&view, &eye, &center, &up);
			obj_transform    (cube1, &persp_proj, &view, &light_dir);
			fmat4_inv        (&(cube1->mvp), &mvp_inv);
			fmat4_fmat4_mult (&shadow_mvp, &mvp_inv, &UNIFORM_MSHADOW);
			if (DEBUG_0) {
				print_fmat4 (&shadow_mvp, "shadow_mvp 1");
				print_fmat4 (&(cube1->mvp), "cube1 mvp");
				print_fmat4 (&mvp_inv, "mvp_inv 1");
				print_fmat4 (&UNIFORM_MSHADOW, "UNIFORM_MSHADOW 1");
			}
			obj_draw         (cube1, depth_vshader_pass2, depth_pshader_pass2, zbuffer, active_fbuffer);
			
			init_view     (&view, &light_src, &center, &up);
			obj_transform (cube2, &ortho_proj, &view, &light_dir);
			obj_draw      (cube2, depth_vshader_pass1, depth_pshader_pass1, depth_buffer, NULL);
			fmat4_copy    (&(cube2->mvp), &shadow_mvp);
			
			init_view        (&view, &eye, &center, &up);
			obj_transform    (cube2, &persp_proj, &view, &light_dir);
			fmat4_inv        (&(cube2->mvp), &mvp_inv);
			fmat4_fmat4_mult (&shadow_mvp, &mvp_inv, &UNIFORM_MSHADOW);
			if (DEBUG_0) {
				print_fmat4 (&shadow_mvp, "shadow_mvp 2");
				print_fmat4 (&(cube2->mvp), "cube2 mvp");
				print_fmat4 (&mvp_inv, "mvp_inv 2");
				print_fmat4 (&UNIFORM_MSHADOW, "UNIFORM_MSHADOW 2");
			}
			obj_draw         (cube2, depth_vshader_pass2, depth_pshader_pass2, zbuffer, active_fbuffer);
		}
		/*
		//UNIFORM_MSHADOW = UNIFORM_M;
		
		init_view (&view, &eye, &center, &up);
		
		//UNIFORM_MSHADOW = 
		
		if (heads) {
			obj_transform       (head1, &persp_proj, &view, &light_dir);
			obj_draw            (head1, phong_vertex_shader, phong_pixel_shader, zbuffer, active_fbuffer);
			
			obj_transform       (head2, &persp_proj, &view, &light_dir);
			obj_draw            (head2, nm_vertex_shader, nm_pixel_shader, zbuffer, active_fbuffer);
		}
		else {
			
			;
		}
		*/
		//obj_transform       (diablo1, &proj, &view, &light_dir);
		////obj_draw            (diablo1, phong_vertex_shader, phong_pixel_shader, zbuffer, active_fbuffer);
		//obj_draw            (diablo1, nm_vertex_shader, nm_pixel_shader, zbuffer, active_fbuffer);
		
		/*	
		obj_transform           (head2, &viewport_proj_view, &proj_view, &light_dir);
		obj_draw            (head2, nm_vertex_shader, nm_pixel_shader, zbuffer, active_fbuffer);
		
		//transform       (floor1, &vpv, &projview, &light_dir);
		//obj_draw            (floor1, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer);
		*/
		light_dir.as_struct.x = -light_dir.as_struct.x;
		light_dir.as_struct.y = -light_dir.as_struct.y;
		
		
		/*
		obj_transform       (floor2, &vpv, &projview, &light_dir);
		obj_draw            (floor2, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer);
			
		obj_transform       (floor3, &vpv, &projview, &light_dir);
		obj_draw            (floor3, my_vertex_shader, my_pixel_shader, zbuffer, active_fbuffer);
		*/
	}// while (0);
	
    write_tga_file ("output_fb0.tga", (tbyte *) fbuffer0, WIDTH, HEIGHT, 24, 1);
    //write_tga_file ("output_fb1.tga", (tbyte *) fbuffer1, WIDTH, HEIGHT, 24, 1);
    write_tga_file (   "zbuffer.tga", (tbyte *)  zbuffer, WIDTH, HEIGHT,  8, 1);
	
	//write_tga_file ("shadow_buffer.tga", (tbyte *) shadow_buffer, WIDTH, HEIGHT, 24, 1);
    write_tga_file ("depth_buffer.tga", (tbyte *)  depth_buffer, WIDTH, HEIGHT,  8, 1);
	
	//wfobj_free(african_head);
	//wfobj_free(my_floor);
	
	free(zbuffer);
	free(fbuffer0);
	free(fbuffer1);
	
    return 0;
}
