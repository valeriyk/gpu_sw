#include "ushader_top.h"

#include <vshader_loop.h>
#include <pshader_loop.h>

#include <inttypes.h>
#include <string.h>
#include <stdio.h>




#ifndef MULTIPROC
 void * ushader_top (void *cfg) {
	assert (cfg != NULL);
	
	volatile shader_cfg_t* volatile shader_cfg = cfg;
	uint32_t shader_num = shader_cfg->shader_num;
	volatile gpu_cfg_t *common_cfg = shader_cfg->common_cfg; 
#else
 int main (void) {
	uint32_t shader_num = (_lr(0x4) >> 8) & 0x000000ff; // ARC-specific code
	volatile gpu_cfg_t * volatile common_cfg = (volatile gpu_cfg_t *) GPU_CFG_ABS_ADDRESS;
#endif
	
	
	if (PTHREAD_DEBUG) printf("ushader%d: ushader up and running\n", shader_num);
	
	while (!(common_cfg->vshaders_stop_req || common_cfg->pshaders_stop_req)) {	
				
		if (PTHREAD_DEBUG) printf("vshader%d: vshader_done=false\n", shader_num);
		common_cfg->vshader_done[shader_num] = false;
		
		if (PTHREAD_DEBUG) printf("vshader%d: wait for vshader_run_req or vshader_stop_req\n", shader_num);
		while (!(common_cfg->vshaders_run_req || common_cfg->vshaders_stop_req));
		
		if (common_cfg->vshaders_stop_req) break;
		
		if (PTHREAD_DEBUG) printf("vshader%d: vshader_run_req detected\n", shader_num);
		
		vshader_loop (common_cfg, shader_num);		
		
		if (PTHREAD_DEBUG) printf("vshader%d: vshader_done=true\n", shader_num);
		common_cfg->vshader_done[shader_num] = true;
				
		while (common_cfg->vshaders_run_req);	
				
		if (PTHREAD_DEBUG) {
			printf("pshader%d: pshader_done=false\n", shader_num);
		}
		common_cfg->pshader_done[shader_num] = false;
		
		if (PTHREAD_DEBUG) {
			printf("pshader%d: wait for pshader_run_req or pshader_stop_req\n", shader_num);
		}
		while (!(common_cfg->pshaders_run_req || common_cfg->pshaders_stop_req));
		
		if (common_cfg->pshaders_stop_req) break;
		
		if (PTHREAD_DEBUG) {
			printf("pshader%d: pshader_run_req detected\n", shader_num);
		}
		
		pshader_loop (common_cfg, shader_num);
		
		if (PTHREAD_DEBUG) {
			printf("pshader%d: pshader_done=true\n", shader_num);
		}
		common_cfg->pshader_done[shader_num] = true;
				
		while (common_cfg->pshaders_run_req);	
	}
	
#ifndef MULTIPROC	
	return NULL;
#else
	return 0;
#endif
}
