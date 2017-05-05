#include "shader_depth.h"
#include "gl.h"

#include "geometry_fixpt.h"

//#include <fixmath.h>

#include <math.h>



// Layout of Varying words:
//     
// 0   intensity
// 1   texture.u
// 2   texture.v
// ....

Float4 vshader_gouraud (Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry) {
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 model   = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
	Float4 model4d = Float3_Float4_conv (&model, 1);
	Float4 clip    = fmat4_Float4_mult (&(obj->mvp), &model4d); // model -> world -> eye -> clip
	
	// transform the normal vector to the vertex
	Float3 norm       = wfobj_get_norm_coords    (obj->wfobj, face_idx, vtx_idx);
	Float4 norm4d     = Float3_Float4_conv  (&norm, 0);
	Float4 norm4d_eye = fmat4_Float4_mult (&(obj->mvit), &norm4d);
	Float3 norm_eye   = Float4_Float3_vect_conv (&norm4d_eye); // normal is a vector, hence W = 0 and I don't care about it
	Float3_normalize (&norm_eye);
	float diff_intensity = -Float3_Float3_smult (&norm_eye, &(LIGHTS[0].eye));
	
	assert (diff_intensity <=  1.0f);
	assert (diff_intensity >= -1.0f);
	
	varying_fifo_push_float (vry, diff_intensity);
	
	
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

bool pshader_gouraud (Object *obj, Varying *vry, pixel_color_t *color) {
	
	float intensity = varying_fifo_pop_float (vry);
	
	assert (intensity <=  1.0f);
	assert (intensity >= -1.0f);
	if ((intensity >  1.0f) || (intensity < -1.0f)) {
		return false;
	}
		
	Float2 text   = varying_fifo_pop_Float2 (vry);
	
	assert (text.as_struct.u >= 0);
	assert (text.as_struct.v >= 0);
	
	size_t uu = (size_t) text.as_struct.u;
	size_t vv = (size_t) text.as_struct.v;
	//int uu = (int) text.as_struct.u;
	//int vv = (int) text.as_struct.v;
	//
	// If texture is not provided, use gray color
	//
	pixel_color_t pix;
	if (obj->texture == NULL) {
		pix = set_color (128, 128, 128, 0);
	}
	else {	
		assert (uu < obj->texture->w);
		assert (vv < obj->texture->h);
		//assert (uu >= 0);
		//assert (vv >= 0);
		
		pix = get_pixel_color_from_bitmap (obj->texture, uu, vv);
	}
	
	float intensity_treshold = 0;
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
	//*color = set_color (128, 128, 0, 0);
	return true;
}
