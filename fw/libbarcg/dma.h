#pragma once

#ifdef TARGET_ARC
	#include <arc_dma.h>
#endif

#include <stdint.h>
#include <stdlib.h>

void dma_init () {

#ifdef TARGET_ARC
	arc_dma_init();
#endif

}

static inline void dma_memcpy (volatile void *volatile dst_ptr, volatile void *volatile src_ptr, size_t byte_size, size_t channel_num) {

	if (dst_ptr == NULL) return;
	
#ifdef TARGET_ARC
	arc_dma_mem2mem_single (dst_ptr, src_ptr, byte_size, channel_num);
#else
	
	for (size_t i = 0; i < byte_size / 4; i++) {
		*((uint32_t *) dst_ptr + i) = ((src_ptr == NULL) ? 0 : *((uint32_t *) src_ptr + i));
	}
	
	//~ void *dptr = dst_ptr;
	//~ void *sptr = src_ptr;
	
	//~ if (src_ptr == NULL) {
		
		//~ memset (dptr, 0, byte_size);
	//~ }
	//~ else {
		//~ memcpy (dptr, sptr, byte_size);
	//~ }

#endif

}

