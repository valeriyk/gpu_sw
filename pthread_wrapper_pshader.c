#include "pthread_wrapper_pshader.h"

#include <string.h>

//#include "libbarcg.h"
//#include "platform.h"

Varying   interpolate_varying (Varying vry[3], fixpt_t w_reciprocal[3], FixPt3 *bar);
 dfixpt_t interpolate_w       (                fixpt_t w_reciprocal[3], FixPt3 *bar);
screenz_t interpolate_z       (                fixpt_t            z[3], FixPt3 *bar);
 

screenz_t interpolate_z (fixpt_t z[3], FixPt3 *bar) {
	
	dfixpt_t mul_0 = (int64_t) z[0] * (int64_t) bar->as_array[0]; // ZM.ZF * BM.BF = (ZM+BM).(ZF+BF)
	dfixpt_t mul_1 = (int64_t) z[1] * (int64_t) bar->as_array[1]; // ZM.ZF * BM.BF = (ZM+BM).(ZF+BF)
	dfixpt_t mul_2 = (int64_t) z[2] * (int64_t) bar->as_array[2]; // ZM.ZF * BM.BF = (ZM+BM).(ZF+BF)
	dfixpt_t acc = mul_0 + mul_1 + mul_2; // (ZM+BM).(ZF+BF) + (ZM+BM).(ZF+BF) + (ZM+BM).(ZF+BF) = (ZM+BM+1).(ZF+BF)
	dfixpt_t sum_of_bars = bar->as_array[0] + bar->as_array[1] + bar->as_array[2]; // BM.BF + BM.BF + BM.BF = (BM+1).BF
	dfixpt_t res = acc / sum_of_bars; // (ZM+BM+1).(ZF+BF) / (BM+1).BF = ZM.ZF
	
	return fixpt_to_screenz ((fixpt_t) res); // ZM
}

dfixpt_t interpolate_w (fixpt_t w_reciprocal[3], FixPt3 *bar) {
    	
	dfixpt_t mul_0 = (dfixpt_t) bar->as_array[0] * (dfixpt_t) w_reciprocal[0]; // BM.BF * WM.WF = (BM+WM).(BF+WF)
	dfixpt_t mul_1 = (dfixpt_t) bar->as_array[1] * (dfixpt_t) w_reciprocal[1]; // BM.BF * WM.WF = (BM+WM).(BF+WF)
	dfixpt_t mul_2 = (dfixpt_t) bar->as_array[2] * (dfixpt_t) w_reciprocal[2]; // BM.BF * WM.WF = (BM+WM).(BF+WF)
	dfixpt_t acc   = mul_0 + mul_1 + mul_2; // (BM+WM).(BF+WF) + (BM+WM).(BF+WF) + (BM+WM).(BF+WF) = (BM+WM+1).(BF+WF)
	assert (acc != 0);
	dfixpt_t one = (1LL << (OOWI_FRACT_BITS + BARC_FRACT_BITS + W_RECIPR_FRACT_BITS)); // 1.(WI+BF+WF)
	dfixpt_t res = one / acc; // 1.(WI+BF+WF) / (BM+WM+1).(BF+WF) = (64-WI).WI

	if (DEBUG_FIXPT_W) {
		float   mul_0f = fixpt_to_float (bar->as_array[0], BARC_FRACT_BITS) * fixpt_to_float (w_reciprocal[0], W_RECIPR_FRACT_BITS);
		float   mul_1f = fixpt_to_float (bar->as_array[1], BARC_FRACT_BITS) * fixpt_to_float (w_reciprocal[1], W_RECIPR_FRACT_BITS);
		float   mul_2f = fixpt_to_float (bar->as_array[2], BARC_FRACT_BITS) * fixpt_to_float (w_reciprocal[2], W_RECIPR_FRACT_BITS);
		float   accf  = mul_0f + mul_1f + mul_2f;
		float   resf = 1.0f / accf;
		
		printf ("mul_0 %f/%f\t", mul_0f, dfixpt_to_float (mul_0, BARC_FRACT_BITS + W_RECIPR_FRACT_BITS));
		printf ("mul_1 %f/%f\t", mul_1f, dfixpt_to_float (mul_1, BARC_FRACT_BITS + W_RECIPR_FRACT_BITS));
		printf ("mul_2 %f/%f\t", mul_2f, dfixpt_to_float (mul_2, BARC_FRACT_BITS + W_RECIPR_FRACT_BITS));
		printf ("acc %f/%f\t", accf, dfixpt_to_float (acc, BARC_FRACT_BITS + W_RECIPR_FRACT_BITS));
		printf ("res %f/%f\n", resf, dfixpt_to_float (res, OOWI_FRACT_BITS));
	}
	
	return (dfixpt_t) res;		
}


