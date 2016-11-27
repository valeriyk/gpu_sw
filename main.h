#pragma once 

/*const int width  = 1280;
const int height = 720;
const int depth  = 1000000;//16000000;//30000;//65536;
*/
#define WIDTH  512//1280
#define HEIGHT 512//720
//#define DEPTH  65535//256
//const int SCREEN_SIZE[3] = {WIDTH, HEIGHT, DEPTH};

//#define NUM_OF_OBJECTS	4
#define NUM_OF_FRAMES 30
#define NUM_OF_FRAMEBUFFERS	2 // must be 2 for now

#define ROTATION_INIT 0
#define ROTATION_INCR 3.141592f / NUM_OF_FRAMES; // 180 degree swing in radians 



#define DEBUG_0 1

#define GL_DEBUG_0 0
#define GL_DEBUG_1 0
#define GL_DEBUG_2 0
#define GL_DEBUG_3 0
#define GL_DEBUG_4 0
