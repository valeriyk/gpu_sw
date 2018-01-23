#include "host_top.h"

//#include <wavefront_obj.h>
//#include <geometry.h>

//#include "bitmap.h"
//#include <tga_addon.h>


#include "profiling.h"


//#include <platform.h>



#include <stdint.h>
//#include <limits.h>
#include <stdlib.h>
#include <math.h>

#include <time.h>

/*
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
const int JPEG_QUALITY = 100; 
enum {GRAY=1, GRAYA=2, RGB=3, RGBA=4};

// POSITIVE Z TOWARDS ME






uint8_t rgb_to_y (pixel_color_t rgb) {
	return (uint8_t) (16.f + rgb.as_byte.r * 0.257f + rgb.as_byte.g * 0.504f + rgb.as_byte.b * 0.098f);
}

uint8_t rgb_to_cb (pixel_color_t rgb) {
	return (uint8_t) (128.f - rgb.as_byte.r * 0.148f - rgb.as_byte.g * 0.291f + rgb.as_byte.b * 0.439f);
}

uint8_t rgb_to_cr (pixel_color_t rgb) {
	return (uint8_t) (128.f + rgb.as_byte.r * 0.439f - rgb.as_byte.g * 0.368f - rgb.as_byte.b * 0.071f);
}
*/


#ifdef MULTIPROC

 int main (void) {
	gpu_cfg_t      *cfg      = (gpu_cfg_t *)      GPU_CFG_ABS_ADDRESS;
	gpu_run_halt_t *run_halt = (gpu_run_halt_t *) GPU_RUN_HALT_ABS_ADDRESS;

#else

 void * videoctrl_top (void *videoctrl_cfg) { 	
	
	pthread_cfg_t  *pthread_cfg = videoctrl_cfg;
    gpu_cfg_t      *cfg      = pthread_cfg->common_cfg;
    gpu_run_halt_t *run_halt = pthread_cfg->gpu_run_halt;
    
    hasha_block_t *this_ptr = pthread_cfg->hasha_block_ptr; 
#endif

    
    
    
	printf ("Video Controller is up and running...\n"); 
	/*
	cfg->num_of_ushaders = GPU_MAX_USHADERS;	
	cfg->num_of_tiles    = 0;
	cfg->num_of_fbuffers = GPU_MAX_FRAMEBUFFERS;
	
    
    size_t screen_size = WIDTH * HEIGHT;
	
	if (RECORD_VIDEO) {		
		printf ("Recording video");
		FILE *fp = fopen ("video.y4m", "w");
		if (!fp) goto error;
		fprintf (fp, "YUV4MPEG2 W%d H%d F25:1 Ip A0:0 C444\n", WIDTH, HEIGHT);
		
		for (int i = 0; i < NUM_OF_FRAMES; i++) {
			printf (".");
			fprintf (fp, "FRAME\n");
			pixel_color_t *fb = cfg->fbuffer_ptr[i];
			for (int j = 0; j < screen_size; j++){
				uint8_t y = rgb_to_y (fb[j]);
				fwrite (&y, sizeof (uint8_t), 1, fp);
			}
			for (int j = 0; j < screen_size; j++){
				uint8_t cb = rgb_to_cb (fb[j]);
				fwrite (&cb, sizeof (uint8_t), 1, fp);
			}
			for (int j = 0; j < screen_size; j++){
				uint8_t cr = rgb_to_cr (fb[j]);
				fwrite (&cr, sizeof (uint8_t), 1, fp);
			}
		}
		printf ("\n");
		fclose (fp);
	}
	*/
		
#ifndef MULTIPROC
    return NULL;
#else
	return 0;
#endif

error:

#ifndef MULTIPROC
    return NULL;
#else
	return 1;
#endif
	
}
