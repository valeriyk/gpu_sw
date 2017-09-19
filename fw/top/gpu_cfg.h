#pragma once

//#include <gl.h>

#include <stdint.h>
#include <stdbool.h>


//#define SINGLEPROC_SINGLETHREAD
//#define SINGLEPROC_MULTITHREAD
//#define MULTIPROC

#ifdef SINGLEPROC_MULTITHREAD
	#define USE_PTHREAD
	#include <pthread.h>
#endif

