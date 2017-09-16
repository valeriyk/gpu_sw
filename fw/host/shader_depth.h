#pragma once

#include "gl.h"

Float4 vshader_depth (Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry, const int vshader_idx, gpu_cfg_t *cfg);
bool   pshader_depth (Object *obj, Varying *vry, pixel_color_t *color, gpu_cfg_t *cfg);
