#include "shader.h"
#include "gl.h"

#include <math.h>


#define VSHADER_DEBUG 0
#define PSHADER_DEBUG 0


// declare the following here instead of the header to make these variables local:
float3 VARYING_UV [2];
float3 VARYING_N [3];

// these are global:
fmat4  UNIFORM_M;
fmat4  UNIFORM_MIT;
float3 UNIFORM_LIGHT;



void my_vertex_shader (WFobj *obj, int face_idx, int vtx_idx, fmat4 *mvpv, float4 *vtx4d) {
	
	float3 obj_coords;
	for (int k = 0; k < 3; k++)
		obj_coords[k] = wfobj_get_vtx_coord (obj, face_idx, vtx_idx, k);
	
	float4 mc; // model coordinates
	float4 sc; // screen coordinates	
	// 0. transform 3d coords to homogenous coords
	float3_float4_pt_conv (&obj_coords, &mc);
	fmat4_float4_mult (mvpv, &mc, &sc);
	//float4_float3_pt_conv (&sc, vtx4d);
	(*vtx4d)[0] = sc[0]/sc[3];
	(*vtx4d)[1] = sc[1]/sc[3];
	(*vtx4d)[2] = sc[2]/sc[3];
	(*vtx4d)[3] = sc[3];
	
	for (int i = 0; i < 2; i++)
		VARYING_UV[i][vtx_idx] = wfobj_get_text_coord (obj, face_idx, vtx_idx, i);	
	
	float3 n3;
	float4 n4, nr;
	
	for (int i = 0; i < 3; i++)
		n3[i] = wfobj_get_norm_coord (obj, face_idx, vtx_idx, i);
		
	float3_float4_vect_conv (&n3, &n4);
	
	fmat4_float4_mult (&UNIFORM_MIT, &n4, &nr);
	
	for (int i = 0; i < 3; i++)
		VARYING_N[i][vtx_idx] = nr[i];
	
	if (VSHADER_DEBUG) {
		printf ("\t\tvtx shader face%d vtx%d: obj norm (%f, %f, %f), transformed normal (%f, %f, %f)\n", face_idx, vtx_idx, n3[0], n3[1], n3[2], nr[0], nr[1], nr[2]);
		printf ("\t\tuniform light (%f, %f, %f)\n", UNIFORM_LIGHT[0], UNIFORM_LIGHT[1], UNIFORM_LIGHT[2]);
	}
}

bool my_pixel_shader (WFobj *obj, float3 *barw, pixel_color_t *color) {
	
	int uu = (int) (obj->texture.w * float3_float3_smult (&VARYING_UV[0], barw));
	int vv = (int) (obj->texture.h * float3_float3_smult (&VARYING_UV[1], barw));
	
	pixel_color_t pix;
	wfobj_get_bitmap_rgb (&(obj->texture), uu, vv, &pix.r, &pix.g, &pix.b);
	
	float intensity = 0;
	float diff_intensity = 0;
	float spec_intensity = 0;
	
	// 0 - phong
	// 1 - gouraud
	// 2 - normalmap
	int shader_type = 2;
	
	float3 normal;
			
	if (shader_type == 0) {
		for (int i = 0; i < 3; i++) normal[i] = float3_float3_smult (&VARYING_N[i], barw);
		float3_normalize(&normal);
		diff_intensity = -float3_float3_smult (&normal, &UNIFORM_LIGHT);
	}
	else if (shader_type == 1) {
		float3 interp_intens;
		for (int i = 0; i < 3; i++) {
			float3 ii = {VARYING_N[0][i], VARYING_N[1][i], VARYING_N[2][i]};
			interp_intens[i] = float3_float3_smult (&ii, &UNIFORM_LIGHT);
		}
		diff_intensity = -float3_float3_smult (&interp_intens, barw);
	}
	else if (shader_type == 2) {
		float3 nm3;
		float4 nm4, tmp;
		wfobj_get_bitmap_xyz (&(obj->normalmap), uu, vv, &nm3[0], &nm3[1], &nm3[2]);
		float3_float4_vect_conv (&nm3, &nm4);
		fmat4_float4_mult (&UNIFORM_MIT, &nm4, &tmp);
		float4_float3_vect_conv (&tmp, &normal);
		float3_normalize (&normal);
		diff_intensity = -float3_float3_smult (&normal, &UNIFORM_LIGHT);
	}
	
	bool lighting = 1;
	if (lighting) {
		float nl = diff_intensity; // this is float3_float3_smult (&normal, &UNIFORM_LIGHT), computed above
		float3 nnl2;
		float3_float_mult (&normal, nl * 2.0f, &nnl2);
		float3 r;
		
		float3_float3_add (&nnl2, &UNIFORM_LIGHT, &r);
		float3_normalize (&r);
		
		int spec_factor = wfobj_get_bitmap_int (&(obj->specularmap), uu, vv);		
		
		spec_intensity = (r[Z] < 0) ? 0 : pow (r[Z], spec_factor);
		if (PSHADER_DEBUG)
			if (spec_intensity >= 0.5)
				printf ("n=(%f;%f;%f) nl=%f nnl2=(%f;%f;%f) r=(%f;%f;%f) spec_f=%d spec_i=%f\n",
					normal[0], normal[1], normal[2], nl, nnl2[0], nnl2[1], nnl2[2], r[0], r[1], r[2], spec_factor, spec_intensity);
	}
	
	intensity = 1.0 * diff_intensity + 0.6 * spec_intensity;
	if (intensity < 0) return false;
	int r = pix.r * intensity + 5;
	int g = pix.g * intensity + 5;
	int b = pix.b * intensity + 5;
		
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	
	*color = set_color (r, g, b, 0);
	return true;
}
