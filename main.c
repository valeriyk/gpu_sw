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
//#include <limits.h>
#include <stdlib.h>
#include <math.h>

// POSITIVE Z TOWARDS ME

// these are global:
fmat4  UNIFORM_M;
fmat4  UNIFORM_MIT;
//fmat4  UNIFORM_MVP_SHADOW;
//fmat4  UNIFORM_MVP_SHADOW_2;
//fmat4  UNIFORM_MVP_INV;

//Float3 UNIFORM_LIGHT[2];

//screenz_t *UNIFORM_SHADOWBUF[2];
//screenz_t *UNIFORM_SHADOWBUF_2;


Light LIGHTS[MAX_NUM_OF_LIGHTS];


void   print_fmat3 (fmat3 *m, char *header);
void   print_fmat4 (fmat4 *m, char *header);





void print_fmat4 (fmat4 *m, char *header) {
	printf ("%s\n", header);
	for (int i = 0; i < 4; i++) {
		printf("row %d: ", i);
		for (int j = 0; j < 4; j++)
			printf ("%f ", (*m)[i][j]);
		printf("\n");
	}
	printf("\n");
}

void print_fmat3 (fmat3 *m, char *header) {
	printf ("%s\n", header);
	for (int i = 0; i < 3; i++) {
		printf("row %d: ", i);
		for (int j = 0; j < 3; j++)
			printf ("%f ", (*m)[i][j]);
		printf("\n");
	}
	printf("\n");
}

// Object transform, aka world transform function.  
// Computes matrices that will be used to transform a model's vertices and normals
// from the object space to the world space
void obj_transform (Object *obj, fmat4 *proj, fmat4 *view) {
	
	fmat4_fmat4_mult ( view, &(obj->model), &UNIFORM_M);
    fmat4_fmat4_mult ( proj, &UNIFORM_M, &(obj->mvp));
	fmat4_inv_transp (&UNIFORM_M, &UNIFORM_MIT);
	
	/*print_fmat4 (&(obj->model), "model matrix");
	print_fmat4 (view, "view matrix");
	print_fmat4 (proj, "projection matrix");
	print_fmat4 (&UNIFORM_M, "UNIFORM_M");
	print_fmat4 (&UNIFORM_MIT, "UNIFORM_MIT");
	*/
}


void light_transform (fmat4 *view) {
	Float4 light4_a;
	Float4 light4_b;
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		if (!LIGHTS[i].enabled) continue;
		light4_a = Float3_Float4_conv (&(LIGHTS[i].dir), 0);
		// Light vector changes after View transformation only,
		// it does not depend on Model and Projection transformations
		light4_b = fmat4_Float4_mult  (view, &light4_a);
		LIGHTS[i].eye = Float4_Float3_vect_conv (&light4_b);
		Float3_normalize (&(LIGHTS[i].eye));
	}
}

void init_objects (Object *object[NUM_OF_OBJECTS]) {
	
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
	
	Bitmap *floor_diff = new_bitmap_from_tga("obj/floor_diffuse.tga");
	WFobj *my_floor = wfobj_new ("obj/floor.obj", floor_diff, NULL, NULL);
	
	
    /*Object *head1  = obj_new (african_head);
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
    */
    
    // Cube 0
    object[0] = obj_new (my_cube);
	obj_set_scale       (object[0], 2, 2, 2);
	obj_set_rotation    (object[0], 45, 45, 0);
	obj_set_translation (object[0], 1.0f, 0.f, 0.f);
	obj_init_model      (object[0]);
	
	// Cube 1
	object[1] = obj_new (my_cube);
	//obj_set_scale       (object[1], 0.5, 0.5, 0.5);
	obj_set_scale       (object[1], 2, 2, 2);
	obj_set_rotation    (object[1], 45, 45, 0);
	obj_set_translation (object[1], -1.0f, 0.f, 0.f);
	obj_init_model      (object[1]);
	
	// Floor
	object[2] = obj_new (my_floor);
    //obj_set_rotation    (object[2], 90.f, 0.f, 0.f);
    obj_set_translation (object[2], 0.f, -0.5f, -2.0f);
    obj_set_scale       (object[2], 6, 4, 4);
	obj_init_model      (object[2]);
	
	/* 
    Object *floor1 = obj_new (my_floor);
    obj_set_rotation    (floor1, 90.f, 0.f, 0.f);
    //obj_set_translation (floor1, 0.f, 0.f, -1.0f);
	obj_init_model      (floor1);
	
	Object *floor2 = obj_new (my_floor);
    obj_set_rotation    (floor2, 90.f, 0.f, 0.f);
	obj_set_translation (floor2, -0.0f, -0.0f, -0.5f);
	obj_set_scale       (floor2, 1.2, 1.2, 1.2);
	obj_init_model      (floor2);
	
	Object *floor3 = obj_new (my_floor);
    obj_set_rotation    (floor3, 0.f, 0.f, -90.f);
	obj_set_translation (floor3, 0.f, 0.f, 0.75f);
	obj_build_model     (floor3);
	*/
}

