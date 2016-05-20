#include "geometry.h"
#include <math.h>


void fmat4_set (fmat4& mat, const int row, const int col, const float val) {
	mat[row][col] = val;
}

float fmat4_get (fmat4& mat, const int row, const int col) {
	return mat[row][col];
}

void  fmat4_fmat4_mult (const fmat4& a, const fmat4& b, fmat4& c) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			c[i][j] = 0;
			for (int k = 0; k < 4; k++)	c[i][j] += a[i][k]*b[k][j];
		}
	}
}

void  fmat4_float4_mult (const fmat4 &a, const float4 &b, float4 &c) {
	for (int i = 0; i < 4; i++) {
		c[i] = 0;
		for (int j = 0; j < 4; j++) c[i] += a[i][j]*b[j];
	}
}

void float3_float3_sub (const float3 &a, const float3 &b, float3 &c) {
	for (int i = 0; i < 3; i++) c[i] = a[i] - b[i];
}

float float3_float3_smult (const float3 &a, const float3 &b) {
	float smult = 0;
	for (int i = 0; i < 3; i++ ) smult += a[i]*b[i];
	return smult;
}

float float3_int3_smult (const float3 &a, const int3 &b) {
	float smult = 0;
	for (int i = 0; i < 3; i++ ) smult += a[i] * (float) b[i];
	return smult;
}

int int3_int3_smult (const int3 &a, const int3 &b) {
	int smult = 0;
	for (int i = 0; i < 3; i++ ) smult += a[i] * b[i];
	return smult;
}

void float3_float3_crossprod(const float3 &a, const float3 &b, float3 &c) {
	c[0] = a[1]*b[2] - a[2]*b[1];
	c[1] = a[2]*b[0] - a[0]*b[2]; 
	c[2] = a[0]*b[1] - a[1]*b[0];
}

void float3_normalize (float3 &v) {
	float length = (float) sqrt(pow(v[0], 2) + pow(v[1], 2) + pow(v[2], 2));
	for (int i = 0; i < 3; i++) v[i] = v[i] / length;
}

void float3_float4_conv (const float3 &in, float4 &out) {
	for (int k = 0; k < 4; k++)
		out[k] = (k < 3) ? in[k] : 1.0f;
}

void float4_float3_conv (const float4 &in, float3 &out) {
	for (int k = 0; k < 3; k++)
		out[k] = in[k] / in[3];
}
