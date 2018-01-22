#include "shader_depth.h"
//#include "gl.h"

//#include "geometry_fixpt.h"

//#include <fixmath.h>

//#include <math.h>



//~ #define FLOAT_NORM   1
//~ #define FLOAT_TEXT   1
//~ #define FLOAT_SHADOW 1




////////////////////////////////////////////////////////////////////////
// This shader illustrates usage of dynamic shadows.
// Shadow buffer must already be generated for this frame.
////////////////////////////////////////////////////////////////////////


//int count_shadows (Varying *vry, gpu_cfg_t *cfg);
bool is_in_shadow (FixPt3 *shadow_screen, int light_num, gpu_cfg_t *cfg);


// Layout of Varying words for this pass:
// Phong:
// 0   normal.x
// 1   normal.y
// 2   normal.z
// 3   texture.u
// 4   texture.v
// 5   shadow0.x
// 6   shadow0.y
// 7   shadow0.z
// 8   shadow1.x
// ....
// Gouraud:
// 0   intensity
// 1   texture.u
// 2   texture.v
// 3   shadow0.x
// 4   shadow0.y
// 5   shadow0.z
// 6   shadow1.x
// ....


#define GOURAUD
#define SHADOWS
//#define SPECULAR

Float4 vshader_depth (Object *obj, VtxAttr *attribs, Varying *vry, gpu_cfg_t *cfg) {
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float4 model4d;
	Float3_Float4_conv_fast (&model4d, &attribs->vtx_coords, 1);
	Float4 clip;
	fmat4_Float4_mult_fast (&clip, &obj->mvp, &model4d); // model -> world -> eye -> clip

	// transform the normal vector to the vertex
	Float4 norm4d;
	Float3_Float4_conv_fast  (&norm4d, &attribs->norm_coords, 0);
	Float4 norm4d_eye;
	fmat4_Float4_mult_fast (&norm4d_eye, &obj->mvit, &norm4d);

	Float3 norm_eye;
	Float4_Float3_vect_conv_fast (&norm_eye, &norm4d_eye); // normal is a vector, hence W = 0 and I don't care about it
	
#ifdef GOURAUD
	Float3_normalize_superfast (&norm_eye);
	float diff_intensity = -Float3_Float3_smult_fast (&norm_eye, &(cfg->lights_arr[0].eye));// TBD, why [0]?
	
	assert (diff_intensity <=  1.0f);
	assert (diff_intensity >= -1.0f);
	
	varying_fifo_push_float_fast (vry, &diff_intensity);
#else // PHONG
	varying_fifo_push_Float3_fast (vry, &norm_eye);
#endif	
	
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
		
#ifdef SHADOWS	
	for (int i = 0; i < GPU_MAX_LIGHTS; i++) {
		if (cfg->lights_arr[i].enabled) {
			Float4 shadow_clip;
			fmat4_Float4_mult_fast (&shadow_clip, &obj->shadow_mvp[i], &model4d); // model -> world -> eye -> clip
			//printf("VS shadow clip: %f %f %f %f\n", shadow_clip.as_struct.x, shadow_clip.as_struct.y, shadow_clip.as_struct.z, shadow_clip.as_struct.w);
			//Perspective divide is only needed when perspective projection is used for shadows
			// By default I use orthographic projection, so commenting out the section below
			// // Compute XYZ in NDC by dividing XYZ in clip space by W (i.e. multiplying by 1/W)
			// // If at least one coord belongs to [-1:1] then the vertex is not clipped
			// for (int k = 0; k < 4; k++) {
			// 	// TBD: danger, no check for div by zero yet implemented
			// 	shadow_clip.as_array[k] = shadow_clip.as_array[k] / shadow_clip.as_struct.w;
			// }			
			
			Float4 shadow_screen;
			fmat4_Float4_mult_fast (&shadow_screen, &cfg->viewport, &shadow_clip);
			//printf("\tshadow screen: %f %f %f %f\n", shadow_screen.as_struct.x, shadow_screen.as_struct.y, shadow_screen.as_struct.z, shadow_screen.as_struct.w);
			Float3 shadow_screen3;
			Float4_Float3_vect_conv_fast (&shadow_screen3, &shadow_screen); // don't need W for ortho projection
			varying_fifo_push_Float3_fast (vry, &shadow_screen3);
			//printf("\t\tshadow screen3: %f %f %f\n", shadow_screen3.as_struct.x, shadow_screen3.as_struct.y, shadow_screen3.as_struct.z);
		}
	}
#endif

	return clip;		
}

