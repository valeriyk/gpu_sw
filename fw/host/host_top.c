#include "host_top.h"

//#include <wavefront_obj.h>
//#include <geometry.h>

//~ #ifdef SINGLEPROC_SINGLETHREAD
	//~ #include <vshader_loop.h>
	//~ #include <pshader_loop.h>
//~ #endif

#include <shader_normalmap.h>
#include <shader_phong.h>
#include <shader_gouraud.h>
#include <shader_depth.h>
#include <shader_fill_shadow_buf.h>
//#include "bitmap.h"
#include <tga_addon.h>


#include "profiling.h"


//#include <platform.h>



#include <stdint.h>
//#include <limits.h>
#include <stdlib.h>
#include <math.h>

#include <time.h>

// POSITIVE Z TOWARDS ME



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

void launch_shaders (volatile gpu_cfg_t *cfg, volatile gpu_run_halt_t *run_halt, vertex_shader_fptr vshader, pixel_shader_fptr pshader, screenz_t *zbuffer, pixel_color_t *fbuffer);



void setup_transformation (volatile ObjectListNode* volatile obj_list_head, fmat4 *proj, fmat4 *view) {
	
	volatile ObjectListNode* volatile node = obj_list_head;
	
	while (node != NULL) {

		fmat4 modelview;
		fmat4_fmat4_mult ( view, &(node->obj->model), &modelview);
		fmat4_fmat4_mult ( proj, &modelview, &(node->obj->mvp));
		fmat4_inv_transp (&modelview, &(node->obj->mvit));
		
		node = node->next;
	}
}

void setup_light_transform (volatile ObjectListNode* volatile obj_list_head, fmat4 *proj, fmat4 *view, int light_num) {
	
	volatile ObjectListNode* volatile node = obj_list_head;
	
	while (node != NULL) {
		
		fmat4 modelview;
		fmat4_fmat4_mult ( view, &(node->obj->model), &modelview);
		fmat4_fmat4_mult ( proj, &modelview, &(node->obj->mvp));
		
		fmat4_copy (&(node->obj->mvp), &(node->obj->shadow_mvp[light_num]));	
		
		node = node->next;
	}
}


