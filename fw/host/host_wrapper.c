#include "host_wrapper.h"

#include <wavefront_obj.h>
#include <geometry.h>

#ifdef SINGLEPROC_SINGLETHREAD
	#include <vshader.h>
	#include <pshader.h>
#endif

//#include "shader_normalmap.h"
#include "shader_phong.h"
#include "shader_gouraud.h"
#include "shader_depth.h"
#include "shader_fill_shadow_buf.h"
#include "bitmap.h"
#include "tga_addon.h"


#include "profiling.h"


//#include <platform.h>



#include <stdint.h>
//#include <limits.h>
#include <stdlib.h>
//#include <math.h>

#include <time.h>

// POSITIVE Z TOWARDS ME

//Light LIGHTS[MAX_NUM_OF_LIGHTS];




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

void launch_shaders (volatile gpu_cfg_t* cfg, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer);



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
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		Light *l = cfg->lights_table_ptr;
		if (!l[i].enabled) continue;
		light4_a = Float3_Float4_conv (&(l[i].dir), 0);
		// Light vector changes after View transformation only,
		// it does not depend on Model and Projection transformations
		light4_b = fmat4_Float4_mult  (view, &light4_a);
		l[i].eye = Float4_Float3_vect_conv (&light4_b);
		Float3_normalize (&(l[i].eye));
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
		african_head_nmap = new_bitmap_from_tga ("../../models/african_head_nm.tga");
		african_head_spec = new_bitmap_from_tga ("../../models/african_head_spec.tga");
		african_head = wfobj_new ("../../models/african_head.obj");

	
		if ((node = calloc (1, sizeof (ObjectListNode))) == NULL) {
			if (DEBUG_MALLOC) printf ("obj list node calloc failed\n");
			return NULL;
		}
		head = node;
		//node->obj = obj_new (african_head, african_head_diff, african_head_nmap, african_head_spec);
		//node->obj = obj_new (african_head, african_head_diff, NULL, NULL);
		node->obj = obj_new (african_head, NULL, NULL, NULL);
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
			//node->obj  = obj_new (african_head, african_head_diff, african_head_nmap, african_head_spec);
			node->obj = obj_new (african_head, NULL, NULL, NULL);
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
	return (uint8_t) (16.f + rgb.r * 0.257f + rgb.g * 0.504f + rgb.b * 0.098f);
}

uint8_t rgb_to_cb (pixel_color_t rgb) {
	return (uint8_t) (128.f - rgb.r * 0.148f - rgb.g * 0.291f + rgb.b * 0.439f);
}

uint8_t rgb_to_cr (pixel_color_t rgb) {
	return (uint8_t) (128.f + rgb.r * 0.439f - rgb.g * 0.368f - rgb.b * 0.071f);
}

/*
void save_host_results (volatile gpu_cfg_t *cfg) {
	cfg->obj_list_ptr;
	//Float3 model = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
}
*/

