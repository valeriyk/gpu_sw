#include "pshader_loop.h"

#include <inttypes.h>
#include <string.h>

#include <math.h>

#ifdef DMA
	#include <arcem_microdma.h>
#endif

#ifdef ARC_APEX
	#include <apexextensions.h>
#endif

//void interpolate_varying (Varying *vry, fixpt_t *w_reciprocal, FixPt3 *bar, Varying *vry_interp);
void interpolate_varying (Varying *vry, fixpt_t *w_reciprocal, fixpt_t *bar0, fixpt_t *bar1, fixpt_t *bar2, Varying *vry_interp);

void draw_triangle (TrianglePShaderData* tri, bbox_uhfixpt_t *tile_bb, screenz_t *zbuffer, pixel_color_t *fbuffer, gpu_cfg_t *cfg);
 

void copy_tiles_to_extmem (volatile void* volatile dst, volatile void* volatile src, gpu_cfg_t *cfg, size_t tile_num, size_t elem_size);
void copy_local_bufs_to_extmem (screenz_t *local_zbuf, pixel_color_t *local_fbuf, size_t tile_num, gpu_cfg_t *cfg);


//void interpolate_varying (Varying *vry, fixpt_t *w_reciprocal, FixPt3 *bar, Varying *vry_interp) {
void interpolate_varying (Varying *vry, fixpt_t *w_reciprocal, fixpt_t *bar0, fixpt_t *bar1, fixpt_t *bar2, Varying *vry_interp) {

	assert (vry[0].num_of_words_written == vry[1].num_of_words_written);
	assert (vry[0].num_of_words_written == vry[2].num_of_words_written);
					
	//vry_interp->num_of_words_written = (vry[0].num_of_words_written + vry[1].num_of_words_written + vry[2].num_of_words_written) / 3;
	vry_interp->num_of_words_written = (vry[0].num_of_words_written);
	vry_interp->num_of_words_read = 0;
	
	//vry_interp->num_of_words_read = vry[0].num_of_words_written;
	
	
#ifdef ARC_APEX

	//Varying tmp;
	// ARC APEX implementation
	if (vry_interp->num_of_words_written > 0) {
		
		//~ _core_write (w_reciprocal[0], CR_W_RCP0);
		//~ _core_write (w_reciprocal[1], CR_W_RCP1);
		//~ _core_write (w_reciprocal[2], CR_W_RCP2);
		//_core_write (bar->as_array[0], CR_BAR0);
		//_core_write (bar->as_array[1], CR_BAR1);
		//_core_write (bar->as_array[2], CR_BAR2);
		for (int i = 0; i < vry_interp->num_of_words_written; i++) {
			_core_write (vry[0].data[i].as_fixpt_t, CR_VRY0);
			_core_write (vry[1].data[i].as_fixpt_t, CR_VRY1);
			_core_write (vry[2].data[i].as_fixpt_t, CR_VRY2);
			vry_interp->data[i].as_fixpt_t = vry_ip(0);
			//tmp.data[i].as_fixpt_t = vry_ip(0);
		}
	}
	
#else
	//~ dfixpt_t bw0, bw1, bw2;
	//~ dfixpt_t vbw0, vbw1, vbw2;
	//~ dfixpt_t bw_acc;
	//~ dfixpt_t vbw_acc;
	if (vry_interp->num_of_words_written > 0) {

		dfixpt_t bw0, bw1, bw2;
		dfixpt_t vbw0, vbw1, vbw2;
		dfixpt_t bw_acc;
		dfixpt_t vbw_acc;
		
		//dfixpt_t one_over_wi = interpolate_w (w_reciprocal, bar); // = (1).(OF)
		bw0 = ((dfixpt_t) *bar0) * ((dfixpt_t) w_reciprocal[0]); // BM.BF * WM.WF = (BM+WM).(BF+WF)
		bw1 = ((dfixpt_t) *bar1) * ((dfixpt_t) w_reciprocal[1]); // BM.BF * WM.WF = (BM+WM).(BF+WF)
		bw2 = ((dfixpt_t) *bar2) * ((dfixpt_t) w_reciprocal[2]); // BM.BF * WM.WF = (BM+WM).(BF+WF)
		bw_acc   = bw0 + bw1 + bw2; // (BM+WM).(BF+WF) + (BM+WM).(BF+WF) + (BM+WM).(BF+WF) = (BM+WM+1).(BF+WF)

		///assert (acc != 0);
		///dfixpt_t one = (1LL << (OOWI_FRACT_BITS + BARC_FRACT_BITS + W_RECIPR_FRACT_BITS)); // 1.(OF+BF+WF)
		///dfixpt_t one_over_wi = one / acc; // 1.(OF+BF+WF) / (BM+WM+1).(BF+WF) = (1).(OF)
		
		//~ //another way of computing 1/x using Newton method:
		//~ fixpt_t acc2 = (fixpt_t) (acc >> 21); // acc = 27.37 -> acc2 = 16.16
		//~ fixpt_t xx = acc2; // 16.16
		//~ bool neg = false;
		//~ if (acc2 & 0x80000000) {
			//~ // negative num
			//~ neg = true;
			//~ xx = (~acc2) + 1;
		//~ }
		//~ uint32_t mask = 0x40000000;
		//~ for (int i = 1; i < 32; i++) {
			//~ if (xx & mask) {
				//~ xx = (1 << (i));
				//~ break;
			//~ }
			//~ else {
				//~ mask >>= 1;
			//~ }
		//~ }
		//~ if (neg) xx = (~xx) + 1;
		//~ //if      (acc2 & 0xfff80000) xx = 1 << 21; // x0 = 32
		//~ //else if (acc2 & 0x00060000) xx = 1 << 17; // x0 = 2
		//~ //else if (acc2 & 0x00018000) xx = 0; // x0 = 0
		//~ //else                                xx = 1 << 13; // x0 = 0.125
		
		//~ //xx = 1 << 13;
		//~ printf ("xx=%x\n", xx);
		//~ for (int i = 0; i < 10; i++) {
			//~ // x = x * (2 - b * x)
			//~ xx = (xx * ((1 << 17) - ((acc2 * xx) >> 16))) >> 16;
		//~ }
		//~ dfixpt_t yy = one_over_wi - (dfixpt_t) xx;
		//~ if (yy  > (1 << 13)) printf ("orig %f, newton %f\n", fixpt_to_float((fixpt_t) one_over_wi, 16), fixpt_to_float((fixpt_t) xx, 16)); //printf ("%d ", yy);
		

		for (int i = 0; i < vry_interp->num_of_words_written; i++) {
			
			#define NNN 20
			// VF = VARYING_FRACT_BITS
			// VM = 32-VF
			// WF = W_RECIPR_FRACT_BITS
			// WM = 32-WF
			// BF = BARC_FRACT_BITS
			// BM = 32-BF
			// OF = OOWI_FRACT_BITS
			vbw0 = (((dfixpt_t) vry[0].data[i].as_fixpt_t) >> 9) * (bw0 >> 13);  // VM.VF * WM.WF = (VM+WM).(VF+WF)
			vbw1 = (((dfixpt_t) vry[1].data[i].as_fixpt_t) >> 9) * (bw1 >> 13);  // VM.VF * WM.WF = (VM+WM).(VF+WF) 
			vbw2 = (((dfixpt_t) vry[2].data[i].as_fixpt_t) >> 9) * (bw2 >> 13);  // VM.VF * WM.WF = (VM+WM).(VF+WF)
			vbw_acc = vbw0 + vbw1 + vbw2; // = (VM+WM+BM+1).(VF+WF+BF-NNN)

			// ((VM+WM+BM+1).(VF+WF+BF-NNN) * ((1).(OF))) >> (WF+BF+OF-NNN) = ((VM+WM+BM+1+1-OF).(VF+WF+BF+OF-NNN) >> (WF+BF+OF-NNN)) = (VM+WM+BM+2-OF).VF
			//vry_interp->data[i].as_fixpt_t = (fixpt_t) ((acc_fixpt * one_over_wi) >> (W_RECIPR_FRACT_BITS + BARC_FRACT_BITS + OOWI_FRACT_BITS - NNN)); // = (VM+WM+BM+65-OF).VF
			
			// .31 / .37 = .14
			vry_interp->data[i].as_fixpt_t = (fixpt_t) (vbw_acc / (bw_acc >> 22)); // = (VM+WM+BM+65-OF).VF

			if (DEBUG_FIXPT_VARYING) {
				fixpt_t vry_interp_fixpt_tmp = vry_interp->data[i].as_fixpt_t;
				//~ fixpt_t vry_interp_newton = (fixpt_t) ((acc_fixpt * xx) >> (W_RECIPR_FRACT_BITS + BARC_FRACT_BITS + OOWI_FRACT_BITS - NNN));
				float vtx0_norm = vry[0].data[i].as_float * fixpt_to_float (w_reciprocal[0], W_RECIPR_FRACT_BITS);
				float vtx1_norm = vry[1].data[i].as_float * fixpt_to_float (w_reciprocal[1], W_RECIPR_FRACT_BITS);
				float vtx2_norm = vry[2].data[i].as_float * fixpt_to_float (w_reciprocal[2], W_RECIPR_FRACT_BITS);
				float mpy0 = vtx0_norm * fixpt_to_float (*bar0, BARC_FRACT_BITS);
				float mpy1 = vtx1_norm * fixpt_to_float (*bar1, BARC_FRACT_BITS);
				float mpy2 = vtx2_norm * fixpt_to_float (*bar2, BARC_FRACT_BITS);
				float acc = mpy0 + mpy1 + mpy2;
				//vry_interp->data[i].as_float = acc * dfixpt_to_float (one_over_wi, OOWI_FRACT_BITS);
				vry_interp->data[i].as_float = acc / dfixpt_to_float (bw_acc, BARC_FRACT_BITS);
			
				if (fabsf (vry_interp->data[i].as_float - fixpt_to_float (vry_interp_fixpt_tmp, VARYING_FRACT_BITS)) > 0.1) {
					printf ("\nvry interp mismatch: %f/%f\n", vry_interp->data[i].as_float,  fixpt_to_float (vry_interp_fixpt_tmp, VARYING_FRACT_BITS));	
				}
				//~ if (fabsf (vry_interp->data[i].as_float - fixpt_to_float (vry_interp_newton, VARYING_FRACT_BITS)) > 0.1) {
					//~ printf ("\nnewton vry interp mismatch: %f/%f\n", vry_interp->data[i].as_float,  fixpt_to_float (vry_interp_newton, VARYING_FRACT_BITS));	
				//~ }
			}
		}
	}
	
	//~ for (int i = 0; i < vry_interp->num_of_words_written; i++) {
		//~ if (vry_interp->data[i].as_fixpt_t != tmp.data[i].as_fixpt_t) {
			//~ printf ("\t--->MISMATCH %x %x\n", vry_interp->data[i].as_fixpt_t, tmp.data[i].as_fixpt_t);
			//~ printf ("\t\tGOLDEN:1: bar: %x %x %x; w_rcp: %x %x %x; vry: %x %x %x\n", bar->as_array[0], bar->as_array[1], bar->as_array[2], w_reciprocal[0], w_reciprocal[1], w_reciprocal[2], vry[0].data[i].as_fixpt_t, vry[1].data[i].as_fixpt_t, vry[2].data[i].as_fixpt_t);
			//~ printf ("\t\tGOLDEN:2: bw: %llx %llx %llx; bw_acc: %llx\n", bw0, bw1, bw2, bw_acc);
			//~ printf ("\t\tGOLDEN:3: vbw: %llx %llx %llx; vbw_acc: %llx\n", vbw0, vbw1, vbw2, vbw_acc);
			//~ printf ("\t\tGOLDEN:4: bw_acc>>22: %llx; div %llx\n", (bw_acc >> 22), vry_interp->data[i].as_fixpt_t);
		//~ }
	//~ }
	

#endif

}

