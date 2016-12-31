#pragma once 

#define WIDTH  512//1280
#define HEIGHT 512//720

#define NUM_OF_FRAMES 1
#define PRINTSCREEN_FRAME NUM_OF_FRAMES-1
#define NUM_OF_FRAMEBUFFERS	NUM_OF_FRAMES

#define ROTATION_INIT 1
#define ROTATION_INCR 2 * 3.141592f / NUM_OF_FRAMES; // 180 degree swing in radians 

#define ENABLE_PERF 1
#define RECORD_VIDEO 0
#define DEBUG_0 1

#define GL_DEBUG_0 0
#define GL_DEBUG_1 0
#define GL_DEBUG_2 0
#define GL_DEBUG_3 0
#define GL_DEBUG_4 0
