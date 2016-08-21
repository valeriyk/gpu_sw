#include "shader_depth.h"
#include "gl.h"

#include "main.h"

#include <math.h>




// declare the following here instead of the header to make these variables local:
Float3 DEPTH_VARYING_U;
Float3 DEPTH_VARYING_V;

Float3 DEPTH_VARYING_N[3];

//Float3 DEPTH_PASS1_VARYING_NDC[3];
Float3 DEPTH_PASS2_VARYING_NDC[3];
Float3 DEPTH_PASS2_VARYING_SCREEN[3];

Float4 depth_vshader_pass1 (WFobj *obj, int face_idx, int vtx_idx, fmat4 *mvp) {
	
	if (DEPTH_VSHADER1_DEBUG) {
		printf ("\tcall depth_vertex_shader()\n");
	}
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 vtx3d = wfobj_get_vtx_coords (obj, face_idx, vtx_idx);
	Float4 mc = Float3_Float4_pt_conv (&vtx3d);
	Float4 vtx4d = fmat4_Float4_mult (mvp, &mc);
	
	/*for (int i = 0; i < 3; i++) {
		if (vtx4d.as_struct.w != 1.0) printf ("unexpected shadow build W != 1, W = %f\n", vtx4d.as_struct.w);
		DEPTH_PASS1_VARYING_NDC[i].as_array[vtx_idx] = vtx4d.as_array[i];// don't divide, W=1 for ortho proj / vtx4d.as_struct.w;
	}*/
	
	return vtx4d;
}

