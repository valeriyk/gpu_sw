//#include "main.h"
#include "platform.h"
#include <pthread_wrapper_host.h>
#include <pthread_wrapper_pshader.h>

#include <pthread.h>


#include <stdlib.h>
#include <stdio.h>
//#include <math.h>
//#include <time.h>



int main(int argc, char** argv) {
       
    platform_t platform;
	
	//platform.tile_idx_table_ptr = NULL;
	
	for (int i = 0; i < MAX_NUM_OF_FRAMEBUFFERS; i++) {
		platform.fbuffer_ptr[i] = NULL;
	}
	platform.zbuffer_ptr = NULL;
	platform.vshader_ptr = NULL;
	platform.pshader_ptr = NULL;
	
	platform.pshaders_run_req  = false;
	platform.pshaders_stop_req = false;
	for (int i = 0; i < NUM_OF_PSHADERS; i++) {
		platform.pshader_done[i] = false;
	}
	
	
	platform.num_of_vshaders = NUM_OF_VSHADERS;
	platform.num_of_pshaders = NUM_OF_PSHADERS;
	platform.num_of_ushaders = NUM_OF_USHADERS;	
	platform.num_of_tiles    = 0;
	platform.num_of_fbuffers = MAX_NUM_OF_FRAMEBUFFERS;
	
	printf ("num_of_fb: %d", platform.num_of_fbuffers);
		
	pthread_t host_thread;
	pthread_t pshader_thread [NUM_OF_PSHADERS];
	
	shader_platform_t pshader_platform[NUM_OF_PSHADERS];
	for (int i = 0; i < NUM_OF_PSHADERS; i++) {
		pshader_platform[i].platform   = &platform;
		pshader_platform[i].shader_num = i;
	}
	
	if (pthread_create (&host_thread, NULL, pthread_wrapper_host, &platform)) {
		printf ("Error creating host_thread\n");
		return 2;	
	}
	
	
	
	for (int i = 0; i < NUM_OF_PSHADERS; i++) {
		if (pthread_create (&pshader_thread[i], NULL, pthread_wrapper_pshader, &pshader_platform[i])) {
			printf ("Error creating pshader_thread%d\n", i);
			return 2;	
		}
	}
		
	if (pthread_join (host_thread, NULL)) {
		printf ("Error joining host_thread\n");
		return 2;
	}
	
	for (int i = 0; i < NUM_OF_PSHADERS; i++) {
		if (pthread_join (pshader_thread[i], NULL)) {
			printf ("Error joining pshader_thread%d\n", i);
			return 2;	
		}
	}
	
    return 0;
}
