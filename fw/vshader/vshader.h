#pragma once

#include <gl.h>
#include <gpu_cfg.h>



void vshader_loop (volatile gpu_cfg_t *cfg, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer);
