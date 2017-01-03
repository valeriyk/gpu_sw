#pragma once

#include "gl.h"

#define DEPTH_VSHADER1_DEBUG 0
#define DEPTH_PSHADER1_DEBUG 0
#define DEPTH_VSHADER2_DEBUG 0
#define DEPTH_PSHADER2_DEBUG 0

//Float4 gouraud_vshader_pass1 (Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry);
//bool   gouraud_pshader_pass1 (Object *obj, Varying *vry, pixel_color_t *color);

//Float4 gouraud_vshader_pass2 (Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry);
//bool   gouraud_pshader_pass2 (Object *obj, Varying *vry, pixel_color_t *color);

Float4 gouraud_vshader (Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry);
bool   gouraud_pshader (Object *obj, Varying *vry, pixel_color_t *color);
