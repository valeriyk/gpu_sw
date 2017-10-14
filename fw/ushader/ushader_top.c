#include "ushader_public.h"
#include "ushader_top.h"
#include "vshader_loop.h"
#include "pshader_loop.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>




#ifdef MULTIPROC

 int main (void) {
	uint32_t shader_num = (_lr(0x4) >> 8) & 0x000000ff; // ARC-specific code
	//volatile gpu_cfg_t * volatile common_cfg = (volatile gpu_cfg_t *) GPU_CFG_ABS_ADDRESS;
	//~ gpu_cfg_t *const common_cfg = (gpu_cfg_t *) GPU_CFG_ABS_ADDRESS;
	volatile gpu_cfg_t      *const common_cfg = (gpu_cfg_t *)      GPU_CFG_ABS_ADDRESS;
	volatile gpu_run_halt_t *const run_halt   = (gpu_run_halt_t *) GPU_RUN_HALT_ABS_ADDRESS;

#else

 void * ushader_top (void *ushader_cfg) {
	assert (ushader_cfg != NULL);
	
	//~ shader_cfg_t *shader_cfg = cfg;
	//~ uint32_t shader_num = shader_cfg->shader_num;
	//~ gpu_cfg_t *common_cfg = shader_cfg->common_cfg; 
	
	pthread_cfg_t  *pthread_cfg = ushader_cfg;
    volatile gpu_cfg_t      *const common_cfg  = pthread_cfg->common_cfg;
    volatile gpu_run_halt_t *const run_halt    = pthread_cfg->gpu_run_halt;
    uint32_t        shader_num  = pthread_cfg->core_num;

#endif
	
	
	if (PTHREAD_DEBUG) { printf("ushader%d: ushader up and running\n", shader_num); }
	
	gpu_cfg_t local_cfg;
	

#ifdef DMA

	_sr (0x1,  AUXR_DMACTRL); // enable DMA controller
	_sr (0xff, AUXR_DMACENB); // enable all channels
	_sr (0x1,  AUXR_DMACHPRI); // set channel 0 priority to high
	
#endif

	
	while (!(run_halt->vshaders_stop_req || run_halt->pshaders_stop_req)) {	
				
		if (PTHREAD_DEBUG) { printf("vshader%d: vshader_done=false\n", shader_num); }
		run_halt->vshader_done[shader_num] = false;
		
		if (PTHREAD_DEBUG) { printf("vshader%d: wait for vshader_run_req or vshader_stop_req\n", shader_num); }
		while (!(run_halt->vshaders_run_req || run_halt->vshaders_stop_req));
		if (run_halt->vshaders_stop_req) break;
		if (PTHREAD_DEBUG) { printf("vshader%d: vshader_run_req detected\n", shader_num); }
		
		local_cfg = *common_cfg;
		vshader_loop (&local_cfg, shader_num);		
		
		if (PTHREAD_DEBUG) { printf("vshader%d: vshader_done=true\n", shader_num); }
		run_halt->vshader_done[shader_num] = true;
		while (run_halt->vshaders_run_req);	
				
		if (PTHREAD_DEBUG) { printf("pshader%d: pshader_done=false\n", shader_num); }
		run_halt->pshader_done[shader_num] = false;
		
		if (PTHREAD_DEBUG) { printf("pshader%d: wait for pshader_run_req or pshader_stop_req\n", shader_num); }
		while (!(run_halt->pshaders_run_req || run_halt->pshaders_stop_req));
		if (run_halt->pshaders_stop_req) break;
		if (PTHREAD_DEBUG) { printf("pshader%d: pshader_run_req detected\n", shader_num); }

		local_cfg = *common_cfg;
		pshader_loop (&local_cfg, shader_num);
		
		if (PTHREAD_DEBUG) { printf("pshader%d: pshader_done=true\n", shader_num); }
		run_halt->pshader_done[shader_num] = true;
		while (run_halt->pshaders_run_req);	
	}
	
#ifndef MULTIPROC	
	return NULL;
#else
	return 0;
#endif
}
