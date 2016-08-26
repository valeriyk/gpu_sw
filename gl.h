#pragma once

#include "geometry.h"
#include "wavefront_obj.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define FIX_PT_PRECISION	3
#define MAX_NUM_OF_LIGHTS	4

typedef enum {X = 0, Y, Z, W} axis;


// If we clip only those triangles which are completely invisible, screenxy_t must be signed
// If we clip all the trianlges which are not completely visible, screenxy_t can be unsigned
typedef uint16_t screenxy_t;

typedef uint16_t screenz_t;

typedef struct ScreenPt {
	screenxy_t x;
	screenxy_t y;
	screenz_t  z;
} ScreenPt;

typedef struct pixel_color_t {
	uint8_t r, g, b;
	//uint8_t r, g, b, a;
	//uint8_t b, g, r, a;
} pixel_color_t;

typedef struct Triangle {
	Float4 vtx[3];
} Triangle;

typedef struct Object {
	WFobj *wfobj;
	float3 scale;
	float3 rotate;
	float3 tran;
	fmat4  model;
	fmat4  mvp; // pre-multiplied ModelViewProjection matrix
	fmat4  shadow_mvp[MAX_NUM_OF_LIGHTS];
} Object;


typedef Float4 (*vertex_shader) (Object *obj, int face_idx, int vtx_idx);
typedef bool   (*pixel_shader)  (WFobj *obj, Float3 *barc, pixel_color_t *color);

typedef struct Light {
	bool 		enabled;
	Float3		dir;
	Float3		src;
	Float3		eye;
	screenz_t	*shadow_buf;
} Light;

extern Light LIGHTS[MAX_NUM_OF_LIGHTS];

void new_light  (int light_num, Float3 dir);
void free_light (int light_num);
void init_scene (void);


Light light[MAX_NUM_OF_LIGHTS];


void create_light (Float3 dir, screenz_t *shadow_buf, int light_num);

void       set_screen_size   (screenxy_t width, screenxy_t height);
screenxy_t get_screen_width  (void);
screenxy_t get_screen_height (void);
screenz_t  get_screen_depth  (void);

int   orient2d (ScreenPt *a, ScreenPt *b, ScreenPt *c);

//void  world2screen (float4 &w, ScreenPt &s);

screenxy_t tri_min_bound (screenxy_t a, screenxy_t b, screenxy_t c, screenxy_t cutoff);
screenxy_t tri_max_bound (screenxy_t a, screenxy_t b, screenxy_t c, screenxy_t cutoff);

void init_view             (fmat4 *m, Float3 *eye, Float3 *center, Float3 *up);
void init_perspective_proj (fmat4 *m, float left, float right, float top, float bot, float near, float far);
void init_ortho_proj       (fmat4 *m, float left, float right, float top, float bot, float near, float far);
//void init_viewport   (fmat4 *m, int x, int y, int w, int h, int d);

void rotate_coords (fmat4 *in, fmat4 *out, float alpha_deg, axis axis);

void draw_triangle (Triangle *t, pixel_shader shader, screenz_t *zbuffer, pixel_color_t *fbuffer, WFobj *obj);

pixel_color_t set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a);

//void write_fbuffer (pixel_color_t *fbuffer, int x, int y, pixel_color_t *val);
//void write_zbuffer (screenz_t     *zbuffer, int x, int y, screenz_t     *val);



Object* obj_new (WFobj *wfobj);
void obj_set_scale       (Object *obj, float x,     float y,     float z);
void obj_set_rotation    (Object *obj, float x_deg, float y_deg, float z_deg);
void obj_set_translation (Object *obj, float x,     float y,     float z);
void obj_init_model      (Object *obj);
//void obj_transform       (Object *obj, fmat4 *vpv, fmat4 *projview, float3 *light_dir);
void obj_draw            (Object *obj, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer);
