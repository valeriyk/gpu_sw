#include "shader_depth.h"
#include "gl.h"

#include <math.h>


void depth_vshader_pass1 (Object *obj, size_t face_idx, size_t vtx_idx, Varying *var) {
	
	if (DEPTH_VSHADER1_DEBUG) {
		printf ("\tcall depth_vertex_shader()\n");
	}
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 vtx3d = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
	Float4 mc    = Float3_Float4_conv   (&vtx3d, 1);
	Float4 vtx4d = fmat4_Float4_mult    (&(obj->mvp), &mc);
	
	//return vtx4d;
	var->as_Float4[0] = vtx4d;
}

bool depth_pshader_pass1 (Object *obj, size_t tri_idx, Varying *var, pixel_color_t *color) {
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void depth_vshader_pass2 (Object *obj, size_t face_idx, size_t vtx_idx, Varying *var) {
	
	if (DEPTH_VSHADER2_DEBUG) {
		printf ("\tcall depth_vertex_shader()\n");
	}
	
	// Layout of 32 varying words:
	//     
	//     0    1    2    3
	// 0   x    y    z    w   - coords of the vertex
	// 1   xn   yn   zn   wn  - coords of the normal
	// 2   u    v             - texture coords
	// 3   xs0  ys0  zs0  ws0 - coords of the shadow 0
	// 4   xs1  ys1  zs1  ws1 - coords of the shadow 1
	// 5   xs2  ys2  zs2  ws2 - coords of the shadow 2
	// 6   xs3  ys3  zs3  ws3 - coords of the shadow 3
	// 7   -    -    -    -   - not used
	
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 vtx3d = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
	Float4 mc = Float3_Float4_conv (&vtx3d, 1);
	var->as_Float4[0] = fmat4_Float4_mult (&(obj->mvp), &mc);
	
	// transform the normal vector to the vertex
	Float3 norm3d = wfobj_get_norm_coords    (obj->wfobj, face_idx, vtx_idx);
	Float4 norm4d = Float3_Float4_conv  (&norm3d, 0);
	var->as_Float4[1] = fmat4_Float4_mult (&UNIFORM_MIT, &norm4d);;
	
	// extract the texture UV coordinates of the vertex
	if (obj->wfobj->texture != NULL) {
		var->as_Float2[4] = wfobj_get_texture_coords (obj->wfobj, face_idx, vtx_idx);
	}
	
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		if (!LIGHTS[i].enabled) continue;
		Float4 shadow_vtx4d = fmat4_Float4_mult (&(obj->shadow_mvp[i]), &mc); // clip
		
		//Perspective divide is only needed when perspective projection is used for shadows
		// By default I sue orthographic projection, so commenting out the section below
		/*	
		// Compute XYZ in NDC by dividing XYZ in clip space by W (i.e. multiplying by 1/W)
		// If at least one coord belongs to [-1:1] then the vertex is not clipped
		for (int k = 0; k < 4; k++) {
			//TBD: danger, no check for div by zero yet implemented
			shadow_vtx4d.as_array[k] = shadow_vtx4d.as_array[k] / shadow_vtx4d.as_struct.w; // normalize
		}
		*/		
		var->as_Float4[3+i] = fmat4_Float4_mult (&VIEWPORT, &shadow_vtx4d);
	}	
}

bool depth_pshader_pass2 (Object *obj, size_t tri_idx, Varying *var, pixel_color_t *color) {
	
	if (DEPTH_PSHADER2_DEBUG) {
		printf ("\t\tcall depth_pshader_pass2()\n");
	}
	
	Float3    screen;
	int shadows = 0;
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		
		if (!LIGHTS[i].enabled) continue;
		
		screen = Float4_Float3_vect_conv (&(var->as_Float4[3+i]));
		
		int x        = (int)       screen.as_struct.x;
		int y        = (int)       screen.as_struct.y;
		screenz_t current_z = (screenz_t) screen.as_struct.z;
		
		if ((x < 0) || (y < 0) || (x >= get_screen_width()) || (y >= get_screen_height())) continue;	
		
		screenz_t shadow_buf_z = LIGHTS[i].shadow_buf[x + y*get_screen_width()];
		
		float z_fighting = 251.77f;
		if (shadow_buf_z > current_z + z_fighting) {
			shadows++;
		}
	}
	
	int uu = (int) var->as_float[8];
	int vv = (int) var->as_float[9];
	if (uu < 0 || vv < 0) return false;
	
	pixel_color_t pix;
	if (obj->wfobj->texture != NULL) {
		if (uu >= obj->wfobj->texture->w || vv >= obj->wfobj->texture->h) return false;
		wfobj_get_rgb_from_texture (obj->wfobj, uu, vv, &pix.r, &pix.g, &pix.b);
	}
	else {
		pix = set_color (128, 128, 128, 0);
	}
	
	Float3 normal = Float4_Float3_vect_conv (&(var->as_Float4[1]));
	Float3_normalize (&normal);
	float diff_intensity[MAX_NUM_OF_LIGHTS];
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		diff_intensity[i] = (LIGHTS[i].enabled) ? -Float3_Float3_smult (&normal, &(LIGHTS[i].eye)) : 0;
	}
	
	float spec_intensity = 0;	
	if (obj->wfobj->specularmap != NULL) {
		float nl = diff_intensity[0]; //TBD this is float3_float3_smult (&normal, &UNIFORM_LIGHT), computed above
		Float3 nnl2 = Float3_float_mult (&normal, nl * 2.0f);
		Float3 r    = Float3_Float3_add (&nnl2, &(LIGHTS[0].eye));
		Float3_normalize (&r);
		
		int spec_factor = wfobj_get_specularity_from_map (obj->wfobj, uu, vv);
		spec_intensity = (r.as_struct.z < 0) ? 0 : powf (r.as_struct.z, spec_factor);
		
		if (DEPTH_PSHADER2_DEBUG) {
			if (spec_intensity >= 0.5f) {
				printf ("n=(%f;%f;%f) ", normal.as_struct.x, normal.as_struct.y, normal.as_struct.z);
				printf ("nl=%f ", nl);
				printf ("nnl2=(%f;%f;%f) ", nnl2.as_struct.x, nnl2.as_struct.y, nnl2.as_struct.z);
				printf ("r=(%f;%f;%f) ", r.as_struct.x, r.as_struct.y, r.as_struct.z);
				printf ("spec_f=%d spec_i=%f\n", spec_factor, spec_intensity);
			}
		}
	}
	
	float shadow_total = 1.0f - shadows/4.0f;
	
	float diff_int_total = 0;
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		diff_int_total += diff_intensity[i];
	}
	float intensity = shadow_total * (0.5f * diff_int_total + 0.6f * spec_intensity);
	
	if (intensity <= 0.1) intensity = 0.2f;
	
	if (DEPTH_PSHADER2_DEBUG) {
		printf ("n=(%f;%f;%f) ", normal.as_struct.x, normal.as_struct.y, normal.as_struct.z);
		printf ("light=(%f;%f;%f) ", LIGHTS[0].eye.as_struct.x, LIGHTS[0].eye.as_struct.y, LIGHTS[0].eye.as_struct.z);
		//printf ("diff_int=%f ", diff_intensity);
	}
		
	int r = pix.r * intensity + 5;
	int g = pix.g * intensity + 5;
	int b = pix.b * intensity + 5;
		
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	
	*color = set_color (r, g, b, 0);
	return true;
}
