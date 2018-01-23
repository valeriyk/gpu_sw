/*#include "ushader_public.h"
#include "ushader_top.h"
#include "vshader_loop.h"
#include "pshader_loop.h"
*/

#ifdef ARC_APEX
	#include <apexextensions.h>
#endif
#include <dma.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "ushader_public.h"


void tiler (TrianglePShaderData *local_data_ptr, FixPt3 *screen_z, uint32_t vshader_idx, uint32_t tri_num, uint16_t *num_of_tris_in_tile_arr, gpu_cfg_t *cfg_ptr);

void tiler (TrianglePShaderData *local_data_ptr, FixPt3 *screen_z, uint32_t vshader_idx, uint32_t tri_num, uint16_t *num_of_tris_in_tile_arr, gpu_cfg_t *cfg_ptr) {
	
	//~ hfixpt_t x[3];
	//~ hfixpt_t y[3];
	// fixpt_t z[3];
	
	// re-pack X, Y coords of the three vertices, hopefully this will be optimized away by the compiler
	//~ for (size_t i = 0; i < 3; i++) {
		//~ x[i] = local_data_ptr->screen_x[i];
		//~ y[i] = local_data_ptr->screen_y[i];
	//~ }
	
	//~ x[0] = local_data_ptr->vtx_a.as_coord.x;
	//~ x[1] = local_data_ptr->vtx_b.as_coord.x;
	//~ x[2] = local_data_ptr->vtx_c.as_coord.x;
	//~ y[0] = local_data_ptr->vtx_a.as_coord.y;
	//~ y[1] = local_data_ptr->vtx_b.as_coord.y;
	//~ y[2] = local_data_ptr->vtx_c.as_coord.y;
	
	//BoundBox bb = clip_boundbox_to_screen (get_tri_boundbox (x, y), cfg_ptr);
	bbox_uhfixpt_t screen_bb = get_screen_bbox (cfg_ptr);
	bbox_uhfixpt_t bb = clip_tri_to_bbox (local_data_ptr->vtx_a, local_data_ptr->vtx_b, local_data_ptr->vtx_c, &screen_bb);
    
    //FixPt3 bar_init = get_bar_coords (x, y, bb.min.x, bb.min.y);
    fixpt_t bar_init0;
    fixpt_t bar_init1;
    fixpt_t bar_init2;
    
#ifdef ARC_APEX
		
	//~ fixpt_t bar0 == CR_BAR0;
	//~ fixpt_t bar1 == CR_BAR1;
	//~ fixpt_t bar2 == CR_BAR2;
	_core_write (bb.min.as_word, CR_BAR_INIT_PT);
	bar_init0 = edgefn (local_data_ptr->vtx_b.as_word, local_data_ptr->vtx_c.as_word); // not normalized
	bar_init1 = edgefn (local_data_ptr->vtx_c.as_word, local_data_ptr->vtx_a.as_word); // not normalized
	bar_init2 = edgefn (local_data_ptr->vtx_a.as_word, local_data_ptr->vtx_b.as_word); // not normalized
	
#else
    //~ fixpt_t bar0;
	//~ fixpt_t bar1;
	//~ fixpt_t bar2;
    bar_init0 = edge_func_fixpt2 (local_data_ptr->vtx_b, local_data_ptr->vtx_c, bb.min); // not normalized
	bar_init1 = edge_func_fixpt2 (local_data_ptr->vtx_c, local_data_ptr->vtx_a, bb.min); // not normalized
	bar_init2 = edge_func_fixpt2 (local_data_ptr->vtx_a, local_data_ptr->vtx_b, bb.min); // not normalized
	
#endif
    	
	//fixpt_t sob = bar_init.as_array[0] + bar_init.as_array[1] + bar_init.as_array[2];
	fixpt_t sob = bar_init0 + bar_init1 + bar_init2;
	
	if (sob == 0) return;
	
	local_data_ptr->z0            =   screen_z->as_array[0];
	local_data_ptr->z1z0_over_sob = ((screen_z->as_array[1] - screen_z->as_array[0]) << (BARC_FRACT_BITS*2 - Z_FRACT_BITS)) / sob;
	local_data_ptr->z2z0_over_sob = ((screen_z->as_array[2] - screen_z->as_array[0]) << (BARC_FRACT_BITS*2 - Z_FRACT_BITS)) / sob;

	// save local data to memory now, because I need its address later for storing it in tri_ptr_list 
	TrianglePShaderData *volatile ext_data_arr = cfg_ptr->tri_for_pshader[vshader_idx];
	//ext_data_arr[tri_num] = *local_data_ptr;
	dma_memcpy (&ext_data_arr[tri_num], local_data_ptr, sizeof (TrianglePShaderData), 0); // CH0 - high priority
	
	//TrianglePShaderData *volatile *ext_tri_ptr_arr = cfg_ptr->tri_ptr_list[vshader_idx];
	TriangleTileData *ext_tri_ptr_arr = cfg_ptr->tri_ptr_list[vshader_idx];
    
    ScreenPt p;

	//~ bb.min.as_coord.x &= ~(GPU_TILE_WIDTH-1); TBD
    //~ bb.min.as_coord.y &= ~(GPU_TILE_HEIGHT-1); TBD
	bb.min.as_word &= 0xfe00fe00;
	    
    //~ uint32_t bb_min_x = bb.min.as_coord.x >> XY_FRACT_BITS;
	//~ uint32_t bb_min_y = bb.min.as_coord.y >> XY_FRACT_BITS;
	//~ uint32_t bb_max_x = bb.max.as_coord.x >> XY_FRACT_BITS;
	//~ uint32_t bb_max_y = bb.max.as_coord.y >> XY_FRACT_BITS;
	
    // For every tile inside the bounding box of the triangle:
    //~ for (p.y = bb_min_y; p.y <= bb_max_y; p.y += GPU_TILE_HEIGHT) {	
		//~ for (p.x = bb_min_x; p.x <= bb_max_x; p.x += GPU_TILE_WIDTH) {
		
	hfixpt_t delta_x = (GPU_TILE_WIDTH << XY_FRACT_BITS);
	hfixpt_t delta_y = (GPU_TILE_HEIGHT << XY_FRACT_BITS);
	for (p.y = bb.min.as_coord.y; p.y <= bb.max.as_coord.y; p.y += delta_y) {	
		for (p.x = bb.min.as_coord.x; p.x <= bb.max.as_coord.x; p.x += delta_x) {


			// Evaluate barycentric coords in all the four corners of each tile:
			
			// find corners of the tile:
			//hfixpt_t x0 = hfixpt_from_screenxy (p.x);
			//hfixpt_t x1 = hfixpt_from_screenxy (p.x + GPU_TILE_WIDTH - 1);
			//hfixpt_t y0 = hfixpt_from_screenxy (p.y);
			//hfixpt_t y1 = hfixpt_from_screenxy (p.y + GPU_TILE_HEIGHT - 1);
			
			//~ // get barycentric coords in each corner:
			//~ FixPt3 b0 = get_bar_coords (x, y, x0, y0);
			//~ FixPt3 b1 = get_bar_coords (x, y, x0, y1);
			//~ FixPt3 b2 = get_bar_coords (x, y, x1, y0);
			//~ FixPt3 b3 = get_bar_coords (x, y, x1, y1);
			
			//~ bool tri_inside_tile = true; // sticky bit
			//~ for (int i = 0; i < 3; i++) {
				//~ // If barycentric coord "i" is negative in all four corners, triangle is outside the tile
				//~ // See here: http://forum.devmaster.net/t/advanced-rasterization/6145
				//~ if ((b0.as_array[i] & b1.as_array[i] & b2.as_array[i] & b3.as_array[i]) < 0) {
					//~ tri_inside_tile = false;
					//~ break;
				//~ }
			//~ }
			
			// If barycentric coord "i" is negative in all four corners, triangle is outside the tile
			// See explanation here: http://forum.devmaster.net/t/advanced-rasterization/6145
			
			xy_uhfixpt_pck_t ll;
			xy_uhfixpt_pck_t ul;
			xy_uhfixpt_pck_t lr;
			xy_uhfixpt_pck_t ur;
			
			ll.as_coord.x = p.x;
			ul.as_coord.x = p.x;
			lr.as_coord.x = p.x + delta_x - 1;
			ur.as_coord.x = p.x + delta_x - 1;

			ll.as_coord.y = p.y;
			ul.as_coord.y = p.y + delta_y - 1;
			lr.as_coord.y = p.y;
			ur.as_coord.y = p.y + delta_y - 1;
			
#ifdef ARC_APEX
		
			//~ _core_write (bb.min.as_word, CR_BAR_INIT_PT);
			//~ bar_init0 = edgefn (local_data_ptr->vtx_b.as_word, local_data_ptr->vtx_c.as_word); // not normalized
			//~ bar_init1 = edgefn (local_data_ptr->vtx_c.as_word, local_data_ptr->vtx_a.as_word); // not normalized
			//~ bar_init2 = edgefn (local_data_ptr->vtx_a.as_word, local_data_ptr->vtx_b.as_word); // not normalized
			
			_core_write (ll.as_word, CR_BAR_INIT_PT);
			fixpt_t bar_init0_ll = edgefn (local_data_ptr->vtx_b.as_word, local_data_ptr->vtx_c.as_word); // not normalized
			
			_core_write (ul.as_word, CR_BAR_INIT_PT);
			fixpt_t bar_init0_ul = edgefn (local_data_ptr->vtx_b.as_word, local_data_ptr->vtx_c.as_word); // not normalized
			
			_core_write (lr.as_word, CR_BAR_INIT_PT);
			fixpt_t bar_init0_lr = edgefn (local_data_ptr->vtx_b.as_word, local_data_ptr->vtx_c.as_word); // not normalized
			
			_core_write (ur.as_word, CR_BAR_INIT_PT);
			fixpt_t bar_init0_ur = edgefn (local_data_ptr->vtx_b.as_word, local_data_ptr->vtx_c.as_word); // not normalized
			
			if ((bar_init0_ll & bar_init0_ul & bar_init0_lr & bar_init0_ur) < 0) continue;
			
			_core_write (ll.as_word, CR_BAR_INIT_PT);
			fixpt_t bar_init1_ll = edgefn(local_data_ptr->vtx_c.as_word, local_data_ptr->vtx_a.as_word); // not normalized
			
			_core_write (ul.as_word, CR_BAR_INIT_PT);
			fixpt_t bar_init1_ul = edgefn(local_data_ptr->vtx_c.as_word, local_data_ptr->vtx_a.as_word); // not normalized
			
			_core_write (lr.as_word, CR_BAR_INIT_PT);
			fixpt_t bar_init1_lr = edgefn(local_data_ptr->vtx_c.as_word, local_data_ptr->vtx_a.as_word); // not normalized
			
			_core_write (ur.as_word, CR_BAR_INIT_PT);
			fixpt_t bar_init1_ur = edgefn(local_data_ptr->vtx_c.as_word, local_data_ptr->vtx_a.as_word); // not normalized
			
			if ((bar_init1_ll & bar_init1_ul & bar_init1_lr & bar_init1_ur) < 0) continue;
			
			_core_write (ll.as_word, CR_BAR_INIT_PT);
			fixpt_t bar_init2_ll = edgefn(local_data_ptr->vtx_a.as_word, local_data_ptr->vtx_b.as_word); // not normalized
			
			_core_write (ul.as_word, CR_BAR_INIT_PT);
			fixpt_t bar_init2_ul = edgefn(local_data_ptr->vtx_a.as_word, local_data_ptr->vtx_b.as_word); // not normalized
			
			_core_write (lr.as_word, CR_BAR_INIT_PT);
			fixpt_t bar_init2_lr = edgefn(local_data_ptr->vtx_a.as_word, local_data_ptr->vtx_b.as_word); // not normalized
			
			_core_write (ur.as_word, CR_BAR_INIT_PT);
			fixpt_t bar_init2_ur = edgefn(local_data_ptr->vtx_a.as_word, local_data_ptr->vtx_b.as_word); // not normalized
			
			if ((bar_init2_ll & bar_init2_ul & bar_init2_lr & bar_init2_ur) < 0) continue;
			
#else

			
			
			fixpt_t bar_init0_ll = edge_func_fixpt2 (local_data_ptr->vtx_b, local_data_ptr->vtx_c, ll); // not normalized
			fixpt_t bar_init0_ul = edge_func_fixpt2 (local_data_ptr->vtx_b, local_data_ptr->vtx_c, ul); // not normalized
			fixpt_t bar_init0_lr = edge_func_fixpt2 (local_data_ptr->vtx_b, local_data_ptr->vtx_c, lr); // not normalized
			fixpt_t bar_init0_ur = edge_func_fixpt2 (local_data_ptr->vtx_b, local_data_ptr->vtx_c, ur); // not normalized
			
			if ((bar_init0_ll & bar_init0_ul & bar_init0_lr & bar_init0_ur) < 0) continue;
			
			fixpt_t bar_init1_ll = edge_func_fixpt2 (local_data_ptr->vtx_c, local_data_ptr->vtx_a, ll); // not normalized
			fixpt_t bar_init1_ul = edge_func_fixpt2 (local_data_ptr->vtx_c, local_data_ptr->vtx_a, ul); // not normalized
			fixpt_t bar_init1_lr = edge_func_fixpt2 (local_data_ptr->vtx_c, local_data_ptr->vtx_a, lr); // not normalized
			fixpt_t bar_init1_ur = edge_func_fixpt2 (local_data_ptr->vtx_c, local_data_ptr->vtx_a, ur); // not normalized
			
			if ((bar_init1_ll & bar_init1_ul & bar_init1_lr & bar_init1_ur) < 0) continue;
			
			fixpt_t bar_init2_ll = edge_func_fixpt2 (local_data_ptr->vtx_a, local_data_ptr->vtx_b, ll); // not normalized
			fixpt_t bar_init2_ul = edge_func_fixpt2 (local_data_ptr->vtx_a, local_data_ptr->vtx_b, ul); // not normalized
			fixpt_t bar_init2_lr = edge_func_fixpt2 (local_data_ptr->vtx_a, local_data_ptr->vtx_b, lr); // not normalized
			fixpt_t bar_init2_ur = edge_func_fixpt2 (local_data_ptr->vtx_a, local_data_ptr->vtx_b, ur); // not normalized
			
			if ((bar_init2_ll & bar_init2_ul & bar_init2_lr & bar_init2_ur) < 0) continue;
			
#endif
			
			//if (tri_inside_tile) {
							
				//fixpt_t sob = b0.as_array[0] + b0.as_array[1] + b0.as_array[2];

				//if (sob != 0) {

					size_t tile_num = (p.y >> (GPU_TILE_HEIGHT_LOG2 + XY_FRACT_BITS)) * (get_screen_width(cfg_ptr) >> GPU_TILE_WIDTH_LOG2) + (p.x >> (GPU_TILE_WIDTH_LOG2 + XY_FRACT_BITS));
					size_t idx = (tile_num << GPU_MAX_TRIANGLES_PER_TILE_LOG2) + num_of_tris_in_tile_arr[tile_num];
					
					ext_tri_ptr_arr[idx].data = &(ext_data_arr[tri_num]); // TBD this ends up in data cache - need to be removed!
					//~ ext_tri_ptr_arr[idx].z0            = local_data_ptr->screen_z[0];
						
					 //~ // ((16.4 - 16.4) << 12) / 24.8 = 16.16 / 24.8 = 16.8
					//~ ext_tri_ptr_arr[idx].z1z0_over_sob = ((local_data_ptr->screen_z[1] - local_data_ptr->screen_z[0]) << (BARC_FRACT_BITS*2 - Z_FRACT_BITS)) / sob;
					//~ ext_tri_ptr_arr[idx].z2z0_over_sob = ((local_data_ptr->screen_z[2] - local_data_ptr->screen_z[0]) << (BARC_FRACT_BITS*2 - Z_FRACT_BITS)) / sob;
					
					//~ ext_tri_ptr_arr[idx].bar = b0;
					//printf ("@");
					if (num_of_tris_in_tile_arr[tile_num] < GPU_MAX_TRIANGLES_PER_TILE-1) {
						num_of_tris_in_tile_arr[tile_num]++;		
					}
				//}
				//~ else {
					//~ continue;
					//~ //printf ("!");
					//~ //ext_tri_ptr_arr[idx].z1z0_over_sob = 0;
					//~ //ext_tri_ptr_arr[idx].z2z0_over_sob = 0;
					//~ //ext_tri_ptr_arr[idx].bar = b0;
				//~ }
				
			//}
		}
	}
}