int main(int argc, char** argv) {
       
    init_scene();
    
    size_t screen_size = WIDTH*HEIGHT;
    
    screenz_t     *zbuffer  = (screenz_t*)     calloc (screen_size, sizeof(screenz_t));
    pixel_color_t *fbuffer0 = (pixel_color_t*) calloc (screen_size, sizeof(pixel_color_t));
    pixel_color_t *fbuffer1 = (pixel_color_t*) calloc (screen_size, sizeof(pixel_color_t));
    
    pixel_color_t *active_fbuffer = NULL;
    
	// 1. Model Matrix - transform local coords to global
	
	Object *object[NUM_OF_OBJECTS];
	init_objects(object);	
	
    // 3. Projection Matrix - perspective correction
	float aspect_ratio = 16/9;
	//float top   = 0.6;
	float top   = 1;
	float bot   = -top;
	float right = top * aspect_ratio;
	float left  = -right;
	float near  = 1;
	float far   = 50;
	
	fmat4 persp_proj  = FMAT4_IDENTITY;
	fmat4 ortho_proj  = FMAT4_IDENTITY;
	init_perspective_proj (&persp_proj, left, right, top, bot, near, far);
	float f = 10.0;
	init_ortho_proj       (&ortho_proj, left*f, right*f, top*f, bot*f, near, far);
	
	// 4. Viewport Matrix - move to screen coords
	//fmat4 viewport = FMAT4_IDENTITY;
	//init_viewport (&viewport, 0, 0, WIDTH, HEIGHT, DEPTH);
	set_screen_size ((screenxy_t) WIDTH, (screenxy_t) HEIGHT);
    
	
    
    // 2. View Matrix - transform global coords to camera coords
	//Float3 eye       = Float3_set ( 3.0f,   2.0f,   5.0f);
    Float3 eye    = Float3_set ( -0.5f,  -0.5f,   6.500f);
	Float3 center = Float3_set (-0.f,  -0.f,   0.0f);
	Float3 up     = Float3_set ( 0.0f,   1.0f,   0.0f);
	fmat4 view    = FMAT4_IDENTITY;	
	init_view (&view, &eye, &center, &up);
    
    new_light (0, Float3_set (-3.0f,  -5.f, -2.5f));					
    new_light (1, Float3_set ( 3.0f,  -5.f, -2.5f));
    
    //do {
    for (int m = 0; m < 1; m++) {
		active_fbuffer = (active_fbuffer == fbuffer0) ? fbuffer1 : fbuffer0;
		
		for (int i = 0; i < screen_size; i++) {
			zbuffer[i] = 0;
			LIGHTS[0].shadow_buf[i] = 0;
			LIGHTS[1].shadow_buf[i] = 0;
		}
		
		for (int i = 0; i < 2; i++) {
			init_view       (&view, &(LIGHTS[i].src), &center, &up);
			for (int j = 0; j < NUM_OF_OBJECTS; j++) {
				obj_transform (object[j], &ortho_proj, &view);
				obj_draw      (object[j], depth_vshader_pass1, depth_pshader_pass1, LIGHTS[i].shadow_buf, NULL);
				fmat4_copy    (&(object[j]->mvp), &(object[j]->shadow_mvp[i]));
			}
		}			
		
		init_view       (&view, &eye, &center, &up);
		light_transform (&view);
		for (int i = 0; i < NUM_OF_OBJECTS; i++) {
			obj_transform    (object[i], &persp_proj, &view);
			obj_draw         (object[i], depth_vshader_pass2, depth_pshader_pass2, zbuffer, active_fbuffer);
		}
	}// while (0);
	
    write_tga_file ("output_fb0.tga", (tbyte *) fbuffer0, WIDTH, HEIGHT, 24, 1);
    //write_tga_file ("output_fb1.tga", (tbyte *) fbuffer1, WIDTH, HEIGHT, 24, 1);
    if (sizeof(screenz_t) == 1) {
		write_tga_file ("zbuffer.tga", (tbyte *)  zbuffer, WIDTH, HEIGHT, 8, 1);
		
		for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
			if (LIGHTS[i].enabled) {
				char sb_file[32];
				char num[2];
				sprintf(num, "%d", i);
				strcpy (sb_file, "shadow_buffer_");
				strcat (sb_file, num);
				strcat (sb_file, ".tga");
				write_tga_file (sb_file, (tbyte *) LIGHTS[i].shadow_buf, WIDTH, HEIGHT, 8, 1);
			}
		}
	}
	
	//wfobj_free(african_head);
	//wfobj_free(my_floor);
	
	free(zbuffer);
	free(fbuffer0);
	free(fbuffer1);
	
    return 0;
}
