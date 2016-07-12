#pragma once

#include <stdint.h>

typedef int32_t int3 [3];

typedef float float2 [2];
typedef float float3 [3];
typedef float float4 [4];

typedef float fmat3 [3][3];
typedef float fmat4 [4][4];

void  fmat4_set        (fmat4 *mat, int row, int col, float val);
void  fmat4_transpose  (fmat4 *in, fmat4 *out);
void  fmat4_inv_transp (fmat4 *in, fmat4 *out);

void  fmat4_fmat4_fmat4_mult (fmat4 *a, fmat4 *b, fmat4 *c, fmat4 *d);
void  fmat4_fmat4_mult  (fmat4 *a, fmat4 *b,  fmat4 *c);
void  fmat4_float4_mult (fmat4 *a, float4 *b, float4 *c);

void  float3_float3_add       (float3 *a, float3 *b, float3 *c);
void  float3_float3_sub       (float3 *a, float3 *b, float3 *c);
float float3_float3_smult     (float3 *a, float3 *b);
void  float3_float3_crossprod (float3 *a, float3 *b, float3 *c);

void  float3_float_mult  (float3 *a, float b, float3 *c);

void  float3_float4_pt_conv      (float3 *in, float4 *out);
void  float4_float3_pt_conv      (float4 *in, float3 *out);

void  float3_float4_vect_conv    (float3 *in, float4 *out);
void  float4_float3_vect_conv    (float4 *in, float3 *out);

void  float3_normalize (float3 *v);

void print_fmat4 (fmat4 *m, char *header);

#define FMAT4_IDENTITY {{1.f, 0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {0.f, 0.f, 0.f, 1.f}}
