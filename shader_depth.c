#include "shader_depth.h"
#include "gl.h"

#include "geometry_fixpt.h"

//#include <fixmath.h>

#include <math.h>

#define FLOAT 1

int count_shadows (Varying *vry);



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SHADERS - 1st PASS: fill shadow buffer

Float4 depth_vshader_pass1 (Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry) {
	
	if (DEPTH_VSHADER1_DEBUG) {
		printf ("\tcall depth_vertex_shader()\n");
	}
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 vtx3d = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
	Float4 mc    = Float3_Float4_conv   (&vtx3d, 1);
	Float4 vtx4d = fmat4_Float4_mult    (&(obj->mvp), &mc);
	return vtx4d;
}



bool depth_pshader_pass1 (Object *obj, Varying *vry, pixel_color_t *color) {
	return false;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SHADERS - 2nd PASS: fill frame buffer

// Layout of Varying words for this pass:
//     
//     0    1    2    3
// 0   xn   yn   zn   wn  - coords of the normal
// 1   u    v    0    0   - texture coords
// 2   xs0  ys0  zs0  ws0 - coords of the shadow 0
// 3   xs1  ys1  zs1  ws1 - coords of the shadow 1
// 4   xs2  ys2  zs2  ws2 - coords of the shadow 2
// 5   xs3  ys3  zs3  ws3 - coords of the shadow 3
// 6   -    -    -    -   - not used 
// 7   -    -    -    -   - ...

Float4 depth_vshader_pass2 (Object *obj, size_t face_idx, size_t vtx_idx, Varying *vry) {
	
	if (DEPTH_VSHADER2_DEBUG) {
		printf ("\tcall depth_vertex_shader()\n");
	}
	
	
	
	// transform 3d coords of the vertex to homogenous clip coords
	Float3 vtx3d = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
	Float4 mc    = Float3_Float4_conv (&vtx3d, 1);
	Float4 clip  = fmat4_Float4_mult (&(obj->mvp), &mc); // this is the return value
	
	// transform the normal vector to the vertex
	Float3 norm3d = wfobj_get_norm_coords    (obj->wfobj, face_idx, vtx_idx);
	Float4 norm4d = Float3_Float4_conv  (&norm3d, 0);
	
	
	Float4 transformed = fmat4_Float4_mult (&(obj->mit), &norm4d);
	if (FLOAT) {
		vry->data.as_Float4[0] = transformed;
	}
	else {
		vry->data.as_FixPt4[0] = Float4_FixPt4_conv (&transformed);
	}
	vry->num_of_words = 4;
	
//printf ("vshader face %d vtx %d\t    norm4d x=%f y=%f z=%f w=%f\n", face_idx, vtx_idx, norm4d.as_struct.x, norm4d.as_struct.y, norm4d.as_struct.z, norm4d.as_struct.w);
//printf ("                      \tMIT*norm4d x=%f y=%f z=%f w=%f\n\n", vry->as_Float4[1].as_struct.x, vry->as_Float4[1].as_struct.y, vry->as_Float4[1].as_struct.z, vry->as_Float4[1].as_struct.w);
	
	
	// extract the texture UV coordinates of the vertex
	if (obj->wfobj->texture != NULL) {
		Float2 text = wfobj_get_texture_coords (obj->wfobj, face_idx, vtx_idx);
		if (FLOAT) {
			vry->data.as_Float2[2] = text;
			vry->data.as_float [6] = 0;
			vry->data.as_float [7] = 0;
		}
		else {
			vry->data.as_FixPt2 [2] = Float2_FixPt2_conv (&text);
			vry->data.as_fixpt_t[6] = 0;
			vry->data.as_fixpt_t[7] = 0;
		}
		vry->num_of_words += 4;
	}
	
	
	
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		if (LIGHTS[i].enabled) {
			//Float4 shadow_clip = fmat4_Float4_mult (&(obj->shadow_mvp[i]), &mc); // clip
			Float4 shadow_clip = fmat4_Float4_mult (&(obj->shadow_mvp[i]), &mc); // clip
			
			//Perspective divide is only needed when perspective projection is used for shadows
			// By default I use orthographic projection, so commenting out the section below
			/*	
			// Compute XYZ in NDC by dividing XYZ in clip space by W (i.e. multiplying by 1/W)
			// If at least one coord belongs to [-1:1] then the vertex is not clipped
			for (int k = 0; k < 4; k++) {
				//TBD: danger, no check for div by zero yet implemented
				shadow_vtx4d.as_array[k] = shadow_vtx4d.as_array[k] / shadow_vtx4d.as_struct.w; // normalize
			}
			*/
			
			Float4 shadow_screen = fmat4_Float4_mult (&VIEWPORT, &shadow_clip);
			if (FLOAT) {
				vry->data.as_Float4[2+i] = shadow_screen;
			}
			else {
				vry->data.as_FixPt4[2+i] = Float4_FixPt4_conv (&shadow_screen);
			}
			vry->num_of_words += 4;
		}
	}
		
	return clip;		
}

