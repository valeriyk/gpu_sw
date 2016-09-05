#include "shader_normalmap.h"
#include "gl.h"

#include <math.h>

/*

// declare the following here instead of the header to make these variables local:
Float3 NM_VARYING_U;
Float3 NM_VARYING_V;

// returns normalized device coordinates (NDC)
Float4 nm_vertex_shader (WFobj *obj, int face_idx, int vtx_idx, fmat4 *mvpv) {
	
	if (NM_VSHADER_DEBUG) {
		printf ("\tcall nm_vertex_shader()\n");
	}
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 vtx3d = wfobj_get_vtx_coords (obj, face_idx, vtx_idx);
	Float4 mc = Float3_Float4_conv (&vtx3d, 1);
	Float4 vtx4d = fmat4_Float4_mult (mvpv, &mc);
	
	// extract the texture UV coordinates of the vertex
	if (obj->texture != NULL) {
		Float2 vtx_uv = wfobj_get_texture_coords (obj, face_idx, vtx_idx);
		NM_VARYING_U.as_array[vtx_idx] = vtx_uv.as_struct.u;
		NM_VARYING_V.as_array[vtx_idx] = vtx_uv.as_struct.v;
	};
	
	return vtx4d;
}

bool nm_pixel_shader (WFobj *obj, Float3 *barw, pixel_color_t *color) {
	
	if (NM_PSHADER_DEBUG) {
		printf ("\t\tcall nm_pixel_shader()\n");
	}
	
	int uu = (int) Float3_Float3_smult (&NM_VARYING_U, barw);
	int vv = (int) Float3_Float3_smult (&NM_VARYING_V, barw);
	
	pixel_color_t pix;
	if (obj->texture != NULL) {
		if (uu >= obj->texture->w || vv >= obj->texture->h) return false;
		wfobj_get_rgb_from_texture (obj, uu, vv, &pix.r, &pix.g, &pix.b);
	}
	else {
		pix = set_color (128, 128, 128, 0);
	}
	
	Float3 normal;
	Float3 nm3 = wfobj_get_normal_from_map (obj, uu, vv);
	Float4 nm4 = Float3_Float4_conv (&nm3, 0);
	Float4 tmp = fmat4_Float4_mult (&UNIFORM_MIT, &nm4);
	normal     = Float4_Float3_vect_conv (&tmp);
	Float3_normalize (&normal);
	float diff_intensity = -Float3_Float3_smult (&normal, &(LIGHTS[0].eye));
	
	float spec_intensity = 0;	
	if (obj->specularmap != NULL) {
		float nl = diff_intensity; // this is float3_float3_smult (&normal, &UNIFORM_LIGHT), computed above
		Float3 nnl2 = Float3_float_mult (&normal, nl * 2.0f);
		Float3 r    = Float3_Float3_add (&nnl2, &(LIGHTS[0].eye));
		Float3_normalize (&r);
		
		int spec_factor = wfobj_get_specularity_from_map (obj, uu, vv);
		spec_intensity = (r.as_struct.z < 0) ? 0 : powf (r.as_struct.z, spec_factor);
		
		if (NM_PSHADER_DEBUG) {
			if (spec_intensity >= 0.5) {
				printf ("n=(%f;%f;%f) ", normal.as_struct.x, normal.as_struct.y, normal.as_struct.z);
				printf ("nl=%f ", nl);
				printf ("nnl2=(%f;%f;%f) ", nnl2.as_struct.x, nnl2.as_struct.y, nnl2.as_struct.z);
				printf ("r=(%f;%f;%f) ", r.as_struct.x, r.as_struct.y, r.as_struct.z);
				printf ("spec_f=%d spec_i=%f\n", spec_factor, spec_intensity);
			}
		}
	}
	
	float intensity = 1.0f * diff_intensity + 0.6f * spec_intensity;
	
	if (NM_PSHADER_DEBUG) {
		if (spec_intensity > 0.5) printf ("spec%f diff%f total%f\n", spec_intensity, diff_intensity, intensity);
	}
	
	if (intensity < 0) return false;
	int r = pix.r * intensity + 5;
	int g = pix.g * intensity + 5;
	int b = pix.b * intensity + 5;
		
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	
	if (NM_PSHADER_DEBUG) {
		if (spec_intensity > 0.5) printf ("r%d g%d b%d\t", r, g, b);
	}
	
	*color = set_color (r, g, b, 0);
	return true;
}
*/
