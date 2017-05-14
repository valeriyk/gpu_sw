//#include "main.h"
#include "gpu_cfg.h"

#include    <host_wrapper.h>
#include <vshader_wrapper.h>
#include <pshader_wrapper.h>

#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
//#include <math.h>
//#include <time.h>


typedef enum {MAIN = 0, PTHREAD} model_type;


int main(int argc, char** argv) {
       
   	//model_type model = MAIN;
   	model_type model = PTHREAD;
	
	gpu_cfg_t gpu_cfg;
	
	gpu_cfg.tile_idx_table_ptr = NULL;
	for (int i = 0; i < MAX_NUM_OF_FRAMEBUFFERS; i++) {
		gpu_cfg.fbuffer_ptr[i] = NULL;
	}
	gpu_cfg.zbuffer_ptr = NULL;
	gpu_cfg.vshader_ptr = NULL;
	gpu_cfg.pshader_ptr = NULL;
	gpu_cfg.vshaders_run_req  = false;
	gpu_cfg.vshaders_stop_req = false;
	gpu_cfg.pshaders_run_req  = false;
	gpu_cfg.pshaders_stop_req = false;
	
	
	for (int i = 0; i < NUM_OF_PSHADERS; i++) {
		gpu_cfg.vshader_done[i] = false;
	}
	for (int i = 0; i < NUM_OF_PSHADERS; i++) {
		gpu_cfg.pshader_done[i] = false;
	}
		
	gpu_cfg.num_of_vshaders = NUM_OF_VSHADERS;
	gpu_cfg.num_of_pshaders = NUM_OF_PSHADERS;
	gpu_cfg.num_of_ushaders = NUM_OF_USHADERS;	
	gpu_cfg.num_of_tiles    = 0;
	gpu_cfg.num_of_fbuffers = MAX_NUM_OF_FRAMEBUFFERS;
	//gpu_cfg.fbuf_mutex = &fbuf_mutex;
	//gpu_cfg.zbuf_mutex = &zbuf_mutex;
	
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
	
	
	if (model == MAIN) {
		//host_wrapper();
		//vshader_wrapper();
		//pshader_wrapper();
		
	}
	else if (model == PTHREAD) {
	
		pthread_t host_thread;
		pthread_t vshader_thread [NUM_OF_PSHADERS];
		pthread_t pshader_thread [NUM_OF_PSHADERS];
		
		//pthread_mutex_t fbuf_mutex;
		//pthread_mutex_t zbuf_mutex;
		pthread_mutex_t cfg_mutex;
		gpu_cfg.mutex = &cfg_mutex;
		
		//pthread_mutex_init (&fbuf_mutex, NULL);
		//pthread_mutex_init (&zbuf_mutex, NULL);
		pthread_mutex_init (gpu_cfg.mutex, NULL);
		
		if (pthread_create (&host_thread, NULL, pthread_wrapper_host, &gpu_cfg)) {
			printf ("Error creating host_thread\n");
			return 2;	
		}	
		for (int i = 0; i < NUM_OF_VSHADERS; i++) {
			if (pthread_create (&vshader_thread[i], NULL, vshader_wrapper, &vshader_cfg[i])) {
				printf ("Error creating vshader_thread%d\n", i);
				return 2;	
			}
		}
		for (int i = 0; i < NUM_OF_PSHADERS; i++) {
			if (pthread_create (&pshader_thread[i], NULL, pshader_wrapper, &pshader_cfg[i])) {
				printf ("Error creating pshader_thread%d\n", i);
				return 2;	
			}
		}
			
		if (pthread_join (host_thread, NULL)) {
			printf ("Error joining host_thread\n");
			return 2;
		}	
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
    return 0;
}
