#include "vshader_wrapper.h"
#include "vshader.h"

#include <inttypes.h>
#include <string.h>
#include <stdio.h>



void * vshader_wrapper (void *cfg) {
	
	assert (cfg != NULL);
	
	volatile shader_cfg_t* volatile shader_cfg = cfg;
	uint32_t shader_num = shader_cfg->shader_num;
	
	if (PTHREAD_DEBUG) printf("vshader%d: vshader up and running\n", shader_num);
	
	while (!shader_cfg->common_cfg->vshaders_stop_req) {	
				
		if (PTHREAD_DEBUG) printf("vshader%d: vshader_done=false\n", shader_num);
		shader_cfg->common_cfg->vshader_done[shader_num] = false;
		
		if (PTHREAD_DEBUG) printf("vshader%d: wait for vshader_run_req or vshader_stop_req\n", shader_num);
		while (!(shader_cfg->common_cfg->vshaders_run_req || shader_cfg->common_cfg->vshaders_stop_req));
		
		if (shader_cfg->common_cfg->vshaders_stop_req) break;
		
		if (PTHREAD_DEBUG) printf("vshader%d: vshader_run_req detected\n", shader_num);
		
		// vshader_loop (shader_cfg);
		//draw_frame (gpu_cfg_t *cfg, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer);
		draw_frame (shader_cfg->common_cfg, shader_cfg->common_cfg->vshader_ptr, shader_cfg->common_cfg->pshader_ptr, shader_cfg->common_cfg->zbuffer_ptr, shader_cfg->common_cfg->active_fbuffer);
		
		
		if (PTHREAD_DEBUG) printf("vshader%d: vshader_done=true\n", shader_num);
		shader_cfg->common_cfg->vshader_done[shader_num] = true;
				
		while (shader_cfg->common_cfg->vshaders_run_req);	
	}	
	return NULL;
}
