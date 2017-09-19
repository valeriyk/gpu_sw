#include "pshader_top.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>

//#include "libbarcg.h"
//#include "platform.h"


#ifndef MULTIPROC
 void * pshader_top (void *cfg) {
	
	shader_cfg_t *shader_cfg = cfg;
	uint32_t shader_num = shader_cfg->shader_num;
	volatile gpu_cfg_t *common_cfg = shader_cfg->common_cfg; 
#else
 int main (void) {
	uint32_t shader_num = (_lr(0x4) >> 8) & 0x000000ff; // ARC-specific code
	volatile gpu_cfg_t * volatile common_cfg = (volatile gpu_cfg_t *) GPU_CFG_ABS_ADDRESS;
#endif
	
	
		
	if (PTHREAD_DEBUG0) {
		printf ("run pshader %d\n", shader_num);
	}
	/*
	size_t elems_in_tile = TILE_WIDTH*TILE_HEIGHT;
				
	screenz_t     local_zbuf[elems_in_tile];
	pixel_color_t local_fbuf[elems_in_tile];
	
	size_t zbuf_tile_byte_size = elems_in_tile * sizeof (screenz_t);
	size_t fbuf_tile_byte_size = elems_in_tile * sizeof (pixel_color_t);
	*/
	
	while (!common_cfg->pshaders_stop_req) {	
				
		if (PTHREAD_DEBUG) {
			printf("pshader%d: pshader_done=false\n", shader_num);
		}
		//pthread_mutex_lock (common_cfg->mutex);
		common_cfg->pshader_done[shader_num] = false;
		//pthread_mutex_unlock (common_cfg->mutex);
		
		if (PTHREAD_DEBUG) {
			printf("pshader%d: wait for pshader_run_req or pshader_stop_req\n", shader_num);
		}
		while (!(common_cfg->pshaders_run_req || common_cfg->pshaders_stop_req));
		
		if (common_cfg->pshaders_stop_req) break;
		
		if (PTHREAD_DEBUG) {
			printf("pshader%d: pshader_run_req detected\n", shader_num);
		}
		
		/*if (shader_num == 1) {
			while (!common_cfg->pshader0_done);
		}*/
		//if ((shader_num % 2) == 1) {
		//if (shader_num > 0) {
			//while (!common_cfg->pshader_done[shader_num-1]);
			//pshader (shader_cfg);
		//}
		
		//pthread_mutex_lock (common_cfg->mutex);
		pshader_loop ((gpu_cfg_t *) common_cfg, shader_num);
		//pthread_mutex_unlock (common_cfg->mutex);
		
		
		
		
		if (PTHREAD_DEBUG) {
			printf("pshader%d: pshader_done=true\n", shader_num);
		}
		//pthread_mutex_lock (common_cfg->mutex);
		common_cfg->pshader_done[shader_num] = true;
		//pthread_mutex_unlock (common_cfg->mutex);
				
		while (common_cfg->pshaders_run_req);	
	}
#ifndef MULTIPROC	
	return NULL;
#else
	return 0;
#endif
}
