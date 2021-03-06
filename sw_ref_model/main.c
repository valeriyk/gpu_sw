#include "main.h"
#include "wavefront_obj.h"
#include "geometry.h"
#include "gl.h"
//#include "shader_normalmap.h"
//#include "shader_phong.h"
#include "shader_gouraud.h"
#include "shader_depth.h"
#include "shader_fill_shadow_buf.h"
#include "bitmap.h"
#include "tga_addon.h"

#include "profiling.h"

#include <stdint.h>
//#include <limits.h>
#include <stdlib.h>
#include <math.h>

#include <time.h>

// POSITIVE Z TOWARDS ME

Light LIGHTS[MAX_NUM_OF_LIGHTS];

/*
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
*/



/*
// Setup object transformation, aka world transformation function.  
// Computes matrices that will be used to transform model's vertices and normals
// from the object space to the world space
void obj_set_transform (ObjectListNode *obj_list_head, fmat4 *proj, fmat4 *view) {
	
	ObjectListNode *node = obj_list_head;
	
	while (node != NULL) {

		fmat4 modelview;
		fmat4_fmat4_mult ( view, &(node->obj->model), &modelview);
		fmat4_fmat4_mult ( proj, &modelview, &(node->obj->mvp));
		fmat4_inv_transp (&modelview, &(node->obj->mvit));
		
		fmat4_copy (&(node->obj->mvp), &(node->obj->shadow_mvp[light_num]));	
		
		node = node->next;
	}
}
*/

void setup_transformation (ObjectListNode *obj_list_head, fmat4 *proj, fmat4 *view) {
	
	ObjectListNode *node = obj_list_head;
	
	while (node != NULL) {

		fmat4 modelview;
		fmat4_fmat4_mult ( view, &(node->obj->model), &modelview);
		fmat4_fmat4_mult ( proj, &modelview, &(node->obj->mvp));
		fmat4_inv_transp (&modelview, &(node->obj->mvit));
		
		node = node->next;
	}
}

