#include "shader_phong.h"
#include "gl.h"

#include "geometry_fixpt.h"

#include <math.h>



#define FLOAT_NORM   1
#define FLOAT_TEXT   1
#define FLOAT_SHADOW 1




////////////////////////////////////////////////////////////////////////
// This shader illustrates usage of dynamic shadows.
// Shadow buffer must already be generated for this frame.
////////////////////////////////////////////////////////////////////////




// Layout of Varying words for this pass:
//     
// 0   normal.x
// 1   normal.y
// 2   normal.z
// 3   texture.u
// 4   texture.v
// ....

Float4 vshader_phong (Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry, gpu_cfg_t *cfg) {
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 model   = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
	Float4 model4d = Float3_Float4_conv (&model, 1);
	Float4 clip    = fmat4_Float4_mult (&(obj->mvp), &model4d); // model -> world -> eye -> clip
	
	// transform the normal vector to the vertex
	Float3 norm       = wfobj_get_norm_coords    (obj->wfobj, face_idx, vtx_idx);
	Float4 norm4d     = Float3_Float4_conv  (&norm, 0);
	Float4 norm4d_eye = fmat4_Float4_mult (&(obj->mvit), &norm4d);
	Float3 norm_eye   = Float4_Float3_vect_conv (&norm4d_eye); // normal is a vector, hence W = 0 and I don't care about it
	
	varying_fifo_push_Float3 (vry, &norm_eye);
	
	
	// extract the texture UV coordinates of the vertex
	if (obj->texture != NULL) {
		Float2 texture = wfobj_get_texture_coords (obj->wfobj, face_idx, vtx_idx);
		
		assert (texture.as_struct.u >= 0.0f);
		assert (texture.as_struct.v >= 0.0f);
		assert (texture.as_struct.u <= 1.0f);
		assert (texture.as_struct.v <= 1.0f);
		
		texture.as_struct.u *= (float) obj->texture->w;
		texture.as_struct.v *= (float) obj->texture->h;
	
		varying_fifo_push_Float2 (vry, &texture);
	}
		
	return clip;		
}

bool pshader_phong (Object *obj, Varying *vry, pixel_color_t *color, gpu_cfg_t *cfg) {
	
	
	
	
	Float3 normal = varying_fifo_pop_Float3 (vry);
	
	Float3_normalize (&normal);
	
	float intensity = -Float3_Float3_smult (&normal, &(cfg->lights_arr[0].eye));
	
					
	//
	// If texture is not provided, use gray color
	//
	pixel_color_t pix;
	if (obj->texture == NULL) {
		pix = set_color (128, 128, 128, 0);
	}
	else {	
	
		Float2 text = varying_fifo_pop_Float2 (vry);
	
		assert (text.as_struct.u >= 0);
		assert (text.as_struct.v >= 0);
		size_t uu = (size_t) text.as_struct.u;
		size_t vv = (size_t) text.as_struct.v;
		
		assert (uu < obj->texture->w);
		assert (vv < obj->texture->h);
		
		pix = get_pixel_color_from_bitmap (obj->texture, uu, vv);
	}	
	
	

	//if ((intensity > 1.0f) || (intensity < -1.0f)) {
	if (intensity < 0) {
		return false;
	}
	
	float intensity_treshold = 0.2;
	if (intensity < intensity_treshold) {
		intensity = intensity_treshold;
	}
	
		
	int r = pix.r * intensity + 5;
	int g = pix.g * intensity + 5;
	int b = pix.b * intensity + 5;
		
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	
	*color = set_color (r, g, b, 0);
	//*color = set_color (255, 0, 0, 0);
	return true;
}


