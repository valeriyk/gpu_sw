#pragma once

#include <stdint.h>

//typedef int32_t int2 [2];
typedef int32_t int3 [3];
//typedef int32_t int4 [4];

typedef float float2 [2];
typedef float float3 [3];
typedef float float4 [4];

typedef float fmat4 [4][4];

void  fmat4_set (      fmat4 &mat, const int row, const int col, const float val);
float fmat4_get (const fmat4 &mat, const int row, const int col                 );

void  fmat4_fmat4_mult  (const fmat4 &a, const  fmat4& b,  fmat4& c);
void  fmat4_float4_mult (const fmat4 &a, const float4& b, float4& c);

void  float3_float3_sub       (const float3 &a, const float3 &b, float3 &c);
float float3_float3_smult     (const float3 &a, const float3 &b);
float float3_int3_smult       (const float3 &a, const int3 &b);
int   int3_int3_smult         (const int3 &a,   const int3 &b);
void  float3_float3_crossprod (const float3 &a, const float3 &b, float3 &c);

void  float3_normalize (float3 &v);

void float3_float4_conv (const float3 &in, float4 &out);

void float4_float3_conv (const float4 &in, float3 &out);

