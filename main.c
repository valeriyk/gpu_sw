#include "main.h"
#include "wavefront_obj.h"
#include "geometry.h"
#include "gl.h"
//#include "shader_normalmap.h"
//#include "shader_phong.h"
#include "shader_depth.h"
#include "bitmap.h"
#include "tga_addon.h"

#include <stdint.h>
//#include <limits.h>
#include <stdlib.h>
#include <math.h>

// POSITIVE Z TOWARDS ME

// these are global:
//fmat4  UNIFORM_M;
//fmat4  UNIFORM_MIT;

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

// Setup object transformation, aka world transformation function.  
// Computes matrices that will be used to transform model's vertices and normals
// from the object space to the world space
void setup_transformation (ObjectNode *obj_list_head, fmat4 *proj, fmat4 *view) {
	
	ObjectNode *node = obj_list_head;
	
	while (node != NULL) {

		fmat4 modelview;
		fmat4_fmat4_mult ( view, &(node->obj->model), &modelview);
		fmat4_fmat4_mult ( proj, &modelview, &(node->obj->mvp));
		fmat4_inv_transp (&modelview, &(node->obj->mit));
		/*print_fmat4 (&(node->obj->mit), "modelview invert transpose (mit) BEFORE");
		float min = 0;
		float max = 0;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				float tmp = node->obj->mit[i][j];
				if (tmp > max) max = tmp;
				if (tmp < min) min = tmp;
			}
		}
		float div = 0;
		if (fabsf(max) > fabsf(min)) div = max;
		else div = min;
		
		if (div != 0) {
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					fmat4_set (&(node->obj->mit), i, j, node->obj->mit[i][j] / div);
				}
			}
		}
		print_fmat4 (&(node->obj->mit), "modelview invert transpose (mit) AFTER");*/
		
		//fmat4_copy (&(node->obj->mvp), &(node->obj->shadow_mvp[light_num]));	
		/*print_fmat4 (&(obj->model), "model matrix");
		print_fmat4 (view, "view matrix");
		print_fmat4 (proj, "projection matrix");
		print_fmat4 (&UNIFORM_M, "UNIFORM_M");
		print_fmat4 (&UNIFORM_MIT, "UNIFORM_MIT");
		*/
		
		node = node->next;
	}
}