Varying interpolate_varying (Varying vry[3], fixpt_t w_reciprocal[3], FixPt3 *bar) {

	assert (vry[0].num_of_words == vry[1].num_of_words);
	assert (vry[0].num_of_words == vry[2].num_of_words);
					
	Varying vry_interp;					
	vry_interp.num_of_words = vry[0].num_of_words;
	
	if (vry_interp.num_of_words > 0) {
		
		dfixpt_t one_over_wi = interpolate_w (w_reciprocal, bar);
		
		for (int i = 0; i < vry_interp.num_of_words; i++) {
			
			#define NNN 20
						
			dfixpt_t vtx0_norm_fixpt = (dfixpt_t) vry[0].data[i].as_fixpt_t * (dfixpt_t) w_reciprocal[0];  // 18.14 * 3.29 = 21.43
			dfixpt_t vtx1_norm_fixpt = (dfixpt_t) vry[1].data[i].as_fixpt_t * (dfixpt_t) w_reciprocal[1];  // 
			dfixpt_t vtx2_norm_fixpt = (dfixpt_t) vry[2].data[i].as_fixpt_t * (dfixpt_t) w_reciprocal[2];  // 

			dfixpt_t mpy0_fixpt = (vtx0_norm_fixpt >> NNN) * (dfixpt_t) bar->as_array[0]; // (21.43>>16) * 24.8 = 21.27 * 24.8 = 29.35
			dfixpt_t mpy1_fixpt = (vtx1_norm_fixpt >> NNN) * (dfixpt_t) bar->as_array[1]; // 
			dfixpt_t mpy2_fixpt = (vtx2_norm_fixpt >> NNN) * (dfixpt_t) bar->as_array[2]; // 

			dfixpt_t acc_fixpt = mpy0_fixpt + mpy1_fixpt + mpy2_fixpt; // 29.35

			vry_interp.data[i].as_fixpt_t = (fixpt_t) ((acc_fixpt * (dfixpt_t) one_over_wi) >> (W_RECIPR_FRACT_BITS - NNN + BARC_FRACT_BITS + OOWI_FRACT_BITS)); // (29.35 * 16.16 = 13.51)>>43 = 56.8
			
			if (DEBUG_FIXPT_VARYING) {
				float vtx0_norm = vry[0].data[i].as_float * fixpt_to_float (w_reciprocal[0], W_RECIPR_FRACT_BITS);
				float vtx1_norm = vry[1].data[i].as_float * fixpt_to_float (w_reciprocal[1], W_RECIPR_FRACT_BITS);
				float vtx2_norm = vry[2].data[i].as_float * fixpt_to_float (w_reciprocal[2], W_RECIPR_FRACT_BITS);
				float mpy0 = vtx0_norm * fixpt_to_float (bar->as_array[0], BARC_FRACT_BITS);
				float mpy1 = vtx1_norm * fixpt_to_float (bar->as_array[1], BARC_FRACT_BITS);
				float mpy2 = vtx2_norm * fixpt_to_float (bar->as_array[2], BARC_FRACT_BITS);
				float acc = mpy0 + mpy1 + mpy2;
				vry_interp.data[i].as_float = acc * dfixpt_to_float (one_over_wi, OOWI_FRACT_BITS);
				
				printf ("vtx0_norm: %f/%f\t", vtx0_norm, dfixpt_to_float(vtx0_norm_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS));
				printf ("vtx1_norm: %f/%f\t", vtx1_norm, dfixpt_to_float(vtx1_norm_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS));
				printf ("vtx2_norm: %f/%f\n", vtx2_norm, dfixpt_to_float(vtx2_norm_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS));
				printf ("mpy0: %f/%f\t", mpy0, dfixpt_to_float(mpy0_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS + BARC_FRACT_BITS - NNN));
				printf ("mpy1: %f/%f\t", mpy1, dfixpt_to_float(mpy1_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS + BARC_FRACT_BITS - NNN));
				printf ("mpy2: %f/%f\n", mpy2, dfixpt_to_float(mpy2_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS + BARC_FRACT_BITS - NNN));
				printf ("acc: %f/%f\n", acc, dfixpt_to_float (acc_fixpt, VARYING_FRACT_BITS + W_RECIPR_FRACT_BITS + BARC_FRACT_BITS - NNN));
				printf ("vry interp: %f/%f\n", vry_interp.data[i].as_float, dfixpt_to_float (vry_interp.data[i].as_fixpt_t, VARYING_FRACT_BITS));
			}
		}
	}
	
	return vry_interp;					
}

