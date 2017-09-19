#pragma once

//#include <gl.h>

#include <stdint.h>
#include <stdbool.h>

#define NUM_OF_USHADERS 4
//#define NUM_OF_VSHADERS 1
//#define NUM_OF_PSHADERS 1
#define NUM_OF_VSHADERS NUM_OF_USHADERS
#define NUM_OF_PSHADERS NUM_OF_USHADERS


#define MAX_NUM_OF_FRAMEBUFFERS	100

#define GPU_CFG_ABS_ADDRESS 0xFFFE0000


//#define SINGLEPROC_SINGLETHREAD
//#define SINGLEPROC_MULTITHREAD
//#define MULTIPROC

#ifdef SINGLEPROC_MULTITHREAD
	#define USE_PTHREAD
	#include <pthread.h>
#endif