void launch_shaders (volatile gpu_cfg_t* cfg, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer) {
	
	cfg->vshader_ptr = vshader;
	cfg->pshader_ptr = pshader;
	cfg->zbuffer_ptr = zbuffer;
	cfg->active_fbuffer = fbuffer;
			
#ifndef SINGLEPROC_SINGLETHREAD

	if (PTHREAD_DEBUG) printf("host: wait till all vshader_done signals are false\n");
	for (int i = 0; i < cfg->num_of_vshaders; i++) {
		while (cfg->vshader_done[i]);
	}
	if (PTHREAD_DEBUG) printf("host: all vshader_done signals are false\n");
	
	if (PTHREAD_DEBUG) {
		printf("host: vshaders_run_req=true\n");
	}
	cfg->vshaders_run_req = true;
	
		
	if (PTHREAD_DEBUG) printf("host: wait till all vshader_done signals are true\n");
	for (int i = 0; i < cfg->num_of_vshaders; i++) {
		while (!cfg->vshader_done[i]);
	}
	
	if (PTHREAD_DEBUG) printf("host: all vshader_done signals are true\n");
	
	if (PTHREAD_DEBUG) printf("host: vshaders_run_req=false\n");
	cfg->vshaders_run_req = false;
	
	////////////////////
	
	if (PTHREAD_DEBUG) printf("host: wait till all pshader_done signals are false\n");
	for (int i = 0; i < cfg->num_of_pshaders; i++) {
		while (cfg->pshader_done[i]);
	}		
	if (PTHREAD_DEBUG) printf("host: all pshader_done signals are false\n");
	
	if (PTHREAD_DEBUG) {
		printf("host: pshaders_run_req=true\n");
	}
	cfg->pshaders_run_req = true;
	
	if (PTHREAD_DEBUG) printf("host: wait till all pshader_done signals are true\n");
	for (int i = 0; i < cfg->num_of_pshaders; i++) {
		while (!cfg->pshader_done[i]);
	}
	
	if (PTHREAD_DEBUG) printf("host: all pshader_done signals are true\n");
	
	if (PTHREAD_DEBUG) printf("host: pshaders_run_req=false\n");
	cfg->pshaders_run_req = false;

#else

	vshader_loop (cfg, cfg->vshader_ptr, cfg->pshader_ptr, cfg->zbuffer_ptr, cfg->active_fbuffer);
	//shader_cfg_t pshader_cfg[NUM_OF_PSHADERS];
	for (int i = 0; i < NUM_OF_PSHADERS; i++) {
		//pshader_cfg[i].common_cfg       = cfg;
		//pshader_cfg[i].shader_num       = i;
		//pshader_loop (&(pshader_cfg[i]));
		pshader_loop (cfg, i);
	}	
	
#endif

}