bool pshader_depth (Object *obj, Varying *vry, gpu_cfg_t *cfg, pixel_color_t *color) {
	
	const size_t light_num = 0; // TBD

	fixpt_t intensity;
	float spec_intensity = 0;	

#ifndef GOURAUD
	Float3 normal;
#endif

	FixPt2 text;
	FixPt3 shadow_screen[GPU_MAX_LIGHTS];


#ifdef ARC_APEX

	#ifdef SHADOWS
		shadow_screen[0].as_struct.z = vry_rd (0);
		shadow_screen[0].as_struct.y = vry_rd (0);
		shadow_screen[0].as_struct.x = vry_rd (0);
	#endif

	text.as_struct.v = vry_rd (0);
	text.as_struct.u = vry_rd (0);
	
	#ifdef GOURAUD
		intensity = vry_rd (0);
	#else
		normal.as_struct.z = fixpt_to_float (vry_rd (0), VARYING_FRACT_BITS);
		normal.as_struct.y = fixpt_to_float (vry_rd (0), VARYING_FRACT_BITS);
		normal.as_struct.x = fixpt_to_float (vry_rd (0), VARYING_FRACT_BITS);
	#endif
#else
	#ifdef GOURAUD
		intensity = varying_fifo_pop_fixpt  (vry);
	#else
		normal    = varying_fifo_pop_Float3 (vry);
	#endif
	text             = varying_fifo_pop_FixPt2 (vry);
	shadow_screen[light_num] = varying_fifo_pop_FixPt3 (vry);
#endif

#ifndef GOURAUD
	float diff_intensity[GPU_MAX_LIGHTS];
	float diff_int_total = 0;
	Float3_normalize_superfast (&normal);
	for (int i = 0; i < GPU_MAX_LIGHTS; i++) {
		// Scalar multiplication of two normalied vectors. We have normalized the normal vector just a few lines above,
		// but the light vector was normalized elsewhere
		diff_intensity[i] = (cfg->lights_arr[i].enabled) ? -Float3_Float3_smult_fast (&normal, &cfg->lights_arr[i].eye) : 0;
		if (diff_intensity[i] > 0) {
			diff_int_total += diff_intensity[i];
		}
	}
	float fintensity = diff_int_total + 0.6f * spec_intensity;
	if (fintensity < 0.f) return false;
	intensity = fixpt_from_float (fintensity, VARYING_FRACT_BITS);
	
#endif

#ifdef SPECULAR
	if (obj->specularmap != NULL) {
		float nl = diff_intensity[0]; //TBD, this is computed above
		Float3 nnl2 = Float3_float_mult (&normal, nl * 2.0f);
		Float3 r    = Float3_Float3_add (&nnl2, &(LIGHTS[0].eye));
		Float3_normalize (&r);
		
		int32_t spec_factor = get_int32_from_bitmap (obj->specularmap, uu, vv);
		spec_intensity = (r.as_struct.z < 0) ? 0 : powf (r.as_struct.z, spec_factor);
	}
#endif
	intensity += fixpt_from_float (spec_intensity, VARYING_FRACT_BITS);
#ifdef SHADOWS	
	//float shadow_factor = 1.0f - count_shadows(vry, cfg) / GPU_MAX_LIGHTS; // 1 - not in shadow; 0 - in all shadows
	
	if (is_in_shadow (&shadow_screen[0], light_num, cfg)) {
		intensity = 0;
	}

#endif

	assert (text.as_struct.u >= 0);
	assert (text.as_struct.v >= 0);

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
	// *color = set_color (255, 0, 255, 0);
	
	return true;
}

bool is_in_shadow (FixPt3 *shadow_screen, int light_num, gpu_cfg_t *cfg) {
		
	assert (cfg->lights_arr[light_num].shadow_buf != NULL);
	
	const float anti_z_fighting = 123.123; // [almost] arbitrary value
	
	//~ screenxy_t x = (screenxy_t) shadow_screen->as_struct.x;
	//~ screenxy_t y = (screenxy_t) shadow_screen->as_struct.y;
	//~ screenz_t  z = (screenz_t)  shadow_screen->as_struct.z;
	screenxy_t x = shadow_screen->as_struct.x >> VARYING_FRACT_BITS;
	screenxy_t y = shadow_screen->as_struct.y >> VARYING_FRACT_BITS;
	screenz_t  z = shadow_screen->as_struct.z >> VARYING_FRACT_BITS;

	assert (x >= 0);
	assert (y >= 0);
	assert (z >= 0);
	
	assert (x < cfg->screen_width);
	assert (y < cfg->screen_height);	
	assert (z < (1 << 16)); // TBD
	
	screenz_t shadow_buf_z = cfg->lights_arr[light_num].shadow_buf[y * cfg->screen_width + x];
	
	return (shadow_buf_z > z + anti_z_fighting);
}
