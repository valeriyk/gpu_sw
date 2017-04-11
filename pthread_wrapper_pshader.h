#pragma once

#include <platform.h>
#include <gl.h>


void * pthread_wrapper_pshader (void *shader_platform);

void draw_triangle (TrianglePShaderData *tri, size_t tile_num, pixel_shader shader, screenz_t *zbuffer, pixel_color_t *fbuffer, platform_t *pf);

