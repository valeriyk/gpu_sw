#pragma once

#include "geometry.h"
//#include "tgaimage.h"
#include "wavefront_obj.h"

#include <stdint.h>
#include <stdbool.h>

typedef enum {X = 0, Y, Z, W} axis;


typedef int16_t screenxy_t;
typedef int32_t screenz_t;
typedef struct ScreenPt {
	screenxy_t x;
	screenxy_t y;
	screenz_t  z;
} ScreenPt;

typedef struct pixel_color_t {
	uint8_t r, g, b;//, a;
	//uint8_t b, g, r, a;
} pixel_color_t;

typedef struct Point2D {
    int x, y;
} Point2D;

/*typedef struct Point2Df {
    float u, v;
} Point2Df;
*/

typedef struct Face {
	int3 vtx_idx; // vertex indices
	int3 txt_idx; // texture indices
} Face;

typedef struct Vertex {
	float3 norm;
	float2 text;
	ScreenPt coords;
} Vertex;

typedef struct ScreenTriangle {
	float4 vtx_coords[3];
} ScreenTriangle;

typedef struct Triangle {
	int3 cx;
	int3 cy;
	int3 cz;
	float4 cw;
} Triangle;


typedef void (*vertex_shader) (WFobj *obj, int face_idx, int vtx_idx, fmat4 *mvpv, float4 *vtx4d);
typedef bool (*pixel_shader)  (WFobj *obj, float3 *barc, pixel_color_t *color);

int   orient2d (ScreenPt *a, ScreenPt *b, ScreenPt *c);

//void  world2screen (float4 &w, ScreenPt &s);

screenxy_t tri_min_bound (screenxy_t a, screenxy_t b, screenxy_t c, screenxy_t cutoff);
screenxy_t tri_max_bound (screenxy_t a, screenxy_t b, screenxy_t c, screenxy_t cutoff);

void init_model      (fmat4 *m, float3 *scale, float3 *rotate, float3 *tran);
void init_view       (fmat4 *m, float3 *eye,   float3 *center, float3 *up);
void init_projection (fmat4 *m, float val);
void init_viewport   (fmat4 *m, int x, int y, int w, int h, int d);

void rotate_coords (fmat4 *in, fmat4 *out, float alpha_deg, axis axis);

void draw_triangle (ScreenTriangle *t, pixel_shader shader, screenz_t *zbuffer, pixel_color_t *fbuffer, WFobj *obj);
void draw_obj      (WFobj *obj, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer, fmat4 *mvpv);

pixel_color_t set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a);
