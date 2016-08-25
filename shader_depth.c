#include "shader_depth.h"
#include "gl.h"

#include <math.h>




// declare the following here instead of the header to make these variables local:
Float3 DEPTH_VARYING_U;
Float3 DEPTH_VARYING_V;

Float3 DEPTH_VARYING_N[3];

//Float3 DEPTH_PASS1_VARYING_NDC[3];
Float3 DEPTH_PASS2_VARYING_NDC[3];

Float3 DEPTH_PASS2_VARYING_SCREEN[2][3];
//Float3 DEPTH_PASS2_VARYING_SCREEN_2[3];

Float4 depth_vshader_pass1 (Object *obj, int face_idx, int vtx_idx) {
	
	if (DEPTH_VSHADER1_DEBUG) {
		printf ("\tcall depth_vertex_shader()\n");
	}
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 vtx3d = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
	Float4 mc = Float3_Float4_conv (&vtx3d, 1);
	Float4 vtx4d = fmat4_Float4_mult (obj->mvp, &mc);
	
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
	Float4 vtx4d = fmat4_Float4_mult (obj->mvp, &mc);
	
	
	Float4 vtx4d_shadow = fmat4_Float4_mult (&UNIFORM_MVP_SHADOW, &mc);
	int WIDTH  = get_screen_width();
	int HEIGHT = get_screen_height();
	int DEPTH  = get_screen_depth();
	DEPTH_PASS2_VARYING_SCREEN[0][0].as_array[vtx_idx] = WIDTH/2.0 + (vtx4d_shadow.as_array[0] / vtx4d_shadow.as_struct.w) * HEIGHT / 2.0;
	DEPTH_PASS2_VARYING_SCREEN[0][1].as_array[vtx_idx] = (vtx4d_shadow.as_array[1] / vtx4d_shadow.as_struct.w + 1.0) * HEIGHT / 2.0;
	DEPTH_PASS2_VARYING_SCREEN[0][2].as_array[vtx_idx] = DEPTH * (1.0 - vtx4d_shadow.as_array[2] / vtx4d_shadow.as_struct.w) / 2.0;
	
	Float4 vtx4d_shadow_2 = fmat4_Float4_mult (&UNIFORM_MVP_SHADOW_2, &mc);
	DEPTH_PASS2_VARYING_SCREEN[1][0].as_array[vtx_idx] = WIDTH/2.0 + (vtx4d_shadow_2.as_array[0] / vtx4d_shadow_2.as_struct.w) * HEIGHT / 2.0;
	DEPTH_PASS2_VARYING_SCREEN[1][1].as_array[vtx_idx] = (vtx4d_shadow_2.as_array[1] / vtx4d_shadow_2.as_struct.w + 1.0) * HEIGHT / 2.0;
	DEPTH_PASS2_VARYING_SCREEN[1][2].as_array[vtx_idx] = DEPTH * (1.0 - vtx4d_shadow_2.as_array[2] / vtx4d_shadow_2.as_struct.w) / 2.0;
	
	
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
	
	Float3 screen;
	screenz_t current_z[2];
	screenz_t shadow_buf_z[2];
	float shadow[2];
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 3; j++) {
			screen.as_array[j] = Float3_Float3_smult (&DEPTH_PASS2_VARYING_SCREEN[i][j], barw);
		}
		int x = (int) screen.as_struct.x;
		int y = (int) screen.as_struct.y;
		current_z[i]    = (screenz_t) screen.as_struct.z;
		shadow_buf_z[i] = (screenz_t) UNIFORM_SHADOWBUF[i][x + y*get_screen_width()];
		
		shadow[i] = (shadow_buf_z[i] > current_z[i]+5) ? 0.2 : 1.0; // +5 for z-fighting
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
	float diff_intensity[NUM_OF_LIGHTS];
	for (int i = 0; i < NUM_OF_LIGHTS; i++) {
		diff_intensity[i] = -Float3_Float3_smult (&normal, &UNIFORM_LIGHT[i]);
	}
	
	float spec_intensity = 0;	
	if (obj->specularmap != NULL) {
		float nl = diff_intensity[0]; //TBD this is float3_float3_smult (&normal, &UNIFORM_LIGHT), computed above
		Float3 nnl2 = Float3_float_mult (&normal, nl * 2.0f);
		Float3 r    = Float3_Float3_add (&nnl2, &UNIFORM_LIGHT[0]);
		Float3_normalize (&r);
		
		int spec_factor = wfobj_get_specularity_from_map (obj, uu, vv);
		spec_intensity = (r.as_struct.z < 0) ? 0 : pow (r.as_struct.z, spec_factor);
		
		if (DEPTH_PSHADER2_DEBUG) {
			if (spec_intensity >= 0.5) {
				printf ("n=(%f;%f;%f) ", normal.as_struct.x, normal.as_struct.y, normal.as_struct.z);
				printf ("nl=%f ", nl);
				printf ("nnl2=(%f;%f;%f) ", nnl2.as_struct.x, nnl2.as_struct.y, nnl2.as_struct.z);
				printf ("r=(%f;%f;%f) ", r.as_struct.x, r.as_struct.y, r.as_struct.z);
				printf ("spec_f=%d spec_i=%f\n", spec_factor, spec_intensity);
			}
		}
	}
	
	float intensity = shadow[0] * shadow[1] * (1.0 * (diff_intensity[0] + diff_intensity[1]) + 0.6 * spec_intensity);
	//else intensity = 0;
	
	//if (intensity <= 0) return false;
	if (intensity <= 0.1) intensity = 0.2;//return false;
	
	if (DEPTH_PSHADER2_DEBUG) {
		printf ("n=(%f;%f;%f) ", normal.as_struct.x, normal.as_struct.y, normal.as_struct.z);
		printf ("light=(%f;%f;%f) ", UNIFORM_LIGHT[0].as_struct.x, UNIFORM_LIGHT[0].as_struct.y, UNIFORM_LIGHT[0].as_struct.z);
		printf ("diff_int=%f ", diff_intensity);
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
