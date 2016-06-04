#include "geometry.h"
#include <math.h>


void fmat4_set (fmat4 *mat, int row, int col, float val) {
	(*mat)[row][col] = val;
}

/*
float fmat4_get (fmat4& mat, int row, int col) {
	return mat[row][col];
}
*/

void  fmat4_fmat4_mult (fmat4 *a, fmat4 *b, fmat4 *c) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			(*c)[i][j] = 0;
			for (int k = 0; k < 4; k++)	(*c)[i][j] += (*a)[i][k] * (*b)[k][j];
		}
	}
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
			int sign = ((i + j) % 2) ? -1 : 1; // TBD check sign
			(*out)[i][j] = det3x3 (&minor) * sign;
			/*printf ("i=%d j=%d ", i, j);
			print_fmat3 (&minor, "Minor: ");
			printf ("cof: %f\n\n", (*out)[i][j]);*/
		}
	}
	//print_fmat4 (out, "Cof mtx: ");
}

void fmat4_adjugate (fmat4 *in, fmat4 *out) {
	// adjoint of M = transposed cofactor of M
	fmat4 cof;
	fmat4_cofactor (in, &cof);
	fmat4_transpose (&cof, out);
	//print_fmat4 (out, "Adj: ");
}

void fmat4_invert (fmat4 *in, fmat4 *out) {
	// inverse of M = adjoint of M / det M
	fmat4 adj;
	fmat4_adjugate(in, &adj);
	float det = 0.0f;
	fmat3 minor;
	for (int i = 0; i < 4; i++) {
		fmat4_get_minor (&adj, 0, i, &minor);
		det += adj[0][i] * det3x3 (&minor) * ((i % 2) ? -1 : 1); // TBD check sign
	}
	//printf ("Det is %f\n", det);
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
/*
float float3_int3_smult (float3 &a, int3 &b) {
	float smult = 0;
	for (int i = 0; i < 3; i++ ) smult += a[i] * (float) b[i];
	return smult;
}

int int3_int3_smult (int3 &a, int3 &b) {
	int smult = 0;
	for (int i = 0; i < 3; i++ ) smult += a[i] * b[i];
	return smult;
}
*/
void float3_float3_crossprod(float3 *a, float3 *b, float3 *c) {
	(*c)[0] = (*a)[1] * (*b)[2] - (*a)[2] * (*b)[1];
	(*c)[1] = (*a)[2] * (*b)[0] - (*a)[0] * (*b)[2]; 
	(*c)[2] = (*a)[0] * (*b)[1] - (*a)[1] * (*b)[0];
}

void float3_normalize (float3 *v) {
	float length = (float) sqrt(pow((*v)[0], 2) + pow((*v)[1], 2) + pow((*v)[2], 2));
	for (int i = 0; i < 3; i++) (*v)[i] = (*v)[i] / length;
}

void float3_float4_conv (float3 *in, float4 *out) {
	for (int k = 0; k < 4; k++)
		(*out)[k] = (k < 3) ? (*in)[k] : 1.0f;
}

void float4_float3_conv (float4 *in, float3 *out) {
	for (int k = 0; k < 3; k++)
		(*out)[k] = (*in)[k] / (*in)[3];
}