//
// obj_draw: Draw 3D Object
//
// Arguments:
//  *obj     - object to be drawn (object coordinates, rotation, scale, translation)
//   vshader - pointer to vertex shader function
//   pshader - pointer to pixel shader function
//  *zbuffer - pointer to Z-buffer
//  *fbuffer - pointer to framebuffer
//
// - For each face (triangle) of the object:
//    - For each vertex of the face (triangle):
//       - call vshader() which returns a Varying union containing vertex
//         coords in clip space
//       - transform vertex coords from clip space to NDC
//       - check that NDC belongs to [-1:1], otherwise vertex is clipped
//       - if the vertex is not clipped then transform its coords from
//         NDC to screen space
//    - If at least one vertex is not clipped, call draw_triangle()
//
void vshader_loop (gpu_cfg_t *cfg, const int vshader_idx) {
	
	vertex_shader_fptr vshader_fptr = (vertex_shader_fptr) cfg->vshader_fptr;
	
	volatile ObjectListNode *volatile obj_list_head = cfg->obj_list_ptr;
	volatile ObjectListNode *volatile obj_list_node;
	
	
	uint16_t num_of_tris_in_tile_arr[GPU_MAX_TILES];
	
	// Clean up data structures for each new frame:
	//TrianglePShaderData *volatile *d = cfg->tri_ptr_list[vshader_idx];
	TriangleTileData *d = cfg->tri_ptr_list[vshader_idx];
	for (int i = 0; i < (cfg->num_of_tiles * GPU_MAX_TRIANGLES_PER_TILE); i++) {
		d[i].data = NULL;
	}
	for (int i = 0; i < cfg->num_of_tiles; i++) {
		num_of_tris_in_tile_arr[i] = 0;
	}
	
	
	obj_list_node = obj_list_head;
	
	int tri_num = 0;
	
	// for each object:	
	while (obj_list_node != NULL) {
		
		uint32_t num_of_faces = obj_list_node->obj->wfobj->num_of_faces;
		uint32_t face_num_init = vshader_idx;
		uint32_t face_num_incr = GPU_MAX_USHADERS;
		
		// For each k-th triangle of an object (k = number of vertex shader units)
		for (size_t i = face_num_init; i < num_of_faces; i += face_num_incr) {
			
			
			Triangle clip;
			Triangle ndc;
			Triangle screen;
			
			TrianglePShaderData d;
			d.obj = obj_list_node->obj;
			
			FixPt3 screen_z;
			
			bool is_clipped = false; // sticky bit
			
			for (size_t j = 0; j < 3; j++) {
				
				// First four floats of Varying contain XYZW of a vertex in clip space
				//tri_data_array[tri_num].varying[j].num_of_words = 0;
				d.varying[j].num_of_words_written = 0;
				d.varying[j].num_of_words_read    = 0;
				VtxAttr attribs;
				wfobj_get_attribs (d.obj->wfobj, i, j, &attribs);
				clip.vtx[j] = vshader_fptr (d.obj, &attribs, &d.varying[j], cfg); // CALL VERTEX SHADER
												
				// Clip & normalize (clip -> NDC):
				// Clip.w contains Eye.z, so first check that it is greater than zero
				// because I don't need to draw anything behind me
				if (clip.vtx[j].as_struct.w <= 0) {
					is_clipped = true;
					break;
				}
				
				// This division is done once here to avoid three divisions below
				// No div by zero because we just checked above that W > 0
				// Clip.w can be less than 1.0, so reciprocal_w can be > 1.0
				float reciprocal_w = 1.0f / clip.vtx[j].as_struct.w; 
		
				// Compute XYZ in NDC by dividing XYZ in clip space by W (i.e. multiplying by 1/W)
				// If at least one coord doesn't belong to [-1:1] then the vertex is clipped
				for (int k = 0; k < 3; k++) {
					
					ndc.vtx[j].as_array[k] = clip.vtx[j].as_array[k] * reciprocal_w; // normalize
					
					if ((ndc.vtx[j].as_array[k] >= 1.0f) || (ndc.vtx[j].as_array[k] < -1.0f)) {
						is_clipped = true;
						break;
					}
				}
				
				// Typically W shouldn't be part of NDC, but I need it here to do correct matrix multiplication below (VIEWPORT is 4x4)
				// Remember that as_struct.w == as_array[3]
				ndc.vtx[j].as_struct.w = 1.0f;

				if (!is_clipped) {
					//screen.vtx[j] = fmat4_Float4_mult (&(cfg->viewport), &(ndc.vtx[j]));
					fmat4_Float4_mult_fast (&screen.vtx[j], &cfg->viewport, &ndc.vtx[j]);
					
					// Replace clip coords with screen coords within the Varying struct
					// before passing it on to Tiler
					hfixpt_t x =  hfixpt_from_float        (screen.vtx[j].as_struct.x,       XY_FRACT_BITS);
					hfixpt_t y =  hfixpt_from_float        (screen.vtx[j].as_struct.y,       XY_FRACT_BITS);
					switch (j) {
						case (0): d.vtx_a.as_coord.x = x; d.vtx_a.as_coord.y = y; break;
						case (1): d.vtx_b.as_coord.x = x; d.vtx_b.as_coord.y = y; break;
						case (2): d.vtx_c.as_coord.x = x; d.vtx_c.as_coord.y = y; break;
					}
					
					//d.screen_z[j] =   fixpt_from_float        (screen.vtx[j].as_struct.z,        Z_FRACT_BITS);
					screen_z.as_array[j] =   fixpt_from_float        (screen.vtx[j].as_struct.z,        Z_FRACT_BITS);
					
					// We don't need W anymore, but we will need 1/W later:
					//d.w_reciprocal [j]             =  fixpt_from_float_no_rnd (reciprocal_w, W_RECIPR_FRACT_BITS);
					d.w_reciprocal [j]             =  fixpt_from_float (reciprocal_w, W_RECIPR_FRACT_BITS);
					
				}
				else break;
			}
			
			if (!is_clipped) {
				tiler (&d, &screen_z, vshader_idx, tri_num, num_of_tris_in_tile_arr, cfg);
				tri_num++;
			}
		}
		
		obj_list_node = obj_list_node->next;
	}
}


