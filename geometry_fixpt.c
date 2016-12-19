#include "geometry_fixpt.h"



#include <math.h>
#include <stdarg.h>


FixPt2 Float2_FixPt2_conv (Float2 *in) {
	FixPt2 fx;
	for (int k = 0; k < 2; k++) {
		fx.as_array[k] = fixpt_from_float (in->as_array[k]);
	}
	return fx;
}

Float2 FixPt2_Float2_conv (FixPt2 *in) {
	Float2 f;
	for (int k = 0; k < 2; k++) {
		f.as_array[k] = fixpt_to_float (in->as_array[k]);
	}
	return f;
}

FixPt3 Float3_FixPt3_conv (Float3 *in) {
	FixPt3 fx;
	for (int k = 0; k < 3; k++) {
		fx.as_array[k] = fixpt_from_float (in->as_array[k]);
	}
	return fx;
}

Float3 FixPt3_Float3_conv (FixPt3 *in) {
	Float3 f;
	for (int k = 0; k < 3; k++) {
		f.as_array[k] = fixpt_to_float (in->as_array[k]);
	}
	return f;
}

FixPt4 Float4_FixPt4_conv (Float4 *in) {
	FixPt4 fx;
	for (int k = 0; k < 4; k++) {
		fx.as_array[k] = fixpt_from_float (in->as_array[k]);
	}
	return fx;
}

Float4 FixPt4_Float4_conv (FixPt4 *in) {
	Float4 f;
	for (int k = 0; k < 4; k++) {
		f.as_array[k] = fixpt_to_float (in->as_array[k]);
	}
	return f;
}

/*
FixPt2 Float2_FixPt2_cast (Float2 *in) {
	FixPt2 fx;
	for (int k = 0; k < 2; k++) {
		fx.as_array[k] = (fixpt_t) in->as_array[k];
	}
	return fx;
}

Float2 FixPt2_Float2_cast (FixPt2 *in) {
	Float2 f;
	for (int k = 0; k < 2; k++) {
		f.as_array[k] = (float) in->as_array[k];
	}
	return f;
}

FixPt4 Float4_FixPt4_cast (Float4 *in) {
	FixPt4 fx;
	for (int k = 0; k < 4; k++) {
		fx.as_array[k] = (fixpt_t) in->as_array[k];
	}
	return fx;
}

Float4 FixPt4_Float4_cast (FixPt4 *in) {
	Float4 f;
	for (int k = 0; k < 4; k++) {
		f.as_array[k] = (float) in->as_array[k];
	}
	return f;
}
*/
