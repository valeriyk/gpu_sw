#pragma once 

#include "geometry.h"

#include <stdint.h>

// This should better be aligned on a data cache line boundary
typedef struct VtxAttr {
	Float3  vtx_coords;
	Float3 norm_coords;
	Float2 text_coords;
} __attribute__ ((aligned (64))) VtxAttr;


typedef struct WaveFrontObj {
	struct WFO_Private *private;
	VtxAttr *vtx_attribs;
	uint32_t num_of_faces;
} WaveFrontObj;

WaveFrontObj * wfobj_new (const char *obj_file);

void wfobj_free (WaveFrontObj *wfobj);

VtxAttr wfobj_get_attribs              (const WaveFrontObj *wfobj, const int face_idx, const int vtx_idx);
Float3  wfobj_get_vtx_coords           (const WaveFrontObj *wfobj, const int face_idx, const int vtx_idx);
Float3  wfobj_get_norm_coords          (const WaveFrontObj *wfobj, const int face_idx, const int vtx_idx);
Float2  wfobj_get_texture_coords       (const WaveFrontObj *wfobj, const int face_idx, const int vtx_idx);