void setup_light_transform (ObjectListNode *obj_list_head, fmat4 *proj, fmat4 *view, int light_num) {
	
	ObjectListNode *node = obj_list_head;
	
	while (node != NULL) {
		
		fmat4 modelview;
		fmat4_fmat4_mult ( view, &(node->obj->model), &modelview);
		fmat4_fmat4_mult ( proj, &modelview, &(node->obj->mvp));
		
		fmat4_copy (&(node->obj->mvp), &(node->obj->shadow_mvp[light_num]));	
		
		node = node->next;
	}
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

Bitmap *floor_diff        = NULL;
Bitmap *african_head_diff = NULL;
Bitmap *african_head_nmap = NULL;
Bitmap *african_head_spec = NULL;
Bitmap *cube_diff         = NULL;
Bitmap *cube_nmap         = NULL;

WaveFrontObj *my_floor     = NULL;
WaveFrontObj *african_head = NULL;
WaveFrontObj *my_cube      = NULL;

ObjectListNode* init_objects (void) {
	
    ObjectListNode *head;
    ObjectListNode *node;
    
    int draw_planes = 0;
    int draw_head = 1;
    int draw_second_head = 1;
    int draw_all_cubes = 1;
    
    if (draw_planes) {
		floor_diff = new_bitmap_from_tga("obj/floor_diffuse.tga");
		my_floor = wfobj_new ("obj/floor.obj");
	
		node = calloc (1, sizeof (ObjectListNode));
		head = node;
		node->obj = obj_new (my_floor, floor_diff, NULL, NULL);
		node->next = NULL;
		obj_set_rotation    (node->obj, 45.f, 0.f, 0.f);
		obj_set_translation (node->obj, 0.f, 2.f, -4.6f);
		obj_set_scale       (node->obj, 5, 5, 5);
		obj_init_model      (node->obj);

		node->next = calloc (1, sizeof (ObjectListNode));
		node       = node->next;
		node->obj  = obj_new (my_floor, floor_diff, NULL, NULL);
		node->next = NULL;
		obj_set_scale       (node->obj, 5, 5, 5);
		obj_set_rotation    (node->obj, 90, 0, 0);
		obj_set_translation (node->obj, 0.0f, 0.f, -6.f);
		obj_init_model      (node->obj);

	}
	else if (draw_head) {
	
		african_head_diff = new_bitmap_from_tga ("obj/african_head_diffuse.tga");
		african_head_nmap = new_bitmap_from_tga ("obj/african_head_nm.tga");
		african_head_spec = new_bitmap_from_tga ("obj/african_head_spec.tga");
		african_head = wfobj_new ("obj/african_head.obj");

	
		node = calloc (1, sizeof (ObjectListNode));
		head = node;
		node->obj = obj_new (african_head, african_head_diff, african_head_nmap, african_head_spec);
		//node->obj = obj_new (african_head, african_head_diff, NULL, NULL);
		//node->obj = obj_new (african_head, NULL, NULL, NULL);
		node->next = NULL;
		obj_set_scale       (node->obj, 7, 7, 7);
		//obj_set_rotation    (node->obj, 45, 45, 0);
		obj_set_translation (node->obj, -1.0f, 0.f, 0.f);
		obj_init_model      (node->obj);

		if (draw_second_head) {
			node->next = calloc (1, sizeof (ObjectListNode));
			node       = node->next;
			node->obj  = obj_new (african_head, african_head_diff, african_head_nmap, african_head_spec);
			node->next = NULL;
			//obj_set_rotation    (object[obj_idx], 90.f, 0.f, 0.f);
			obj_set_translation (node->obj, 1.f, 0.f, -10.0f);
			obj_set_scale       (node->obj, 7, 7, 7);
			obj_init_model      (node->obj);
		}
	}
	else {
		cube_diff = new_bitmap_from_tga ("obj/floor_diffuse.tga");
		cube_nmap = new_bitmap_from_tga ("obj/floor_nm_tangent.tga");
		my_cube = wfobj_new ("obj/cube.obj");


		// Central cube
		node = calloc (1, sizeof (ObjectListNode));
		
		head = node;
		
		node->obj  = obj_new (my_cube, cube_diff, cube_nmap, NULL);
		node->next = NULL;
		obj_set_scale       (node->obj, 5, 5, 5);
		//obj_set_scale       (node->obj, 2, 2, 2);
		obj_set_rotation    (node->obj, 0.f, 0.f, 0.f);
		obj_set_translation (node->obj, 0.f, 0.f, 0.f);
		obj_init_model      (node->obj);		
	
		if (draw_all_cubes) {
			// Cube
			node->next = calloc (1, sizeof (ObjectListNode));
			node = node->next;
			node->obj = obj_new (my_cube, cube_diff, cube_nmap, NULL);
			node->next = NULL;
			obj_set_scale       (node->obj, 2, 2, 2);
			obj_set_rotation    (node->obj, 45, 45, 0);
			obj_set_translation (node->obj, 1.0f, 0.f, 7.f);
			obj_init_model      (node->obj);
			
			// Cube
			node->next = calloc (1, sizeof (ObjectListNode));
			node = node->next;
			node->obj = obj_new (my_cube, cube_diff, cube_nmap, NULL);
			node->next = NULL;
			obj_set_scale       (node->obj, 2, 2, 2);
			obj_set_rotation    (node->obj, 45, 45, 0);
			obj_set_translation (node->obj, -1.0f, 0.f, 7.f);
			obj_init_model      (node->obj);
			
		}
	}
	
	return head;
}

void free_objects (ObjectListNode *obj_list_head) {
	
    ObjectListNode *node = obj_list_head;
    ObjectListNode *tmp;
    
    while (node != NULL) {
		if (node->obj != NULL) obj_free (node->obj);
		tmp = node->next;
		free (node);
		node = tmp;
	}
	
	if (floor_diff        != NULL) bitmap_free (floor_diff);
	if (african_head_diff != NULL) bitmap_free (african_head_diff);
	if (african_head_nmap != NULL) bitmap_free (african_head_nmap);
	if (african_head_spec != NULL) bitmap_free (african_head_spec);
	if (cube_diff         != NULL) bitmap_free (cube_diff);
	if (cube_nmap         != NULL) bitmap_free (cube_nmap);
	
	if (my_floor     != NULL) wfobj_free (my_floor);
	if (african_head != NULL) wfobj_free (african_head);
	if (my_cube      != NULL) wfobj_free (my_cube);
}


uint8_t rgb_to_y (pixel_color_t rgb) {
	return (uint8_t) (16.f + rgb.r * 0.257f + rgb.g * 0.504f + rgb.b * 0.098f);
}

uint8_t rgb_to_cb (pixel_color_t rgb) {
	return (uint8_t) (128.f - rgb.r * 0.148f - rgb.g * 0.291f + rgb.b * 0.439f);
}

uint8_t rgb_to_cr (pixel_color_t rgb) {
	return (uint8_t) (128.f + rgb.r * 0.439f - rgb.g * 0.368f - rgb.b * 0.071f);
}


int main(int argc, char** argv) {
       
    size_t screen_size = WIDTH * HEIGHT;
    
    screenz_t *zbuffer = (screenz_t*) calloc (screen_size, sizeof(screenz_t));
    
    pixel_color_t *fbuffer[NUM_OF_FRAMEBUFFERS];
    for (int i = 0; i < NUM_OF_FRAMEBUFFERS; i++) {
		fbuffer[i] = (pixel_color_t*) calloc (screen_size, sizeof(pixel_color_t));
	}
    	
    pixel_color_t *active_fbuffer = NULL;
    		
	ObjectListNode *obj_list_head = init_objects ();
	
    // 3. Projection Matrix - perspective correction
	float aspect_ratio = 16/9;
	float top   = 0.5;
	float bot   = -top;
	float right = top * aspect_ratio;
	float left  = -right;
	float near  = 1;
	float far   = 100;
	
	fmat4 persp_proj;
	fmat4 ortho_proj;
	init_perspective_proj (&persp_proj, left, right, top, bot, near, far);
	
	float f = 18.5;
	init_ortho_proj       (&ortho_proj, left*f, right*f, top*f, bot*f, near, far);
	
	// 4. Viewport Matrix - move to screen coords
	set_screen_size ((size_t) WIDTH, (size_t) HEIGHT);
    init_viewport (0, 0, get_screen_width(), get_screen_height(), get_screen_depth());
	
    
    // 2. View Matrix - transform global coords to camera coords
	//Float3 eye       = Float3_set ( 3.0f,   2.0f,   5.0f);
    Float3 eye;
	Float3 center = Float3_set (-0.f,  -0.f,   0.0f);
	Float3 up     = Float3_set ( 0.0f,   1.0f,   0.0f);
	fmat4 view;	
	//init_view (&view, &eye, &center, &up);
    
    init_lights();
    //new_light (0, Float3_set ( 0.f,  -2.f, -10.f), false);	
    new_light (0, Float3_set ( 0.f,  -2.f, -10.f), true);	
    
    
    float eye_x = 0;
    float eye_y = 1;
    float eye_z = 0;
    float eye_angle = ROTATION_INIT; // rad
    float eye_distance = 20;
    
    float obj_angle = 0;
    
    //do {
    
	if (ENABLE_PERF) {
		setup_counters();
		start_counters();
	}
    
    for (int m = 0; m < NUM_OF_FRAMES; m++) {
		
		//active_fbuffer = (active_fbuffer == fbuffer[0]) ? fbuffer[1] : fbuffer[0];
		active_fbuffer = fbuffer[m % NUM_OF_FRAMEBUFFERS];
		
		// clean up active framebuffer, zbuffer and all shadowbuffers
		for (int i = 0; i < screen_size; i++) {
			active_fbuffer[i] = set_color (0, 0, 0, 0);
			zbuffer[i] = 0;
			for (int j = 0; j < MAX_NUM_OF_LIGHTS; j++) {
				if (LIGHTS[j].enabled && LIGHTS[j].has_shadow_buf) {
					LIGHTS[j].shadow_buf[i] = 0;
				}
			}
		}
		
		for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
			if (LIGHTS[i].enabled && LIGHTS[i].has_shadow_buf) {
				init_view             (&view, &(LIGHTS[i].src), &center, &up);
				setup_light_transform (obj_list_head, &ortho_proj, &view, i);
				draw_frame            (obj_list_head, vshader_fill_shadow_buf, pshader_fill_shadow_buf, LIGHTS[i].shadow_buf, NULL);	
			}
		}			
		
		// move the camera
		eye_x = center.as_struct.x + eye_distance * cosf(eye_angle);
		eye_z = center.as_struct.z + eye_distance * sinf(eye_angle);
		eye    = Float3_set ( eye_x, eye_y, eye_z);
		eye_angle += ROTATION_INCR;
		
		// move the objects and recalculate their Model matrices
		ObjectListNode *scene_obj = obj_list_head;
		while (scene_obj != NULL) {	
			obj_set_rotation (scene_obj->obj, 0, obj_angle, 0);
			obj_init_model   (scene_obj->obj);			
			scene_obj = scene_obj->next;
		}
		obj_angle += 360 / NUM_OF_FRAMES;
		
		
		// 
		init_view            (&view, &eye, &center, &up);
		light_transform      (&view);
		setup_transformation (obj_list_head, &persp_proj, &view);
		
		//draw_frame           (obj_list_head, vshader_gouraud, pshader_gouraud, zbuffer, active_fbuffer);
		draw_frame           (obj_list_head, vshader_depth, pshader_depth, zbuffer, active_fbuffer);
		
		if (m == RECORD_FRAME_NUM) {
		
			if (ENABLE_PERF) stop_counters();
			
			char tga_file[32];
			char frame_num[32];
			char shadow_num[32];
			
			
			sprintf(frame_num, "%d", m);
			strcpy (tga_file, "frame_buffer_");
			strcat (tga_file, frame_num);
			strcat (tga_file, ".tga");	
			write_tga_file (tga_file, (tbyte *) fbuffer[m], WIDTH, HEIGHT, 24, 1);
			
			tbyte *tmp = (tbyte*) calloc (screen_size, sizeof(tbyte));
			
			for (int i = 0; i < screen_size; i++) {
				tmp[i] = zbuffer[i] >> 4;// >> (8 * (sizeof(screenz_t) - 1) );
			}
			write_tga_file ("zbuffer.tga", tmp, WIDTH, HEIGHT, 8, 1);
			
			for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
				if (LIGHTS[i].enabled) {
					sprintf(shadow_num, "%d", i);
					strcpy (tga_file, "shadow_buffer_");
					strcat (tga_file, frame_num);
					strcat (tga_file, "_");
					strcat (tga_file, shadow_num);
					strcat (tga_file, ".tga");
					
					for (int j = 0; j < screen_size; j++) {
						tmp[j] = LIGHTS[i].shadow_buf[j] >> (8 * (sizeof(screenz_t) - 1) );
					}
					write_tga_file (tga_file, tmp, WIDTH, HEIGHT, 8, 1);		
				}
			}		
			
			free (tmp);		
			
			if (ENABLE_PERF) start_counters();
		}
		
	}
	// while (0);
	
	if (ENABLE_PERF) {
		stop_counters();
		read_counters();
	}
	
	if (RECORD_VIDEO) {		
		FILE *fp = fopen ("video.y4m", "w");
		if (!fp) return 1;
		fprintf (fp, "YUV4MPEG2 W%d H%d F25:1 Ip A0:0 C444\n", WIDTH, HEIGHT);
		
		for (int i = 0; i < NUM_OF_FRAMES; i++) {
			fprintf (fp, "FRAME\n");
			for (int j = 0; j < screen_size; j++){
				uint8_t y = rgb_to_y (fbuffer[i][j]);
				fwrite (&y, sizeof (uint8_t), 1, fp);
			}
			for (int j = 0; j < screen_size; j++){
				uint8_t cb = rgb_to_cb (fbuffer[i][j]);
				fwrite (&cb, sizeof (uint8_t), 1, fp);
			}
			for (int j = 0; j < screen_size; j++){
				uint8_t cr = rgb_to_cr (fbuffer[i][j]);
				fwrite (&cr, sizeof (uint8_t), 1, fp);
			}
		}
		fclose (fp);
	}
	
	free (zbuffer);
	free_objects (obj_list_head);
	for (int i = 0; i < NUM_OF_FRAMEBUFFERS; i++) free (fbuffer[i]);
	for (int i = 0; i < MAX_NUM_OF_LIGHTS;   i++) free_light (i);
	
	
    return 0;
}
