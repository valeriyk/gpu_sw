#pragma once

#include "geometry.h"
//#include <fixmath.h>
//#include "fixpt.h"

#define NDEBUG
#include <assert.h>

#include <math.h>
#include <stdint.h>
#include <stdbool.h>





typedef  int32_t  fixpt_t;
typedef  int64_t dfixpt_t;

typedef uint32_t nfixpt_t;

#define  FIXPT_BITS (sizeof( fixpt_t) * 8)
#define DFIXPT_BITS (sizeof(dfixpt_t) * 8)
#define NFIXPT_BITS (sizeof(nfixpt_t) * 8)

#define  FRACT_BITS 8
#define DFRACT_BITS (FRACT_BITS*2)
#define NFRACT_BITS 30


#define XY_FRACT_BITS 8
#define  Z_FRACT_BITS 4
#define  W_FRACT_BITS 29

#define BARC_FRACT_BITS (XY_FRACT_BITS*2)

//#define FIX_PT_PRECISION	4



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

FixPt3 Float3_FixPt3_conv (Float3 *in);
Float3 FixPt3_Float3_conv (FixPt3 *in);

FixPt4 Float4_FixPt4_conv (Float4 *in);
Float4 FixPt4_Float4_conv (FixPt4 *in);

FixPt2 Float2_FixPt2_cast (Float2 *in);
Float2 FixPt2_Float2_cast (FixPt2 *in);

FixPt4 Float4_FixPt4_cast (Float4 *in);
Float4 FixPt4_Float4_cast (FixPt4 *in);




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

static inline dfixpt_t fixpt_dfixpt_mul (fixpt_t a, dfixpt_t b) {
	dfixpt_t c = (dfixpt_t) a * b;
	assert ((a > 0) && (b > 0) && (c > 0));
	assert ((a < 0) && (b < 0) && (c < 0));
	assert ((a > 0) && (b < 0) && (c < 0));
	assert ((a < 0) && (b > 0) && (c < 0));
	assert ((a == 0) && (c == 0));
	assert ((b == 0) && (c == 0));
	//assert ((c >> (FIXPT_BITS + FRACT_BITS)) == 0 );
	return c;
}


static inline fixpt_t fixpt_nfixpt_mul (fixpt_t a, nfixpt_t b) {
	dfixpt_t c = (dfixpt_t) a * (dfixpt_t) b;
	// 28.4 * 2.30 = 30.34 --> 30.4
	return (fixpt_t) (c >> NFRACT_BITS);
}


static inline fixpt_t fixpt_div (fixpt_t a, fixpt_t b) {
	dfixpt_t ad = ((dfixpt_t) a) << FRACT_BITS;
	assert (b != 0);
	dfixpt_t c;
	if (b != 0) c = ad / (dfixpt_t) b;
	else c = 0; //TBD
	return (fixpt_t) c;
}


static inline FixPt3 FixPt3_FixPt3_add (FixPt3 a, FixPt3 b) {
	FixPt3 c;
	for (int i = 0; i < 3; i++) {
		c.as_array[i] = a.as_array[i] + b.as_array[i];
	}
	return c;
}

static inline fixpt_t fixpt_get_min (void) {
	return 0x80000000;
}

static inline fixpt_t fixpt_get_max (void) {
	return 0x7FFFFFFF;
}

static inline fixpt_t fixpt_get_one (void) {
	return (1 << FRACT_BITS);
}




static inline fixpt_t fixpt_from_float (float a) {
	fixpt_t c = (fixpt_t) (roundf (a) * (1 << FRACT_BITS));
	//assert ();
	return c;
}

static inline float fixpt_to_float (fixpt_t a) {
	return ((float) a) / ((float) (1 << FRACT_BITS));
}

static inline fixpt_t fixpt_fract_from_float (float a, uint8_t fract_bits) {
	fixpt_t c = (fixpt_t) (roundf (a) * (1 << fract_bits));
	return c;
}

static inline float fixpt_fract_to_float (fixpt_t a, uint8_t fract_bits) {
	return ((float) a) / ((float) (1 << fract_bits));
}


static inline dfixpt_t dfixpt_from_float (float a) {
	dfixpt_t c = (dfixpt_t) (roundf (a) * (1 << DFRACT_BITS));
	//assert ();
	return c;
}

static inline float dfixpt_to_float (dfixpt_t a) {
	return ((float) a) / ((float) (1 << DFRACT_BITS));
}



static inline nfixpt_t nfixpt_from_float (float a) {
	assert (a <= 1.0f);
	fixpt_t c = (fixpt_t) (a * (1 << NFRACT_BITS));
	//assert ();
	return c;
}

static inline float nfixpt_to_float (nfixpt_t a) {
	return ((float) a) / ((float) (1 << NFRACT_BITS));
}




static inline fixpt_t fixpt_from_int32 (int32_t a) {
	fixpt_t c = (fixpt_t) (a << FRACT_BITS);
	//assert ();
	return c;
}

static inline int32_t   fixpt_to_int32 (fixpt_t a) {
	/*bool round_up = (a >> (FRACT_BITS-1)) & 1;
	
	int32_t c = a >> FRACT_BITS;
	return (round_up) ? ++c : c;*/
	return (a >> FRACT_BITS);
}



static inline fixpt_t fixpt_from_dfixpt (dfixpt_t a) {
	return (fixpt_t) (a >> FRACT_BITS);
}

static inline dfixpt_t fixpt_to_dfixpt (fixpt_t a) {
	return (a << FRACT_BITS);
}