void light_transform (fmat4 *view, volatile gpu_cfg_t *cfg) {
	Float4 light4_a;
	Float4 light4_b;
	for (int i = 0; i < GPU_MAX_LIGHTS; i++) {
		if (!cfg->lights_arr[i].enabled) continue;
		light4_a = Float3_Float4_conv (&(cfg->lights_arr[i].dir), 0);
		// Light vector changes after View transformation only,
		// it does not depend on Model and Projection transformations
		light4_b = fmat4_Float4_mult  (view, &light4_a);
		cfg->lights_arr[i].eye = Float4_Float3_vect_conv (&light4_b);
		Float3_normalize (&(cfg->lights_arr[i].eye));
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
		floor_diff = new_bitmap_from_tga("../../models/floor_diffuse.tga");
		my_floor = wfobj_new ("../../models/floor.obj");
	
		if ((node = calloc (1, sizeof (ObjectListNode))) == NULL) {
			if (DEBUG_MALLOC) printf ("obj list node calloc failed\n");
			return NULL;
		}
		head = node;
		node->obj = obj_new (my_floor, floor_diff, NULL, NULL);
		node->next = NULL;
		obj_set_rotation    (node->obj, 45.f, 0.f, 0.f);
		obj_set_translation (node->obj, 0.f, 2.f, -4.6f);
		obj_set_scale       (node->obj, 5, 5, 5);
		obj_init_model      (node->obj);

		if ((node->next = calloc (1, sizeof (ObjectListNode))) == NULL) {
			if (DEBUG_MALLOC) printf ("next obj list node calloc failed\n");
			return NULL;
		}
		node       = node->next;
		node->obj  = obj_new (my_floor, floor_diff, NULL, NULL);
		node->next = NULL;
		obj_set_scale       (node->obj, 5, 5, 5);
		obj_set_rotation    (node->obj, 90, 0, 0);
		obj_set_translation (node->obj, 0.0f, 0.f, -6.f);
		obj_init_model      (node->obj);

	}
	else if (draw_head) {
	
		african_head_diff = new_bitmap_from_tga ("../../models/african_head_diffuse.tga");
		//african_head_nmap = new_bitmap_from_tga ("../../models/african_head_nm.tga");
		//african_head_spec = new_bitmap_from_tga ("../../models/african_head_spec.tga");
		african_head = wfobj_new ("../../models/african_head.obj");

	
		if ((node = calloc (1, sizeof (ObjectListNode))) == NULL) {
			if (DEBUG_MALLOC) printf ("obj list node calloc failed\n");
			return NULL;
		}
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
			if ((node->next = calloc (1, sizeof (ObjectListNode))) == NULL) {
				if (DEBUG_MALLOC) printf ("next obj list node calloc failed\n");
				return NULL;
			}
			node       = node->next;
			node->obj  = obj_new (african_head, african_head_diff, african_head_nmap, african_head_spec);
			//node->obj = obj_new (african_head, NULL, NULL, NULL);
			node->next = NULL;
			//obj_set_rotation    (object[obj_idx], 90.f, 0.f, 0.f);
			obj_set_translation (node->obj, 1.f, 0.f, -10.0f);
			obj_set_scale       (node->obj, 7, 7, 7);
			obj_init_model      (node->obj);
		}
	}
	else {
		cube_diff = new_bitmap_from_tga ("../../models/floor_diffuse.tga");
		cube_nmap = new_bitmap_from_tga ("../../models/floor_nm_tangent.tga");
		my_cube = wfobj_new ("../../models/cube.obj");


		// Central cube
		if ((node = calloc (1, sizeof (ObjectListNode))) == NULL) {
			if (DEBUG_MALLOC) printf ("obj list calloc failed\n");
			return NULL;
		}
		
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
			if ((node->next = calloc (1, sizeof (ObjectListNode))) == NULL) {
				if (DEBUG_MALLOC) printf ("next obj list node calloc failed\n");
				return NULL;
			}
			node = node->next;
			node->obj = obj_new (my_cube, cube_diff, cube_nmap, NULL);
			node->next = NULL;
			obj_set_scale       (node->obj, 2, 2, 2);
			obj_set_rotation    (node->obj, 45, 45, 0);
			obj_set_translation (node->obj, 1.0f, 0.f, 7.f);
			obj_init_model      (node->obj);
			
			// Cube
			if ((node->next = calloc (1, sizeof (ObjectListNode))) == NULL) {
				if (DEBUG_MALLOC) printf ("next obj list node calloc failed\n");
				return NULL;
			}
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
	return (uint8_t) (16.f + rgb.as_byte.r * 0.257f + rgb.as_byte.g * 0.504f + rgb.as_byte.b * 0.098f);
}

uint8_t rgb_to_cb (pixel_color_t rgb) {
	return (uint8_t) (128.f - rgb.as_byte.r * 0.148f - rgb.as_byte.g * 0.291f + rgb.as_byte.b * 0.439f);
}

uint8_t rgb_to_cr (pixel_color_t rgb) {
	return (uint8_t) (128.f + rgb.as_byte.r * 0.439f - rgb.as_byte.g * 0.368f - rgb.as_byte.b * 0.071f);
}


