#pragma once 

#define WIDTH  320//1280
#define HEIGHT 320//720

#define NUM_OF_FRAMES 50


#define ROTATION_INIT 1
#define ROTATION_INCR 2 * 3.141592f / NUM_OF_FRAMES; // 180 degree swing in radians 
//#define ROTATION_INCR 0.75 * 3.141592f;//NUM_OF_FRAMES; // 180 degree swing in radians 

#define ENABLE_PERF 0
#define DEBUG_0 1

#define RECORD_VIDEO 1
//#define RECORD_FRAME_NUM NUM_OF_FRAMES-1
#define RECORD_FRAME_NUM 5

#define GL_DEBUG_0 0
#define GL_DEBUG_1 0
#define GL_DEBUG_2 0
#define GL_DEBUG_3 0
#define GL_DEBUG_4 0

#include <gpu_cfg.h>
#include <gl.h>

void * host_top (void *platform);



