#pragma once

#include "geometry.h"
//#include <fixmath.h>
//#include "fixpt.h"

#define NDEBUG
#include <assert.h>

#include <stdint.h>
#include <stdbool.h>





typedef int32_t  fixpt_t;
typedef int64_t dfixpt_t;


#define FIXPT_BITS (sizeof(fixpt_t) * 8)
#define FRACT_BITS 16

#define FIX_PT_PRECISION	4



/*#define FMAT4_IDENTITY	{{1.f, 0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {0.f, 0.f, 0.f, 1.f}}


typedef int32_t int3 [3];
*/

typedef fixpt_t fixpt_t2[2];
typedef struct FixPtUV {
	fixpt_t u, v;
} FixPtUV;
typedef union FixPt2 {
	fixpt_t2 as_array;
	FixPtUV as_struct;
} FixPt2;

typedef fixpt_t fixpt_t3[3];
typedef struct FixPtXYZ {
	fixpt_t x, y, z;
} FixPtXYZ;
typedef union FixPt3 {
	fixpt_t3 as_array;
	FixPtXYZ as_struct;
} FixPt3;

typedef fixpt_t fixpt_t4[4];
typedef struct FixPtXYZW {
	fixpt_t x, y, z, w;
} FixPtXYZW;
typedef union FixPt4 {
	fixpt_t4 as_array;
	FixPtXYZW as_struct;
} FixPt4;


FixPt2 Float2_FixPt2_conv (Float2 *in);
Float2 FixPt2_Float2_conv (FixPt2 *in);

FixPt4 Float4_FixPt4_conv (Float4 *in);
Float4 FixPt4_Float4_conv (FixPt4 *in);

FixPt2 Float2_FixPt2_cast (Float2 *in);
Float2 FixPt2_Float2_cast (FixPt2 *in);

FixPt4 Float4_FixPt4_cast (Float4 *in);
Float4 FixPt4_Float4_cast (FixPt4 *in);




static inline fixpt_t fixpt_add (fixpt_t a, fixpt_t b) {
	fixpt_t c = a + b;
	assert ((a > 0) && (b > 0) && (c > 0));
	assert ((a < 0) && (b < 0) && (c < 0));
	return c;
}

static inline fixpt_t fixpt_sub (fixpt_t a, fixpt_t b) {
	fixpt_t c = a - b;
	assert ((a > 0) && (b < 0) && (c > 0));
	assert ((a < 0) && (b > 0) && (c < 0));
	return c;
}

static inline fixpt_t fixpt_mul (fixpt_t a, fixpt_t b) {
	dfixpt_t c = (dfixpt_t) a * (dfixpt_t) b;
	assert ((a > 0) && (b > 0) && (c > 0));
	assert ((a < 0) && (b < 0) && (c < 0));
	assert ((a > 0) && (b < 0) && (c < 0));
	assert ((a < 0) && (b > 0) && (c < 0));
	assert ((a == 0) && (c == 0));
	assert ((b == 0) && (c == 0));
	assert ((c >> (FIXPT_BITS + FRACT_BITS)) == 0 );
	return (fixpt_t) (c >> FRACT_BITS);
}

static inline fixpt_t fixpt_div (fixpt_t a, fixpt_t b) {
	dfixpt_t ad = ((dfixpt_t) a) << FRACT_BITS;
	assert (b != 0);
	dfixpt_t c;
	if (b != 0) c = ad / (dfixpt_t) b;
	else c = 0; //TBD
	return (fixpt_t) c;
}


static inline fixpt_t fixpt_get_min (void) {
	return 0x80000000;
}

static inline fixpt_t fixpt_get_max (void) {
	return 0x7FFFFFFF;
}

static inline fixpt_t fixpt_from_float (float a) {
	fixpt_t c = (fixpt_t) (a * (1 << FRACT_BITS));
	//assert ();
	return c;
}

static inline float   fixpt_to_float (fixpt_t a) {
	return ((float) a) / ((float) (1 << FRACT_BITS));
}




static inline fixpt_t fixpt_from_int32 (int32_t a) {
	fixpt_t c = (fixpt_t) (a << FRACT_BITS);
	//assert ();
	return c;
}

static inline int32_t   fixpt_to_int32 (fixpt_t a) {
	return (a >> FRACT_BITS);
}



/*

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
*/
