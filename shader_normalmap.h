#pragma once

#include "shader.h"

Float4 nm_vertex_shader (WFobj *obj, int face_idx, int vtx_idx, fmat4 *mvpv);
bool   nm_pixel_shader  (WFobj *obj, Float3 *barw, pixel_color_t *color);
