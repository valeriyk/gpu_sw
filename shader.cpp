#include "shader.h"
//#include "wavefront_obj.h"
//#include "geometry.h"
//#include "dyn_array.h"
//#include "gl.h"
#include "main.h"

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


void my_vertex_shader (const WFobj &obj, const int face_idx, const int vtx_idx, const fmat4 &model, const fmat4 &view, const fmat4 &projection, const fmat4 &viewport, float4 &vtx4d) {
	
	int3 vtx_elems;
	for (int k = 0; k < 3; k++) {
		vtx_elems[k] = *((int*) dyn_array_get (obj.face, face_idx*9 + vtx_idx*3 + k));
	}
	
	float3 obj_coords;
	for (int k = 0; k < 3; k++) {
		obj_coords[k] = *((float*) dyn_array_get (obj.vtx, vtx_elems[0]*3 + k));
	}
	
	float4 mc; // model coordinates
	float4 sc; // screen coordinates	
	// 0. transform 3d coords to homogenous coords
	float3_float4_conv (obj_coords, mc);
	
	
	// 1. Model - transform local coords to global
	// 2. View - transform global coords to adjust for camera position
	// 3. Projection - perspective correction
	// 4. Viewport - move to screen coords
	// Doing everyhting in reverse order:
	fmat4 tmp1, tmp2, tmp3;
	fmat4_fmat4_mult (&viewport, &projection, &tmp1);
	fmat4_fmat4_mult (&tmp1, &view, &tmp2);
	fmat4_fmat4_mult (&tmp2, &model, &tmp3); 
	fmat4_float4_mult (tmp3, mc, sc);
	vtx4d[0] = sc[0]/sc[3];
	vtx4d[1] = sc[1]/sc[3];
	vtx4d[2] = sc[2]/sc[3];
	vtx4d[3] = sc[3];
	
	VARYING_U[vtx_idx] = *((float*) dyn_array_get (obj.text, vtx_elems[1]*2));
	VARYING_V[vtx_idx] = *((float*) dyn_array_get (obj.text, vtx_elems[1]*2 + 1));
	
	if (0) {
		VARYING_NX[vtx_idx] = *((float*) dyn_array_get (obj.norm, vtx_elems[2]*3));
		VARYING_NY[vtx_idx] = *((float*) dyn_array_get (obj.norm, vtx_elems[2]*3+1));
		VARYING_NZ[vtx_idx] = *((float*) dyn_array_get (obj.norm, vtx_elems[2]*3+2));
	}
	else {
		float4 n, nr;
		n[0] = *((float*) dyn_array_get (obj.norm, vtx_elems[2]*3));
		n[1] = *((float*) dyn_array_get (obj.norm, vtx_elems[2]*3+1));
		n[2] = *((float*) dyn_array_get (obj.norm, vtx_elems[2]*3+2));
		n[3] = 1.0f;
		fmat4_float4_mult (UNIFORM_MIT, n, nr);
		VARYING_NX[vtx_idx] = nr[0]/nr[3];
		VARYING_NY[vtx_idx] = nr[1]/nr[3];
		VARYING_NZ[vtx_idx] = nr[2]/nr[3];
	}
}

bool my_pixel_shader (const WFobj &obj, const float3 &barw, pixel_color_t &color) {
	
	int uu = (int) (obj.textw * float3_float3_smult (VARYING_U, barw));
	int vv = (int) (obj.texth * float3_float3_smult (VARYING_V, barw));
	
	TGAColor tmpcolor = obj.texture.get(uu, obj.texth-vv-1);
	
	float intensity = 0;
	bool phong = 1;
	bool gouraud = !phong;	
	
	if (phong) {
		float3 interp_norm;
		interp_norm[0] = float3_float3_smult (VARYING_NX, barw);
		interp_norm[1] = float3_float3_smult (VARYING_NY, barw);
		interp_norm[2] = float3_float3_smult (VARYING_NZ, barw);
		float3_normalize(interp_norm);
		intensity = -float3_float3_smult (interp_norm, UNIFORM_LIGHT);
	}
	else if (gouraud) {
		float3 interp_intens;
		for (int i = 0; i < 3; i++) {
			float3 ii = {VARYING_NX[i], VARYING_NY[i], VARYING_NZ[i]};
			interp_intens[i] = float3_float3_smult (ii, UNIFORM_LIGHT);
		}
		intensity = -float3_float3_smult (interp_intens, barw);
	}
	if (intensity > 0) {
		//if (intensity < 0.2) intensity = 0.2;
		color = set_color (tmpcolor.r * intensity, tmpcolor.g * intensity, tmpcolor.b * intensity, 0);
		//color = set_color (255 * intensity, 255 * intensity, 255 * intensity, 0);
		return true;
	}
	return false;
}

