#include "pthread_wrapper_pshader.h"

//#include "libbarcg.h"
//#include "platform.h"

BoundBox clip_boundbox_to_tile (BoundBox in, size_t tile_num);


void * pthread_wrapper_pshader (void *shader_platform) {
	
	shader_platform_t *p = shader_platform;
	uint32_t i = p->shader_num;
		
	if (PTHREAD_DEBUG0) {
		printf ("run pshader %d\n", i);
	}
	
	// TBD: copy zbuffer tile to local memory
	
	// TBD: copy fbuffer tile to local memory
	
	
	while (!p->platform->pshaders_stop_req) {	
		
		
		//for (int i = 0; i < p->platform->num_of_pshaders; i++) {
			
			if (PTHREAD_DEBUG) {
				printf("pshader%d: pshader_done[%d]=false\n", i, i);
			}
			p->platform->pshader_done[i] = false;
			
			if (PTHREAD_DEBUG) {
				printf("pshader%d: wait for pshader_run_req or pshader_stop_req\n", i);
			}
			while (!p->platform->pshaders_run_req && !p->platform->pshaders_stop_req);
			
			if (p->platform->pshaders_stop_req) break;
			
			if (PTHREAD_DEBUG) {
				printf("pshader%d: pshader_run_req detected\n", i);
			}
			
			/*
			int starting_tile = i % p->platform->num_of_pshaders;
			int incr_tile = p->platform->num_of_pshaders;
			*/
			int starting_tile = 1;
			int incr_tile     = 2;
			
			if (i == 0) {
					
				
				for (int j = starting_tile; j < p->platform->num_of_tiles; j += incr_tile) {
					
					printf ("%d ",j);
					
					//while (!p->platform->pshaders_run_req);
					
					TriangleListNode **tln = p->platform->tile_idx_table_ptr;
					TriangleListNode *node = tln[j];
					while (node != NULL) {
						TrianglePShaderData *tri = node->tri;
						draw_triangle (tri, j, (pixel_shader) p->platform->pshader_ptr, (screenz_t*) p->platform->zbuffer_ptr, (pixel_color_t*) p->platform->active_fbuffer, p->platform);
						node = node->next;
					}		
					
				}
				printf ("\n");
			}			
			
			if (PTHREAD_DEBUG) {
				printf("pshader%d: pshader_done[%d]=true\n", i, i);
			}
			p->platform->pshader_done[i] = true;
			
			while (p->platform->pshaders_run_req);	
		//}
	}	
	return NULL;
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
void draw_triangle (TrianglePShaderData *tri, size_t tile_num, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer, platform_t *pf) {    
	if (GL_DEBUG_0) {
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
				size_t pix_num = p.x + p.y * pf->screen_width;
				
				/*if (zi > zbuffer[pix_num]) {
					zbuffer[pix_num] = zi;

					Varying vry_interp = interpolate_varying (tri->varying, tri->w_reciprocal, &bar);
					
					pixel_color_t color;
					if (pshader (tri->obj, &vry_interp, &color) && (fbuffer != NULL)) {
						fbuffer[p.x + (pf->screen_height - p.y - 1) * pf->screen_width] = color;
						//fbuffer[p.x + (pf->screen_height - p.y - 1) * pf->screen_width] = set_color (200, 0, 0, 0);
					}
				}*/
				fbuffer[p.x + (pf->screen_height - p.y - 1) * pf->screen_width] = set_color (200, 0, 0, 0);
			}
			bar = FixPt3_FixPt3_add (bar, bar_col_incr);
        }
        bar_row = FixPt3_FixPt3_add (bar_row, bar_row_incr);
    }
}

