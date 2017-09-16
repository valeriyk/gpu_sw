#pragma once

#include <stdint.h>
#include <stdbool.h>

#define NUM_OF_VSHADERS 1
#define NUM_OF_PSHADERS 1
#define NUM_OF_USHADERS 0
#define MAX_NUM_OF_FRAMEBUFFERS	100

#define GPU_CFG_ABS_ADDRESS 0xFFFE0000


//#define SINGLEPROC_SINGLETHREAD
//#define SINGLEPROC_MULTITHREAD
//#define MULTIPROC

#ifdef SINGLEPROC_MULTITHREAD
	#define USE_PTHREAD
	#include <pthread.h>
#endif




typedef struct gpu_cfg_t {
	
	volatile void* volatile tile_idx_table_ptr;
	volatile void* volatile tri_data_array;
	volatile void* volatile obj_list_ptr;
	
	volatile void* volatile tri_for_vshader[NUM_OF_VSHADERS]; // N arrays of TriangleVShaderData structs
	volatile void* volatile tri_for_pshader[NUM_OF_PSHADERS]; // M arrays of TrianglePShaderData structs
	
	volatile void* volatile active_fbuffer;
	
	void* fbuffer_ptr[MAX_NUM_OF_FRAMEBUFFERS];
	void* zbuffer_ptr;
	void* vshader_ptr;
	void* pshader_ptr;
	
	void* lights_table_ptr;
	
	void* viewport_ptr;
		
	volatile bool vshaders_run_req;
	volatile bool vshaders_stop_req;
	volatile bool vshader_done[NUM_OF_VSHADERS];
	
	volatile bool pshaders_run_req;
	volatile bool pshaders_stop_req;
	volatile bool pshader_done[NUM_OF_PSHADERS];
	
	uint32_t num_of_vshaders;
	uint32_t num_of_pshaders;
	uint32_t num_of_ushaders;
	uint32_t num_of_tiles;
	uint32_t num_of_fbuffers;
	
	uint32_t screen_width;
	uint32_t screen_height;
	uint32_t screen_depth;
	
	uint32_t tile_width;
	uint32_t tile_height;
	
} gpu_cfg_t;


typedef struct shader_cfg_t {
	volatile gpu_cfg_t* common_cfg;
	uint32_t   shader_num;
} shader_cfg_t;
