#pragma once

#include "shader.h"

#define PHONG_VSHADER_DEBUG 1

#define PHONG_PSHADER_DEBUG_0 0
#define PHONG_PSHADER_DEBUG_1 0

Float4 phong_vertex_shader (WFobj *obj, int face_idx, int vtx_idx, fmat4 *mvpv);
bool phong_pixel_shader  (WFobj *obj, Float3 *barw, pixel_color_t *color);
