#pragma once

#include "gl.h"

Float4 vshader_gouraud (Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry);
bool   pshader_gouraud (Object *obj, Varying *vry, pixel_color_t *color);
