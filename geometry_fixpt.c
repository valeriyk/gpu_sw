#include "geometry_fixpt.h"



#include <math.h>
#include <stdarg.h>

/*
float det3x3 (fmat3 *m);
void fmat4_invert_transpose (fmat4 *in, fmat4 *out, bool transpose);

void fmat4_identity (fmat4 *m) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			fmat4_set (m, i, j, (i == j) ? 1.0f : 0.0f);
		}
	}
}

void  fmat4_fmat4_mult (fmat4 *a, fmat4 *b, fmat4 *c) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			(*c)[i][j] = 0;
			for (int k = 0; k < 4; k++)	{
				(*c)[i][j] += (*a)[i][k] * (*b)[k][j];
			}
		}
	}
}

float det3x3 (fmat3 *m) {
	return (*m)[0][0] * (*m)[1][1] * (*m)[2][2] -
	       (*m)[0][0] * (*m)[1][2] * (*m)[2][1] -
	       (*m)[0][1] * (*m)[1][0] * (*m)[2][2] +
	       (*m)[0][1] * (*m)[1][2] * (*m)[2][0] +
	       (*m)[0][2] * (*m)[1][0] * (*m)[2][1] -
	       (*m)[0][2] * (*m)[1][1] * (*m)[2][0];
}

void fmat4_transpose (fmat4 *in, fmat4 *out) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			(*out)[i][j] = (*in)[j][i];
		}
	}
}

void fmat4_copy (fmat4 *in, fmat4 *out) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			(*out)[i][j] = (*in)[i][j];
		}
	}
}

void fmat4_get_minor (fmat4 *in, int r, int c, fmat3 *out) {
	int m = 0;
	int n = 0;
	for (int i = 0; i < 4; i++) {
		if (r != i) {
			for (int j = 0; j < 4; j++) {
				if (c != j) (*out)[m][n++] = (*in)[i][j];
			}
			m++;
			n = 0;
		}
	}
}

// cofactor of M = matrix of determinants of minors of M, multiplied by -1^(i+j)
void fmat4_cofactor (fmat4 *in, fmat4 *out) {
	fmat3 minor;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			fmat4_get_minor (in, i, j, &minor);
			int sign = ((i + j) % 2) ? -1 : 1;
			(*out)[i][j] = det3x3 (&minor) * sign;
		}
	}
}

// adjoint of M = transposed cofactor of M
void fmat4_adjugate (fmat4 *in, fmat4 *out) {
	fmat4 cof;
	fmat4_cofactor (in, &cof);
	fmat4_transpose (&cof, out);
}

// inverse of M = adjoint of M / det M
// + optionally transpose
void fmat4_invert_transpose (fmat4 *in, fmat4 *out, bool transpose) {
	fmat4 adj;
	fmat4_adjugate(in, &adj);
	float det = 0.0f;
	for (int i = 0; i < 4; i++) {
		det += (*in)[i][0] * adj[0][i] * ((i % 2) ? -1 : 1);
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			(*out)[i][j] = (transpose ? adj[j][i] : adj[i][j]) / det;
		}
	}
}

void fmat4_inv (fmat4 *in, fmat4 *out) {
	fmat4_invert_transpose (in, out, 0);
}

void fmat4_inv_transp (fmat4 *in, fmat4 *out) {
	fmat4_invert_transpose (in, out, 1);
}

Float4 fmat4_Float4_mult (fmat4 *a, Float4 *b) {
	Float4 f4;
	for (int i = 0; i < 4; i++) {
		f4.as_array[i] = 0;
		for (int j = 0; j < 4; j++) {
			f4.as_array[i] += (*a)[i][j] * b->as_array[j];
		}
	}
	return f4;
}

Float3 Float3_Float3_add (Float3 *a, Float3 *b) {
	Float3 c;
	for (int i = 0; i < 3; i++) {
		c.as_array[i] = a->as_array[i] + b->as_array[i];
	}
	return c;
}

Float3 Float3_Float3_sub (Float3 *a, Float3 *b) {
	Float3 c;
	for (int i = 0; i < 3; i++) {
		c.as_array[i] = a->as_array[i] - b->as_array[i];
	}
	return c;
}


float Float3_Float3_smult (Float3 *a, Float3 *b) {
	float smult = 0;
	for (int i = 0; i < 3; i++ ) {
		smult += a->as_array[i] * b->as_array[i];
	}
	return smult;
}

Float3 Float3_Float3_crossprod (Float3 *a, Float3 *b) {
	Float3 c;
	c.as_struct.x = a->as_struct.y * b->as_struct.z - a->as_struct.z * b->as_struct.y;
	c.as_struct.y = a->as_struct.z * b->as_struct.x - a->as_struct.x * b->as_struct.z; 
	c.as_struct.z = a->as_struct.x * b->as_struct.y - a->as_struct.y * b->as_struct.x;
	return c;
}

Float3 Float3_float_mult(Float3 *a, float b) {
	Float3 c;
	for (int i = 0; i < 3; i++) {
		c.as_array[i] = a->as_array[i] * b;
	}
	return c;
}

void Float3_normalize (Float3 *v) {
	float length = (float) sqrtf(v->as_struct.x * v->as_struct.x + v->as_struct.y * v->as_struct.y + v->as_struct.z * v->as_struct.z);
	for (int i = 0; i < 3; i++) {
		v->as_array[i] = v->as_array[i] / length;
	}
}

Float4 Float3_Float4_conv (Float3 *in, float w) {
	Float4 f4;
	for (int k = 0; k < 3; k++) {
		f4.as_array[k] = in->as_array[k];
	}
	f4.as_struct.w = w;
	return f4;
}

Float3 Float4_Float3_pt_conv (Float4 *in) {
	Float3 f3;
	for (int k = 0; k < 3; k++)
		f3.as_array[k] = in->as_array[k] / in->as_struct.w;
	return f3;
}

Float3 Float4_Float3_vect_conv (Float4 *in) {
	Float3 f3;
	for (int k = 0; k < 3; k++)
		f3.as_array[k] = in->as_array[k];
	return f3;
}
*/

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
