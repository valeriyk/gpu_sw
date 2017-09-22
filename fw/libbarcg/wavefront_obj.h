#pragma once 

#include "geometry.h"

#include <stdint.h>

typedef struct WaveFrontObj {
	struct WFO_Private *private;
	uint32_t num_of_faces;
} WaveFrontObj;

WaveFrontObj * wfobj_new (const char *obj_file);

void wfobj_free (WaveFrontObj *wfobj);

Float3 wfobj_get_vtx_coords           (const WaveFrontObj *wfobj, const int face_idx, const int vtx_idx);
Float3 wfobj_get_norm_coords          (const WaveFrontObj *wfobj, const int face_idx, const int vtx_idx);
Float2 wfobj_get_texture_coords       (const WaveFrontObj *wfobj, const int face_idx, const int vtx_idx);