void launch_shaders (volatile gpu_cfg_t *cfg, volatile gpu_run_halt_t *run_halt, vertex_shader_fptr vshader, pixel_shader_fptr pshader, screenz_t *zbuffer, pixel_color_t *fbuffer) {
	
	cfg->vshader_fptr = vshader;
	cfg->pshader_fptr = pshader;
	cfg->zbuffer_ptr = zbuffer;
	cfg->active_fbuffer = fbuffer;
			
#ifndef SINGLEPROC_SINGLETHREAD

	if (PTHREAD_DEBUG) printf("host: wait till all vshader_done signals are false\n");
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		while (run_halt->vshader_done[i]);
	}
	if (PTHREAD_DEBUG) printf("host: all vshader_done signals are false\n");
	
	if (PTHREAD_DEBUG) {
		printf("host: vshaders_run_req=true\n");
	}
	run_halt->vshaders_run_req = true;
	
		
	if (PTHREAD_DEBUG) printf("host: wait till all vshader_done signals are true\n");
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		while (!run_halt->vshader_done[i]);
	}
	
	if (PTHREAD_DEBUG) printf("host: all vshader_done signals are true\n");
	
	if (PTHREAD_DEBUG) printf("host: vshaders_run_req=false\n");
	run_halt->vshaders_run_req = false;
	
	////////////////////
	
	if (PTHREAD_DEBUG) printf("host: wait till all pshader_done signals are false\n");
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		while (run_halt->pshader_done[i]);
	}		
	if (PTHREAD_DEBUG) printf("host: all pshader_done signals are false\n");
	
	if (PTHREAD_DEBUG) {
		printf("host: pshaders_run_req=true\n");
	}
	run_halt->pshaders_run_req = true;
	
	if (PTHREAD_DEBUG) printf("host: wait till all pshader_done signals are true\n");
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		while (!run_halt->pshader_done[i]);
	}
	
	if (PTHREAD_DEBUG) printf("host: all pshader_done signals are true\n");
	
	if (PTHREAD_DEBUG) printf("host: pshaders_run_req=false\n");
	run_halt->pshaders_run_req = false;

#else

	vshader_loop (cfg, cfg->vshader_ptr, cfg->pshader_ptr, cfg->zbuffer_ptr, cfg->active_fbuffer);
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		pshader_loop (cfg, i);
	}	
	
#endif

}