//void interpolate_varying (Varying *vry, fixpt_t *w_reciprocal, FixPt3 *bar, Varying *vry_interp);
void interpolate_varying (Varying *vry, fixpt_t *w_reciprocal, fixpt_t *bar0, fixpt_t *bar1, fixpt_t *bar2, Varying *vry_interp);

void draw_triangle (TrianglePShaderData* tri, bbox_uhfixpt_t *tile_bb, screenz_t *zbuffer, pixel_color_t *fbuffer, gpu_cfg_t *cfg);
 

//void copy_tiles_to_extmem (volatile void* volatile dst, volatile void* volatile src, gpu_cfg_t *cfg, size_t tile_num, size_t elem_size);
void preload_tiles (screenz_t *zbuf_dst_ptr, screenz_t *zbuf_src_ptr, pixel_color_t *fbuf_dst_ptr, pixel_color_t *fbuf_src_ptr, size_t elems_in_tile);
void flush_tiles (screenz_t *local_zbuf, pixel_color_t *local_fbuf, size_t tile_num, gpu_cfg_t *cfg);


//void interpolate_varying (Varying *vry, fixpt_t *w_reciprocal, FixPt3 *bar, Varying *vry_interp) {
void interpolate_varying (Varying *vry, fixpt_t *w_reciprocal, fixpt_t *bar0, fixpt_t *bar1, fixpt_t *bar2, Varying *vry_interp) {

	assert (vry[0].num_of_words_written == vry[1].num_of_words_written);
	assert (vry[0].num_of_words_written == vry[2].num_of_words_written);
					
	vry_interp->num_of_words_written = (vry[0].num_of_words_written);
		
#ifdef ARC_APEX

	//Varying tmp;
	// ARC APEX implementation
	//if (vry_interp->num_of_words_written > 0) {
		
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
			//printf ("vry_ip_push %x\n", vry_interp->data[i].as_fixpt_t);
			//tmp.data[i].as_fixpt_t = vry_ip(0);
		}
	//}
	
