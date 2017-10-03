#include "main.h"
//#include "gpu_cfg.h"

#include    <host_top.h>
#include <ushader_top.h>

#include <stdlib.h>
#include <stdio.h>
//#include <math.h>
//#include <time.h>


#define USE_PTHREAD
#include <pthread.h>

//gpu_cfg_t USHDR_CFG;

int main(int argc, char** argv) {
       
   	volatile gpu_cfg_t gpu_cfg;
	
	//gpu_cfg.tile_idx_table_ptr = NULL;
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		gpu_cfg.tri_ptr_list[i] = NULL;
		gpu_cfg.tri_for_pshader[i] = NULL;
		
		gpu_cfg.vshader_done[i] = false;
		gpu_cfg.pshader_done[i] = false;
	}
	
	for (int i = 0; i < GPU_MAX_FRAMEBUFFERS; i++) {
		gpu_cfg.fbuffer_ptr[i] = NULL;
	}
	
	gpu_cfg.zbuffer_ptr = NULL;
	
	//gpu_cfg.lights_arr = NULL; TBD
	
	fmat4_identity (&(gpu_cfg.viewport));
	
	gpu_cfg.vshader_fptr = NULL;
	gpu_cfg.pshader_fptr = NULL;
		
	gpu_cfg.vshaders_run_req  = false;
	gpu_cfg.vshaders_stop_req = false;
	
	gpu_cfg.pshaders_run_req  = false;
	gpu_cfg.pshaders_stop_req = false;
			
	gpu_cfg.num_of_ushaders = GPU_MAX_USHADERS;	
	gpu_cfg.num_of_tiles    = 0;
	gpu_cfg.num_of_fbuffers = GPU_MAX_FRAMEBUFFERS;
	
	
#ifdef USE_PTHREAD
	
	shader_cfg_t ushader_cfg[GPU_MAX_USHADERS];
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		ushader_cfg[i].common_cfg       = &gpu_cfg;
		ushader_cfg[i].shader_num       = i;
	}
	
	pthread_t host_thread;
	pthread_t ushader_thread [GPU_MAX_USHADERS];
	
	if (pthread_create (&host_thread, NULL, host_top, &gpu_cfg)) {
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
		
	if (pthread_join (host_thread, NULL)) {
		printf ("Error joining host_thread\n");
		return 2;
	}	
	for (int i = 0; i < GPU_MAX_USHADERS; i++) {
		if (pthread_join (ushader_thread[i], NULL)) {
			printf ("Error joining ushader_thread%d\n", i);
			return 2;	
		}
	}
	
#else

	host_top    (&gpu_cfg);
	
#endif

    return 0;
}
