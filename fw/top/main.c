#include "main.h"
#include "gpu_cfg.h"

#include    <host_top.h>
#include <ushader_top.h>
#include <vshader_top.h>
#include <pshader_top.h>



#include <stdlib.h>
#include <stdio.h>
//#include <math.h>
//#include <time.h>





int main(int argc, char** argv) {
       
   	volatile gpu_cfg_t gpu_cfg;
	
	//gpu_cfg.tile_idx_table_ptr = NULL;
	for (int i = 0; i < NUM_OF_VSHADERS; i++) {
		gpu_cfg.tri_ptr_list[i] = NULL;
		gpu_cfg.tri_for_pshader[i] = NULL;
	}
	
	for (int i = 0; i < MAX_NUM_OF_FRAMEBUFFERS; i++) {
		gpu_cfg.fbuffer_ptr[i] = NULL;
	}
	
	gpu_cfg.zbuffer_ptr = NULL;
	
	gpu_cfg.lights_table_ptr = NULL;
	
	gpu_cfg.viewport_ptr = NULL;
	
	gpu_cfg.vshader_fptr = NULL;
	gpu_cfg.pshader_fptr = NULL;
		
	gpu_cfg.vshaders_run_req  = false;
	gpu_cfg.vshaders_stop_req = false;
	
	gpu_cfg.pshaders_run_req  = false;
	gpu_cfg.pshaders_stop_req = false;
	
	
	for (int i = 0; i < NUM_OF_VSHADERS; i++) {
		gpu_cfg.vshader_done[i] = false;
	}
	for (int i = 0; i < NUM_OF_PSHADERS; i++) {
		gpu_cfg.pshader_done[i] = false;
	}
		
	gpu_cfg.num_of_ushaders = NUM_OF_USHADERS;	
	gpu_cfg.num_of_vshaders = NUM_OF_VSHADERS;
	gpu_cfg.num_of_pshaders = NUM_OF_PSHADERS;
	gpu_cfg.num_of_tiles    = 0;
	gpu_cfg.num_of_fbuffers = MAX_NUM_OF_FRAMEBUFFERS;
	
	
#ifdef USE_PTHREAD
	
	shader_cfg_t vshader_cfg[NUM_OF_VSHADERS];
	for (int i = 0; i < NUM_OF_VSHADERS; i++) {
		vshader_cfg[i].common_cfg       = &gpu_cfg;
		vshader_cfg[i].shader_num       = i;
	}
	shader_cfg_t pshader_cfg[NUM_OF_PSHADERS];
	for (int i = 0; i < NUM_OF_PSHADERS; i++) {
		pshader_cfg[i].common_cfg       = &gpu_cfg;
		pshader_cfg[i].shader_num       = i;
	}
	shader_cfg_t ushader_cfg[NUM_OF_USHADERS];
	for (int i = 0; i < NUM_OF_USHADERS; i++) {
		ushader_cfg[i].common_cfg       = &gpu_cfg;
		ushader_cfg[i].shader_num       = i;
	}
	
	pthread_t host_thread;
	pthread_t vshader_thread [NUM_OF_VSHADERS];
	pthread_t pshader_thread [NUM_OF_PSHADERS];
	pthread_t ushader_thread [NUM_OF_USHADERS];
	
	if (pthread_create (&host_thread, NULL, host_top, &gpu_cfg)) {
		printf ("Error creating host_thread\n");
		return 2;	
	}
	
	if (gpu_cfg.num_of_ushaders == 0) {	
		for (int i = 0; i < NUM_OF_VSHADERS; i++) {
			if (pthread_create (&vshader_thread[i], NULL, vshader_top, &vshader_cfg[i])) {
				printf ("Error creating vshader_thread%d\n", i);
				return 2;	
			}
		}
		for (int i = 0; i < NUM_OF_PSHADERS; i++) {
			if (pthread_create (&pshader_thread[i], NULL, pshader_top, &pshader_cfg[i])) {
				printf ("Error creating pshader_thread%d\n", i);
				return 2;	
			}
		}
	}
	else {
		printf ("I am using unified shaders\n");
		for (int i = 0; i < NUM_OF_USHADERS; i++) {
			if (pthread_create (&ushader_thread[i], NULL, ushader_top, &ushader_cfg[i])) {
				printf ("Error creating ushader_thread%d\n", i);
				return 2;	
			}
		}
	}
			
	if (pthread_join (host_thread, NULL)) {
		printf ("Error joining host_thread\n");
		return 2;
	}	
	if (gpu_cfg.num_of_ushaders == 0) {
		for (int i = 0; i < NUM_OF_VSHADERS; i++) {
			if (pthread_join (vshader_thread[i], NULL)) {
				printf ("Error joining vshader_thread%d\n", i);
				return 2;	
			}
		}
		for (int i = 0; i < NUM_OF_PSHADERS; i++) {
			if (pthread_join (pshader_thread[i], NULL)) {
				printf ("Error joining pshader_thread%d\n", i);
				return 2;	
			}
		}
	}
	else {
		for (int i = 0; i < NUM_OF_USHADERS; i++) {
			if (pthread_join (ushader_thread[i], NULL)) {
				printf ("Error joining ushader_thread%d\n", i);
				return 2;	
			}
		}
	}
	
#else

	host_top    (&gpu_cfg);
	
#endif

    return 0;
}
