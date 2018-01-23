#include <host_top.h>
#include <ushader_public.h>


int main (void) {
	gpu_cfg_t      *cfg      = (gpu_cfg_t *)      GPU_CFG_ABS_ADDRESS;
	
	pthread_cfg_t host_cfg;
	
	host_cfg.common_cfg      = NULL; // TBD
	host_cfg.hasha_block_ptr = NULL; // TBD
	
	host_top (&host_cfg);
	
	return 0;
}
