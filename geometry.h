#pragma once

#include <stdint.h>

typedef int32_t int3 [3];

typedef float float2 [2];
typedef float float3 [3];
typedef float float4 [4];

typedef struct FloatXYZ {
	float x, y, z;
} FloatXYZ;

typedef struct FloatXYZW {
	float x, y, z, w;
} FloatXYZW;

typedef union Float3 {
	float3 as_array;
	FloatXYZ as_struct;
} Float3;

typedef union Float4 {
	float4 as_array;
	FloatXYZW as_struct;
} Float4;

typedef float fmat3 [3][3];
typedef float fmat4 [4][4];

typedef union Fmat4 {
	fmat4 as_array;
	float4 row[4];
} Fmat4;


Float3 Float3_set (float a, float b, float c);

void  fmat4_set        (fmat4 *mat, int row, int col, float val);
void  fmat4_transpose  (fmat4 *in, fmat4 *out);
void  fmat4_inv_transp (fmat4 *in, fmat4 *out);

void  fmat4_fmat4_fmat4_mult (fmat4 *a, fmat4 *b, fmat4 *c, fmat4 *d);
void  fmat4_fmat4_mult  (fmat4 *a, fmat4 *b,  fmat4 *c);
Float4 fmat4_Float4_mult (fmat4 *a, Float4 *b);

void  float3_float3_add       (float3 *a, float3 *b, float3 *c);
void  float3_float3_sub       (float3 *a, float3 *b, float3 *c);
float float3_float3_smult     (float3 *a, float3 *b);
void  float3_float3_crossprod (float3 *a, float3 *b, float3 *c);

void  float3_float_mult  (float3 *a, float b, float3 *c);

Float4 Float3_Float4_pt_conv (Float3 *in);
Float3 Float4_Float3_pt_conv (Float4 *in);

Float4 Float3_Float4_vect_conv (Float3 *in);
Float3 Float4_Float3_vect_conv (Float4 *in);

void  Float3_normalize (Float3 *v);

void print_fmat4 (fmat4 *m, char *header);

#define FMAT4_IDENTITY {{1.f, 0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {0.f, 0.f, 0.f, 1.f}}
