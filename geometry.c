#include "geometry.h"

#include <math.h>
#include <stdarg.h>

float det3x3 (fmat3 *m);

void fmat4_set (fmat4 *mat, int row, int col, float val) {
	(*mat)[row][col] = val;
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

/*
void fmat4_mult (int m_num, ...) {
	
	fmat4 tmp1 = FMAT4_IDENTITY;
	fmat4 tmp2;
	fmat4 *tmp_a;
	fmat4 *tmp_b;
	fmat4 *tmp_c;
	fmat4 *tmp;
	
	va_list list;
	va_start (list, m_num);
	//tmp_in = &tmp1;
	//tmp_out = &tmp2;
	if (m_num < 2) return;
	else if (m_num == 2) {
		tmp_a = va_arg(list, fmat4*);
		tmp_b = va_arg(list, fmat4*);
		tmp_c = va_arg(list, fmat4*);
		fmat4_fmat4_mult (tmp_a, tmp_b, tmp_c);
	}
	else if (m_num > 2) {
		tmp_a = va_arg(list, fmat4*);
		tmp_b = va_arg(list, fmat4*);
		tmp_c = &tmp1;
		fmat4_fmat4_mult (tmp_a, tmp_b, tmp_c);
		tmp_a = &tmp2;
		for (int i = 2; i < m_num; i++) {
			if (i != (m_num-1)) {
				//swap_ptrs (tmp_a, tmp_c);
				tmp = tmp_a;
				tmp_a = tmp_c;
				tmp_c = tmp;
				tmp_b = va_arg(list, fmat4*);
				fmat4_fmat4_mult (tmp_a, tmp_b, tmp_c);
			}
			else {
				tmp_a = tmp_c;
				tmp_b = va_arg(list, fmat4*);
				tmp_c = va_arg(list, fmat4*);
				fmat4_fmat4_mult (tmp_a, tmp_b, tmp_c);
			}
		}
	}
	
	va_end (list);
}
*/

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

void fmat4_invert (fmat4 *in, fmat4 *out) {
	// inverse of M = adjoint of M / det M
	fmat4 adj;
	fmat4_adjugate(in, &adj);
	float det = 0.0f;
	fmat3 minor;
	for (int i = 0; i < 4; i++) {
		fmat4_get_minor (&adj, 0, i, &minor);
		det += adj[0][i] * det3x3 (&minor) * ((i % 2) ? -1 : 1);
	}
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			(*out)[i][j] = adj[i][j] / det;
}

void  fmat4_float4_mult (fmat4 *a, float4 *b, float4 *c) {
	for (int i = 0; i < 4; i++) {
		(*c)[i] = 0;
		for (int j = 0; j < 4; j++) (*c)[i] += (*a)[i][j] * (*b)[j];
	}
}

void float3_float3_sub (float3 *a, float3 *b, float3 *c) {
	for (int i = 0; i < 3; i++) (*c)[i] = (*a)[i] - (*b)[i];
}

float float3_float3_smult (float3 *a, float3 *b) {
	float smult = 0;
	for (int i = 0; i < 3; i++ ) smult += (*a)[i] * (*b)[i];
	return smult;
}

void float3_float3_crossprod(float3 *a, float3 *b, float3 *c) {
	(*c)[0] = (*a)[1] * (*b)[2] - (*a)[2] * (*b)[1];
	(*c)[1] = (*a)[2] * (*b)[0] - (*a)[0] * (*b)[2]; 
	(*c)[2] = (*a)[0] * (*b)[1] - (*a)[1] * (*b)[0];
}

void float3_normalize (float3 *v) {
	float length = (float) sqrt(pow((*v)[0], 2) + pow((*v)[1], 2) + pow((*v)[2], 2));
	for (int i = 0; i < 3; i++) (*v)[i] = (*v)[i] / length;
}

void float3_float4_pt_conv (float3 *in, float4 *out) {
	for (int k = 0; k < 4; k++)
		(*out)[k] = (k < 3) ? (*in)[k] : 1.0f;
}

void float3_float4_vect_conv (float3 *in, float4 *out) {
	for (int k = 0; k < 4; k++)
		(*out)[k] = (k < 3) ? (*in)[k] : 0.0f;
}

void float4_float3_pt_conv (float4 *in, float3 *out) {
	for (int k = 0; k < 3; k++)
		(*out)[k] = (*in)[k] / (*in)[3];
}

void float4_float3_vect_conv (float4 *in, float3 *out) {
	for (int k = 0; k < 3; k++)
		(*out)[k] = (*in)[k];
}
