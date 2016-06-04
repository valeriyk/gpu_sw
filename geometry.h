#pragma once

#include <stdint.h>

//typedef int32_t int2 [2];
typedef int32_t int3 [3];
//typedef int32_t int4 [4];

typedef float float2 [2];
typedef float float3 [3];
typedef float float4 [4];

typedef float fmat3 [3][3];
typedef float fmat4 [4][4];

#define FMAT4_IDENTITY {{1.f, 0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {0.f, 0.f, 0.f, 1.f}}

void  fmat4_set (      fmat4 *mat, int row, int col, float val);
//float fmat4_get (const fmat4 *mat, const int row, const int col                 );

void  fmat4_fmat4_mult  (fmat4 *a, fmat4 *b,  fmat4 *c);
void  fmat4_float4_mult (fmat4 *a, float4 *b, float4 *c);

void  float3_float3_sub       (float3 *a, float3 *b, float3 *c);
float float3_float3_smult     (float3 *a, float3 *b);
//float float3_int3_smult       (const float3 *a, const int3 *b);
//int   int3_int3_smult         (const int3 &a,   const int3 &b);
void  float3_float3_crossprod (float3 *a, float3 *b, float3 *c);

void  float3_normalize (float3 *v);

void float3_float4_conv (float3 *in, float4 *out);
void float4_float3_conv (float4 *in, float3 *out);

void fmat4_transpose (fmat4 *in, fmat4 *out);
void fmat4_invert (fmat4 *in, fmat4 *out);



void print_fmat4 (fmat4 *m, char *header);

float det3x3 (fmat3 *m);
