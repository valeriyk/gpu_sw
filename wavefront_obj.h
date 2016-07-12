#pragma once 

#include "bitmap.h"

#include <stdint.h>

typedef struct WFobj {
	Bitmap *texture;
	Bitmap *normalmap;
	Bitmap *specularmap;
	struct WFobjPrivate *priv;
} WFobj;

//WFobj * wfobj_new (const char *obj_file, const char *texture_file);
//WFobj * wfobj_new (const char *obj_file, const char *texture_file, const char *normalmap_file);
WFobj * wfobj_new (const char *obj_file, Bitmap *texture, Bitmap *normalmap, Bitmap *specularmap);
void    wfobj_free (WFobj *obj);

//void    wfobj_load_texture      (WFobj *obj, const char *texture_file);
//void    wfobj_load_normal_map   (WFobj *obj, const char *normal_map_file);
//void    wfobj_load_specular_map (WFobj *obj, const char *specular_map_file);


void    wfobj_set_face_idx   (const WFobj *obj, const int face_idx);
void    wfobj_set_vtx_idx    (const WFobj *obj, const int vtx_idx);
//float   wfobj_get_vtx_coord  (const WFobj *obj, const int coord_idx);
//float   wfobj_get_norm_coord (const WFobj *obj, const int coord_idx);
//float   wfobj_get_text_coord (const WFobj *obj, const int coord_idx);
float   wfobj_get_vtx_coord  (const WFobj *obj, const int face_idx, const int vtx_idx, const int coord_idx);
float   wfobj_get_norm_coord (const WFobj *obj, const int face_idx, const int vtx_idx, const int coord_idx);
float   wfobj_get_text_coord (const WFobj *obj, const int face_idx, const int vtx_idx, const int coord_idx);

//void wfobj_get_bitmap_rgb (const Bitmap *bmp, const int u, const int v, uint8_t *r, uint8_t *g, uint8_t *b);
//void wfobj_get_bitmap_xyz (const Bitmap *bmp, const int u, const int v, float *x, float *y, float *z);
//int  wfobj_get_bitmap_int (const Bitmap *bmp, const int u, const int v);

void wfobj_get_rgb_from_texture     (const WFobj *obj, const int u, const int v, uint8_t *r, uint8_t *g, uint8_t *b);
void wfobj_get_normal_from_map      (const WFobj *obj, const int u, const int v, float *x, float *y, float *z);
int  wfobj_get_specularity_from_map (const WFobj *obj, const int u, const int v);


int  wfobj_get_num_of_faces (const WFobj *obj);