void copy_tile_to_extmem (volatile void *volatile dst, volatile void *volatile src, gpu_cfg_t *cfg, size_t tile_num, size_t elem_size) {
	
	size_t tiles_in_row = cfg->screen_width >> GPU_TILE_WIDTH_LOG2;
	size_t tile_row = tile_num / tiles_in_row;
	size_t tile_col = tile_num % tiles_in_row;
	
	size_t rows_in_tile = GPU_TILE_HEIGHT;
	
	size_t first_tile_row_offset = (tile_row * tiles_in_row * rows_in_tile) + tile_col;
	
	size_t tile_row_byte_size = elem_size << GPU_TILE_WIDTH_LOG2;
	
	size_t next_tile_row_offset;
	
#ifndef DMA_TILES

	for (int i = 0; i < GPU_TILE_HEIGHT; i++) {
		next_tile_row_offset = i * tiles_in_row;
		memcpy ((void*) dst + ((first_tile_row_offset + next_tile_row_offset) * tile_row_byte_size), (void*) src + (i * tile_row_byte_size), tile_row_byte_size);
	}

#else

	_sr (0x1, AUXR_DMACTRL);
	_sr (0x1, AUXR_DMACENB);
	_sr (DMACTRLX_OP_SINGLE |
	       DMACTRLX_RT_AUTO |
	       DMACTRLX_DTT_MEM2MEM |
	       DMACTRLX_DW4_INCR4 |
	       DMACTRLX_BLOCK_SIZE(tile_row_byte_size) |
	       DMACTRLX_ARB_DISABLE |
	       DMACTRLX_IRQ_DISABLE |
	       DMACTRLX_AM_INCR_ALL,
	     AUXR_DMACTRL0);
	 
	for (int i = 0; i < GPU_TILE_HEIGHT; i++) {
		next_tile_row_offset = i * tiles_in_row;
		//memcpy ((void*) dst + ((first_tile_row_offset + next_tile_row_offset) * tile_row_byte_size), (void*) src + (i * tile_row_byte_size), tile_row_byte_size);
		void *src_ptr = (void*) src + (i * tile_row_byte_size);
		void *dst_ptr = (void*) dst + ((first_tile_row_offset + next_tile_row_offset) * tile_row_byte_size);
	
		_sr ((uint32_t) src_ptr + ((tile_row_byte_size - 1) & ~0x3) , AUXR_DMASAR0);
		_sr ((uint32_t) dst_ptr + ((tile_row_byte_size - 1) & ~0x3) , AUXR_DMADAR0);
		_sr (0x1, AUXR_DMACREQ);
		while (_lr(AUXR_DMACSTAT0) & 0x1); // wait till completion    
	}    
	        
    //~ bool sticky_error = false;
    //~ for (int i = 0; i < BUF_SIZE / 4; i++) {
        //~ if (*(iccm0_buf + i) != *(extmem_buf + i)) {
            //~ printf ("Mismatch in word %d\n", i);
            //~ sticky_error = true;
        //~ }
    //~ }
    
    //~ if (sticky_error == false) {
        //~ printf ("SUCCESS!\n");
    //~ }

#endif

}


