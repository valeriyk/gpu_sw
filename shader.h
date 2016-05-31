#pragma once

//#include "wavefront_obj.h"
//#include "geometry.h"
#include "gl.h"

extern fmat4  UNIFORM_M;
extern fmat4  UNIFORM_MI;
extern fmat4  UNIFORM_MIT;
extern float3 UNIFORM_LIGHT;

void my_vertex_shader (const WFobj *obj, const int face_idx, const int vtx_idx, const fmat4 *mvpv, float4 *vtx4d);
bool my_pixel_shader (const WFobj *obj, const float3 *barw, pixel_color_t *color);
