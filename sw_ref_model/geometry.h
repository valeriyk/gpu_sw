#pragma once

#include <stdint.h>
#include <stdbool.h>


#define FMAT4_IDENTITY	{{1.f, 0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {0.f, 0.f, 0.f, 1.f}}


typedef int32_t int3 [3];


typedef float float2 [2];
typedef struct FloatUV {
	float u, v;
} FloatUV;
typedef union Float2 {
	float2 as_array;
	FloatUV as_struct;
} Float2;


typedef float float3 [3];
typedef struct FloatXYZ {
	float x, y, z;
} FloatXYZ;
typedef union Float3 {
	float3 as_array;
	FloatXYZ as_struct;
} Float3;


typedef float float4 [4];
typedef struct FloatXYZW {
	float x, y, z, w;
} FloatXYZW;
typedef union Float4 {
	float4 as_array;
	FloatXYZW as_struct;
} Float4;


typedef float fmat3 [3][3];
typedef float fmat4 [4][4];




static inline Float2 Float2_set (float u, float v) {
	Float2 m;
	m.as_struct.u = u;
	m.as_struct.v = v;
	return m;
}

static inline Float3 Float3_set (float x, float y, float z) {
	Float3 v;
	v.as_struct.x = x;
	v.as_struct.y = y;
	v.as_struct.z = z;
	return v;
}

static inline Float4 Float4_set (float x, float y, float z, float w) {
	Float4 v;
	v.as_struct.x = x;
	v.as_struct.y = y;
	v.as_struct.z = z;
	v.as_struct.w = w;
	return v;
}

static inline void fmat3_set (fmat3 *mat, int row, int col, float val) {
	(*mat)[row][col] = val;
}

static inline void fmat4_set (fmat4 *mat, int row, int col, float val) {
	(*mat)[row][col] = val;
}

void fmat4_identity (fmat4 *m);
	
void  fmat4_transpose  (fmat4 *in, fmat4 *out);
void  fmat4_inv_transp (fmat4 *in, fmat4 *out);
void  fmat4_inv        (fmat4 *in, fmat4 *out);
void  fmat4_copy       (fmat4 *in, fmat4 *out);

//void  fmat4_fmat4_fmat4_mult (fmat4 *a, fmat4 *b, fmat4 *c, fmat4 *d);
void   fmat4_fmat4_mult  (fmat4 *a, fmat4 *b,  fmat4 *c);
Float4 fmat4_Float4_mult (fmat4 *a, Float4 *b);

Float3 Float3_Float3_add       (Float3 *a, Float3 *b);
Float3 Float3_Float3_sub       (Float3 *a, Float3 *b);
float  Float3_Float3_smult     (Float3 *a, Float3 *b);
Float3 Float3_Float3_crossprod (Float3 *a, Float3 *b);

Float3 Float3_float_mult (Float3 *a, float b);

Float4 Float3_Float4_conv      (Float3 *in, float w);
Float3 Float4_Float3_pt_conv   (Float4 *in);
Float3 Float4_Float3_vect_conv (Float4 *in);

void   Float3_normalize (Float3 *v);