void dma_mem2mem_single (volatile void *volatile dst_ptr, volatile void *volatile src_ptr, size_t byte_size) {
	
#ifndef DMA
	
	for (size_t i = 0; i < byte_size / 4; i++) {
		*((uint32_t *) dst_ptr + i) = *((uint32_t *) src_ptr + i);
	}
		
#else

	while (_lr(AUXR_DMACSTAT0) & 0x1); // wait till completion    
	
	_sr (0x1, AUXR_DMACTRL);
	_sr (0x1, AUXR_DMACENB);
	_sr (DMACTRLX_OP_SINGLE |
	       DMACTRLX_RT_AUTO |
	       DMACTRLX_DTT_MEM2MEM |
	       DMACTRLX_DW4_INCR4 |
	       DMACTRLX_BLOCK_SIZE(byte_size) |
	       DMACTRLX_ARB_DISABLE |
	       DMACTRLX_IRQ_DISABLE |
	       DMACTRLX_AM_INCR_ALL,
	     AUXR_DMACTRL0);
	 
	_sr ((uint32_t) src_ptr + ((byte_size - 1) & ~0x3) , AUXR_DMASAR0);
	_sr ((uint32_t) dst_ptr + ((byte_size - 1) & ~0x3) , AUXR_DMADAR0);
	_sr (0x1, AUXR_DMACREQ);
	
#endif

}

