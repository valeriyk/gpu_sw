#pragma once

#include "gl.h"

extern fmat4  UNIFORM_M;
extern fmat4  UNIFORM_MI;
extern fmat4  UNIFORM_MIT;
extern float3 UNIFORM_LIGHT;

void my_vertex_shader (WFobj *obj, int face_idx, int vtx_idx, fmat4 *mvpv, float4 *vtx4d);
bool my_pixel_shader (WFobj *obj, float3 *barw, pixel_color_t *color);
