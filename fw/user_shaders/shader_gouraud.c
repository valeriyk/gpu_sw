#include "shader_gouraud.h"

#ifdef ARC_APEX
	#include <apexextensions.h>
#endif

//#include "gl.h"

//#include "geometry_fixpt.h"

//#include <fixmath.h>

//#include <math.h>



// Layout of Varying words:
//     
// 0   intensity
// 1   texture.u
// 2   texture.v
// ....

Float4 vshader_gouraud (Object *obj, VtxAttr *attribs, Varying *vry, gpu_cfg_t *cfg) {
	
	// transform 3d coords of the vertex to homogenous clip coords
	//Float3 model   = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
	//Float4 model4d = Float3_Float4_conv (&model, 1);
	//Float4 model4d = Float3_Float4_conv (&attribs->vtx_coords, 1);
	Float4 model4d;
	Float3_Float4_conv_fast (&model4d, &attribs->vtx_coords, 1);
	//Float4 clip    = fmat4_Float4_mult_v3 (&obj->mvp, &model4d); // model -> world -> eye -> clip
	Float4 clip;
	fmat4_Float4_mult_fast (&clip, &obj->mvp, &model4d); // model -> world -> eye -> clip
	
	// transform the normal vector to the vertex
	//Float3 norm       = wfobj_get_norm_coords    (obj->wfobj, face_idx, vtx_idx);
	//Float4 norm4d     = Float3_Float4_conv  (&norm, 0);
	//Float4 norm4d     = Float3_Float4_conv  (&attribs->norm_coords, 0);
	Float4 norm4d;
	Float3_Float4_conv_fast  (&norm4d, &attribs->norm_coords, 0);
	//Float4 norm4d_eye = fmat4_Float4_mult_v3 (&obj->mvit, &norm4d);
	Float4 norm4d_eye;
	fmat4_Float4_mult_fast (&norm4d_eye, &obj->mvit, &norm4d);
	
	//Float3 norm_eye   = Float4_Float3_vect_conv (&norm4d_eye); // normal is a vector, hence W = 0 and I don't care about it
	Float3 norm_eye;
	Float4_Float3_vect_conv_fast (&norm_eye, &norm4d_eye); // normal is a vector, hence W = 0 and I don't care about it
	Float3_normalize_fast (&norm_eye);
	float diff_intensity = -Float3_Float3_smult_fast (&norm_eye, &(cfg->lights_arr[0].eye));// TBD, why [0]?
	
	assert (diff_intensity <=  1.0f);
	assert (diff_intensity >= -1.0f);
	
	varying_fifo_push_float_fast (vry, &diff_intensity);
	
	
	// extract the texture UV coordinates of the vertex
	if (obj->texture != NULL) {
		//Float2 texture = wfobj_get_texture_coords (obj->wfobj, face_idx, vtx_idx);
		Float2 texture = attribs->text_coords;
		
		assert (texture.as_struct.u >= 0.0f);
		assert (texture.as_struct.v >= 0.0f);
		assert (texture.as_struct.u <= 1.0f);
		assert (texture.as_struct.v <= 1.0f);
		
		texture.as_struct.u *= (float) obj->texture->w;
		texture.as_struct.v *= (float) obj->texture->h;
	
		varying_fifo_push_Float2_fast (vry, &texture);
	}
			
	return clip;		
}

bool pshader_gouraud (Object *obj, Varying *vry, gpu_cfg_t *cfg, pixel_color_t *color) {
	
	FixPt2  text;
	fixpt_t intensity;
	
#ifdef ARC_APEX
		
		// In APEX implementation we read Varyings in reverse order (works as a true stack)
		text.as_struct.v = vry_rd(0); // argument is meaningless here
		text.as_struct.u = vry_rd(0); // argument is meaningless here
		intensity        = vry_rd(0); // argument is meaningless here
#else
		intensity = varying_fifo_pop_fixpt  (vry);
		text      = varying_fifo_pop_FixPt2 (vry);
#endif
	
	//
	// If texture is not provided, use gray color
	//
	pixel_color_t pix;
	if (obj->texture == NULL) {
		pix = set_color (128, 128, 128, 0);
	}
	else {	
		
		size_t uu = text.as_struct.u >> VARYING_FRACT_BITS;
		size_t vv = text.as_struct.v >> VARYING_FRACT_BITS;
		
		assert (uu < obj->texture->w);
		assert (vv < obj->texture->h);
		
		pix = get_pixel_color_from_bitmap (obj->texture, uu, vv);
	}
	
	
	fixpt_t intensity_treshold = 1 << (VARYING_FRACT_BITS - 2); // 0.25 in Varying fixpt format
		
	
	if (intensity < intensity_treshold) {
		intensity = intensity_treshold;
	}
	
	*color = color_mult (pix, intensity);	
	
	//*color = set_color (r, g, b, 0);
	//*color = set_color (128, 128, 0, 0);
	return true;
}