#else
	
	vry_interp->num_of_words_read = 0;
	
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
			vbw0 = (((dfixpt_t) vry[0].data[i].as_fixpt_t) >> 7) * (bw0 >> 13);  // VM.VF * WM.WF = (VM+WM).(VF+WF)
			vbw1 = (((dfixpt_t) vry[1].data[i].as_fixpt_t) >> 7) * (bw1 >> 13);  // VM.VF * WM.WF = (VM+WM).(VF+WF) 
			vbw2 = (((dfixpt_t) vry[2].data[i].as_fixpt_t) >> 7) * (bw2 >> 13);  // VM.VF * WM.WF = (VM+WM).(VF+WF)
			vbw_acc = vbw0 + vbw1 + vbw2; // = (VM+WM+BM+1).(VF+WF+BF-NNN)

			// ((VM+WM+BM+1).(VF+WF+BF-NNN) * ((1).(OF))) >> (WF+BF+OF-NNN) = ((VM+WM+BM+1+1-OF).(VF+WF+BF+OF-NNN) >> (WF+BF+OF-NNN)) = (VM+WM+BM+2-OF).VF
			//vry_interp->data[i].as_fixpt_t = (fixpt_t) ((acc_fixpt * one_over_wi) >> (W_RECIPR_FRACT_BITS + BARC_FRACT_BITS + OOWI_FRACT_BITS - NNN)); // = (VM+WM+BM+65-OF).VF
			
			// .31 / .37 = .14
			vry_interp->data[i].as_fixpt_t = (fixpt_t) (vbw_acc / (bw_acc >> 20)); // = (VM+WM+BM+65-OF).VF

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

