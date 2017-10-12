#include "vshader_loop.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>


void tiler (TrianglePShaderData *local_data_ptr, FixPt3 *screen_z, uint32_t vshader_idx, uint32_t tri_num, uint16_t *num_of_tris_in_tile_arr, gpu_cfg_t *cfg_ptr);

//~ void tiler_memcpy (volatile TrianglePShaderData *volatile ext_data_arr, uint32_t tri_num);

//~ void tiler_memcpy () {
//~ }

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
	bbox_uhfixpt_t bb = clip_tri_to_tile2 (local_data_ptr->vtx_a, local_data_ptr->vtx_b, local_data_ptr->vtx_c, &screen_bb);
    
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
	ext_data_arr[tri_num] = *local_data_ptr;
	
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
					num_of_tris_in_tile_arr[tile_num]++;		
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
	for (int i = 0; i < (cfg->num_of_tiles << GPU_MAX_TRIANGLES_PER_TILE_LOG2); i++) {
		d[i].data = NULL;
	}
	for (int i = 0; i < cfg->num_of_tiles; i++) {
		num_of_tris_in_tile_arr[i] = 0;
	}
	
	
	obj_list_node = obj_list_head;
	
	int tri_num = 0;
		
	while (obj_list_node != NULL) {
		
		uint32_t num_of_faces = obj_list_node->obj->wfobj->num_of_faces;
		uint32_t face_num_init = vshader_idx;
		uint32_t face_num_incr = GPU_MAX_USHADERS;
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
				clip.vtx[j] = vshader_fptr (d.obj, i, j, &(d.varying[j]), cfg); // CALL VERTEX SHADER
												
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
					screen.vtx[j] = fmat4_Float4_mult (&(cfg->viewport), &(ndc.vtx[j]));
					
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
