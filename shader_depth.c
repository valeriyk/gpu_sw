#include "shader_depth.h"
#include "gl.h"

#include <math.h>




// declare the following here instead of the header to make these variables local:
Float3 DEPTH_VARYING_U;
Float3 DEPTH_VARYING_V;

Float3 DEPTH_VARYING_N[3];

//Float3 DEPTH_PASS1_VARYING_NDC[3];
Float3 DEPTH_PASS2_VARYING_NDC[3];

Float3 DEPTH_PASS2_VARYING_SCREEN[MAX_NUM_OF_LIGHTS][3];
//Float3 DEPTH_PASS2_VARYING_SCREEN_2[3];

Float4 depth_vshader_pass1 (Object *obj, int face_idx, int vtx_idx) {
	
	if (DEPTH_VSHADER1_DEBUG) {
		printf ("\tcall depth_vertex_shader()\n");
	}
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 vtx3d = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
	Float4 mc = Float3_Float4_conv (&vtx3d, 1);
	Float4 vtx4d = fmat4_Float4_mult (&(obj->mvp), &mc);
	
	return vtx4d;
}

bool depth_pshader_pass1 (WFobj *obj, Float3 *barw, pixel_color_t *color) {
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Float4 depth_vshader_pass2 (Object *obj, int face_idx, int vtx_idx) {
	
	if (DEPTH_VSHADER2_DEBUG) {
		printf ("\tcall depth_vertex_shader()\n");
	}
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 vtx3d = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
	Float4 mc = Float3_Float4_conv (&vtx3d, 1);
	Float4 vtx4d = fmat4_Float4_mult (&(obj->mvp), &mc);
	
	int WIDTH  = get_screen_width();
	int HEIGHT = get_screen_height();
	int DEPTH  = get_screen_depth();
	
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		if (!LIGHTS[i].enabled) continue;
		Float4 vtx4d_shadow = fmat4_Float4_mult (&(obj->shadow_mvp[i]), &mc);
		DEPTH_PASS2_VARYING_SCREEN[i][0].as_array[vtx_idx] = WIDTH/2.0f + (vtx4d_shadow.as_array[0] / vtx4d_shadow.as_struct.w) * HEIGHT / 2.0f;
		DEPTH_PASS2_VARYING_SCREEN[i][1].as_array[vtx_idx] = (vtx4d_shadow.as_array[1] / vtx4d_shadow.as_struct.w + 1.0f) * HEIGHT / 2.0f;
		DEPTH_PASS2_VARYING_SCREEN[i][2].as_array[vtx_idx] = DEPTH * (1.0f - vtx4d_shadow.as_array[2] / vtx4d_shadow.as_struct.w) / 2.0f;
	}
	
	// extract the texture UV coordinates of the vertex
	if (obj->wfobj->texture != NULL) {
		Float2 vtx_uv = wfobj_get_texture_coords (obj->wfobj, face_idx, vtx_idx);
		DEPTH_VARYING_U.as_array[vtx_idx] = vtx_uv.as_struct.u;
		DEPTH_VARYING_V.as_array[vtx_idx] = vtx_uv.as_struct.v;
	}
	
	// transform the normal vector to the vertex
	Float3 norm3d = wfobj_get_norm_coords    (obj->wfobj, face_idx, vtx_idx);
	Float4 norm4d = Float3_Float4_conv  (&norm3d, 0);
	norm4d = fmat4_Float4_mult (&UNIFORM_MIT, &norm4d);
	for (int i = 0; i < 3; i++) {
		DEPTH_VARYING_N[i].as_array[vtx_idx] = norm4d.as_array[i];
	}
	
	
	
	return vtx4d;
}

bool depth_pshader_pass2 (WFobj *obj, Float3 *barw, pixel_color_t *color) {
	
	if (DEPTH_PSHADER2_DEBUG) {
		printf ("\t\tcall depth_pshader_pass2()\n");
	}
	
	Float3    screen;
	screenz_t current_z    [MAX_NUM_OF_LIGHTS];
	screenz_t shadow_buf_z [MAX_NUM_OF_LIGHTS];
	
	int shadows = 0;
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		//shadow[i] = 0;
		
		if (!LIGHTS[i].enabled) continue;
		
		for (int j = 0; j < 3; j++) {
			screen.as_array[j] = Float3_Float3_smult (&DEPTH_PASS2_VARYING_SCREEN[i][j], barw);
		}
		int x = (int) screen.as_struct.x;
		int y = (int) screen.as_struct.y;
		
		if ((x < 0) || (y < 0) || (x >= get_screen_width()) || (y >= get_screen_height())) continue;
		
		current_z[i]    = (screenz_t) screen.as_struct.z;
		
		shadow_buf_z[i] = LIGHTS[i].shadow_buf[x + y*get_screen_width()];
		
		//
		//shadow[i] = (shadow_buf_z[i] > current_z[i]+5) ? 0.2 : 1.0; // +5 for z-fighting
		float z_fighting = 50.0f;
		if (shadow_buf_z[i] > current_z[i] + z_fighting) {
			shadows++;
		}
		/*else {
			if (shadows > 0) shadows--;
		}*/
	}
	
	int uu = (int) Float3_Float3_smult (&DEPTH_VARYING_U, barw);
	int vv = (int) Float3_Float3_smult (&DEPTH_VARYING_V, barw);
	if (uu < 0 || vv < 0) return false;
	
	pixel_color_t pix;
	if (obj->texture != NULL) {
		if (uu >= obj->texture->w || vv >= obj->texture->h) return false;
		wfobj_get_rgb_from_texture (obj, uu, vv, &pix.r, &pix.g, &pix.b);
	}
	else {
		pix = set_color (128, 128, 128, 0);
	}
	
	Float3 normal;
	for (int i = 0; i < 3; i++) {
		normal.as_array[i] = Float3_Float3_smult (&DEPTH_VARYING_N[i], barw);
	}
	Float3_normalize(&normal);
	float diff_intensity[MAX_NUM_OF_LIGHTS];
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		diff_intensity[i] = (LIGHTS[i].enabled) ? -Float3_Float3_smult (&normal, &(LIGHTS[i].eye)) : 0;
	}
	
	float spec_intensity = 0;	
	if (obj->specularmap != NULL) {
		float nl = diff_intensity[0]; //TBD this is float3_float3_smult (&normal, &UNIFORM_LIGHT), computed above
		Float3 nnl2 = Float3_float_mult (&normal, nl * 2.0f);
		Float3 r    = Float3_Float3_add (&nnl2, &(LIGHTS[0].eye));
		Float3_normalize (&r);
		
		int spec_factor = wfobj_get_specularity_from_map (obj, uu, vv);
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
