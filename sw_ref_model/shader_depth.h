#pragma once

#include "gl.h"

Float4 vshader_depth (Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry);
bool   pshader_depth (Object *obj, Varying *vry, pixel_color_t *color);
