#pragma once

#include "geometry.h"
#include "tgaimage.h"
#include "wavefront_obj.h"

#include <stdint.h>

typedef enum {X = 0, Y = 1, Z = 2, W = 3} axis;


typedef int16_t screenxy_t;
typedef int32_t screenz_t;
typedef struct ScreenPt {
	screenxy_t x;
	screenxy_t y;
	screenz_t  z;
} ScreenPt;

typedef struct pixel_color_t {
	uint8_t r, g, b, a;
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


typedef void (*vertex_shader) (const WFobj *obj, const int face_idx, const int vtx_idx, const fmat4 *mvpv, float4 *vtx4d);
typedef bool (*pixel_shader)  (const WFobj *obj, const float3 *barc, pixel_color_t *color);

int   orient2d (const ScreenPt &a, const ScreenPt &b, const ScreenPt &c);

//void  world2screen (const float4 &w, ScreenPt &s);

screenxy_t tri_min_bound (const screenxy_t a, const screenxy_t b, const screenxy_t c, const screenxy_t cutoff);
screenxy_t tri_max_bound (const screenxy_t a, const screenxy_t b, const screenxy_t c, const screenxy_t cutoff);

void init_model      (fmat4 *m, const float3 &scale, const float3 &rotate, const float3 &tran);
void init_view       (fmat4 *m, const float3 &eye,   const float3 &center, const float3 &up);
void init_projection (fmat4 &m, const float val);
void init_viewport   (fmat4 &m, const int x, const int y, const int w, const int h, const int d);

void rotate_coords (const fmat4 *in, fmat4 *out, float alpha_deg, axis axis);

void draw_triangle (const ScreenTriangle *t, pixel_shader shader, screenz_t *zbuffer, pixel_color_t *fbuffer, const WFobj *obj);
void draw_obj      (const WFobj *obj, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer, const fmat4 *mvpv);

pixel_color_t set_color (const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);
