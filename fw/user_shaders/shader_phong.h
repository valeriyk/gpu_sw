#pragma once

#include <ushader_public.h>
//#include "gl.h"

Float4 vshader_phong (Object *obj, VtxAttr *attribs, Varying *vry, gpu_cfg_t *cfg);
bool   pshader_phong (Object *obj, Varying *vry, gpu_cfg_t *cfg, pixel_color_t *color);
