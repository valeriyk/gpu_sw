#include "vshader_loop.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>


void tiler (TrianglePShaderData *local_data_ptr, FixPt3 *screen_z, uint32_t vshader_idx, uint32_t tri_num, uint16_t *num_of_tris_in_tile_arr, gpu_cfg_t *cfg_ptr);

//~ void tiler_memcpy (volatile TrianglePShaderData *volatile ext_data_arr, uint32_t tri_num);

//~ void tiler_memcpy () {
//~ }

void tiler (TrianglePShaderData *local_data_ptr, FixPt3 *screen_z, uint32_t vshader_idx, uint32_t tri_num, uint16_t *num_of_tris_in_tile_arr, gpu_cfg_t *cfg_ptr) {
	
	hfixpt_t x[3];
	hfixpt_t y[3];
	// fixpt_t z[3];
	
	// re-pack X, Y coords of the three vertices, hopefully this will be optimized away by the compiler
	//~ for (size_t i = 0; i < 3; i++) {
		//~ x[i] = local_data_ptr->screen_x[i];
		//~ y[i] = local_data_ptr->screen_y[i];
	//~ }
	
	x[0] = local_data_ptr->vtx_a.as_coord.x;
	x[1] = local_data_ptr->vtx_b.as_coord.x;
	x[2] = local_data_ptr->vtx_c.as_coord.x;
	y[0] = local_data_ptr->vtx_a.as_coord.y;
	y[1] = local_data_ptr->vtx_b.as_coord.y;
	y[2] = local_data_ptr->vtx_c.as_coord.y;
	
	//~ local_data_ptr->screen_z0   = screen_z[0];
	//~ local_data_ptr->screen_z1z0 = screen_z[1] - screen_z[0];
	//~ local_data_ptr->screen_z2z0 = screen_z[2] - screen_z[0];
	
	// save local data to memory now, because I need its address later for storing it in tri_ptr_list 
	TrianglePShaderData *volatile ext_data_arr = cfg_ptr->tri_for_pshader[vshader_idx];
	ext_data_arr[tri_num] = *local_data_ptr;
	
	//TrianglePShaderData *volatile *ext_tri_ptr_arr = cfg_ptr->tri_ptr_list[vshader_idx];
	TriangleTileData *ext_tri_ptr_arr = cfg_ptr->tri_ptr_list[vshader_idx];
	
	BoundBox bb = clip_boundbox_to_screen (get_tri_boundbox (x, y), cfg_ptr);
    bb.min.x &= ~(GPU_TILE_WIDTH-1);
    bb.min.y &= ~(GPU_TILE_HEIGHT-1);
    
    ScreenPt p;
    
    // For every tile inside the bounding box of the triangle:
    for (p.y = bb.min.y; p.y <= bb.max.y; p.y += GPU_TILE_HEIGHT) {	
		for (p.x = bb.min.x; p.x <= bb.max.x; p.x += GPU_TILE_WIDTH) {

			// Evaluate barycentric coords in all the four corners of each tile:
			
			// find corners of the tile:
			hfixpt_t x0 = hfixpt_from_screenxy (p.x);
			hfixpt_t x1 = hfixpt_from_screenxy (p.x + GPU_TILE_WIDTH - 1);
			hfixpt_t y0 = hfixpt_from_screenxy (p.y);
			hfixpt_t y1 = hfixpt_from_screenxy (p.y + GPU_TILE_HEIGHT - 1);
			
			// get barycentric coords in each corner:
			FixPt3 b0 = get_bar_coords (x, y, x0, y0);
			FixPt3 b1 = get_bar_coords (x, y, x0, y1);
			FixPt3 b2 = get_bar_coords (x, y, x1, y0);
			FixPt3 b3 = get_bar_coords (x, y, x1, y1);
			
			bool tri_inside_tile = true; // sticky bit
			for (int i = 0; i < 3; i++) {
				// If barycentric coord "i" is negative in all four corners, triangle is outside the tile
				// See here: http://forum.devmaster.net/t/advanced-rasterization/6145
				if ((b0.as_array[i] & b1.as_array[i] & b2.as_array[i] & b3.as_array[i]) < 0) {
					tri_inside_tile = false;
					break;
				}
			}
			
			if (tri_inside_tile) {
							
				fixpt_t sob = b0.as_array[0] + b0.as_array[1] + b0.as_array[2];

				if (sob != 0) {

					size_t tile_num = (p.y >> GPU_TILE_HEIGHT_LOG2) * (get_screen_width(cfg_ptr) >> GPU_TILE_WIDTH_LOG2) + (p.x >> GPU_TILE_WIDTH_LOG2);
					size_t idx = (tile_num << GPU_MAX_TRIANGLES_PER_TILE_LOG2) + num_of_tris_in_tile_arr[tile_num];
					
					ext_tri_ptr_arr[idx].data = &(ext_data_arr[tri_num]); // TBD this ends up in data cache - need to be removed!
					//~ ext_tri_ptr_arr[idx].z0            = local_data_ptr->screen_z[0];
						
					 //~ // ((16.4 - 16.4) << 12) / 24.8 = 16.16 / 24.8 = 16.8
					//~ ext_tri_ptr_arr[idx].z1z0_over_sob = ((local_data_ptr->screen_z[1] - local_data_ptr->screen_z[0]) << (BARC_FRACT_BITS*2 - Z_FRACT_BITS)) / sob;
					//~ ext_tri_ptr_arr[idx].z2z0_over_sob = ((local_data_ptr->screen_z[2] - local_data_ptr->screen_z[0]) << (BARC_FRACT_BITS*2 - Z_FRACT_BITS)) / sob;
					
					//~ ext_tri_ptr_arr[idx].bar = b0;
					//printf ("@");
					num_of_tris_in_tile_arr[tile_num]++;		
				}
				//~ else {
					//~ continue;
					//~ //printf ("!");
					//~ //ext_tri_ptr_arr[idx].z1z0_over_sob = 0;
					//~ //ext_tri_ptr_arr[idx].z2z0_over_sob = 0;
					//~ //ext_tri_ptr_arr[idx].bar = b0;
				//~ }
				
			}
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
					
					d.screen_z[j] =   fixpt_from_float        (screen.vtx[j].as_struct.z,        Z_FRACT_BITS);
					//screen_z[j] =   fixpt_from_float        (screen.vtx[j].as_struct.z,        Z_FRACT_BITS);
					
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