void setup_light_transform (ObjectNode *obj_list_head, fmat4 *proj, fmat4 *view, int light_num) {
	
	ObjectNode *node = obj_list_head;
	
	while (node != NULL) {
		
		fmat4 modelview;
		fmat4_fmat4_mult ( view, &(node->obj->model), &modelview);
		fmat4_fmat4_mult ( proj, &modelview, &(node->obj->mvp));
		//fmat4_inv_transp (&UNIFORM_M, &UNIFORM_MIT);

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

ObjectNode* init_objects (void) {
	
	Bitmap *african_head_diff = new_bitmap_from_tga ("obj/african_head_diffuse.tga");
    Bitmap *african_head_nmap = new_bitmap_from_tga ("obj/african_head_nm.tga");
    Bitmap *african_head_spec = new_bitmap_from_tga ("obj/african_head_spec.tga");
    WFobj  *african_head = wfobj_new ("obj/african_head.obj", african_head_diff, african_head_nmap, african_head_spec);
	/*
	Bitmap *diablo_diff = new_bitmap_from_tga ("obj/diablo/diablo3_pose_diffuse.tga");
	Bitmap *diablo_nmap = new_bitmap_from_tga ("obj/diablo/diablo3_pose_nm.tga");
	Bitmap *diablo_spec = new_bitmap_from_tga ("obj/diablo/diablo3_pose_spec.tga");
	WFobj  *diablo = wfobj_new ("obj/diablo/diablo3_pose.obj", diablo_diff, diablo_nmap, diablo_spec);
	*/
	
	
	Bitmap *cube_diff = new_bitmap_from_tga ("obj/floor_diffuse.tga");
	Bitmap *cube_nmap = new_bitmap_from_tga ("obj/floor_nm_tangent.tga");
	WFobj  *my_cube = wfobj_new ("obj/cube2.obj", cube_diff, cube_nmap, NULL);
	
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
    
    ObjectNode *head;
    ObjectNode *node;
    
    int draw_head = 1;
    
    if (draw_head) {
		node = calloc (1, sizeof (ObjectNode));
		head = node;
		node->obj = obj_new (african_head);
		node->next = NULL;
		obj_set_scale       (node->obj, 7, 7, 7);
		//obj_set_rotation    (node->obj, 45, 45, 0);
		//obj_set_translation (node->obj, -1.0f, 0.f, 7.f);
		obj_init_model      (node->obj);
		}
	else {
		// Central cube
		node = calloc (1, sizeof (ObjectNode));
		
		head = node;
		
		node->obj  = obj_new (my_cube);
		node->next = NULL;
		obj_set_scale       (node->obj, 10, 10, 10);
		obj_set_rotation    (node->obj, 0.f, 0.f, 0.f);
		obj_set_translation (node->obj, 0.f, 0.f, 0.f);
		obj_init_model      (node->obj);		
	}
	/*
	// Bottom Plane
	node->next = calloc (1, sizeof (ObjectNode));
	node       = node->next;
	node->obj  = obj_new (my_floor);
	node->next = NULL;
    //obj_set_rotation    (object[obj_idx], 90.f, 0.f, 0.f);
    obj_set_translation (node->obj, 0.f, 0.f, 5.0f);
    obj_set_scale       (node->obj, 6, 4, 4);
	obj_init_model      (node->obj);
	
	node->next = calloc (1, sizeof (ObjectNode));
	node       = node->next;
	node->obj  = obj_new (my_floor);
	node->next = NULL;
    obj_set_rotation    (node->obj, 0.f, 0.f, 180.f);
    obj_set_translation (node->obj, 0.f, -6.01f, 5.0f);
    obj_set_scale       (node->obj, 6, 4, 4);
	obj_init_model      (node->obj);
	
	
	// Cube
	node->next = calloc (1, sizeof (ObjectNode));
	node = node->next;
	node->obj = obj_new (my_cube);
	node->next = NULL;
	obj_set_scale       (node->obj, 2, 2, 2);
	obj_set_rotation    (node->obj, 45, 45, 0);
	obj_set_translation (node->obj, 1.0f, 0.f, 7.f);
	obj_init_model      (node->obj);
	
	// Cube
	node->next = calloc (1, sizeof (ObjectNode));
	node = node->next;
	node->obj = obj_new (my_cube);
	node->next = NULL;
	obj_set_scale       (node->obj, 2, 2, 2);
	obj_set_rotation    (node->obj, 45, 45, 0);
	obj_set_translation (node->obj, -1.0f, 0.f, 7.f);
	obj_init_model      (node->obj);
	
	
	*/
	
	return head;
}

/*
typedef struct color_ycbcr_t {
	uint8_t y, cb, cr;
} color_ycbcr_t;
	
color_ycbcr_t rgb_to_ycbcr (pixel_color_t rgb) {
	color_ycbcr_t ycbcr;
	ycbcr.y  =  16 + rgb.r * 0.257 + rgb.g * 0.504 + rgb.b * 0.098;
	ycbcr.cb = 128 - rgb.r * 0.148 - rgb.g * 0.291 + rgb.b * 0.439;
	ycbcr.cr = 128 + rgb.r * 0.439 - rgb.g * 0.368 - rgb.b * 0.071;
	return ycbcr;
}
*/
uint8_t rgb_to_y (pixel_color_t rgb) {
	return 16 + rgb.r * 0.257 + rgb.g * 0.504 + rgb.b * 0.098;
}

uint8_t rgb_to_cb (pixel_color_t rgb) {
	return 128 - rgb.r * 0.148 - rgb.g * 0.291 + rgb.b * 0.439;
}

uint8_t rgb_to_cr (pixel_color_t rgb) {
	return 128 + rgb.r * 0.439 - rgb.g * 0.368 - rgb.b * 0.071;
}

int main(int argc, char** argv) {
       
    init_scene();
    
    size_t screen_size = WIDTH * HEIGHT;
    
    screenz_t *zbuffer = (screenz_t*) calloc (screen_size, sizeof(screenz_t));
    
    pixel_color_t *fbuffer[NUM_OF_FRAMEBUFFERS];
    for (int i = 0; i < NUM_OF_FRAMEBUFFERS; i++) {
		fbuffer[i] = (pixel_color_t*) calloc (screen_size, sizeof(pixel_color_t));
	}
    	
    pixel_color_t *active_fbuffer = NULL;
    
	FILE *fp = fopen ("video.y4m", "w");
    if (!fp) return 1;
    fprintf (fp, "YUV4MPEG2 W%d H%d F25:1 Ip A0:0 C444\n", WIDTH, HEIGHT);
    
		
	ObjectNode *obj_list_head = init_objects ();
	
    // 3. Projection Matrix - perspective correction
	float aspect_ratio = 16/9;
	float top   = 0.5;
	float bot   = -top;
	float right = top * aspect_ratio;
	float left  = -right;
	float near  = 1;
	float far   = 50;
	
	fmat4 persp_proj  = FMAT4_IDENTITY;
	fmat4 ortho_proj  = FMAT4_IDENTITY;
	init_perspective_proj (&persp_proj, left, right, top, bot, near, far);
	float f = 20.0;
	init_ortho_proj       (&ortho_proj, left*f, right*f, top*f, bot*f, near, far);
	
	// 4. Viewport Matrix - move to screen coords
	//fmat4 viewport = FMAT4_IDENTITY;
	set_screen_size ((screenxy_t) WIDTH, (screenxy_t) HEIGHT);
    init_viewport (0, 0, get_screen_width(), get_screen_height(), get_screen_depth());
	
    
    // 2. View Matrix - transform global coords to camera coords
	//Float3 eye       = Float3_set ( 3.0f,   2.0f,   5.0f);
    Float3 eye;
	Float3 center = Float3_set (-0.f,  -0.f,   0.0f);
	Float3 up     = Float3_set ( 0.0f,   1.0f,   0.0f);
	fmat4 view    = FMAT4_IDENTITY;	
	//init_view (&view, &eye, &center, &up);
    
    new_light (0, Float3_set ( 0.f,  3.f, -10.f));					
    
    
    
    float eye_x = 0;
    float eye_y = 1;
    float eye_z = 0;
    float eye_angle = 0;
    float eye_distance = 15;
    
    printf ("Frame");
    //do {
    for (int m = 0; m < NUM_OF_FRAMES; m++) {
		printf (" %d", m);
		active_fbuffer = (active_fbuffer == fbuffer[0]) ? fbuffer[1] : fbuffer[0];
		
		// clean up active framebuffer, zbuffer and all shadowbuffers
		for (int i = 0; i < screen_size; i++) {
			active_fbuffer[i].r = 0;
			active_fbuffer[i].g = 0;
			active_fbuffer[i].b = 0;
			zbuffer[i] = fix16_minimum;
			for (int j = 0; j < MAX_NUM_OF_LIGHTS; j++) {
				if (LIGHTS[j].enabled) {
					LIGHTS[j].shadow_buf[i] = 0;
				}
			}
		}
		
		for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
			if (!LIGHTS[i].enabled) continue;
			init_view             (&view, &(LIGHTS[i].src), &center, &up);
			setup_light_transform (obj_list_head, &ortho_proj, &view, i);
			draw_frame            (obj_list_head, depth_vshader_pass1, depth_pshader_pass1, LIGHTS[i].shadow_buf, NULL);	
		}			
		
		// move the camera
		eye_x = center.as_struct.x + eye_distance * cosf(eye_angle);
		eye_z = center.as_struct.z + eye_distance * sinf(eye_angle);
		eye    = Float3_set ( eye_x, eye_y, eye_z);
		eye_angle += 3.141592f / NUM_OF_FRAMES; // 180 degree swing in radians 
				
		
		init_view            (&view, &eye, &center, &up);
		light_transform      (&view);
		setup_transformation (obj_list_head, &persp_proj, &view);
		draw_frame           (obj_list_head, depth_vshader_pass2, depth_pshader_pass2, zbuffer, active_fbuffer);
			
		fprintf (fp, "FRAME\n");
		for (int j = 0; j < screen_size; j++){
			uint8_t y = rgb_to_y (active_fbuffer[j]);
			fwrite (&y, sizeof (uint8_t), 1, fp);
		}
		for (int j = 0; j < screen_size; j++){
			uint8_t cb = rgb_to_cb (active_fbuffer[j]);
			fwrite (&cb, sizeof (uint8_t), 1, fp);
		}
		for (int j = 0; j < screen_size; j++){
			uint8_t cr = rgb_to_cr (active_fbuffer[j]);
			fwrite (&cr, sizeof (uint8_t), 1, fp);
		}
		
		if (m == NUM_OF_FRAMES / 3) {
			write_tga_file ("framebuffer_0.tga", (tbyte *) fbuffer[0], WIDTH, HEIGHT, 24, 1);
			//write_tga_file ("output_fb1.tga", (tbyte *) fbuffer1, WIDTH, HEIGHT, 24, 1);
			if (sizeof(screenz_t) == 1) {
				write_tga_file ("zbuffer.tga", (tbyte *)  zbuffer, WIDTH, HEIGHT, 8, 1);
				
				for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
					if (LIGHTS[i].enabled) {
						char sb_file[32];
						char num[32];
						sprintf(num, "%d", i);
						strcpy (sb_file, "shadow_buffer_");
						strcat (sb_file, num);
						strcat (sb_file, ".tga");
						write_tga_file (sb_file, (tbyte *) LIGHTS[i].shadow_buf, WIDTH, HEIGHT, 8, 1);
					}
				}
			}
		}
	}
	// while (0);
	
	
    
	
	
	fclose (fp);
	
	//wfobj_free(african_head);
	//wfobj_free(my_floor);
	
	free(zbuffer);
	for (int i = 0; i < NUM_OF_FRAMEBUFFERS; i++) {
		free(fbuffer[i]);
	}
	
    return 0;
}
