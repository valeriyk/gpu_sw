#include "geometry.h"

#include <math.h>
#include <stdarg.h>

float det3x3 (fmat3 *m);
//void fmat4_invert_transpose (fmat4 *in, fmat4 *out, bool transpose);
	
	
void fmat3_set_col (fmat3 *mat, Float3 *in, int col_idx) {
	for (int i = 0; i < 3; i++) fmat3_set (mat, i, col_idx, in->as_array[i]);
}

void fmat4_identity (fmat4 *m) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (i == j) {
				fmat4_set (m, i, j, 1.0);
			}
			else {
				fmat4_set (m, i, j, 0.0);
			}
		}
	}
}

void  fmat4_fmat4_mult (fmat4 *a, fmat4 *b, fmat4 *c) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			(*c)[i][j] = 0;
			for (int k = 0; k < 4; k++)	(*c)[i][j] += (*a)[i][k] * (*b)[k][j];
		}
	}
}

void  fmat4_fmat4_fmat4_mult (fmat4 *a, fmat4 *b, fmat4 *c, fmat4 *d) {
	fmat4 tmp;
	fmat4_fmat4_mult (a, b, &tmp);
	fmat4_fmat4_mult (&tmp, c, d);
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
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			(*out)[i][j] = (*in)[j][i];
}

void fmat4_copy (fmat4 *in, fmat4 *out) {
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			(*out)[i][j] = (*in)[i][j];
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

void fmat4_cofactor (fmat4 *in, fmat4 *out) {
	// cofactor of M = matrix of determinants of minors of M, multiplied by -1^(i+j)
	fmat3 minor;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			fmat4_get_minor (in, i, j, &minor);
			int sign = ((i + j) % 2) ? -1 : 1;
			(*out)[i][j] = det3x3 (&minor) * sign;
		}
	}
}

void fmat4_adjugate (fmat4 *in, fmat4 *out) {
	// adjoint of M = transposed cofactor of M
	fmat4 cof;
	fmat4_cofactor (in, &cof);
	fmat4_transpose (&cof, out);
}

void fmat4_inv (fmat4 *in, fmat4 *out) {
	fmat4_invert_transpose (in, out, 0);
}

void fmat4_inv_transp (fmat4 *in, fmat4 *out) {
	fmat4_invert_transpose (in, out, 1);
}

void fmat4_invert_transpose (fmat4 *in, fmat4 *out, bool transpose) {
	// inverse of M = adjoint of M / det M
	if (DEBUG_GEOM_0) print_fmat4 (in, "inv_tr in");
	fmat4 adj;
	fmat4_adjugate(in, &adj);
	if (DEBUG_GEOM_0) print_fmat4 (adj, "inv_tr adj");
	float det = 0.0f;
	//fmat3 minor;
	for (int i = 0; i < 4; i++) {
		//fmat4_get_minor (&adj, 0, i, &minor);
		//fmat4_get_minor (in, i, 0, &minor);
		//print_fmat3 (minor, "minor");
		//float det3 = det3x3 (&minor);
		//printf("det3 %d = %f\n", i, det3);
		//det += adj[0][i] * det3 * ((i % 2) ? -1 : 1);
		det += (*in)[i][0] * adj[0][i] * ((i % 2) ? -1 : 1);
	}
	//printf ("det %f\n", det);
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			(*out)[i][j] = adj[j][i] / det; // transpose right here
			//if (transpose) (*out)[i][j] = adj[j][i] / det; // transpose right here
			//else (*out)[i][j] = adj[i][j] / det; // don't transpose
}

Float4 fmat4_Float4_mult (fmat4 *a, Float4 *b) {
	Float4 f4;
	for (int i = 0; i < 4; i++) {
		f4.as_array[i] = 0;
		for (int j = 0; j < 4; j++) f4.as_array[i] += (*a)[i][j] * b->as_array[j];
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
	for (int i = 0; i < 3; i++ ) smult += a->as_array[i] * b->as_array[i];
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
	float length = (float) sqrt(pow(v->as_struct.x, 2) + pow(v->as_struct.y, 2) + pow(v->as_struct.z, 2));
	for (int i = 0; i < 3; i++) {
		v->as_array[i] = v->as_array[i] / length;
	}
}

Float4 Float3_Float4_pt_conv (Float3 *in) {
	Float4 f4;
	for (int k = 0; k < 4; k++) {
		f4.as_array[k] = (k < 3) ? in->as_array[k] : 1.0f;
	}
	return f4;
}

Float4 Float3_Float4_vect_conv (Float3 *in) {
	Float4 f4;
	for (int k = 0; k < 4; k++) {
		f4.as_array[k] = (k < 3) ? in->as_array[k] : 0.0f;
	}
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

#include <stdio.h>
void print_fmat4 (fmat4 *m, char *header) {
	printf ("%s\n", header);
	for (int i = 0; i < 4; i++) {
		printf("row %d: ", i);
		for (int j = 0; j < 4; j++)
			printf ("%f ", (*m)[i][j]);
		printf("\n");
	}
	printf("\n");
}

void print_fmat3 (fmat3 *m, char *header) {
	printf ("%s\n", header);
	for (int i = 0; i < 3; i++) {
		printf("row %d: ", i);
		for (int j = 0; j < 3; j++)
			printf ("%f ", (*m)[i][j]);
		printf("\n");
	}
	printf("\n");
}