#ifdef MULTIPROC

 int main (void) {
	volatile gpu_cfg_t      *volatile cfg      = (gpu_cfg_t *)      GPU_CFG_ABS_ADDRESS;
	volatile gpu_run_halt_t *volatile run_halt = (gpu_run_halt_t *) GPU_RUN_HALT_ABS_ADDRESS;

#else

 void * host_top (void *host_cfg) { 	
	
	volatile pthread_cfg_t  *volatile pthread_cfg = host_cfg;
    volatile gpu_cfg_t      *volatile cfg      = pthread_cfg->common_cfg;
    volatile gpu_run_halt_t *volatile run_halt = pthread_cfg->gpu_run_halt;
     
#endif

    
    
    
    
	cfg->num_of_ushaders = GPU_MAX_USHADERS;	
	cfg->num_of_tiles    = 0;
	cfg->num_of_fbuffers = GPU_MAX_FRAMEBUFFERS;
	
    
    size_t screen_size = WIDTH * HEIGHT;
    
    
    if ((cfg->zbuffer_ptr = (screenz_t *) calloc (screen_size, sizeof(screenz_t))) == NULL) {
		if (DEBUG_MALLOC) printf ("zbuffer calloc failed\n");
		goto error;
	}
    
    
    for (int i = 0; i < cfg->num_of_fbuffers; i++) {
		if ((cfg->fbuffer_ptr[i] = (pixel_color_t *) calloc (screen_size, sizeof(pixel_color_t))) == NULL) {
			if (DEBUG_MALLOC) printf ("fbuffer%d calloc failed\n", i);
			goto error;
		}
	}
	
	cfg->obj_list_ptr = init_objects ();
	
    // Projection Matrix - perspective correction
	float aspect_ratio = 16/9;
	float top   = 0.5;
	float bot   = -top;
	float right = top * aspect_ratio;
	float left  = -right;
	float near  = 1;
	float far   = 100;
	
	fmat4 persp_proj;
	init_perspective_proj (&persp_proj, left, right, top, bot, near, far);

	fmat4 ortho_proj;	
	float f = 18.5;
	init_ortho_proj       (&ortho_proj, left*f, right*f, top*f, bot*f, near, far);
	
	
	// Viewport Matrix - move to screen coords
	set_screen_size (cfg, (size_t) WIDTH, (size_t) HEIGHT);
    init_viewport   (&(cfg->viewport), 0, 0, get_screen_width(cfg), get_screen_height(cfg), get_screen_depth(cfg));
	
    
    // View Matrix - transform global coords to camera coords
	//Float3 eye       = Float3_set ( 3.0f,   2.0f,   5.0f);
    Float3 eye;
	Float3 center = Float3_set (-0.f,  -0.f,   0.0f);
	Float3 up     = Float3_set ( 0.0f,   1.0f,   0.0f);
	
	fmat4 view;	
	
    cfg->lights_arr[0] = light_turn_on (Float3_set ( 0.f,  -2.f, -10.f), false, cfg);
    for (int i = 1; i < GPU_MAX_LIGHTS; i++) {
		light_turn_off ((Light *) &(cfg->lights_arr[i]));
	}
    
    float eye_x = 0;
    float eye_y = 1;
    float eye_z = 0;
    float eye_angle = ROTATION_INIT; // rad
    float eye_distance = 20;
    
    float obj_angle = 0;
    
    
    
    
	if (ENABLE_PERF) {
		setup_counters();
		start_counters();
	}
    
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		if ((cfg->tri_ptr_list[i] = (volatile TrianglePShaderData *volatile *) calloc (cfg->num_of_tiles << GPU_MAX_TRIANGLES_PER_TILE_LOG2, sizeof(TrianglePShaderData *))) == NULL) {
			if (DEBUG_MALLOC) printf ("tile_idx_table calloc failed\n");
			goto error;
		}
	}
	

		
    run_halt->vshaders_stop_req = false;
    run_halt->pshaders_stop_req = false;
    for (int m = 0; m < NUM_OF_FRAMES; m++) {
		
		printf ("host: FRAME %d\n", m);
		pixel_color_t *active_fbuffer = cfg->fbuffer_ptr[m % cfg->num_of_fbuffers];
	
		
		// Clean up active framebuffer, zbuffer and all shadowbuffers
		for (int i = 0; i < screen_size; i++) {
			active_fbuffer[i] = set_color (1, 0, 0, 0);
			
			if (cfg->zbuffer_ptr != NULL) {
				screenz_t *zb = cfg->zbuffer_ptr;
				zb[i] = 0;
			}
			
			for (int j = 0; j < GPU_MAX_LIGHTS; j++) {
				if (cfg->lights_arr[j].enabled && cfg->lights_arr[j].has_shadow_buf) {
					cfg->lights_arr[j].shadow_buf[i] = 0;
				}
			}
		}
		
		
		// Number of faces may change from frame to frame if objects are getting added or removed,
		// so need to calculate it for every frame and allocate memory each time
		/*
		volatile ObjectListNode* volatile obj_list_node = cfg->obj_list_ptr;		
		int num_of_faces = 0;
		while (obj_list_node != NULL) {
			num_of_faces += obj_list_node->obj->wfobj->num_of_faces;
			obj_list_node = obj_list_node->next;
		}
		if ((cfg->tri_data_array = calloc (num_of_faces, sizeof (TrianglePShaderData))) == NULL) {
			if (DEBUG_MALLOC) printf ("tri_data_array calloc failed\n");
			goto error;
		}
		*/
		
		volatile ObjectListNode* volatile obj_list_node = cfg->obj_list_ptr;	
		uint32_t num_of_faces = 0;
		while (obj_list_node != NULL) {
			num_of_faces += obj_list_node->obj->wfobj->num_of_faces;
			obj_list_node = obj_list_node->next;
		}
		// + 1 needed because we do integer division and there may be remainder, we need space for it too.
		// For simplicity I simply enlarge all arrays, although some of them don't need this extra space.
		uint32_t num_of_faces_per_vshader = (num_of_faces / GPU_MAX_USHADERS) + 1;
		for (int i = 0; i < GPU_MAX_USHADERS; i++) {			
			if ((cfg->tri_for_pshader[i] = (TrianglePShaderData *) calloc (num_of_faces_per_vshader, sizeof (TrianglePShaderData))) == NULL) {
				if (DEBUG_MALLOC) printf ("tri_for_pshader[%d] calloc failed\n", i);
				goto error;
			}	
		}
			
		
		
		for (int i = 0; i < GPU_MAX_LIGHTS; i++) {
			if (cfg->lights_arr[i].enabled && cfg->lights_arr[i].has_shadow_buf) {
				
				init_view             (&view, &(cfg->lights_arr[i].src), &center, &up);
				setup_light_transform (cfg->obj_list_ptr, &ortho_proj, &view, i);
				launch_shaders (cfg, run_halt, vshader_fill_shadow_buf, pshader_fill_shadow_buf, cfg->lights_arr[i].shadow_buf, NULL);	
			}
		}			
		
		
		// move the camera
		eye_x = center.as_struct.x + eye_distance * cosf(eye_angle);
		eye_z = center.as_struct.z + eye_distance * sinf(eye_angle);
		eye    = Float3_set ( eye_x, eye_y, eye_z);
		eye_angle += ROTATION_INCR;
		
		// move the objects and recalculate their Model matrices
		volatile ObjectListNode* volatile scene_obj = cfg->obj_list_ptr;//obj_list_head;
		while (scene_obj != NULL) {	
			obj_set_rotation (scene_obj->obj, 0, obj_angle, 0);
			obj_init_model   (scene_obj->obj);			
			scene_obj = scene_obj->next;
		}
		obj_angle += 360 / NUM_OF_FRAMES;
		
		
		// 
		init_view            (&view, &eye, &center, &up);
		light_transform      (&view, cfg);
		setup_transformation (cfg->obj_list_ptr, &persp_proj, &view);
		
		
		launch_shaders (cfg, run_halt, vshader_gouraud, pshader_gouraud, NULL, active_fbuffer);
		//launch_shaders (cfg, run_halt, vshader_depth, pshader_depth, NULL, active_fbuffer);
		//launch_shaders (cfg, run_halt, vshader_phong, pshader_phong, NULL, active_fbuffer);
		
		for (int i = 0; i < GPU_MAX_USHADERS; i++) {
			free ((void*) cfg->tri_for_pshader[i]);
		}
		
		
		if (m == RECORD_FRAME_NUM) {
			printf ("recording frame number %d\n", m);
			if (ENABLE_PERF) stop_counters();
			
			char tga_file[32];
			char frame_num[32];
			char shadow_num[32];
			
			
			sprintf(frame_num, "%d", m);
			strcpy (tga_file, "frame_buffer_");
			strcat (tga_file, frame_num);
			strcat (tga_file, ".tga");
			
			
			tbyte *rgb24 = calloc (screen_size * 3, sizeof (tbyte));
			if (rgb24 == NULL) goto error;
			
			for (int i = 0; i < HEIGHT; i++) {
				for (int j = 0; j < WIDTH; j++) {
					size_t word_offset = (j + WIDTH * i);	
					size_t byte_offset = word_offset * 3;
					pixel_color_t *p = cfg->fbuffer_ptr[m];
					//pixel_color32_t pix;
					*(rgb24 + byte_offset + 0) = p[word_offset].as_byte.r;
					*(rgb24 + byte_offset + 1) = p[word_offset].as_byte.g;
					*(rgb24 + byte_offset + 2) = p[word_offset].as_byte.b;
				}
			}			
			//write_tga_file (tga_file, (tbyte *) cfg->fbuffer_ptr[m], WIDTH, HEIGHT, 24, 1);
			write_tga_file (tga_file, rgb24, WIDTH, HEIGHT, 24, 1);
			free (rgb24);
			
			tbyte *tmp;
			if ((tmp = (tbyte*) calloc (screen_size, sizeof(tbyte))) == NULL) {
				if (DEBUG_MALLOC) printf ("tga frame calloc failed\n");
				goto error;
			}
			if (cfg->zbuffer_ptr != NULL) {
				for (int i = 0; i < screen_size; i++) {
					screenz_t *zb = cfg->zbuffer_ptr;
					tmp[i] = zb[i] >> 4;// >> (8 * (sizeof(screenz_t) - 1) );
				}
				write_tga_file ("zbuffer.tga", tmp, WIDTH, HEIGHT, 8, 1);
			}
			for (int i = 0; i < GPU_MAX_LIGHTS; i++) {
				if (cfg->lights_arr[i].enabled && cfg->lights_arr[i].has_shadow_buf) {
					sprintf(shadow_num, "%d", i);
					strcpy (tga_file, "shadow_buffer_");
					strcat (tga_file, frame_num);
					strcat (tga_file, "_");
					strcat (tga_file, shadow_num);
					strcat (tga_file, ".tga");
					
					for (int j = 0; j < screen_size; j++) {
						tmp[j] = cfg->lights_arr[i].shadow_buf[j] >> (8 * (sizeof(screenz_t) - 1) );
					}
					write_tga_file (tga_file, tmp, WIDTH, HEIGHT, 8, 1);		
				}
			}		
			
			free (tmp);		
			
			if (ENABLE_PERF) start_counters();
		}
		
	}
	run_halt->vshaders_stop_req = true;
	run_halt->pshaders_stop_req = true;
	
	if (ENABLE_PERF) {
		stop_counters();
		read_counters();
	}
	
	if (RECORD_VIDEO) {		
		printf ("Recording video");
		FILE *fp = fopen ("video.y4m", "w");
		if (!fp) goto error;
		fprintf (fp, "YUV4MPEG2 W%d H%d F25:1 Ip A0:0 C444\n", WIDTH, HEIGHT);
		
		for (int i = 0; i < NUM_OF_FRAMES; i++) {
			printf (".");
			fprintf (fp, "FRAME\n");
			pixel_color_t *fb = cfg->fbuffer_ptr[i];
			for (int j = 0; j < screen_size; j++){
				uint8_t y = rgb_to_y (fb[j]);
				fwrite (&y, sizeof (uint8_t), 1, fp);
			}
			for (int j = 0; j < screen_size; j++){
				uint8_t cb = rgb_to_cb (fb[j]);
				fwrite (&cb, sizeof (uint8_t), 1, fp);
			}
			for (int j = 0; j < screen_size; j++){
				uint8_t cr = rgb_to_cr (fb[j]);
				fwrite (&cr, sizeof (uint8_t), 1, fp);
			}
		}
		printf ("\n");
		fclose (fp);
	}
	
	if (cfg->zbuffer_ptr != NULL) {
		free ((void *) cfg->zbuffer_ptr);
	}
	
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		free ((void*) cfg->tri_ptr_list[i]);
	}
		
	free_objects ((void*) cfg->obj_list_ptr);
	
	for (int i = 0; i < cfg->num_of_fbuffers; i++) {
		if (cfg->fbuffer_ptr[i] != NULL) {
			free ((void *) cfg->fbuffer_ptr[i]);
		}
	}
	
	for (int i = 0; i < GPU_MAX_LIGHTS; i++) {
		light_turn_off ((Light *) &(cfg->lights_arr[i]));
	}
	
#ifndef MULTIPROC
    return NULL;
#else
	return 0;
#endif

error:

#ifndef MULTIPROC
    return NULL;
#else
	return 1;
#endif
	
}
