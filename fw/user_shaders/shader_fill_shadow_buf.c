#include "shader_fill_shadow_buf.h"
//#include "gl.h"
//#include "geometry_fixpt.h"


//#include <math.h>



#define FLOAT_NORM   1
#define FLOAT_TEXT   1
#define FLOAT_SHADOW 1


int count_shadows (Varying *vry);



////////////////////////////////////////////////////////////////////////
// This shader fills shadow buffer (which is treated as a Z-buffer):
// - Vertex shader transforms world coordinates to clip space
// - Pixel shader does nothing because we don't write anything to frame
//   buffer
////////////////////////////////////////////////////////////////////////


Float4 vshader_fill_shadow_buf (Object *obj, VtxAttr *attribs, Varying *vry, gpu_cfg_t *cfg) {
	// transform 3d coords of the vertex to homogenous clip coords
	//Float3 model   = wfobj_get_vtx_coords (obj->wfobj, face_idx, vtx_idx);
	//Float4 model4d = Float3_Float4_conv   (&model, 1);
	Float4 model4d = Float3_Float4_conv   (&attribs->vtx_coords, 1);
	Float4 clip    = fmat4_Float4_mult    (&(obj->mvp), &model4d); // model -> world -> eye -> clip
	return clip;
}

bool pshader_fill_shadow_buf (Object *obj, Varying *vry, gpu_cfg_t *cfg, pixel_color_t *color) {
	return false;
}
