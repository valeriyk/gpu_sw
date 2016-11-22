#pragma once 

#include "bitmap.h"
#include "geometry.h"

#include <stdint.h>

typedef struct WFobj {
	Bitmap *texture;
	Bitmap *normalmap;
	Bitmap *specularmap;
	struct WFobjPrivate *priv;
} WFobj;

WFobj * wfobj_new (const char *obj_file, Bitmap *texture, Bitmap *normalmap, Bitmap *specularmap);

void wfobj_free (WFobj *obj);

/*
void wfobj_set_face_idx   (const WFobj *obj, const int face_idx);
void wfobj_set_vtx_idx    (const WFobj *obj, const int vtx_idx);
*/

Float3 wfobj_get_vtx_coords           (const WFobj *obj, const int face_idx, const int vtx_idx);
Float3 wfobj_get_norm_coords          (const WFobj *obj, const int face_idx, const int vtx_idx);
Float2 wfobj_get_texture_coords       (const WFobj *obj, const int face_idx, const int vtx_idx);

void wfobj_get_rgb_from_texture       (const WFobj *obj, const int u, const int v, uint8_t *r, uint8_t *g, uint8_t *b);
Float3 wfobj_get_normal_from_map      (const WFobj *obj, const int u, const int v);
int  wfobj_get_specularity_from_map   (const WFobj *obj, const int u, const int v);
int  wfobj_get_num_of_faces           (const WFobj *obj);
