#include "pthread_wrapper_pshader.h"

#include <string.h>

//#include "libbarcg.h"
//#include "platform.h"



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


void * pthread_wrapper_pshader (void *shader_cfg) {
	
	shader_cfg_t *cfg = shader_cfg;
	uint32_t shader_num = cfg->shader_num;
		
	if (PTHREAD_DEBUG0) {
		printf ("run pshader %d\n", shader_num);
	}
	
	size_t elems_in_tile = TILE_WIDTH*TILE_HEIGHT;
				
	screenz_t     local_zbuf[elems_in_tile];
	pixel_color_t local_fbuf[elems_in_tile];
	
	size_t zbuf_tile_byte_size = elems_in_tile * sizeof (screenz_t);
	size_t fbuf_tile_byte_size = elems_in_tile * sizeof (pixel_color_t);
	
	while (!cfg->common_cfg->pshaders_stop_req) {	
				
		if (PTHREAD_DEBUG) {
			printf("pshader%d: pshader_done=false\n", shader_num);
		}
		cfg->common_cfg->pshader_done[shader_num] = false;
		
		if (PTHREAD_DEBUG) {
			printf("pshader%d: wait for pshader_run_req or pshader_stop_req\n", shader_num);
		}
		while (!cfg->common_cfg->pshaders_run_req && !cfg->common_cfg->pshaders_stop_req);
		
		if (cfg->common_cfg->pshaders_stop_req) break;
		
		if (PTHREAD_DEBUG) {
			printf("pshader%d: pshader_run_req detected\n", shader_num);
		}
		
		
		int starting_tile = shader_num % cfg->common_cfg->num_of_pshaders;
		int     incr_tile = cfg->common_cfg->num_of_pshaders;
		
		//int starting_tile = 1;
		//int incr_tile     = 2;
		
		
		for (int j = starting_tile; j < cfg->common_cfg->num_of_tiles; j += incr_tile) {
			
			// initialize zbuffer tile in local memory
			memset (&local_zbuf, 0, zbuf_tile_byte_size);
			
			// initialize fbuffer tile in local memory
			memset (&local_fbuf, 0, fbuf_tile_byte_size);
			
			printf ("%d ",j);
			
			TriangleListNode **tln = cfg->common_cfg->tile_idx_table_ptr;
			TriangleListNode *node = tln[j];
			while (node != NULL) {
				TrianglePShaderData *tri = node->tri;
				draw_triangle (tri, j, (pixel_shader) cfg->common_cfg->pshader_ptr, local_zbuf, local_fbuf, cfg->common_cfg);

				node = node->next;
			}	
			
			// flush local zbuffer tile
			
			//pthread_mutex_lock (cfg->common_cfg->zbuf_mutex);
			copy_tile_to_extmem (cfg->common_cfg->zbuffer_ptr, &local_zbuf, cfg->common_cfg, j, sizeof (screenz_t));
			//pthread_mutex_unlock (cfg->common_cfg->zbuf_mutex);
			
			// flush local fbuffer tile
			
			//pthread_mutex_lock (cfg->common_cfg->fbuf_mutex);
			copy_tile_to_extmem (cfg->common_cfg->active_fbuffer, &local_fbuf, cfg->common_cfg, j, sizeof (pixel_color_t));
			//pthread_mutex_unlock (cfg->common_cfg->fbuf_mutex);
			
		}
		printf ("\n");
		
		if (PTHREAD_DEBUG) {
			printf("pshader%d: pshader_done=true\n", shader_num);
		}
		cfg->common_cfg->pshader_done[shader_num] = true;
		
		while (cfg->common_cfg->pshaders_run_req);	
	}	
	return NULL;
}
