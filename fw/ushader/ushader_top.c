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
	volatile gpu_cfg_t      *const common_cfg = (gpu_cfg_t *)      GPU_CFG_ABS_ADDRESS;
	volatile gpu_run_halt_t *const run_halt   = (gpu_run_halt_t *) GPU_RUN_HALT_ABS_ADDRESS;

#else

 void * ushader_top (void *ushader_cfg) {
	assert (ushader_cfg != NULL);
	
	pthread_cfg_t  *pthread_cfg = ushader_cfg;
    volatile gpu_cfg_t      *const common_cfg  = pthread_cfg->common_cfg;
    volatile gpu_run_halt_t *const run_halt    = pthread_cfg->gpu_run_halt;
    uint32_t        shader_num  = pthread_cfg->core_num;

	volatile hasha_block_t *const this_ptr = pthread_cfg->hasha_block_ptr; 
	
#endif
	
	
	if (PTHREAD_DEBUG) { printf("ushader%d: ushader up and running\n", shader_num); }
	
	gpu_cfg_t local_cfg;
	

#ifdef DMA

	_sr (0x1,  AUXR_DMACTRL); // enable DMA controller
	_sr (0xff, AUXR_DMACENB); // enable all channels
	_sr (0x1,  AUXR_DMACHPRI); // set channel 0 priority to high
	
#endif

	while (1) {
		
		hasha_wait_for_mst (this_ptr, HASHA_USHADER_UPSTREAM_SLV);
		hasha_notify_slv   (this_ptr, HASHA_USHADER_DOWNSTREAM_MST);

		local_cfg = *common_cfg;
		vshader_loop (&local_cfg, shader_num);
		
		hasha_wait_for_mst (this_ptr, HASHA_USHADER_DOWNSTREAM_SLV);
		hasha_notify_slv   (this_ptr, HASHA_USHADER_UPSTREAM_MST);
		
		////////////////////////////////////////////////////////////////
		
		hasha_wait_for_mst (this_ptr, HASHA_USHADER_UPSTREAM_SLV);
		hasha_notify_slv   (this_ptr, HASHA_USHADER_DOWNSTREAM_MST);
		
		local_cfg = *common_cfg;
		pshader_loop (&local_cfg, shader_num);

		hasha_wait_for_mst (this_ptr, HASHA_USHADER_DOWNSTREAM_SLV);
		hasha_notify_slv   (this_ptr, HASHA_USHADER_UPSTREAM_MST);
		
	}
	
#ifndef MULTIPROC	
	return NULL;
#else
	return 0;
#endif
}