void copy_local_bufs_to_extmem (screenz_t *local_zbuf, pixel_color_t *local_fbuf, size_t tile_num, gpu_cfg_t *cfg) {
	
	// flush local zbuffer tile
	if (cfg->zbuffer_ptr != NULL) {
		copy_tile_to_extmem (cfg->zbuffer_ptr, local_zbuf, cfg, tile_num, sizeof (screenz_t));
	}
	
	// flush local fbuffer tile
	if (cfg->active_fbuffer != NULL) {
		copy_tile_to_extmem (cfg->active_fbuffer, local_fbuf, cfg, tile_num, sizeof (pixel_color_t));
	}
}
	
		
// Rasterize:
// 1. compute barycentric coordinates (bar0,bar1,bar2), don't normalize them
// 1.a. dumb method: just compute all the values for each pixel
// 1.b. smart method: compute values once per triangle, then just increment every
//      pixel and every row
// 2. interpolate values such as Z for every pixel:
// 2.a. dumb method: Z = (z0*bar0+z1*bar1+z2*bar2)/sum_of_bar
//      *divide by sum_of_bar because bar values are not normalized
// 2.b. smart method: Z = z0 + bar1*(z1-z0)/sum_of_bar + bar2*(z2-z0)/sum_of_bar
//      *can get rid of bar0
//      **(z1-z0)/sum_of_bar is constant for a triangle
//      ***(z2-z0)/sum_of_bar is constant for a triangle
//      See https://fgiesen.wordpress.com/2013/02/11/depth-buffers-done-quick-part/
//
void draw_triangle (TrianglePShaderData *local_tpd_ptr, bbox_uhfixpt_t *tile_bb, screenz_t *local_zbuf, pixel_color_t *local_fbuf, gpu_cfg_t *cfg) {    
	
	bbox_uhfixpt_t tri_bb = get_tri_bbox (local_tpd_ptr->vtx_a, local_tpd_ptr->vtx_b, local_tpd_ptr->vtx_c);
	bbox_uhfixpt_t bb = clip_bbox_to_tile (&tri_bb, tile_bb);
	
	uint32_t tile_bb_min_x = tile_bb->min.as_coord.x >> XY_FRACT_BITS;
	uint32_t tile_bb_min_y = tile_bb->min.as_coord.y >> XY_FRACT_BITS;
	
	uint32_t bb_min_x = (bb.min.as_coord.x >> XY_FRACT_BITS) - tile_bb_min_x;
	uint32_t bb_min_y = (bb.min.as_coord.y >> XY_FRACT_BITS) - tile_bb_min_y;
	uint32_t bb_max_x = (bb.max.as_coord.x >> XY_FRACT_BITS) - tile_bb_min_x;
	uint32_t bb_max_y = (bb.max.as_coord.y >> XY_FRACT_BITS) - tile_bb_min_y;

	

	//FixPt3   bar;
	//FixPt3   bar_row;
	fixpt_t  sum_of_bars = 0; // Q23.8 (1 sign + 23 integer + 8 fractional bits)
		
	//FixPt3 bar_initial = get_bar_coords2 (local_tpd_ptr->vtx_a, local_tpd_ptr->vtx_b, local_tpd_ptr->vtx_c, bb.min);

	fixpt_t bar_initial0;
	fixpt_t bar_initial1;
	fixpt_t bar_initial2;
	fixpt_t bar_row0;
	fixpt_t bar_row1;
	fixpt_t bar_row2;
	
	
#ifdef ARC_APEX
	_core_write (local_tpd_ptr->w_reciprocal[0], CR_W_RCP0);
	_core_write (local_tpd_ptr->w_reciprocal[1], CR_W_RCP1);
	_core_write (local_tpd_ptr->w_reciprocal[2], CR_W_RCP2);
		
		
	fixpt_t bar0 == CR_BAR0;
	fixpt_t bar1 == CR_BAR1;
	fixpt_t bar2 == CR_BAR2;
	_core_write (bb.min.as_word, CR_BAR_INIT_PT);
	bar_initial0 = edgefn (local_tpd_ptr->vtx_b.as_word, local_tpd_ptr->vtx_c.as_word); // not normalized
	bar_initial1 = edgefn (local_tpd_ptr->vtx_c.as_word, local_tpd_ptr->vtx_a.as_word); // not normalized
	bar_initial2 = edgefn (local_tpd_ptr->vtx_a.as_word, local_tpd_ptr->vtx_b.as_word); // not normalized
	
#else
    fixpt_t bar0;
	fixpt_t bar1;
	fixpt_t bar2;
    bar_initial0 = edge_func_fixpt2 (local_tpd_ptr->vtx_b, local_tpd_ptr->vtx_c, bb.min); // not normalized
	bar_initial1 = edge_func_fixpt2 (local_tpd_ptr->vtx_c, local_tpd_ptr->vtx_a, bb.min); // not normalized
	bar_initial2 = edge_func_fixpt2 (local_tpd_ptr->vtx_a, local_tpd_ptr->vtx_b, bb.min); // not normalized
	
#endif
	
	//~ for (int i = 0; i < 3; i++) {
		//~ sum_of_bars += bar_initial.as_array[i];
	//~ }
	sum_of_bars = bar_initial0 + bar_initial1 + bar_initial2;
	
	//bar_row = bar_initial;
	bar_row0 = bar_initial0;
	bar_row1 = bar_initial1;
	bar_row2 = bar_initial2;
	
	if (sum_of_bars == 0) return;
		
	//FixPt3 bar_row_incr;
	fixpt_t bar_row_incr0;
	fixpt_t bar_row_incr1;
	fixpt_t bar_row_incr2;
	// additional shift left is needed to align fractional width of barycentric coords (8 bits) and Z (4 bits)
	bar_row_incr0 = (local_tpd_ptr->vtx_c.as_coord.x - local_tpd_ptr->vtx_b.as_coord.x) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	bar_row_incr1 = (local_tpd_ptr->vtx_a.as_coord.x - local_tpd_ptr->vtx_c.as_coord.x) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	bar_row_incr2 = (local_tpd_ptr->vtx_b.as_coord.x - local_tpd_ptr->vtx_a.as_coord.x) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	
	//FixPt3 bar_col_incr;
	fixpt_t bar_col_incr0;
	fixpt_t bar_col_incr1;
	fixpt_t bar_col_incr2;
	// additional shift left is needed to align fractional width of barycentric coords (8 bits) and Z (4 bits)
    bar_col_incr0 = (local_tpd_ptr->vtx_b.as_coord.y - local_tpd_ptr->vtx_c.as_coord.y) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	bar_col_incr1 = (local_tpd_ptr->vtx_c.as_coord.y - local_tpd_ptr->vtx_a.as_coord.y) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	bar_col_incr2 = (local_tpd_ptr->vtx_a.as_coord.y - local_tpd_ptr->vtx_b.as_coord.y) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	

	// Left shift by 12: 4 bits to compensate for fractional width difference between Z (4 bits) and sum_of_bars (8 bits) plus
	//  8 bits to compensate for division precision loss.
	fixpt_t z1z0 = ((local_tpd_ptr->screen_z[1] - local_tpd_ptr->screen_z[0]) << 12) / sum_of_bars; // ((16.4 - 16.4) << 12) / 24.8 = 16.16 / 24.8 = 16.8
	fixpt_t z2z0 = ((local_tpd_ptr->screen_z[2] - local_tpd_ptr->screen_z[0]) << 12) / sum_of_bars;
				
	//xy_uhfixpt_pck_t p;
	uint32_t px;
	uint32_t py;
	//for (p.as_coord.y = bb.min.as_coord.y; p.as_coord.y <= bb.max.as_coord.y; p.as_coord.y += (1 << XY_FRACT_BITS)) {	
	for (py = bb_min_y; py <= bb_max_y; py++) {	
		
		//bar = bar_row;
		bar0 = bar_row0;
		bar1 = bar_row1;
		bar2 = bar_row2;
			
		for (px = bb_min_x; px <= bb_max_x; px++) {
					
			// If p is on or inside all edges, render pixel.
#ifdef ARC_APEX
			if ((_core_read_bar0() > 0) && (_core_read_bar1() > 0) && (_core_read_bar2() > 0)) { // left-top fill rule
#else
			if ((bar0 > 0) && (bar1 > 0) && (bar2 > 0)) { // left-top fill rule
#endif
			
				uint32_t pix_num = px + (py << GPU_TILE_WIDTH_LOG2);
				
				screenz_t zi = fixpt_to_screenz (
					local_tpd_ptr->screen_z[0] +
					((z1z0 * bar1) >> (BARC_FRACT_BITS*2 - Z_FRACT_BITS)) +
					((z2z0 * bar2) >> (BARC_FRACT_BITS*2 - Z_FRACT_BITS))
				); // 16.4 + (16.8 * 24.8 >> 12) + (16.8 * 24.8 >> 12) = 28.4
				
				if (zi > local_zbuf[pix_num]) {
						
					local_zbuf[pix_num] = zi;

					Varying vry_interp;
					interpolate_varying (local_tpd_ptr->varying, local_tpd_ptr->w_reciprocal, &bar0, &bar1, &bar2, &vry_interp);
					
					if (cfg->active_fbuffer != NULL) {
						
						pixel_color_t color;
						
						pixel_shader_fptr pshader_fptr = (pixel_shader_fptr) cfg->pshader_fptr;
						if (pshader_fptr (local_tpd_ptr->obj, &vry_interp, cfg, &color)) {
							//fbuffer[p.x + (cfg->screen_height - p.y - 1) * cfg->screen_width] = color;
							local_fbuf[pix_num] = color;
						}
					}
				}				
			}	
					
#ifdef ARC_APEX			

			_core_write (_core_read(CR_BAR0) + bar_col_incr0, CR_BAR0);
			_core_write (_core_read(CR_BAR1) + bar_col_incr1, CR_BAR1);
			_core_write (_core_read(CR_BAR2) + bar_col_incr2, CR_BAR2);
#else
			//bar = FixPt3_FixPt3_add (bar, bar_col_incr);
			bar0 += bar_col_incr0;
			bar1 += bar_col_incr1;
			bar2 += bar_col_incr2;
			
#endif
        }
        //bar_row = FixPt3_FixPt3_add (bar_row, bar_row_incr);
        bar_row0 += bar_row_incr0;
        bar_row1 += bar_row_incr1;
        bar_row2 += bar_row_incr2;
    }
}

void pshader_loop (gpu_cfg_t *cfg, const uint32_t shader_num) {
	
	size_t elems_in_tile = GPU_TILE_WIDTH * GPU_TILE_HEIGHT;
	
	screenz_t           local_zbuf[2][elems_in_tile];
	pixel_color_t       local_fbuf[2][elems_in_tile];
	TrianglePShaderData local_tri_data[2];
	
	size_t zbuf_tile_byte_size = elems_in_tile * sizeof (screenz_t);
	size_t fbuf_tile_byte_size = elems_in_tile * sizeof (pixel_color_t);
	
	size_t starting_tile  = shader_num % GPU_MAX_USHADERS;
	size_t     incr_tile  =              GPU_MAX_USHADERS;
	size_t   num_of_tiles =              cfg->num_of_tiles;
	
	size_t active_local_buf_idx = 0;
	for (size_t tile_num = starting_tile; tile_num < num_of_tiles; tile_num += incr_tile) {
		
		bbox_uhfixpt_t tile_bb = get_tile_bbox (tile_num, cfg);
		
		// initialize zbuffer tile in local memory
		memset (&local_zbuf[active_local_buf_idx], 0, zbuf_tile_byte_size);
		
		// initialize fbuffer tile in local memory
		memset (&local_fbuf[active_local_buf_idx], 0, fbuf_tile_byte_size);
		
		for (int i = 0; i < GPU_MAX_USHADERS; i++) {
			
			volatile TrianglePShaderData *ext_tri_data_ptr = cfg->tri_ptr_list[i][tile_num << GPU_MAX_TRIANGLES_PER_TILE_LOG2];
			if (ext_tri_data_ptr == NULL) continue;
			
			size_t active_ltd = 0;
			size_t inactive_ltd = 1;
			local_tri_data[active_ltd] = *ext_tri_data_ptr; // making a local copy
			dma_mem2mem_single (&local_tri_data[active_ltd], ext_tri_data_ptr, sizeof(TrianglePShaderData));
				
			for (int j = 0; j < GPU_MAX_TRIANGLES_PER_TILE; j++) {
				
				// prefetch:
				volatile TrianglePShaderData *ext_tri_data_nxt_ptr;
				if (j < GPU_MAX_TRIANGLES_PER_TILE - 1) {
					ext_tri_data_nxt_ptr = cfg->tri_ptr_list[i][(tile_num << GPU_MAX_TRIANGLES_PER_TILE_LOG2) + j + 1];
					if (ext_tri_data_nxt_ptr != NULL) {
						dma_mem2mem_single (&local_tri_data[inactive_ltd], ext_tri_data_nxt_ptr, sizeof(TrianglePShaderData));
					}
				}
				else {
					ext_tri_data_nxt_ptr = NULL;
				}
								
				draw_triangle (&local_tri_data[active_ltd], &tile_bb, local_zbuf[active_local_buf_idx], local_fbuf[active_local_buf_idx], cfg);
				
				  active_ltd = (  active_ltd == 0) ? 1 : 0;
				inactive_ltd = (inactive_ltd == 0) ? 1 : 0;
				
				if (ext_tri_data_nxt_ptr == NULL) break;
			}
		}
		
		copy_local_bufs_to_extmem (local_zbuf[active_local_buf_idx], local_fbuf[active_local_buf_idx], tile_num, cfg);
		
		active_local_buf_idx = (active_local_buf_idx == 0) ? 1 : 0;
	}
}