bool depth_pshader_pass2 (Object *obj, Varying *vry, pixel_color_t *color) {
	
	if (DEPTH_PSHADER2_DEBUG) {
		printf ("\t\tcall depth_pshader_pass2()\n");
	}
	
	Float2 text;
	if (FLOAT) {
		text = vry->data.as_Float2[2];
	}
	else {
		text = FixPt2_Float2_conv (&(vry->data.as_FixPt2[2]));
	}
	
	size_t uu = (int) text.as_struct.u;
	size_t vv = (int) text.as_struct.v;
	
	//
	// If texture is not provided, use gray color
	//
	pixel_color_t pix;
	if (obj->wfobj->texture == NULL) {
		pix = set_color (128, 128, 128, 0);
	}
	else {
		
		assert (uu < obj->wfobj->texture->w);
		assert (vv < obj->wfobj->texture->h);
		assert (uu >= 0);
		assert (vv >= 0);
		
		wfobj_get_rgb_from_texture (obj->wfobj, uu, vv, &pix.r, &pix.g, &pix.b);
	}
	
	
	Float4 norm4;
	if (FLOAT) {
		norm4 = vry->data.as_Float4[0];
	}
	else {
		norm4 = FixPt4_Float4_conv (&(vry->data.as_FixPt4[0]));
	}
	Float3 normal  = Float4_Float3_vect_conv (&norm4);
	Float3_normalize (&normal);
	
	
	
	
	float diff_intensity[MAX_NUM_OF_LIGHTS];
	float diff_int_total = 0;
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		diff_intensity[i] = (LIGHTS[i].enabled) ? -Float3_Float3_smult (&normal, &(LIGHTS[i].eye)) : 0;
		diff_int_total += diff_intensity[i];
	}
		
	
	
	float spec_intensity = 0;	
	if (obj->wfobj->specularmap != NULL) {
		float nl = diff_intensity[0]; //TBD, this is computed above
		Float3 nnl2 = Float3_float_mult (&normal, nl * 2.0f);
		Float3 r    = Float3_Float3_add (&nnl2, &(LIGHTS[0].eye));
		Float3_normalize (&r);
		
		int spec_factor = wfobj_get_specularity_from_map (obj->wfobj, uu, vv);
		spec_intensity = (r.as_struct.z < 0) ? 0 : powf (r.as_struct.z, spec_factor);
	}
	
	
	
	
	float shadow_total = 1.0f - count_shadows(vry) / MAX_NUM_OF_LIGHTS;
	
	
	
	
	
	float intensity = shadow_total * (1.f * diff_int_total + 0.6f * spec_intensity);
	
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
	Float3 screen;
	int    shadows = 0;
	float  z_fighting = 123; // arbitrary value
	
	for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
		
		if (!LIGHTS[i].enabled) continue;
		
		
		screenxy_t x;
		screenxy_t y;
		screenz_t  z;
		
		if (FLOAT) {
			Float4 screen4 = vry->data.as_Float4[2+i];
			
			assert (screen.as_struct.x >= 0);
			assert (screen.as_struct.x < get_screen_width ());
			x = (screenxy_t) screen4.as_struct.x;
			
			assert (screen.as_struct.y >= 0);
			assert (screen.as_struct.y < get_screen_height());	
			y = (screenxy_t) screen4.as_struct.y;
			
			assert (screen.as_struct.z >= 0);
			assert (screen.as_struct.z < (1 << 16)); // TBD
			z = (screenz_t) screen4.as_struct.z;
		}
		else {
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
		
		screenz_t shadow_buf_z = LIGHTS[i].shadow_buf[x + y * get_screen_width()];
		
		if (shadow_buf_z > z + z_fighting) {
			shadows++;
		}
	}
	return shadows;
}
