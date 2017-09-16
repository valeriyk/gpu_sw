#pragma once

#include "gl.h"

Float4 vshader_gouraud (volatile Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry, const int vshader_idx, volatile gpu_cfg_t *cfg);
bool   pshader_gouraud (volatile Object *obj, volatile Varying *vry, pixel_color_t *color, volatile gpu_cfg_t *cfg);
