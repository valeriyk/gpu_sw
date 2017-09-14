#pragma once

#include "geometry.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>


typedef  int32_t  fixpt_t;
typedef  int64_t dfixpt_t;

#define  FIXPT_BITS (sizeof( fixpt_t) * 8)
#define DFIXPT_BITS (sizeof(dfixpt_t) * 8)

#define       XY_FRACT_BITS 4
#define        Z_FRACT_BITS 4
#define     BARC_FRACT_BITS (XY_FRACT_BITS*2)
#define W_RECIPR_FRACT_BITS 29
#define     OOWI_FRACT_BITS 16
#define  VARYING_FRACT_BITS 14

/*
typedef fixpt_t fixpt_t1[1];
typedef float   float_t1[1];

typedef struct FixPtValue {
	fixpt_t value;
} FixPtValue;
typedef struct FloatValue {
	float value;
} FloatValue;

typedef union FixPt1 {
	fixpt_t1   as_array;
	FixPtValue as_struct;
} FixPt1;
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


static inline fixpt_t fixpt_get_min (void) {
	return 0x80000000;
}

static inline fixpt_t fixpt_get_max (void) {
	return 0x7FFFFFFF;
}

static inline fixpt_t fixpt_from_float (float a, uint8_t fract_bits) {
	//fixpt_t c = (fixpt_t) roundf (a * (1 << fract_bits));
	fixpt_t c = (fixpt_t) (roundf (a) * (1 << fract_bits));
	return c;
}
	
	
static inline fixpt_t fixpt_from_float_no_rnd (float a, uint8_t fract_bits) {
	fixpt_t c = (fixpt_t) (a * (1 << fract_bits));
	return c;
}

static inline float fixpt_to_float (fixpt_t a, uint8_t fract_bits) {
	assert (fract_bits < 32);
	float tmp = (float) (1 << fract_bits);
	assert (tmp != 0);
	return ((float) a) / tmp;
}

static inline float dfixpt_to_float (dfixpt_t a, uint8_t dfract_bits) {
	assert (dfract_bits < 64);
	float tmp = (float) (1L << dfract_bits);
	assert (tmp != 0);
	return ((float) a) / tmp;
}

static inline FixPt2 Float2_FixPt2_conv (Float2 *in) {
	FixPt2 fx;
	for (int k = 0; k < 2; k++) {
		fx.as_array[k] = fixpt_from_float (in->as_array[k], VARYING_FRACT_BITS);
	}
	return fx;
}

static inline Float2 FixPt2_Float2_conv (FixPt2 *in) {
	Float2 f;
	for (int k = 0; k < 2; k++) {
		f.as_array[k] = fixpt_to_float (in->as_array[k], VARYING_FRACT_BITS);
	}
	return f;
}

static inline FixPt3 Float3_FixPt3_conv (Float3 *in) {
	FixPt3 fx;
	for (int k = 0; k < 3; k++) {
		fx.as_array[k] = fixpt_from_float (in->as_array[k], VARYING_FRACT_BITS);
	}
	return fx;
}

static inline Float3 FixPt3_Float3_conv (FixPt3 *in) {
	Float3 f;
	for (int k = 0; k < 3; k++) {
		f.as_array[k] = fixpt_to_float (in->as_array[k], VARYING_FRACT_BITS);
	}
	return f;
}

static inline FixPt4 Float4_FixPt4_conv (Float4 *in) {
	FixPt4 fx;
	for (int k = 0; k < 4; k++) {
		fx.as_array[k] = fixpt_from_float (in->as_array[k], VARYING_FRACT_BITS);
	}
	return fx;
}

static inline Float4 FixPt4_Float4_conv (FixPt4 *in) {
	Float4 f;
	for (int k = 0; k < 4; k++) {
		f.as_array[k] = fixpt_to_float (in->as_array[k], VARYING_FRACT_BITS);
	}
	return f;
}

static inline fixpt_t fixpt_add (fixpt_t a, fixpt_t b) {
	fixpt_t c = a + b;
	assert (((a > 0) && (b > 0)) ? (c > 0) : 1);
	assert (((a < 0) && (b < 0)) ? (c < 0) : 1);
	return c;
}

static inline fixpt_t fixpt_sub (fixpt_t a, fixpt_t b) {
	fixpt_t c = a - b;
	assert (((a > 0) && (b < 0)) ? (c > 0) : 1);
	assert (((a < 0) && (b > 0)) ? (c < 0) : 1);
	return c;
}

static inline FixPt3 FixPt3_FixPt3_add (FixPt3 a, FixPt3 b) {
	FixPt3 c;
	for (int i = 0; i < 3; i++) {
		c.as_array[i] = a.as_array[i] + b.as_array[i];
	}
	return c;
}
