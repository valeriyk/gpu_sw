#pragma once

#include "shader.h"

#define DEPTH_VSHADER1_DEBUG 0
#define DEPTH_PSHADER1_DEBUG 0
#define DEPTH_VSHADER2_DEBUG 0
#define DEPTH_PSHADER2_DEBUG 0

//#define NUM_OF_LIGHTS 2

Float4 depth_vshader_pass1 (Object *obj, int face_idx, int vtx_idx);
bool   depth_pshader_pass1  (WFobj *obj, Float3 *barw, pixel_color_t *color);

Float4 depth_vshader_pass2 (Object *obj, int face_idx, int vtx_idx);
bool   depth_pshader_pass2  (WFobj *obj, Float3 *barw, pixel_color_t *color);