bool depth_pshader_pass1 (WFobj *obj, Float3 *barw, pixel_color_t *color) {
	
	if (DEPTH_PSHADER1_DEBUG) {
		printf ("\t\tcall phong_pixel_shader()\n");
	}
	
	/*Float3 ndc;
	for (int i = 0; i < 3; i++) {
		ndc.as_array[i] = Float3_Float3_smult (&DEPTH_PASS1_VARYING_NDC[i], barw);
	}
	for (int i = 0; i < 3; i++)
		if ((ndc.as_array[i] > 1.0) || (ndc.as_array[i] < -1.0)) return false;
	
	
	int sb_x = (int) WIDTH / 2.0 + ndc.as_struct.x * HEIGHT / 2.0;
	int sb_y = (int) ((ndc.as_struct.y + 1.0) * HEIGHT / 2.0);
	int sb_z = (int) DEPTH * (1.0 - ndc.as_struct.z) / 2.0;
	printf ("shadowbuffer fill: %f %f %f - %d %d %d\n", ndc.as_struct.x, ndc.as_struct.y, ndc.as_struct.z, sb_x, sb_y, sb_z);
		
	//screenz_t val = DEPTH * (1.0 - ndc.as_struct.z) / 2.0;
	//*color = set_color (val, val, val, 0);*/
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Float4 depth_vshader_pass2 (WFobj *obj, int face_idx, int vtx_idx, fmat4 *mvp) {
	
	if (DEPTH_VSHADER2_DEBUG) {
		printf ("\tcall depth_vertex_shader()\n");
	}
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 vtx3d = wfobj_get_vtx_coords (obj, face_idx, vtx_idx);
	Float4 mc = Float3_Float4_pt_conv (&vtx3d);
	Float4 vtx4d = fmat4_Float4_mult (mvp, &mc);
	
	
	Float4 vtx4d_shadow = fmat4_Float4_mult (&UNIFORM_MVP_SHADOW, &mc);
	if (0) {
		for (int i = 0; i < 3; i++) {
			DEPTH_PASS2_VARYING_NDC[i].as_array[vtx_idx] = vtx4d.as_array[i] / vtx4d.as_struct.w;
		}	
	}
	else {
		DEPTH_PASS2_VARYING_SCREEN[0].as_array[vtx_idx] = WIDTH/2.0 + (vtx4d_shadow.as_array[0] / vtx4d_shadow.as_struct.w) * HEIGHT / 2.0;
		DEPTH_PASS2_VARYING_SCREEN[1].as_array[vtx_idx] = (vtx4d_shadow.as_array[1] / vtx4d_shadow.as_struct.w + 1.0) * HEIGHT / 2.0;
		DEPTH_PASS2_VARYING_SCREEN[2].as_array[vtx_idx] = DEPTH * (1.0 - vtx4d_shadow.as_array[2] / vtx4d_shadow.as_struct.w) / 2.0;
	}
	
	// extract the texture UV coordinates of the vertex
	if (obj->texture != NULL) {
		Float2 vtx_uv = wfobj_get_texture_coords (obj, face_idx, vtx_idx);
		DEPTH_VARYING_U.as_array[vtx_idx] = vtx_uv.as_struct.u;
		DEPTH_VARYING_V.as_array[vtx_idx] = vtx_uv.as_struct.v;
	}
	
	// transform the normal vector to the vertex
	Float3 norm3d = wfobj_get_norm_coords    (obj, face_idx, vtx_idx);
	Float4 norm4d = Float3_Float4_vect_conv  (&norm3d);
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
	for (int i = 0; i < 3; i++) {
		screen.as_array[i] = Float3_Float3_smult (&DEPTH_PASS2_VARYING_SCREEN[i], barw);
	}
	//for (int i = 0; i < 3; i++)
		//if ((ndc.as_array[i] > 1.0) || (ndc.as_array[i] < -1.0)) return false;
	
	//Float4 ndc4 = Float3_Float4_pt_conv (&ndc);
	//Float4 clip;
	//for (int i = 0; i < 3; i++) clip.as_array[i] = ndc.as_array[i] * DEPTH_PASS2_VARYING_W.as_array[i];
	//clip.as_struct.w = Float3_Float3_smult (&DEPTH_PASS2_VARYING_W, barw);
	
	//Float4 inv = fmat4_Float4_mult (&UNIFORM_MVP_INV, &ndc4);
	//printf ("inverted bar xy: %f %f\n", inv.as_struct.x, inv.as_struct.y); //, sb_p.as_struct.z, sb_x, sb_y, sb_z, shadow_z);
	
	//Float4 sb_p = fmat4_Float4_mult (&UNIFORM_MVP_SHADOW, &inv);
	// value in shadowbuffer for this pixel
	//if (sb_p.as_struct.w != 1.0) printf ("unexpected shadow reconstruction W != 1, W = %f\n", sb_p.as_struct.w);
	//int sb_x = (int) WIDTH / 2.0 + sb_p.as_struct.x * HEIGHT / 2.0;
	//int sb_y = (int) ((sb_p.as_struct.y + 1.0) * HEIGHT / 2.0);
	//int sb_z = (int) DEPTH * (1.0 - sb_p.as_struct.z) / 2.0;
	//printf ("depth pshader2: ndc3: %f %f %f  ndc4: %f %f %f %f  shadowbuf: %f(%d) %f(%d) %f(%d)\n", ndc.as_struct.x, ndc.as_struct.y, ndc.as_struct.z, ndc4.as_struct.x, ndc4.as_struct.y, ndc4.as_struct.z, ndc4.as_struct.w, sb_p.as_struct.x, sb_x, sb_p.as_struct.y, sb_y, sb_p.as_struct.z, sb_z);
	// actual value in the shadowbuffer
	//screenz_t shadow_z = (screenz_t) *(UNIFORM_SHADOWBUF + (sb_x + sb_y*WIDTH)*sizeof(screenz_t));
	int sb_x = (int) screen.as_struct.x;
	int sb_y = (int) screen.as_struct.y;
	int sb_z = (int) screen.as_struct.z;
	screenz_t shadow_z = (screenz_t) UNIFORM_SHADOWBUF[sb_x + sb_y*WIDTH];
	
	//printf ("shadowbuffer: %f %f %f - %d %d %d - %d", sb_p.as_struct.x, sb_p.as_struct.y, sb_p.as_struct.z, sb_x, sb_y, sb_z, shadow_z);
	//if (shadow_z > sb_z) {printf (" - in shadow\n");} else printf("\n");
	
	float shadow = 1.0;
	printf ("\tshadowbuffer: %d, my shadow: %d\n", shadow_z, sb_z);
	if (shadow_z > sb_z) shadow = 0.3;
	
	
	
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
	float diff_intensity = -Float3_Float3_smult (&normal, &UNIFORM_LIGHT);
	
	float spec_intensity = 0;	
	if (obj->specularmap != NULL) {
		float nl = diff_intensity; // this is float3_float3_smult (&normal, &UNIFORM_LIGHT), computed above
		Float3 nnl2 = Float3_float_mult (&normal, nl * 2.0f);
		Float3 r    = Float3_Float3_add (&nnl2, &UNIFORM_LIGHT);
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
	
	float intensity = shadow * (1.0 * diff_intensity + 0.6 * spec_intensity);
	//else intensity = 0;
	
	//if (intensity <= 0) return false;
	if (intensity <= 0.1) intensity = 0.1;//return false;
	
	if (DEPTH_PSHADER2_DEBUG) {
		printf ("n=(%f;%f;%f) ", normal.as_struct.x, normal.as_struct.y, normal.as_struct.z);
		printf ("light=(%f;%f;%f) ", UNIFORM_LIGHT.as_struct.x, UNIFORM_LIGHT.as_struct.y, UNIFORM_LIGHT.as_struct.z);
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
