#pragma once 

#include <gl.h>

#define WIDTH  320//1280
#define HEIGHT 320

#define NUM_OF_FRAMES 60


#define ROTATION_INIT 1
#define ROTATION_INCR 2 * 3.141592f / NUM_OF_FRAMES; // 180 degree swing in radians 

#define ENABLE_PERF 0

#define RECORD_VIDEO 1
#define RECORD_FRAME_NUM NUM_OF_FRAMES-1
//#define RECORD_FRAME_NUM 5

void * host_top (void *platform);



