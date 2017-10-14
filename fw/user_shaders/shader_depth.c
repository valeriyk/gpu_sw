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


int count_shadows (Varying *vry, gpu_cfg_t *cfg);

int count_shadows2 (Varying *vry, gpu_cfg_t *cfg);


// Layout of Varying words for this pass:
//     
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

Float4 vshader_depth (Object *obj, VtxAttr *attribs, Varying *vry, gpu_cfg_t *cfg) {
	
	// transform 3d coords of the vertex to homogenous clip coords
	//Float3 model   = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
	//Float4 model4d = Float3_Float4_conv (&model, 1);
	Float4 model4d = Float3_Float4_conv (&attribs->vtx_coords, 1);
	Float4 clip    = fmat4_Float4_mult (&(obj->mvp), &model4d); // model -> world -> eye -> clip
	
	// transform the normal vector to the vertex
	//Float3 norm       = wfobj_get_norm_coords    (obj->wfobj, face_idx, vtx_idx);
	//Float4 norm4d     = Float3_Float4_conv  (&norm, 0);
	Float4 norm4d     = Float3_Float4_conv  (&attribs->norm_coords, 0);
	Float4 norm4d_eye = fmat4_Float4_mult (&(obj->mvit), &norm4d);
	Float3 norm_eye   = Float4_Float3_vect_conv (&norm4d_eye); // normal is a vector, hence W = 0 and I don't care about it
	
	varying_fifo_push_Float3 (vry, &norm_eye);
	
	
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
	
		varying_fifo_push_Float2 (vry, &texture);
	}
		
	
	for (int i = 0; i < GPU_MAX_LIGHTS; i++) {
		if (cfg->lights_arr[i].enabled) {
			Float4 shadow_clip = fmat4_Float4_mult (&(obj->shadow_mvp[i]), &model4d); // model -> world -> eye -> clip
			//printf("VS shadow clip: %f %f %f %f\n", shadow_clip.as_struct.x, shadow_clip.as_struct.y, shadow_clip.as_struct.z, shadow_clip.as_struct.w);
			//Perspective divide is only needed when perspective projection is used for shadows
			// By default I use orthographic projection, so commenting out the section below
			// // Compute XYZ in NDC by dividing XYZ in clip space by W (i.e. multiplying by 1/W)
			// // If at least one coord belongs to [-1:1] then the vertex is not clipped
			// for (int k = 0; k < 4; k++) {
			// 	// TBD: danger, no check for div by zero yet implemented
			// 	shadow_clip.as_array[k] = shadow_clip.as_array[k] / shadow_clip.as_struct.w;
			// }			
			
			Float4 shadow_screen = fmat4_Float4_mult (&(cfg->viewport), &shadow_clip);
			//printf("\tshadow screen: %f %f %f %f\n", shadow_screen.as_struct.x, shadow_screen.as_struct.y, shadow_screen.as_struct.z, shadow_screen.as_struct.w);
			Float3 shadow_screen3 = Float4_Float3_vect_conv (&shadow_screen); // don't need W for ortho projection
			varying_fifo_push_Float3 (vry, &shadow_screen3);
			//printf("\t\tshadow screen3: %f %f %f\n", shadow_screen3.as_struct.x, shadow_screen3.as_struct.y, shadow_screen3.as_struct.z);
		}
	}
		
	return clip;		
}

bool pshader_depth (Object *obj, Varying *vry, gpu_cfg_t *cfg, pixel_color_t *color) {
	
	Float3 normal = varying_fifo_pop_Float3 (vry);
	Float3_normalize (&normal);
	
		
	Float2 text   = varying_fifo_pop_Float2 (vry);
	
	assert (text.as_struct.u >= 0);
	assert (text.as_struct.v >= 0);
	size_t uu = (size_t) text.as_struct.u;
	size_t vv = (size_t) text.as_struct.v;
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
		pix = get_pixel_color_from_bitmap (obj->texture, uu, vv);
	}
		
	float diff_intensity[GPU_MAX_LIGHTS];
	float diff_int_total = 0;
	for (int i = 0; i < GPU_MAX_LIGHTS; i++) {
		diff_intensity[i] = (cfg->lights_arr[i].enabled) ? -Float3_Float3_smult (&normal, &(cfg->lights_arr[i].eye)) : 0;
		if (diff_intensity[i] > 0) {
			diff_int_total += diff_intensity[i];
		}
	}		
	
	
	float spec_intensity = 0;	
	//~ if (obj->specularmap != NULL) {
		//~ float nl = diff_intensity[0]; //TBD, this is computed above
		//~ Float3 nnl2 = Float3_float_mult (&normal, nl * 2.0f);
		//~ Float3 r    = Float3_Float3_add (&nnl2, &(LIGHTS[0].eye));
		//~ Float3_normalize (&r);
		
		//~ int32_t spec_factor = get_int32_from_bitmap (obj->specularmap, uu, vv);
		//~ spec_intensity = (r.as_struct.z < 0) ? 0 : powf (r.as_struct.z, spec_factor);
	//~ }
	
	
	float shadow_factor = 1.0f - count_shadows(vry, cfg) / GPU_MAX_LIGHTS; // 1 - not in shadow; 0 - in all shadows
	float intensity = shadow_factor * (1.f * diff_int_total + 0.6f * spec_intensity);
	
	float intensity_treshold = 0.2;
	if (intensity < 0.f) {
		return false;
	}	
	else if (intensity < intensity_treshold) {
		intensity = intensity_treshold;
	}
	
		
	int r = pix.as_byte.r * intensity + 5;
	int g = pix.as_byte.g * intensity + 5;
	int b = pix.as_byte.b * intensity + 5;
		
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	
	*color = set_color (r, g, b, 0);
	// *color = set_color (255, 0, 255, 0);
	
	
	//*color = color_mult (pix, intensity);
	
	return true;
}

