#include "shader.h"
#include "gl.h"

#include <math.h>


#define VSHADER_DEBUG 0
#define PSHADER_DEBUG 0


// declare the following here instead of the header to make these variables local:
float3 VARYING_U;
float3 VARYING_V;
float3 VARYING_N[3];

// these are global:
fmat4  UNIFORM_M;
fmat4  UNIFORM_MIT;
Float3 UNIFORM_LIGHT;



Float4 my_vertex_shader (WFobj *obj, int face_idx, int vtx_idx, fmat4 *mvpv) {
	
	Float4 vtx4d;
	
	Float3 vtx_coords;
	wfobj_get_vtx_coords (obj, face_idx, vtx_idx, &vtx_coords.as_struct.x, &vtx_coords.as_struct.y, &vtx_coords.as_struct.z);
	
	// transform 3d coords to homogenous coords
	Float4 mc = Float3_Float4_pt_conv (&vtx_coords);
	Float4 sc =	fmat4_Float4_mult (mvpv, &mc);
	//float4_float3_pt_conv (&sc, vtx4d);
	vtx4d.as_array[0] = sc.as_array[0] / sc.as_array[3];
	vtx4d.as_array[1] = sc.as_array[1] / sc.as_array[3];
	vtx4d.as_array[2] = sc.as_array[2] / sc.as_array[3];
	vtx4d.as_array[3] = sc.as_array[3];
	
	wfobj_get_texture_coords (obj, face_idx, vtx_idx, &VARYING_U[vtx_idx], &VARYING_V[vtx_idx]);	
	
	Float3 n3;
	wfobj_get_norm_coords    (obj, face_idx, vtx_idx, &n3.as_struct.x, &n3.as_struct.y, &n3.as_struct.z);
	Float4 n4 = Float3_Float4_vect_conv  (&n3);
	Float4 nr = fmat4_Float4_mult (&UNIFORM_MIT, &n4);
	
	for (int i = 0; i < 3; i++)
		VARYING_N[i][vtx_idx] = nr.as_array[i];
	
	if (VSHADER_DEBUG) {
		printf ("\t\tvtx shader face%d vtx%d: obj norm (%f, %f, %f), transformed normal (%f, %f, %f)\n", face_idx, vtx_idx, n3.as_struct.x, n3.as_struct.y, n3.as_struct.z, nr.as_struct.x, nr.as_struct.y, nr.as_struct.z);
		printf ("\t\tuniform light (%f, %f, %f)\n", UNIFORM_LIGHT.as_struct.x, UNIFORM_LIGHT.as_struct.y, UNIFORM_LIGHT.as_struct.z);
	}
	
	return vtx4d;
}

bool my_pixel_shader (WFobj *obj, float3 *barw, pixel_color_t *color) {
	
	int uu = (int) (obj->texture->w * float3_float3_smult (&VARYING_U, barw));
	int vv = (int) (obj->texture->h * float3_float3_smult (&VARYING_V, barw));
	
	pixel_color_t pix;
	wfobj_get_rgb_from_texture (obj, uu, vv, &pix.r, &pix.g, &pix.b);
	
	float intensity = 0;
	float diff_intensity = 0;
	float spec_intensity = 0;
	
	// 0 - phong
	// 1 - gouraud
	// 2 - normalmap
	int shader_type = 2;
	
	Float3 normal;
			
	if (shader_type == 0) {
		for (int i = 0; i < 3; i++) normal.as_array[i] = float3_float3_smult (&VARYING_N[i], barw);
		Float3_normalize(&normal);
		diff_intensity = -float3_float3_smult (&normal.as_array, &UNIFORM_LIGHT.as_array);
	}
	else if (shader_type == 1) {
		float3 interp_intens;
		for (int i = 0; i < 3; i++) {
			float3 ii = {VARYING_N[0][i], VARYING_N[1][i], VARYING_N[2][i]};
			interp_intens[i] = float3_float3_smult (&ii, &UNIFORM_LIGHT.as_array);
		}
		diff_intensity = -float3_float3_smult (&interp_intens, barw);
	}
	else if (shader_type == 2) {
		Float3 nm3;
		wfobj_get_normal_from_map (obj, uu, vv, &nm3.as_struct.x, &nm3.as_struct.y, &nm3.as_struct.z);
		Float4 nm4 = Float3_Float4_vect_conv (&nm3);
		Float4 tmp = fmat4_Float4_mult (&UNIFORM_MIT, &nm4);
		normal = Float4_Float3_vect_conv (&tmp);
		Float3_normalize (&normal);
		diff_intensity = -float3_float3_smult (&normal.as_array, &UNIFORM_LIGHT.as_array);
	}
	
	bool lighting = 1;
	if (lighting) {
		float nl = diff_intensity; // this is float3_float3_smult (&normal, &UNIFORM_LIGHT), computed above
		float3 nnl2;
		float3_float_mult (&normal.as_array, nl * 2.0f, &nnl2);
		Float3 r;
		
		float3_float3_add (&nnl2, &UNIFORM_LIGHT.as_array, &r.as_array);
		Float3_normalize  (&r);
		
		int spec_factor = wfobj_get_specularity_from_map (obj, uu, vv);
		
		spec_intensity = (r.as_struct.z < 0) ? 0 : pow (r.as_struct.z, spec_factor);
		if (PSHADER_DEBUG)
			if (spec_intensity >= 0.5)
				printf ("n=(%f;%f;%f) nl=%f nnl2=(%f;%f;%f) r=(%f;%f;%f) spec_f=%d spec_i=%f\n",
					normal.as_struct.x, normal.as_struct.y, normal.as_struct.z, nl, nnl2[0], nnl2[1], nnl2[2], r.as_struct.x, r.as_struct.y, r.as_struct.z, spec_factor, spec_intensity);
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