#endif

}



//~ void __attribute__ ((noinline)) dma_zero2mem_linkedlist (screenz_t *zbuf_dst_ptr, size_t zbuf_tile_byte_size, pixel_color_t *fbuf_dst_ptr, size_t fbuf_tile_byte_size, dma_ch channel_num, dma_desc *dd) {

	//~ uint32_t channel_mask = (1 << channel_num);
	
	//~ while (_lr(AUXR_DMACSTAT0) & channel_mask); // wait till completion    
	
	//~ _sr (  DMACTRLX_OP_LINKL_AUTO |
	       //~ DMACTRLX_RT_AUTO |
	       //~ DMACTRLX_DTT_MEM2MEM |
	       //~ DMACTRLX_DW_CLEAR |
	       //~ DMACTRLX_BLOCK_SIZE(zbuf_tile_byte_size) |
	       //~ //DMACTRLX_ARB_AFTER (8) |
	       //~ DMACTRLX_ARB_DISABLE |
	       //~ DMACTRLX_IRQ_DISABLE |
	       //~ DMACTRLX_AM_INCR_DST_ONLY,
	     //~ AUXR_DMACTRLX(channel_num));
	 
	//~ _sr ((uint32_t) zbuf_dst_ptr + ((zbuf_tile_byte_size - 1) & ~0x3) , AUXR_DMADARX(channel_num));
	
	//~ dd->DMACTRL =   DMACTRLX_OP_SINGLE |
	                //~ DMACTRLX_RT_AUTO |
	                //~ DMACTRLX_DTT_MEM2MEM |
	                //~ DMACTRLX_DW_CLEAR |
	                //~ DMACTRLX_BLOCK_SIZE (fbuf_tile_byte_size) |
	                //~ //DMACTRLX_ARB_AFTER (8) |
	                //~ DMACTRLX_ARB_DISABLE |
	                //~ DMACTRLX_IRQ_DISABLE |
	                //~ DMACTRLX_AM_INCR_DST_ONLY;
	//~ dd->DMASAR = 0;
	//~ dd->DMADAR = (uint32_t) fbuf_dst_ptr + ((fbuf_tile_byte_size - 1) & ~0x3);
	//~ dd->DMALLP = 0;

	//~ dma_desc **local_dd = &dd;
	//~ _sr ((uint32_t) local_dd, AUXR_DMACBASE);
	
	//~ _sr (channel_mask, AUXR_DMACREQ);
//~ }




//~ void dma_zero2mem_single (volatile void *volatile dst_ptr, size_t byte_size, dma_ch channel_num) {
	
//~ #ifdef DMA
	
	//~ uint32_t channel_mask = (1 << channel_num);
	
	//~ while (_lr(AUXR_DMACSTAT0) & channel_mask); // wait till completion    
	
	//~ _sr (  DMACTRLX_OP_SINGLE |
	       //~ DMACTRLX_RT_AUTO |
	       //~ DMACTRLX_DTT_MEM2MEM |
	       //~ DMACTRLX_DW_CLEAR |
	       //~ DMACTRLX_BLOCK_SIZE(byte_size) |
	       //~ DMACTRLX_ARB_AFTER (8) |
	       //~ DMACTRLX_IRQ_DISABLE |
	       //~ DMACTRLX_AM_INCR_DST_ONLY,
	     //~ AUXR_DMACTRLX(channel_num));
	 
	//~ _sr ((uint32_t) dst_ptr + ((byte_size - 1) & ~0x3) , AUXR_DMADARX(channel_num));
	//~ _sr (channel_mask, AUXR_DMACREQ);
	