int count_shadows (Varying *vry, gpu_cfg_t *cfg) {
	int    shadows = 0;
	float  z_fighting = 123; // [almost] arbitrary value
	
	for (int i = 0; i < GPU_MAX_LIGHTS; i++) {
		
		if (!cfg->lights_arr[i].enabled) continue;
		
		
		screenxy_t x;
		screenxy_t y;
		screenz_t  z;
		
		
		/*
		FixPt3 screen = varying_fifo_pop_FixPt3 (vry);
		
		x = fixpt_to_screenxy (screen.as_struct.x);
		y = fixpt_to_screenxy (screen.as_struct.y);
		z = fixpt_to_screenz  (screen.as_struct.z);
		*/
		
		Float3 screen = varying_fifo_pop_Float3 (vry);
		
		//printf("CntShdw: %f %f %f\n", screen.as_struct.x, screen.as_struct.y, screen.as_struct.z);
			
		assert (screen.as_struct.x >= 0);
		assert (screen.as_struct.x < cfg->screen_width);
		x = (screenxy_t) screen.as_struct.x;
		
		assert (screen.as_struct.y >= 0);
		//if (screen.as_struct.y >= get_screen_height()) printf ("screen.as_struct.y=%f, get_screen_height()=%zu\n", screen.as_struct.y, get_screen_height());	
		assert (screen.as_struct.y < cfg->screen_height);	
		//if (screen.as_struct.y >= get_screen_height()) screen.as_struct.y = get_screen_height() - 1; // TBD
		y = (screenxy_t) screen.as_struct.y;
		
		//assert (screen.as_struct.z >= 0);
		//assert (screen.as_struct.z < (1 << 16)); // TBD
		z = (screenz_t) screen.as_struct.z;
		
		/*
		{
			FixPt4 screen4 = vry->data.as_FixPt4[2+i];
			
			assert (screen.as_struct.x >= 0);
			assert (screen.as_struct.x < fixpt_from_int32 (get_screen_width()));
			x = fixpt_to_screenxy (screen4.as_struct.x);
			
			assert (screen.as_struct.y >= 0);
			assert (screen.as_struct.y < fixpt_from_int32 (get_screen_height()));
			y = fixpt_to_screenxy (screen4.as_struct.y);
			
			assert (screen.as_struct.z >= 0);
			assert (screen.as_struct.z < (1 << (16 + FRACT_BITS))); // TBD
			z = fixpt_to_screenz  (screen4.as_struct.z);
		}
		*/
		assert (cfg->lights_arr[i].shadow_buf != NULL);
		
		screenz_t shadow_buf_z = cfg->lights_arr[i].shadow_buf[y * cfg->screen_width + x];
		
		if (shadow_buf_z > z + z_fighting) shadows++;
	}
	return shadows;
	//return 1;
}

int count_shadows2 (Varying *vry, gpu_cfg_t *cfg) {
	int    shadows = 0;
	float  z_fighting = 123; // [almost] arbitrary value
	
	Light *l = cfg->lights_arr;
	
	for (int i = 0; i < GPU_MAX_LIGHTS; i++) {
		
		if (!l[i].enabled) continue;
		
		
		screenxy_t x;
		screenxy_t y;
		screenz_t  z;
		
		
		/*
		FixPt3 screen = varying_fifo_pop_FixPt3 (vry);
		
		x = fixpt_to_screenxy (screen.as_struct.x);
		y = fixpt_to_screenxy (screen.as_struct.y);
		z = fixpt_to_screenz  (screen.as_struct.z);
		*/
		
		Float3 screen = varying_fifo_pop_Float3 (vry);
			
		assert (screen.as_struct.x >= 0);
		//assert (screen.as_struct.x < get_screen_width (cfg));
		x = (screenxy_t) screen.as_struct.x;
		
		assert (screen.as_struct.y >= 0);
		//if (screen.as_struct.y >= get_screen_height()) printf ("screen.as_struct.y=%f, get_screen_height()=%zu\n", screen.as_struct.y, get_screen_height());	
		assert (screen.as_struct.y < get_screen_height(cfg));	
		//if (screen.as_struct.y >= get_screen_height()) screen.as_struct.y = get_screen_height() - 1; // TBD
		y = (screenxy_t) screen.as_struct.y;
		
		assert (screen.as_struct.z >= 0);
		assert (screen.as_struct.z < (1 << 16)); // TBD
		z = (screenz_t) screen.as_struct.z;
		
		/*
		{
			FixPt4 screen4 = vry->data.as_FixPt4[2+i];
			
			assert (screen.as_struct.x >= 0);
			assert (screen.as_struct.x < fixpt_from_int32 (get_screen_width()));
			x = fixpt_to_screenxy (screen4.as_struct.x);
			
			assert (screen.as_struct.y >= 0);
			assert (screen.as_struct.y < fixpt_from_int32 (get_screen_height()));
			y = fixpt_to_screenxy (screen4.as_struct.y);
			
			assert (screen.as_struct.z >= 0);
			assert (screen.as_struct.z < (1 << (16 + FRACT_BITS))); // TBD
			z = fixpt_to_screenz  (screen4.as_struct.z);
		}
		*/
		assert (l[i].shadow_buf != NULL);
		
		screenz_t shadow_buf_z = l[i].shadow_buf[y * get_screen_width(cfg) + x];
		
		if (shadow_buf_z > z + z_fighting) shadows++;
	}
	return shadows;
}
