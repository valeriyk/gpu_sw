#include "shader_depth.h"
#include "gl.h"

#include "geometry_fixpt.h"

//#include <fixmath.h>

#include <math.h>



#define FLOAT_NORM   1
#define FLOAT_TEXT   1
#define FLOAT_SHADOW 1


int count_shadows (Varying *vry);



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SHADERS - 1st PASS: fill shadow buffer

Float4 depth_vshader_pass1 (Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry) {
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 model   = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
	Float4 model4d = Float3_Float4_conv   (&model, 1);
	Float4 clip    = fmat4_Float4_mult    (&(obj->mvp), &model4d); // model -> world -> eye -> clip
	return clip;
}

bool depth_pshader_pass1 (Object *obj, Varying *vry, pixel_color_t *color) {
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SHADERS - 2nd PASS: fill frame buffer

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

Float4 depth_vshader_pass2 (Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry) {
	
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
		texture.as_struct.u *= (float) obj->texture->w;
		texture.as_struct.v *= (float) obj->texture->h;
	
		varying_fifo_push_Float2 (vry, &texture);
	}
		
	
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		if (LIGHTS[i].enabled) {
			Float4 shadow_clip = fmat4_Float4_mult (&(obj->shadow_mvp[i]), &model4d); // model -> world -> eye -> clip
			
			//Perspective divide is only needed when perspective projection is used for shadows
			// By default I use orthographic projection, so commenting out the section below
			// // Compute XYZ in NDC by dividing XYZ in clip space by W (i.e. multiplying by 1/W)
			// // If at least one coord belongs to [-1:1] then the vertex is not clipped
			// for (int k = 0; k < 4; k++) {
			// 	// TBD: danger, no check for div by zero yet implemented
			// 	shadow_clip.as_array[k] = shadow_clip.as_array[k] / shadow_clip.as_struct.w;
			// }			
			
			Float4 shadow_screen = fmat4_Float4_mult (&VIEWPORT, &shadow_clip);
			Float3 shadow_screen3 = Float4_Float3_vect_conv (&shadow_screen); // don't need W for ortho projection
			varying_fifo_push_Float3 (vry, &shadow_screen3);
		}
	}
		
	return clip;		
}

bool depth_pshader_pass2 (Object *obj, Varying *vry, pixel_color_t *color) {
	
	if (DEPTH_PSHADER2_DEBUG) {
		printf ("\t\tcall depth_pshader_pass2()\n");
	}
	
	Float3 normal = varying_fifo_pop_Float3 (vry);
	Float3_normalize (&normal);
	
	Float2 text   = varying_fifo_pop_Float2 (vry);
	size_t uu = (int) text.as_struct.u;
	size_t vv = (int) text.as_struct.v;
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
		assert (uu >= 0);
		assert (vv >= 0);
		
		pix = get_rgb_from_texture (obj, uu, vv);
	}
	
	
	
	
	
	float diff_intensity[MAX_NUM_OF_LIGHTS];
	float diff_int_total = 0;
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		diff_intensity[i] = (LIGHTS[i].enabled) ? -Float3_Float3_smult (&normal, &(LIGHTS[i].eye)) : 0;
		diff_int_total += diff_intensity[i];
	}
		
	
	
	float spec_intensity = 0;	
	if (obj->specularmap != NULL) {
		float nl = diff_intensity[0]; //TBD, this is computed above
		Float3 nnl2 = Float3_float_mult (&normal, nl * 2.0f);
		Float3 r    = Float3_Float3_add (&nnl2, &(LIGHTS[0].eye));
		Float3_normalize (&r);
		
		int spec_factor = get_specularity_from_map (obj, uu, vv);
		spec_intensity = (r.as_struct.z < 0) ? 0 : powf (r.as_struct.z, spec_factor);
	}
	
	
	float shadow_factor = 1.0f - count_shadows(vry) / MAX_NUM_OF_LIGHTS; // 1 - not in shadow; 0 - in all shadows
	float intensity = shadow_factor * (1.f * diff_int_total + 0.6f * spec_intensity);
	
	
	float intensity_treshold = 0.3;
	if (intensity < intensity_treshold) {
		intensity = intensity_treshold;
	}
	
	//intensity = 0.8f;
	//pix = set_color (128, 128, 128, 0);
	
	if (DEPTH_PSHADER2_DEBUG) {
		printf ("n=(%f;%f;%f) ", normal.as_struct.x, normal.as_struct.y, normal.as_struct.z);
		printf ("light=(%f;%f;%f) ", LIGHTS[0].eye.as_struct.x, LIGHTS[0].eye.as_struct.y, LIGHTS[0].eye.as_struct.z);
		//printf ("diff_int=%f ", diff_intensity);
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

int count_shadows (Varying *vry) {
	int    shadows = 0;
	float  z_fighting = 123; // [almost] arbitrary value
	
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		
		if (!LIGHTS[i].enabled) continue;
		
		
		screenxy_t x;
		screenxy_t y;
		screenz_t  z;
		
		
		Float3 screen = varying_fifo_pop_Float3 (vry);
			
		assert (screen.as_struct.x >= 0);
		assert (screen.as_struct.x < get_screen_width ());
		x = (screenxy_t) screen.as_struct.x;
		
		assert (screen.as_struct.y >= 0);
		assert (screen.as_struct.y < get_screen_height());	
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
		screenz_t shadow_buf_z = LIGHTS[i].shadow_buf[y * get_screen_width() + x];
		
		if (shadow_buf_z > z + z_fighting) shadows++;
	}
	return shadows;
}
