#include "shader.h"
#include "gl.h"

#include <math.h>


#define VSHADER_DEBUG 0



// declare the following here instead of the header to make these variables local:
float3 VARYING_U;
float3 VARYING_V;
float3 VARYING_NX;
float3 VARYING_NY;
float3 VARYING_NZ;
float3 VARYING_INTENSITY;

// these are global:
fmat4  UNIFORM_M;
fmat4  UNIFORM_MI;
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
	
	VARYING_U[vtx_idx] = wfobj_get_text_coord (obj, face_idx, vtx_idx, 0);
	VARYING_V[vtx_idx] = wfobj_get_text_coord (obj, face_idx, vtx_idx, 1);
	
	
	
	if (0) {
		VARYING_NX[vtx_idx] = wfobj_get_norm_coord (obj, face_idx, vtx_idx, 0);
		VARYING_NY[vtx_idx] = wfobj_get_norm_coord (obj, face_idx, vtx_idx, 1);
		VARYING_NZ[vtx_idx] = wfobj_get_norm_coord (obj, face_idx, vtx_idx, 2);
	}
	else {
		float4 n, nr;
		n[0] = wfobj_get_norm_coord (obj, face_idx, vtx_idx, 0);
		n[1] = wfobj_get_norm_coord (obj, face_idx, vtx_idx, 1);
		n[2] = wfobj_get_norm_coord (obj, face_idx, vtx_idx, 2);
		n[3] = 0.0f; // set to 0 since a normal is a vector
		fmat4_float4_mult (&UNIFORM_MIT, &n, &nr);
		VARYING_NX[vtx_idx] = nr[0];
		VARYING_NY[vtx_idx] = nr[1];
		VARYING_NZ[vtx_idx] = nr[2];
		
		if (VSHADER_DEBUG) {
			printf ("\t\tvtx shader face%d vtx%d: obj norm (%f, %f, %f), transformed normal (%f, %f, %f)\n", face_idx, vtx_idx, n[0], n[1], n[2], nr[0], nr[1], nr[2]);
			printf ("\t\tuniform light (%f, %f, %f)\n", UNIFORM_LIGHT[0], UNIFORM_LIGHT[1], UNIFORM_LIGHT[2]);
		}
	}
}

bool my_pixel_shader (WFobj *obj, float3 *barw, pixel_color_t *color) {
	
	int uu = (int) (obj->textw * float3_float3_smult (&VARYING_U, barw));
	int vv = (int) (obj->texth * float3_float3_smult (&VARYING_V, barw));
	
	pixel_color_t pix;
	pix.r = *(obj->texture + (uu + obj->textw*vv) * (obj->textbytespp) + 0);
	pix.g = *(obj->texture + (uu + obj->textw*vv) * (obj->textbytespp) + 1);
	pix.b = *(obj->texture + (uu + obj->textw*vv) * (obj->textbytespp) + 2);
	
	float intensity = 0;
	float diff_intensity = 0;
	float spec_intensity = 0;
	
	bool normalmap = 0;
	bool phong = 1;
	bool gouraud = 0;	
	
	float3 normal;
	
		
	if (phong) {
		normal[0] = float3_float3_smult (&VARYING_NX, barw);
		normal[1] = float3_float3_smult (&VARYING_NY, barw);
		normal[2] = float3_float3_smult (&VARYING_NZ, barw);
		float3_normalize(&normal);
		diff_intensity = -float3_float3_smult (&normal, &UNIFORM_LIGHT);
	}
	else if (gouraud) {
		float3 interp_intens;
		for (int i = 0; i < 3; i++) {
			float3 ii = {VARYING_NX[i], VARYING_NY[i], VARYING_NZ[i]};
			interp_intens[i] = float3_float3_smult (&ii, &UNIFORM_LIGHT);
		}
		diff_intensity = -float3_float3_smult (&interp_intens, barw);
	}
	else if (normalmap) {
		float4 nm, tmp;
		nm[0] = *((obj->normalmap) + (uu + obj->nmw*vv) * (obj->nmbytespp) + 0);
		nm[1] = *((obj->normalmap) + (uu + obj->nmw*vv) * (obj->nmbytespp) + 1);
		nm[2] = *((obj->normalmap) + (uu + obj->nmw*vv) * (obj->nmbytespp) + 2);
		nm[3] = 0.0f; // 0 means a vector
		fmat4_float4_mult (&UNIFORM_MIT, &nm, &tmp);
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
		int spec_factor;
		if (obj->specmap != NULL) spec_factor = *(obj->specmap + (uu + obj->smw*vv) * (obj->smbytespp));
		else spec_factor = 0;
		//if (spec_factor > 0) printf ("spec_factor %d, r[Z]=%f ", spec_factor, r[Z]);
		spec_intensity = (r[Z] < 0) ? 0 : pow (r[Z], spec_factor);
		//spec_intensity = pow (r[Z], 10);//spec_factor);//(uu+vv)/256.0);
		if (spec_intensity >= 0.5) {
			;//printf ("n=(%f;%f;%f) nl=%f nnl2=(%f;%f;%f) r=(%f;%f;%f) spec_f=%d spec_i=%f\n", normal[0], normal[1], normal[2], nl, nnl2[0], nnl2[1], nnl2[2], r[0], r[1], r[2], spec_factor, spec_intensity);
		}
		
		//if ((uu > 400) && (uu < 500)) spec_intensity = 1;
		//else spec_intensity = 0;
		
		//spec_intensity = pow (r[Z], spec_factor);//(uu+vv)/256.0);
		//if (spec_intensity > 0.5) printf ("%f ", spec_intensity);
		//spec_intensity = 0;
		//pix.r = spec_factor;
		//pix.g = spec_factor;
	//	pix.b = spec_factor;
	}
	
	intensity = 1.0 * diff_intensity + 0.6 * spec_intensity;
	//if (intensity > 1.0) intensity = 1.0;
	int r = pix.r * intensity + 5;
	int g = pix.g * intensity + 5;
	int b = pix.b * intensity + 5;
		
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	
	if (intensity > 0) {
		if (intensity < 0.1) intensity = 0.1; // ambient light
		//*color = set_color (tmpcolor.r * intensity, tmpcolor.g * intensity, tmpcolor.b * intensity, 0);
		*color = set_color (r, g, b, 0);
		//*color = set_color (255 * intensity, 0 * intensity, 255 * intensity, 0);
		return true;
	}
	return false;
}