/*#include "shader_phong.h"
#include "gl.h"

#include <math.h>




// declare the following here instead of the header to make these variables local:
Float3 PHONG_VARYING_U;
Float3 PHONG_VARYING_V;

Float3 PHONG_VARYING_N[3];
//fmat3 PHONG_VARYING_N; // cols are normals to vertices; rows are XYZ of each normal

// these are global:
//fmat4  UNIFORM_M;
//fmat4  UNIFORM_MIT;
//Float3 UNIFORM_LIGHT;


Float4 phong_vertex_shader (WFobj *obj, int face_idx, int vtx_idx, fmat4 *mvp) {
	
	if (PHONG_VSHADER_DEBUG) {
		printf ("\tcall phong_vertex_shader()\n");
	}
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 vtx3d = wfobj_get_vtx_coords (obj, face_idx, vtx_idx);
	Float4 mc = Float3_Float4_conv (&vtx3d, 1);
	Float4 vtx4d = fmat4_Float4_mult (mvp, &mc);
	
	// extract the texture UV coordinates of the vertex
	if (obj->texture != NULL) {
		Float2 vtx_uv = wfobj_get_texture_coords (obj, face_idx, vtx_idx);
		PHONG_VARYING_U.as_array[vtx_idx] = vtx_uv.as_struct.u;
		PHONG_VARYING_V.as_array[vtx_idx] = vtx_uv.as_struct.v;
	}
	
	// transform the normal vector to the vertex
	Float3 norm3d = wfobj_get_norm_coords    (obj, face_idx, vtx_idx);
	Float4 norm4d = Float3_Float4_conv  (&norm3d, 0);
	norm4d = fmat4_Float4_mult (&UNIFORM_MIT, &norm4d);
	for (int i = 0; i < 3; i++) {
		PHONG_VARYING_N[i].as_array[vtx_idx] = norm4d.as_array[i];
	}
		
	if (PHONG_VSHADER_DEBUG) {
		printf ("\t\tclip coord: %f, %f, %f, %f\n", vtx4d.as_struct.x, vtx4d.as_struct.y, vtx4d.as_struct.z, vtx4d.as_struct.w);
		printf ("\t\tvtx coord:  %f, %f, %f\n",     vtx3d.as_struct.x, vtx3d.as_struct.y, vtx3d.as_struct.z);
		printf ("\t\tvtx shader face %d vtx %d: ", face_idx, vtx_idx);
		printf ("obj norm (%f, %f, %f) ",            norm3d.as_struct.x, norm3d.as_struct.y, norm3d.as_struct.z);
		printf ("transformed normal (%f, %f, %f)\n", norm4d.as_struct.x, norm4d.as_struct.y, norm4d.as_struct.z);
		//printf ("\t\tuniform light (%f, %f, %f)\n",  UNIFORM_LIGHT[0].as_struct.x, UNIFORM_LIGHT[0].as_struct.y, UNIFORM_LIGHT[0].as_struct.z);
	}
	
	return vtx4d;
}

bool phong_pixel_shader (WFobj *obj, Float3 *barw, pixel_color_t *color) {
	
	if (PHONG_PSHADER_DEBUG_0) {
		printf ("\t\tcall phong_pixel_shader()\n");
	}
	
	int uu = (int) Float3_Float3_smult (&PHONG_VARYING_U, barw);
	int vv = (int) Float3_Float3_smult (&PHONG_VARYING_V, barw);
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
		normal.as_array[i] = Float3_Float3_smult (&PHONG_VARYING_N[i], barw);
	}
	Float3_normalize(&normal);
	float diff_intensity = -Float3_Float3_smult (&normal, &(LIGHTS[0].eye));
	
	float spec_intensity = 0;	
	if (obj->specularmap != NULL) {
		float nl = diff_intensity; // this is float3_float3_smult (&normal, &UNIFORM_LIGHT), computed above
		Float3 nnl2 = Float3_float_mult (&normal, nl * 2.0f);
		Float3 r    = Float3_Float3_add (&nnl2, &(LIGHTS[0].eye));
		Float3_normalize (&r);
		
		int spec_factor = wfobj_get_specularity_from_map (obj, uu, vv);
		spec_intensity = (r.as_struct.z < 0) ? 0 : powf (r.as_struct.z, spec_factor);
		
		if (PHONG_PSHADER_DEBUG_1) {
			if (spec_intensity >= 0.5) {
				printf ("n=(%f;%f;%f) ", normal.as_struct.x, normal.as_struct.y, normal.as_struct.z);
				printf ("nl=%f ", nl);
				printf ("nnl2=(%f;%f;%f) ", nnl2.as_struct.x, nnl2.as_struct.y, nnl2.as_struct.z);
				printf ("r=(%f;%f;%f) ", r.as_struct.x, r.as_struct.y, r.as_struct.z);
				printf ("spec_f=%d spec_i=%f\n", spec_factor, spec_intensity);
			}
		}
	}
	
	float intensity = 1.0 * diff_intensity + 0.6 * spec_intensity;
	//if (intensity <= 0) return false;
	if (intensity <= 0.1) intensity = 0.1;//return false;
	
	if (PHONG_PSHADER_DEBUG_0) {
		printf ("n=(%f;%f;%f) ", normal.as_struct.x, normal.as_struct.y, normal.as_struct.z);
		//printf ("light=(%f;%f;%f) ", UNIFORM_LIGHT[0].as_struct.x, UNIFORM_LIGHT[0].as_struct.y, UNIFORM_LIGHT[0].as_struct.z);
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
*/
