#pragma once 

#include "tgaimage.h"
#include <stdint.h>

#define X 0
#define Y 1
#define Z 2
#define W 3

typedef int32_t int2 [2];
typedef int32_t int3 [3];
typedef int32_t int4 [4];

typedef float float2 [2];
typedef float float3 [3];
typedef float float4 [4];

typedef float fmat4 [4][4];

void  fmat4_set (      fmat4 &mat, const int row, const int col, const float val);
float fmat4_get (const fmat4 &mat, const int row, const int col                 );

void  fmat4_fmat4_mult  (const fmat4 &a, const  fmat4& b,  fmat4& c);
void  fmat4_float4_mult (const fmat4 &a, const float4& b, float4& c);


void init_viewport (fmat4 &m, int x, int y, int w, int h, int d);
void init_projection (fmat4 &m, const float val);
void init_view (fmat4 &m, float3 &eye, float3 &center, float3 &up);

void float3_float3_sub (const float3 &a, const float3 &b, float3 &c);
void float3_normalize (float3 &v);
void float3_float3_crossprod(const float3 &a, const float3 &b, float3 &c);



typedef uint16_t screenxy_t;
typedef uint16_t screenz_t;
typedef struct ScreenPt {
	screenxy_t x;
	screenxy_t y;
	screenz_t  z;
} ScreenPt;


typedef struct Point2D {
    int x, y;
} Point2D;

typedef struct Point2Df {
    float u, v;
} Point2Df;

/*
typedef struct Point3Df {
	float x, y, z;
} Point3Df;
*/

typedef struct Face {
	int3 vtx_idx; // vertex indices
	int3 txt_idx; // texture indices
} Face;

typedef struct Vertex {
	float3 norm;
	Point2Df txt_uv;
	ScreenPt coords;
	//int3 coords;
	//float3 coords;
} Vertex;


void draw_triangle (const Vertex v[3], screenz_t *zbuffer, TGAImage &image, TGAImage &texture, float3 light_dir);
