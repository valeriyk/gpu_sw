#include <videoctrl_top.h>
#include <ushader_public.h>


int main (void) {
	gpu_cfg_t      *cfg      = (gpu_cfg_t *)      GPU_CFG_ABS_ADDRESS;
	
	pthread_cfg_t videoctrl_cfg;
	
	videoctrl_cfg.common_cfg      = NULL; // TBD
	videoctrl_cfg.hasha_block_ptr = NULL; // TBD
	
	videoctrl_top (&videoctrl_cfg);
	
	return 0;
}