#ifndef MULTIPROC
 void * host_wrapper (void *gpu_cfg) {
#else
 int main (void) {
	volatile gpu_cfg_t * volatile gpu_cfg = (volatile gpu_cfg_t *) GPU_CFG_ABS_ADDRESS;
#endif

    volatile gpu_cfg_t * volatile cfg = gpu_cfg;
    
    
    /*
    cfg->tile_idx_table_ptr = NULL;
	
	for (int i = 0; i < MAX_NUM_OF_FRAMEBUFFERS; i++) {
		cfg->fbuffer_ptr[i] = NULL;
	}
	
	cfg->zbuffer_ptr = NULL;
	
	cfg->lights_table_ptr = NULL;
	cfg->viewport_ptr = NULL;
	
	cfg->vshader_ptr = NULL;
	cfg->pshader_ptr = NULL;
		
	cfg->vshaders_run_req  = false;
	cfg->vshaders_stop_req = false;
	
	cfg->pshaders_run_req  = false;
	cfg->pshaders_stop_req = false;
	
	
	for (int i = 0; i < NUM_OF_VSHADERS; i++) {
		cfg->vshader_done[i] = false;
	}
	for (int i = 0; i < NUM_OF_PSHADERS; i++) {
		cfg->pshader_done[i] = false;
	}
	*/
	cfg->num_of_vshaders = NUM_OF_VSHADERS;
	cfg->num_of_pshaders = NUM_OF_PSHADERS;
	cfg->num_of_ushaders = NUM_OF_USHADERS;	
	cfg->num_of_tiles    = 0;
	cfg->num_of_fbuffers = MAX_NUM_OF_FRAMEBUFFERS;
	
    
    size_t screen_size = WIDTH * HEIGHT;
    
    /*
    if ((cfg->zbuffer_ptr = calloc (screen_size, sizeof(screenz_t))) == NULL) {
		if (DEBUG_MALLOC) printf ("zbuffer calloc failed\n");
		return NULL;
	}
    */
    
    
    for (int i = 0; i < cfg->num_of_fbuffers; i++) {
		if ((cfg->fbuffer_ptr[i] = calloc (screen_size, sizeof(pixel_color_t))) == NULL) {
			if (DEBUG_MALLOC) printf ("fbuffer%d calloc failed\n", i);
			goto error;
		}
	}
	
	//Light LIGHTS[MAX_NUM_OF_LIGHTS];
	if ((cfg->lights_table_ptr = calloc (MAX_NUM_OF_LIGHTS, sizeof(Light))) == NULL) {
		if (DEBUG_MALLOC) printf ("light table calloc failed\n");
		goto error;
	}
    
    if ((cfg->viewport_ptr = calloc (1, sizeof(fmat4))) == NULL) {
		if (DEBUG_MALLOC) printf ("viewport calloc failed\n");
		goto error;
	}	
	
    //pixel_color_t *active_fbuffer = NULL;
    //cfg->active_fbuffer = NULL;
    		
	//ObjectListNode *obj_list_head = init_objects ();
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
    init_viewport   (cfg, 0, 0, get_screen_width(cfg), get_screen_height(cfg), get_screen_depth(cfg));
	
    
    // View Matrix - transform global coords to camera coords
	//Float3 eye       = Float3_set ( 3.0f,   2.0f,   5.0f);
    Float3 eye;
	Float3 center = Float3_set (-0.f,  -0.f,   0.0f);
	Float3 up     = Float3_set ( 0.0f,   1.0f,   0.0f);
	
	
	fmat4 view;	
	//init_view (&view, &eye, &center, &up);
    
    init_lights(cfg);
    //new_light (0, Float3_set ( 0.f,  -2.f, -10.f), false);	
    
    screenz_t* shadow_buf_0 = calloc (get_screen_width(cfg)*get_screen_height(cfg), sizeof(screenz_t));
    if (!shadow_buf_0) goto error;
    //new_light (0, Float3_set ( 0.f,  -2.f, -10.f), shadow_buf_0, cfg);	
    new_light (0, Float3_set ( 0.f,  -2.f, -10.f), NULL, cfg);	
    
    
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
    
    //////////////////
	/*if ((cfg->tile_idx_table_ptr = calloc (cfg->num_of_tiles, sizeof(TriangleListNode*))) == NULL) {
		if (DEBUG_MALLOC) printf ("tile_idx_table calloc failed\n");
		goto error;
	}*/
	if ((cfg->tile_idx_table_ptr = calloc (cfg->num_of_tiles, sizeof(TrianglePShaderData*) * 2000)) == NULL) {
		if (DEBUG_MALLOC) printf ("tile_idx_table calloc failed\n");
		goto error;
	}
		
    cfg->vshaders_stop_req = false;
    cfg->pshaders_stop_req = false;
    for (int m = 0; m < NUM_OF_FRAMES; m++) {
		
		printf ("host: FRAME %d\n", m);
		pixel_color_t *active_fbuffer = cfg->fbuffer_ptr[m % cfg->num_of_fbuffers];
	
		
		// Clean up active framebuffer, zbuffer and all shadowbuffers
		for (int i = 0; i < screen_size; i++) {
			//((pixel_color_t*) cfg->active_fbuffer)[i] = set_color (0, 0, 0, 0);
			active_fbuffer[i] = set_color (1, 0, 0, 0);
			
			if (cfg->zbuffer_ptr != NULL) {
				screenz_t *zb = cfg->zbuffer_ptr;
				zb[i] = 0;
			}
			
			for (int j = 0; j < MAX_NUM_OF_LIGHTS; j++) {
				Light *l = cfg->lights_table_ptr;
				if (l[j].enabled && l[j].has_shadow_buf) {
					l[j].shadow_buf[i] = 0;
				}
			}
		}
		
		// TEST
		for (int i = 0; i < cfg->num_of_tiles * 2000; i++) {
			TrianglePShaderData **d = cfg->tile_idx_table_ptr;
			d[i] = NULL;
		}
		

		// Number of faces may change from frame to frame if objects are getting added or removed,
		// so need to calculate it for every frame and allocate memory each time
		volatile ObjectListNode* volatile obj_list_node = cfg->obj_list_ptr;		
		int num_of_faces = 0;
		while (obj_list_node != NULL) {
			num_of_faces += wfobj_get_num_of_faces(obj_list_node->obj->wfobj);
			obj_list_node = obj_list_node->next;
		}
		if ((cfg->tri_data_array = calloc (num_of_faces, sizeof (TrianglePShaderData))) == NULL) {
			if (DEBUG_MALLOC) printf ("tri_data_array calloc failed\n");
			goto error;
		}
			
		
		/*
		for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
			Light *l = cfg->lights_table_ptr;
			if (l[i].enabled && l[i].has_shadow_buf) {
				
				init_view             (&view, &(l[i].src), &center, &up);
				setup_light_transform (cfg->obj_list_ptr, &ortho_proj, &view, i);
				launch_shaders (cfg, vshader_fill_shadow_buf, pshader_fill_shadow_buf, l[i].shadow_buf, NULL);	
			}
		}			
		*/
		
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
		
		
		launch_shaders (cfg, vshader_gouraud, pshader_gouraud, NULL, active_fbuffer);
		//launch_shaders (cfg, vshader_depth, pshader_depth, NULL, active_fbuffer);
		//launch_shaders (cfg, vshader_phong, pshader_phong, NULL, active_fbuffer);
		
		/*
		for (int i = 0; i < cfg->num_of_tiles; i++) {
			volatile TriangleListNode* volatile* volatile tln = cfg->tile_idx_table_ptr;
			
			//TriangleListNode *node = (*cfg->tile_idx_table_ptr)[i];
			volatile TriangleListNode* volatile node = tln[i];
			TriangleListNode *tmp;
			while (node != NULL) {
				tmp = node->next;
				free ((void*) node);
				node = tmp;
			}
			tln[i] = NULL;		
		}
		*/	
		
		
		free ((void*) cfg->tri_data_array);
		
		
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
			write_tga_file (tga_file, (tbyte *) cfg->fbuffer_ptr[m], WIDTH, HEIGHT, 24, 1);
			
			
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
			for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
				Light *l = cfg->lights_table_ptr;
				if (l[i].enabled && l[i].has_shadow_buf) {
					sprintf(shadow_num, "%d", i);
					strcpy (tga_file, "shadow_buffer_");
					strcat (tga_file, frame_num);
					strcat (tga_file, "_");
					strcat (tga_file, shadow_num);
					strcat (tga_file, ".tga");
					
					for (int j = 0; j < screen_size; j++) {
						tmp[j] = l[i].shadow_buf[j] >> (8 * (sizeof(screenz_t) - 1) );
					}
					write_tga_file (tga_file, tmp, WIDTH, HEIGHT, 8, 1);		
				}
			}		
			
			free (tmp);		
			
			if (ENABLE_PERF) start_counters();
		}
		
	}
	cfg->vshaders_stop_req = true;
	cfg->pshaders_stop_req = true;
	
	if (ENABLE_PERF) {
		stop_counters();
		read_counters();
	}
	
	if (RECORD_VIDEO) {		
		FILE *fp = fopen ("video.y4m", "w");
		if (!fp) goto error;
		fprintf (fp, "YUV4MPEG2 W%d H%d F25:1 Ip A0:0 C444\n", WIDTH, HEIGHT);
		
		for (int i = 0; i < NUM_OF_FRAMES; i++) {
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
		fclose (fp);
	}
	
	if (cfg->zbuffer_ptr != NULL) {
		free (cfg->zbuffer_ptr);
	}
	
	if (cfg->viewport_ptr != NULL) {
		free (cfg->viewport_ptr);
	}
	
	free ((void*) cfg->tile_idx_table_ptr);
	
	free_objects ((void*) cfg->obj_list_ptr);
	
	for (int i = 0; i < cfg->num_of_fbuffers; i++) {
		if (cfg->fbuffer_ptr[i] != NULL) {
			free (cfg->fbuffer_ptr[i]);
		}
	}
	
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		free_light (i, cfg);
	}
	if (cfg->lights_table_ptr != NULL) {
		free (cfg->lights_table_ptr);
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
