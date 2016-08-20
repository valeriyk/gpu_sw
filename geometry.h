#pragma once

#include <stdint.h>
#include <stdbool.h>

#define DEBUG_GEOM_0 0

typedef int32_t int3 [3];

typedef float float2 [2];
typedef float float3 [3];
typedef float float4 [4];

typedef struct FloatUV {
	float u, v;
} FloatUV;

typedef struct FloatXYZ {
	float x, y, z;
} FloatXYZ;

typedef struct FloatXYZW {
	float x, y, z, w;
} FloatXYZW;

typedef union Float2 {
	float2 as_array;
	FloatUV as_struct;
} Float2;

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

#define FMAT4_IDENTITY {{1.f, 0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {0.f, 0.f, 0.f, 1.f}}

typedef union Fmat4 {
	fmat4 as_array;
	float4 row[4];
} Fmat4;


static inline Float3 Float3_set (float x, float y, float z) {
	Float3 v;
	v.as_struct.x = x;
	v.as_struct.y = y;
	v.as_struct.z = z;
	return v;
}

static inline void fmat3_set (fmat3 *mat, int row, int col, float val) {
	(*mat)[row][col] = val;
}

static inline void fmat4_set (fmat4 *mat, int row, int col, float val) {
	(*mat)[row][col] = val;
}

//void fmat4_identity (fmat4 *m);
	
void fmat3_set_col (fmat3 *mat, Float3 *in, int col_idx);

void  fmat4_transpose  (fmat4 *in, fmat4 *out);
void  fmat4_inv_transp (fmat4 *in, fmat4 *out);
void  fmat4_invert_transpose (fmat4 *in, fmat4 *out, bool transpose);
void  fmat4_inv        (fmat4 *in, fmat4 *out);
void  fmat4_copy       (fmat4 *in, fmat4 *out);

void  fmat4_fmat4_fmat4_mult (fmat4 *a, fmat4 *b, fmat4 *c, fmat4 *d);
void  fmat4_fmat4_mult  (fmat4 *a, fmat4 *b,  fmat4 *c);
Float4 fmat4_Float4_mult (fmat4 *a, Float4 *b);

Float3 Float3_Float3_add       (Float3 *a, Float3 *b);
Float3 Float3_Float3_sub       (Float3 *a, Float3 *b);
float  Float3_Float3_smult     (Float3 *a, Float3 *b);
Float3 Float3_Float3_crossprod (Float3 *a, Float3 *b);

Float3 Float3_float_mult (Float3 *a, float b);

Float4 Float3_Float4_pt_conv (Float3 *in);
Float3 Float4_Float3_pt_conv (Float4 *in);

Float4 Float3_Float4_vect_conv (Float3 *in);
Float3 Float4_Float3_vect_conv (Float4 *in);

void   Float3_normalize (Float3 *v);

void   print_fmat4 (fmat4 *m, char *header);
