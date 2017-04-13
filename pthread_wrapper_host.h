#pragma once 

#define WIDTH  160//1280
#define HEIGHT 160//720

#define NUM_OF_FRAMES 1000


#define ROTATION_INIT 1
#define ROTATION_INCR 2 * 3.141592f / NUM_OF_FRAMES; // 180 degree swing in radians 

#define ENABLE_PERF 0
#define DEBUG_0 1

#define RECORD_VIDEO 1
#define RECORD_FRAME_NUM NUM_OF_FRAMES-1
//#define RECORD_FRAME_NUM 0

#define GL_DEBUG_0 0
#define GL_DEBUG_1 0
#define GL_DEBUG_2 0
#define GL_DEBUG_3 0
#define GL_DEBUG_4 0

#include <platform.h>
#include <gl.h>

void * pthread_wrapper_host (void *platform);

void draw_frame (platform_t *platform, ObjectListNode *obj_list, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer);