void copy_tile_to_extmem (void *dst, void *src, gpu_cfg_t *cfg, size_t tile_num, size_t elem_size) {
	
	size_t tiles_in_row = cfg->screen_width / cfg->tile_width;
	size_t tile_row = tile_num / tiles_in_row;
	size_t tile_col = tile_num % tiles_in_row;
	
	size_t rows_in_tile = cfg->tile_height;
	
	size_t first_tile_row_offset = (tile_row * tiles_in_row * rows_in_tile) + tile_col;
	
	size_t tile_row_byte_size = cfg->tile_width * elem_size;
	
	size_t next_tile_row_offset;
	
	for (int i = 0; i < cfg->tile_height; i++) {
		next_tile_row_offset = i * tiles_in_row;
		memcpy (dst + ((first_tile_row_offset + next_tile_row_offset) * tile_row_byte_size), src + (i * tile_row_byte_size), tile_row_byte_size);
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
void draw_triangle (TrianglePShaderData *tri, size_t tile_num, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer, gpu_cfg_t *cfg) {    
	if (GL_DEBUG_0 == 1) {
		printf("\tcall draw_triangle()\n");
	}
	
	fixpt_t    x[3];
	fixpt_t    y[3];
	fixpt_t    z[3];
		
	// re-pack X, Y, Z coords of the three vertices
	for (int i = 0; i < 3; i++) {
		x[i] = tri->screen_coords[i].as_struct.x;
		y[i] = tri->screen_coords[i].as_struct.y;
		z[i] = tri->screen_coords[i].as_struct.z;
		
		assert (z[i] >= 0);
	}
	
	BoundBox bb = clip_boundbox_to_tile (get_tri_boundbox (x, y), tile_num);
	
	fixpt_t px = fixpt_from_screenxy (bb.min.x);
	fixpt_t py = fixpt_from_screenxy (bb.min.y);
	
	FixPt3 bar_initial = get_bar_coords (x, y, px, py);
	
	fixpt_t sum_of_2bars = fixpt_add (bar_initial.as_array[0], bar_initial.as_array[1]);
	fixpt_t sum_of_bars = fixpt_add (bar_initial.as_array[2], sum_of_2bars);
	if (sum_of_bars == 0) return;
	
	FixPt3 bar_row_incr;
	bar_row_incr.as_array[0] = fixpt_sub (x[2], x[1]) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	bar_row_incr.as_array[1] = fixpt_sub (x[0], x[2]) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	bar_row_incr.as_array[2] = fixpt_sub (x[1], x[0]) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	
	FixPt3 bar_col_incr;
    bar_col_incr.as_array[0] = fixpt_sub (y[1], y[2]) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	bar_col_incr.as_array[1] = fixpt_sub (y[2], y[0]) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	bar_col_incr.as_array[2] = fixpt_sub (y[0], y[1]) << (BARC_FRACT_BITS - XY_FRACT_BITS);
	
	
	FixPt3   bar;
	FixPt3   bar_row = bar_initial;
	ScreenPt p;
    for (p.y = bb.min.y; p.y <= bb.max.y; p.y++) {	
		
		bar = bar_row;
		
		for (p.x = bb.min.x; p.x <= bb.max.x; p.x++) {
			
			// If p is on or inside all edges, render pixel.
			if ((bar.as_array[0] > 0) && (bar.as_array[1] > 0) && (bar.as_array[2] > 0)) { // left-top fill rule
				
				screenz_t zi = interpolate_z (z, &bar);
				
				//size_t pix_num = p.x + p.y * cfg->tile_width;
				screenxy_t tile_x_offset = (tile_num % (cfg->screen_width/TILE_WIDTH)) * TILE_WIDTH;
				screenxy_t tile_y_offset = (tile_num / (cfg->screen_width/TILE_WIDTH)) * TILE_HEIGHT;
				screenxy_t tile_x = p.x - tile_x_offset;
				screenxy_t tile_y = p.y - tile_y_offset;
				size_t pix_num = tile_x + tile_y * TILE_WIDTH;
				
				
				if (zi > zbuffer[pix_num]) {
					zbuffer[pix_num] = zi;

					Varying vry_interp = interpolate_varying (tri->varying, tri->w_reciprocal, &bar);
														
					if (fbuffer != NULL) {
						pixel_color_t color;
						if (pshader (tri->obj, &vry_interp, &color)) {
							//fbuffer[p.x + (cfg->screen_height - p.y - 1) * cfg->screen_width] = color;
							fbuffer[pix_num] = color;
							//fbuffer[p.x + (cfg->screen_height - p.y - 1) * cfg->screen_width] = set_color (200, 0, 0, 0);
						}
					}
				}
				
			}
			bar = FixPt3_FixPt3_add (bar, bar_col_incr);
        }
        bar_row = FixPt3_FixPt3_add (bar_row, bar_row_incr);
    }
}


void * pthread_wrapper_pshader (void *cfg) {
	
	shader_cfg_t *shader_cfg = cfg;
	uint32_t shader_num = shader_cfg->shader_num;
		
	if (PTHREAD_DEBUG0) {
		printf ("run pshader %d\n", shader_num);
	}
	
	size_t elems_in_tile = TILE_WIDTH*TILE_HEIGHT;
				
	screenz_t     local_zbuf[elems_in_tile];
	pixel_color_t local_fbuf[elems_in_tile];
	
	size_t zbuf_tile_byte_size = elems_in_tile * sizeof (screenz_t);
	size_t fbuf_tile_byte_size = elems_in_tile * sizeof (pixel_color_t);
	
	while (!shader_cfg->common_cfg->pshaders_stop_req) {	
				
		if (PTHREAD_DEBUG) {
			printf("pshader%d: pshader_done=false\n", shader_num);
		}
		shader_cfg->common_cfg->pshader_done[shader_num] = false;
		
		if (PTHREAD_DEBUG) {
			printf("pshader%d: wait for pshader_run_req or pshader_stop_req\n", shader_num);
		}
		while (!shader_cfg->common_cfg->pshaders_run_req && !shader_cfg->common_cfg->pshaders_stop_req);
		
		if (shader_cfg->common_cfg->pshaders_stop_req) break;
		
		if (PTHREAD_DEBUG) {
			printf("pshader%d: pshader_run_req detected\n", shader_num);
		}
		
		
		int starting_tile = shader_num % shader_cfg->common_cfg->num_of_pshaders;
		int     incr_tile =              shader_cfg->common_cfg->num_of_pshaders;
		
		
		if (PSHADER_DEBUG) {
			printf ("\tpshader %d processes tiles: ", shader_num);
		}
		for (int j = starting_tile; j < shader_cfg->common_cfg->num_of_tiles; j += incr_tile) {
			
			// initialize zbuffer tile in local memory
			memset (&local_zbuf, 0, zbuf_tile_byte_size);
			
			// initialize fbuffer tile in local memory
			memset (&local_fbuf, 0, fbuf_tile_byte_size);
			
			if (PSHADER_DEBUG) printf ("%d ", j);
			
			TriangleListNode **tln = shader_cfg->common_cfg->tile_idx_table_ptr;
			TriangleListNode *node = tln[j];
			while (node != NULL) {
				TrianglePShaderData *tri = node->tri;
				draw_triangle (tri, j, (pixel_shader) shader_cfg->common_cfg->pshader_ptr, local_zbuf, local_fbuf, shader_cfg->common_cfg);

				node = node->next;
			}	
			
			// flush local zbuffer tile
			
			//pthread_mutex_lock (cfg->common_cfg->zbuf_mutex);
			if (shader_cfg->common_cfg->zbuffer_ptr != NULL) {
				copy_tile_to_extmem (shader_cfg->common_cfg->zbuffer_ptr, &local_zbuf, shader_cfg->common_cfg, j, sizeof (screenz_t));
			}
			//pthread_mutex_unlock (cfg->common_cfg->zbuf_mutex);
			
			// flush local fbuffer tile
			
			//pthread_mutex_lock (cfg->common_cfg->fbuf_mutex);
			if (shader_cfg->common_cfg->active_fbuffer != NULL) {
				copy_tile_to_extmem (shader_cfg->common_cfg->active_fbuffer, &local_fbuf, shader_cfg->common_cfg, j, sizeof (pixel_color_t));
			}
			//pthread_mutex_unlock (cfg->common_cfg->fbuf_mutex);
			
		}
		if (PSHADER_DEBUG) printf ("\n");
		
		if (PTHREAD_DEBUG) {
			printf("pshader%d: pshader_done=true\n", shader_num);
		}
		shader_cfg->common_cfg->pshader_done[shader_num] = true;
		
		while (shader_cfg->common_cfg->pshaders_run_req);	
	}	
	return NULL;
}
