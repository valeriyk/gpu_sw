#pragma once

#include <stdint.h>
#include <stdbool.h>

#define NUM_OF_VSHADERS 0
#define NUM_OF_PSHADERS 1
#define NUM_OF_USHADERS 0
#define MAX_NUM_OF_FRAMEBUFFERS	100

//#define PLATFORM_SCREEN_WIDTH  160//1280
//#define PLATFORM_SCREEN_HEIGHT 160//720

typedef struct platform_t {
	
	void *tile_idx_table_ptr;
	
	void *fbuffer_ptr[MAX_NUM_OF_FRAMEBUFFERS];
	void *active_fbuffer;
	void *zbuffer_ptr;
	void *vshader_ptr;
	void *pshader_ptr;
		
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
	
} platform_t;


typedef struct shader_platform_t {
	platform_t *platform;
	uint32_t    shader_num;
} shader_platform_t;
