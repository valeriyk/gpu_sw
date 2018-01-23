#include <ushader_top.h>
#include <ushader_public.h>


int main (void) {
	gpu_cfg_t      *cfg      = (gpu_cfg_t *)      GPU_CFG_ABS_ADDRESS;
	
	pthread_cfg_t ushader_cfg;
	
	ushader_cfg.common_cfg      = NULL; // TBD
	ushader_cfg.hasha_block_ptr = NULL; // TBD
	
	ushader_top (&ushader_cfg);
	
	return 0;
}

