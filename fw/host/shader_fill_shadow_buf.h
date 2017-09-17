#pragma once

#include "gl.h"

#define DEPTH_VSHADER1_DEBUG 0
#define DEPTH_PSHADER1_DEBUG 0
#define DEPTH_VSHADER2_DEBUG 0
#define DEPTH_PSHADER2_DEBUG 0

Float4 vshader_fill_shadow_buf (volatile Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry, volatile gpu_cfg_t *cfg);
bool   pshader_fill_shadow_buf (volatile Object *obj, volatile Varying *vry, pixel_color_t *color, volatile gpu_cfg_t *cfg);
