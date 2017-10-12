#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <math.h>

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

Float4             fmat4_Float4_mult      (             fmat4 *a, Float4 *b);
static inline void fmat4_Float4_mult_fast (Float4 *out, fmat4 *a, Float4 *b) {
	for (int i = 0; i < 4; i++) {
		out->as_array[i] = 0;
		for (int j = 0; j < 4; j++) {
			out->as_array[i] += (*a)[i][j] * b->as_array[j];
		}
	}
}

Float3 Float3_Float3_add       (Float3 *a, Float3 *b);
Float3 Float3_Float3_sub       (Float3 *a, Float3 *b);
float  Float3_Float3_smult     (Float3 *a, Float3 *b);
Float3 Float3_Float3_crossprod (Float3 *a, Float3 *b);

Float3 Float3_float_mult (Float3 *a, float b);

Float4 Float3_Float4_conv      (Float3 *in, float w);
Float3 Float4_Float3_pt_conv   (Float4 *in);
Float3 Float4_Float3_vect_conv (Float4 *in);

static inline void Float3_Float4_conv_fast (Float4 *out, Float3 *in, float w) {
	for (int k = 0; k < 3; k++) {
		out->as_array[k] = in->as_array[k];
	}
	out->as_struct.w = w;
}

//~ Float3 Float4_Float3_pt_conv (Float4 *in) {
	//~ Float3 f3;
	//~ for (int k = 0; k < 3; k++)
		//~ f3.as_array[k] = in->as_array[k] / in->as_struct.w;
	//~ return f3;
//~ }

static inline void Float4_Float3_vect_conv_fast (Float3 *out, Float4 *in) {
	for (int k = 0; k < 3; k++) {
		out->as_array[k] = in->as_array[k];
	}
}

void   Float3_normalize (Float3 *v);
static inline void Float3_normalize_fast (Float3 *v) {
	float length = (float) sqrtf(v->as_struct.x * v->as_struct.x + v->as_struct.y * v->as_struct.y + v->as_struct.z * v->as_struct.z);
	float length_inv = 1.0f / length;
	for (int i = 0; i < 3; i++) {
		v->as_array[i] = v->as_array[i] * length_inv;
	}
}
