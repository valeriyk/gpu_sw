#pragma once

#include <ushader_public.h>
//#include "gl.h"

Float4 vshader_fill_shadow_buf (Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry, gpu_cfg_t *cfg);
bool   pshader_fill_shadow_buf (Object *obj, Varying *vry, gpu_cfg_t *cfg, pixel_color_t *color);
