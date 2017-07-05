#include "vshader.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>


//void tiler (volatile TrianglePShaderData * volatile tri, TriangleListNode *tri_ptr[]);
void tiler (volatile TrianglePShaderData* volatile tri, volatile TriangleListNode* volatile tri_ptr[], gpu_cfg_t *cfg);



void tiler (volatile TrianglePShaderData* volatile tri, volatile TriangleListNode* volatile tri_ptr[], gpu_cfg_t *cfg) {
	
	fixpt_t x[3];
	fixpt_t y[3];
	
	// re-pack X, Y, Z, W coords of the three vertices
	for (int i = 0; i < 3; i++) {
		x[i] = tri->screen_coords[i].as_struct.x;
		y[i] = tri->screen_coords[i].as_struct.y;
	}
	
	BoundBox bb = clip_boundbox_to_screen (get_tri_boundbox (x, y), cfg);
    bb.min.x &= ~(TILE_WIDTH-1);
    bb.min.y &= ~(TILE_HEIGHT-1);
    
    ScreenPt p;
    
    // Evaluate barycentric coords in all the four corners of each tile
    for (p.y = bb.min.y; p.y <= bb.max.y; p.y += TILE_HEIGHT) {	
		for (p.x = bb.min.x; p.x <= bb.max.x; p.x += TILE_WIDTH) {

			// find corners of the tile:
			fixpt_t x0 = fixpt_from_screenxy (p.x);
			fixpt_t x1 = fixpt_from_screenxy (p.x + TILE_WIDTH - 1);
			fixpt_t y0 = fixpt_from_screenxy (p.y);
			fixpt_t y1 = fixpt_from_screenxy (p.y + TILE_HEIGHT - 1);
			
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
				
				size_t tile_num = (p.y >> (int) log2f(get_tile_height(cfg))) * (get_screen_width(cfg) / get_tile_width(cfg)) + (p.x >> (int) log2f(get_tile_width(cfg)));
				
				volatile TriangleListNode* volatile node = tri_ptr[tile_num];
				if (node == NULL) {
					node = calloc (1, sizeof (TriangleListNode));
					node->tri  = tri;
					node->next = NULL;	
					
					tri_ptr[tile_num] = node;
				}
				else {
					while (node->next != NULL) {
						node = node->next;
					}
					node->next = calloc (1, sizeof (TriangleListNode));
					node->next->tri  = tri;
					node->next->next = NULL;	
				}
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
void draw_frame (gpu_cfg_t *cfg, vertex_shader vshader, pixel_shader pshader, screenz_t *zbuffer, pixel_color_t *fbuffer) {
	
	if (GL_DEBUG_0)
	{
		printf("call draw_frame()\n");
	}
	
	
	
	
	cfg->pshader_ptr = pshader;
	cfg->zbuffer_ptr = zbuffer;
	cfg->active_fbuffer = fbuffer;
					
	volatile ObjectListNode* volatile obj_list_head = cfg->obj_list_ptr;
	volatile ObjectListNode* volatile obj_list_node;
	
	
	// Clean up data structures for each new frame:
	for (int i = 0; i < cfg->num_of_tiles; i++) {
		TriangleListNode **tln = cfg->tile_idx_table_ptr;
		tln[i] = NULL;
	}
	
	obj_list_node = obj_list_head;
	
	int tri_num = 0;
		
	while (obj_list_node != NULL) {
		for (size_t i = 0; i < wfobj_get_num_of_faces(obj_list_node->obj->wfobj); i++) {
						
			Triangle clip;
			Triangle ndc;
			Triangle screen;
			
			TrianglePShaderData d;
			//tri_data_array[tri_num].obj = obj_list_node->obj;
			d.obj = obj_list_node->obj;
			
			bool is_clipped = false; // sticky bit
			
			for (size_t j = 0; j < 3; j++) {
				
				// First four floats of Varying contain XYZW of a vertex in clip space
				//tri_data_array[tri_num].varying[j].num_of_words = 0;
				d.varying[j].num_of_words_written = 0;
				d.varying[j].num_of_words_read    = 0;
				clip.vtx[j] = vshader (d.obj, i, j, &(d.varying[j]), cfg); // CALL VERTEX SHADER
								
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
				for (int k = 0; k < 4; k++) {
					
					ndc.vtx[j].as_array[k] = clip.vtx[j].as_array[k] * reciprocal_w; // normalize
					
					if ((ndc.vtx[j].as_array[k] > 1.0f) || (ndc.vtx[j].as_array[k] < -1.0f)) {
						is_clipped = true;
						break;
					}
				}
				// Typically W shouldn't be part of NDC, but I need it here to do correct matrix multiplication below (VIEWPORT is 4x4)
				// I could also do this in the loop above because as_struct.w == as_array[3], but I think keeping it separate is clearer
				//ndc.vtx[j].as_struct.w = 1.0f;

				if (!is_clipped) {
					screen.vtx[j] = fmat4_Float4_mult (&VIEWPORT, &(ndc.vtx[j]));
				
					// Replace clip coords with screen coords within the Varying struct
					// before passing it on to Tiler
					d.screen_coords[j].as_struct.x =  fixpt_from_float        (screen.vtx[j].as_struct.x,       XY_FRACT_BITS);
					d.screen_coords[j].as_struct.y =  fixpt_from_float        (screen.vtx[j].as_struct.y,       XY_FRACT_BITS);
					d.screen_coords[j].as_struct.z =  fixpt_from_float        (screen.vtx[j].as_struct.z,        Z_FRACT_BITS);
					// We don't need W anymore, but we will need 1/W later:
					d.w_reciprocal [j]             =  fixpt_from_float_no_rnd (reciprocal_w, W_RECIPR_FRACT_BITS);
					
				}
				else break;
			}
			
			if (!is_clipped) {
				
				volatile TrianglePShaderData* volatile tpsd = cfg->tri_data_array;
				//cfg->tri_data_array[tri_num] = d;
				tpsd[tri_num] = d;
				//tiler(cfg->tri_data_array[tri_num], cfg->tile_idx_table_ptr);
				tiler (&(tpsd[tri_num]), cfg->tile_idx_table_ptr, cfg);
				tri_num++;
			}
		}
		
		obj_list_node = obj_list_node->next;
	}
}
