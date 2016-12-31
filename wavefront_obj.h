#pragma once 

#include "geometry.h"

#include <stdint.h>

typedef struct WFobj {
	struct WFobjPrivate *priv;
} WFobj;

//WFobj * wfobj_new (const char *obj_file, Bitmap *texture, Bitmap *normalmap, Bitmap *specularmap);
WFobj * wfobj_new (const char *obj_file);

void wfobj_free (WFobj *obj);

/*
void wfobj_set_face_idx   (const WFobj *obj, const int face_idx);
void wfobj_set_vtx_idx    (const WFobj *obj, const int vtx_idx);
*/

Float3 wfobj_get_vtx_coords           (const WFobj *obj, const int face_idx, const int vtx_idx);
Float3 wfobj_get_norm_coords          (const WFobj *obj, const int face_idx, const int vtx_idx);
Float2 wfobj_get_texture_coords       (const WFobj *obj, const int face_idx, const int vtx_idx);


int  wfobj_get_num_of_faces           (const WFobj *obj);