//~ #else
	
	//~ for (size_t i = 0; i < byte_size / 4; i++) {
		//~ *((uint32_t *) dst_ptr + i) = 0;
	//~ }
		
//~ #endif

//~ }

/*
void preload_tiles (screenz_t *zbuf_dst_ptr, screenz_t *zbuf_src_ptr, pixel_color_t *fbuf_dst_ptr, pixel_color_t *fbuf_src_ptr, size_t elems_in_tile) {
	
	size_t zbuf_tile_byte_size = elems_in_tile * sizeof (screenz_t);
	size_t fbuf_tile_byte_size = elems_in_tile * sizeof (pixel_color_t);

	// Always initialize local zbuf because it will always be used locally, even if we don't write it back
	// to the global zbuf,
	// but initialize local fbuf only if global fbuf is set; if it's not set then we don't write to fbuf
	// and hence no need to pre-load it (this is the case, for example, when we generate a shadow map)
	//~ if ((zbuf_dst_ptr != NULL) && (fbuf_dst_ptr != NULL)) {
		//~ //dma_zero2mem_linkedlist (zbuf_dst_ptr, zbuf_tile_byte_size, fbuf_dst_ptr, fbuf_tile_byte_size, DMA_CH0, mem_desc);
		//~ dma_mem2mem_single     (zbuf_dst_ptr, NULL, zbuf_tile_byte_size, DMA_CH1);
		//~ dma_mem2mem_single     (fbuf_dst_ptr, NULL, fbuf_tile_byte_size, DMA_CH2);
	//~ }
	//~ else if ((zbuf_dst_ptr != NULL) && (fbuf_dst_ptr == NULL)) {
		//~ dma_mem2mem_single     (zbuf_dst_ptr, NULL, zbuf_tile_byte_size, DMA_CH1);
	//~ }
	//~ else if ((zbuf_dst_ptr == NULL) && (fbuf_dst_ptr != NULL)) {
		//~ dma_mem2mem_single     (fbuf_dst_ptr, NULL, fbuf_tile_byte_size, DMA_CH2);
	//~ }
	if (zbuf_dst_ptr != NULL) {
		dma_mem2mem_single (zbuf_dst_ptr, zbuf_src_ptr, zbuf_tile_byte_size, 1); // src_ptr allowed to be zero
	}
	if (fbuf_dst_ptr != NULL) {
		dma_mem2mem_single (fbuf_dst_ptr, fbuf_src_ptr, fbuf_tile_byte_size, 2); // src_ptr allowed to be zero
	}
}
*/

