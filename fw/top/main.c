//#include "gpu_cfg.h"
#include <hasha.h>
#include "main.h"
//#include <math.h>
#include <host_top.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
//#include <time.h>
#include <ushader_top.h>
#include <videoctrl_top.h>



//gpu_cfg_t USHDR_CFG;

int main(int argc, char** argv) {
       
   	printf ("Constructing the system...\n");
    
    hasha_block_t host_cpu;
	hasha_block_t ushader[GPU_MAX_USHADERS];
	hasha_block_t videoctrl;
	
	hasha_init_block (&host_cpu,  "Host CPU",         DUMMY_ID,      HOST_MAX_HASHA_MST_LANES,      HOST_MAX_HASHA_SLV_LANES);
	hasha_init_block (&videoctrl, "Video Controller", DUMMY_ID, VIDEOCTRL_MAX_HASHA_MST_LANES, VIDEOCTRL_MAX_HASHA_SLV_LANES);
	for (size_t i = 0; i < GPU_MAX_USHADERS; i++) {
		hasha_init_block (&ushader[i], "Unified Shader", i, USHADER_MAX_HASHA_MST_LANES, USHADER_MAX_HASHA_SLV_LANES);
	}
	
	hasha_link_blocks (&host_cpu,   HASHA_HOST_TO_USHADER_MST,  &ushader[0], HASHA_USHADER_UPSTREAM_SLV);
	hasha_link_blocks (&ushader[0], HASHA_USHADER_UPSTREAM_MST, &host_cpu,   HASHA_HOST_TO_USHADER_SLV);
	for (size_t i = 1; i < GPU_MAX_USHADERS; i++) {
		hasha_link_blocks (&ushader[i-1], HASHA_USHADER_DOWNSTREAM_MST, &ushader[i],   HASHA_USHADER_UPSTREAM_SLV);
		hasha_link_blocks (&ushader[i],   HASHA_USHADER_UPSTREAM_MST,   &ushader[i-1], HASHA_USHADER_DOWNSTREAM_SLV);
	}
	hasha_link_blocks (&host_cpu, HASHA_HOST_TO_VIDEOCTRL_MST, &videoctrl, HASHA_VIDEOCTRL_SLV);
	
	printf ("System constructed!\n");
	
	
	
	gpu_cfg_t      gpu_cfg;
   	gpu_run_halt_t gpu_run_halt; 
	
	//gpu_cfg.tile_idx_table_ptr = NULL;
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		gpu_cfg.tri_ptr_list[i] = NULL;
		gpu_cfg.tri_for_pshader[i] = NULL;
		
		gpu_run_halt.vshader_done[i] = false;
		gpu_run_halt.pshader_done[i] = false;
	}
	
	for (int i = 0; i < GPU_MAX_FRAMEBUFFERS; i++) {
		gpu_cfg.fbuffer_ptr[i] = NULL;
	}
	
	gpu_cfg.zbuffer_ptr = NULL;
	
	//gpu_cfg.lights_arr = NULL; TBD
	
	fmat4_identity (&(gpu_cfg.viewport));
	
	gpu_cfg.vshader_fptr = NULL;
	gpu_cfg.pshader_fptr = NULL;
		
	gpu_run_halt.vshaders_run_req  = false;
	gpu_run_halt.vshaders_stop_req = false;
	
	gpu_run_halt.pshaders_run_req  = false;
	gpu_run_halt.pshaders_stop_req = false;
			
	gpu_cfg.num_of_ushaders = GPU_MAX_USHADERS;	
	gpu_cfg.num_of_tiles    = 0;
	gpu_cfg.num_of_fbuffers = GPU_MAX_FRAMEBUFFERS;
	
		
	pthread_cfg_t host_cfg;
	host_cfg.common_cfg       = &gpu_cfg;
	host_cfg.gpu_run_halt     = &gpu_run_halt;
	host_cfg.core_num         = 255;
	host_cfg.hasha_block_ptr  = &host_cpu;
	
	pthread_cfg_t ushader_cfg[GPU_MAX_USHADERS];
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		ushader_cfg[i].common_cfg       = &gpu_cfg;
		ushader_cfg[i].gpu_run_halt     = &gpu_run_halt;
		ushader_cfg[i].core_num         = i;
		ushader_cfg[i].hasha_block_ptr  = &ushader[i];
	}
	
		
	pthread_cfg_t videoctrl_cfg;
	videoctrl_cfg.common_cfg       = &gpu_cfg;
	videoctrl_cfg.gpu_run_halt     = &gpu_run_halt;
	videoctrl_cfg.core_num         = 0;
	videoctrl_cfg.hasha_block_ptr  = &videoctrl;
	
	pthread_t host_thread;
	pthread_t ushader_thread [GPU_MAX_USHADERS];
	pthread_t videoctrl_thread;
	
	if (pthread_create (&host_thread, NULL, host_top, &host_cfg)) {
		printf ("Error creating host_thread\n");
		return 2;	
	}
	
	printf ("I am using unified shaders\n");
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		if (pthread_create (&ushader_thread[i], NULL, ushader_top, &ushader_cfg[i])) {
			printf ("Error creating ushader_thread%d\n", i);
			return 2;	
		}
	}
	
	if (pthread_create (&videoctrl_thread, NULL, videoctrl_top, &videoctrl_cfg)) {
		printf ("Error creating videoctrl_thread\n");
		return 2;	
	}
	
	
		
	if (pthread_join (host_thread, NULL)) {
		printf ("Error joining host_thread\n");
		return 2;
	}	
	/*for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		if (pthread_join (ushader_thread[i], NULL)) {
			printf ("Error joining ushader_thread%d\n", i);
			return 2;	
		}
	}*/
	
	// KILL REMAINING THREADS to emulate halt/reset
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		pthread_cancel (ushader_thread[i]);
	}
	pthread_cancel (videoctrl_thread);

    return 0;
}