void flush_tiles (screenz_t *local_zbuf, pixel_color_t *local_fbuf, size_t tile_num, gpu_cfg_t *cfg) {
	
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
	
	//bbox_uhfixpt_t tri_bb = get_tri_bbox (local_tpd_ptr->vtx_a, local_tpd_ptr->vtx_b, local_tpd_ptr->vtx_c);
	//bbox_uhfixpt_t bb = clip_tri_bbox_to_tile (&tri_bb, tile_bb);
	bbox_uhfixpt_t bb = clip_tri_to_bbox (local_tpd_ptr->vtx_a, local_tpd_ptr->vtx_b, local_tpd_ptr->vtx_c, tile_bb);
	
	//~ uint32_t tile_bb_min_x = tile_bb->min.as_coord.x >> XY_FRACT_BITS;
	//~ uint32_t tile_bb_min_y = tile_bb->min.as_coord.y >> XY_FRACT_BITS;
	
	//~ uint32_t bb_min_x = (bb.min.as_coord.x >> XY_FRACT_BITS) - tile_bb_min_x;
	//~ uint32_t bb_min_y = (bb.min.as_coord.y >> XY_FRACT_BITS) - tile_bb_min_y;
	//~ uint32_t bb_max_x = (bb.max.as_coord.x >> XY_FRACT_BITS) - tile_bb_min_x;
	//~ uint32_t bb_max_y = (bb.max.as_coord.y >> XY_FRACT_BITS) - tile_bb_min_y;

	uint32_t bb_min_x = (bb.min.as_coord.x - tile_bb->min.as_coord.x) >> XY_FRACT_BITS;
	uint32_t bb_min_y = (bb.min.as_coord.y - tile_bb->min.as_coord.y) >> XY_FRACT_BITS;
	uint32_t bb_max_x = (bb.max.as_coord.x - tile_bb->min.as_coord.x) >> XY_FRACT_BITS;
	uint32_t bb_max_y = (bb.max.as_coord.y - tile_bb->min.as_coord.y) >> XY_FRACT_BITS;
	

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
	
	
	//~ fixpt_t bar_initial0t;
	//~ fixpt_t bar_initial1t;
	//~ fixpt_t bar_initial2t;
	
		
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

	//~ bar_initial0 = local_ttd_ptr->bar.as_array[0];
	//~ bar_initial1 = local_ttd_ptr->bar.as_array[1];
	//~ bar_initial2 = local_ttd_ptr->bar.as_array[2];
	
	//printf ("vtx shdr bar: %x %x %x, pix shd bar: %x %x %x\n", bar_initial0, bar_initial1, bar_initial2, bar_initial0t, bar_initial1t, bar_initial2t);
	//printf ("vtx shdr z: %x, pix shd z: %x\n", local_ttd_ptr->z0, local_tpd_ptr->screen_z[0]);
	//~ for (int i = 0; i < 3; i++) {
		//~ sum_of_bars += bar_initial.as_array[i];
	//~ }
	sum_of_bars = bar_initial0 + bar_initial1 + bar_initial2;
		
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
	
	//bar_row = bar_initial;
	bar_row0 = bar_initial0;// + (bb_min_x * bar_col_incr0) + (bb_min_y * bar_row_incr0);
	bar_row1 = bar_initial1;// + (bb_min_x * bar_col_incr1) + (bb_min_y * bar_row_incr1);
	bar_row2 = bar_initial2;// + (bb_min_x * bar_col_incr2) + (bb_min_y * bar_row_incr2);
	
	// Left shift by 12: 4 bits to compensate for fractional width difference between Z (4 bits) and sum_of_bars (8 bits)
	//   plus 8 bits to compensate for division precision loss.
	//fixpt_t z1z0 = ((local_tpd_ptr->screen_z[1] - local_tpd_ptr->screen_z[0]) << (BARC_FRACT_BITS*2 - Z_FRACT_BITS)) / sum_of_bars; // ((16.4 - 16.4) << 12) / 24.8 = 16.16 / 24.8 = 16.8
	//fixpt_t z2z0 = ((local_tpd_ptr->screen_z[2] - local_tpd_ptr->screen_z[0]) << (BARC_FRACT_BITS*2 - Z_FRACT_BITS)) / sum_of_bars;
				
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
				
				//~ screenz_t zi = fixpt_to_screenz (
					//~ local_tpd_ptr->screen_z[0] +
					//~ ((z1z0 * bar1) >> (BARC_FRACT_BITS*2 - Z_FRACT_BITS)) +
					//~ ((z2z0 * bar2) >> (BARC_FRACT_BITS*2 - Z_FRACT_BITS))
				//~ ); // 16.4 + (16.8 * 24.8 >> 12) + (16.8 * 24.8 >> 12) = 28.4
				
				screenz_t zi = fixpt_to_screenz (
					local_tpd_ptr->z0 +
					((local_tpd_ptr->z1z0_over_sob * bar1) >> (BARC_FRACT_BITS*2 - Z_FRACT_BITS)) +
					((local_tpd_ptr->z2z0_over_sob * bar2) >> (BARC_FRACT_BITS*2 - Z_FRACT_BITS))
				); // 16.4 + (16.8 * 24.8 >> 12) + (16.8 * 24.8 >> 12) = 28.4
				
				
				
				if (zi > local_zbuf[pix_num]) {
						
					local_zbuf[pix_num] = zi;

					if (cfg->active_fbuffer != NULL) {
						
						Varying vry_interp;

#ifdef ARC_APEX
						for (int i = 0; i < local_tpd_ptr->varying[0].num_of_words_written; i++) {
							_core_write (local_tpd_ptr->varying[0].data[i].as_fixpt_t, CR_VRY0);
							_core_write (local_tpd_ptr->varying[1].data[i].as_fixpt_t, CR_VRY1);
							_core_write (local_tpd_ptr->varying[2].data[i].as_fixpt_t, CR_VRY2);
							vry_interp.data[i].as_fixpt_t = vry_ip(0); // APEX instruction
						}
#else
						interpolate_varying (local_tpd_ptr->varying, local_tpd_ptr->w_reciprocal, &bar0, &bar1, &bar2, &vry_interp);
#endif				

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
	
	enum {CH0=0, CH1, CH2};
	
	
	size_t elems_in_tile = GPU_TILE_WIDTH * GPU_TILE_HEIGHT;
	size_t zbuf_tile_byte_size = elems_in_tile * sizeof (screenz_t);
	size_t fbuf_tile_byte_size = elems_in_tile * sizeof (pixel_color_t);
	
	
	screenz_t           local_zbuf[2][elems_in_tile];
	pixel_color_t       local_fbuf[2][elems_in_tile];
	TrianglePShaderData local_tri_data[2];
	//TriangleTileData    local_tile_data;
	
	
	
		
	size_t starting_tile  = shader_num % GPU_MAX_USHADERS;
	size_t     incr_tile  =              GPU_MAX_USHADERS;
	size_t   num_of_tiles =              cfg->num_of_tiles;
	
	size_t   active_local_buf_idx = 0;
	size_t inactive_local_buf_idx = 1;
	
	// no source, initialize with zero for now
	screenz_t     *zbuf_src_ptr     = NULL;
	screenz_t     *zbuf_dst_ptr     = local_zbuf[  active_local_buf_idx];
	
	
	pixel_color_t *fbuf_src_ptr     = NULL;
	// initialize local fbuf only if the active global fbuf is set, otherwise fbuf is not used
	pixel_color_t *fbuf_dst_ptr     = (cfg->active_fbuffer == NULL) ? NULL : local_fbuf[  active_local_buf_idx];
	
	//preload_tiles (local_zbuf[active_local_buf_idx], zbuf_src_ptr, fbuf_dst_ptr, fbuf_src_ptr, elems_in_tile);
	dma_memcpy (zbuf_dst_ptr, zbuf_src_ptr, zbuf_tile_byte_size, CH1);
	dma_memcpy (fbuf_dst_ptr, fbuf_src_ptr, fbuf_tile_byte_size, CH2);
	
	for (size_t tile_num = starting_tile; tile_num < num_of_tiles; tile_num += incr_tile) {
		
		bbox_uhfixpt_t tile_bb = get_tile_bbox (tile_num, cfg);
		
		// prefetch:
		if (tile_num < num_of_tiles - 1) {
			//preload_tiles (local_zbuf[inactive_local_buf_idx], zbuf_src_ptr, (cfg->active_fbuffer == NULL) ? NULL : local_fbuf[inactive_local_buf_idx], fbuf_src_ptr, elems_in_tile);
			screenz_t     *zbuf_dst_nxt_ptr = local_zbuf[inactive_local_buf_idx];
			pixel_color_t *fbuf_dst_nxt_ptr = (cfg->active_fbuffer == NULL) ? NULL : local_fbuf[inactive_local_buf_idx];
			
			dma_memcpy (zbuf_dst_nxt_ptr, zbuf_src_ptr, zbuf_tile_byte_size, CH1);
			dma_memcpy (fbuf_dst_nxt_ptr, fbuf_src_ptr, fbuf_tile_byte_size, CH2);
		}
		
		for (int i = 0; i < GPU_MAX_USHADERS; i++) {
			//volatile TriangleTileData ext_tri_ptr = cfg->tri_ptr_list[i][tile_num << GPU_MAX_TRIANGLES_PER_TILE_LOG2];
			volatile TrianglePShaderData *ext_tri_data_ptr = cfg->tri_ptr_list[i][tile_num << GPU_MAX_TRIANGLES_PER_TILE_LOG2].data;
			//~ fixpt_t z0 = cfg->tri_ptr_list[i][tile_num << GPU_MAX_TRIANGLES_PER_TILE_LOG2].z0;
			//~ fixpt_t z1z0_over_sob = cfg->tri_ptr_list[i][tile_num << GPU_MAX_TRIANGLES_PER_TILE_LOG2].z1z0_over_sob;
			//~ fixpt_t z2z0_over_sob = cfg->tri_ptr_list[i][tile_num << GPU_MAX_TRIANGLES_PER_TILE_LOG2].z2z0_over_sob;
			if (ext_tri_data_ptr == NULL) continue;
			//if (ext_tri_ptr == NULL) continue;
			//if (ext_tri_ptr.data == NULL) continue;
			
			size_t   active_ltd = 0;
			size_t inactive_ltd = 1; // TBD
			
			//local_tile_data[active_ltd] = *ext_tri_ptr; // making a local copy
			//local_tri_data [active_ltd] = *(ext_tri_ptr->data); // making a local copy
			
			dma_memcpy (&local_tri_data[active_ltd], ext_tri_data_ptr, sizeof(TrianglePShaderData), CH0);
				
			for (int j = 0; j < GPU_MAX_TRIANGLES_PER_TILE; j++) {
				
				// prefetch:
				volatile TrianglePShaderData *ext_tri_data_nxt_ptr;
				if (j < GPU_MAX_TRIANGLES_PER_TILE - 1) {
					ext_tri_data_nxt_ptr = cfg->tri_ptr_list[i][(tile_num << GPU_MAX_TRIANGLES_PER_TILE_LOG2) + j + 1].data;
					if (ext_tri_data_nxt_ptr != NULL) {
						dma_memcpy (&local_tri_data[inactive_ltd], ext_tri_data_nxt_ptr, sizeof(TrianglePShaderData), CH0);
					}
				}
				else {
					ext_tri_data_nxt_ptr = NULL;
				}
				
				//volatile TriangleTileData *ext_tile_data_ptr = &(cfg->tri_ptr_list[i][(tile_num << GPU_MAX_TRIANGLES_PER_TILE_LOG2) + j]);
				//local_tile_data = *ext_tile_data_ptr;
				draw_triangle (&local_tri_data[active_ltd], &tile_bb, local_zbuf[active_local_buf_idx], local_fbuf[active_local_buf_idx], cfg);
				
				  active_ltd = (  active_ltd == 0) ? 1 : 0;
				inactive_ltd = (inactive_ltd == 0) ? 1 : 0;
				
				if (ext_tri_data_nxt_ptr == NULL) break;
			}
		}
		
		flush_tiles (local_zbuf[active_local_buf_idx], local_fbuf[active_local_buf_idx], tile_num, cfg);
		
		  active_local_buf_idx = (  active_local_buf_idx == 0) ? 1 : 0;
		inactive_local_buf_idx = (inactive_local_buf_idx == 0) ? 1 : 0;
	}
}


#ifdef MULTIPROC

 int main (void) {
	uint32_t shader_num = (_lr(0x4) >> 8) & 0x000000ff; // ARC-specific code
	volatile gpu_cfg_t      *const common_cfg = (gpu_cfg_t *)      GPU_CFG_ABS_ADDRESS;
	volatile gpu_run_halt_t *const run_halt   = (gpu_run_halt_t *) GPU_RUN_HALT_ABS_ADDRESS;

#else

 void * ushader_top (void *ushader_cfg) {
	assert (ushader_cfg != NULL);
	
	pthread_cfg_t  *pthread_cfg = ushader_cfg;
    volatile gpu_cfg_t      *const common_cfg  = pthread_cfg->common_cfg;
    volatile gpu_run_halt_t *const run_halt    = pthread_cfg->gpu_run_halt;
    uint32_t        shader_num  = pthread_cfg->core_num;

	volatile hasha_block_t *const this_ptr = pthread_cfg->hasha_block_ptr; 
	
#endif
	
	
	if (PTHREAD_DEBUG) { printf("ushader%d: ushader up and running\n", shader_num); }
	
	gpu_cfg_t local_cfg;
	

	dma_init ();

	while (1) {
		
		hasha_wait_for_mst (this_ptr, HASHA_USHADER_UPSTREAM_SLV);
		hasha_notify_slv   (this_ptr, HASHA_USHADER_DOWNSTREAM_MST);

		local_cfg = *common_cfg;
		vshader_loop (&local_cfg, shader_num);
		
		hasha_wait_for_mst (this_ptr, HASHA_USHADER_DOWNSTREAM_SLV);
		hasha_notify_slv   (this_ptr, HASHA_USHADER_UPSTREAM_MST);
		
		////////////////////////////////////////////////////////////////
		
		hasha_wait_for_mst (this_ptr, HASHA_USHADER_UPSTREAM_SLV);
		hasha_notify_slv   (this_ptr, HASHA_USHADER_DOWNSTREAM_MST);
		
		local_cfg = *common_cfg;
		pshader_loop (&local_cfg, shader_num);

		hasha_wait_for_mst (this_ptr, HASHA_USHADER_DOWNSTREAM_SLV);
		hasha_notify_slv   (this_ptr, HASHA_USHADER_UPSTREAM_MST);
		
	}
	
#ifndef MULTIPROC	
	return NULL;
#else
	return 0;
#endif
}
