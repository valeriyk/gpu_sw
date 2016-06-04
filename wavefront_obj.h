#pragma once 

//#include "tgaimage.h"
#include "tga.h"

typedef struct WFobj {
	//TGAImage *texture;
	tbyte *texture2;
	int textw;
	int texth;
	int textbytespp;
	struct WFobjPrivate *priv;
} WFobj;


WFobj * wfobj_new (const char *obj_file);
void    wfobj_free (WFobj *obj);
void    wfobj_load_texture (WFobj *obj, const char *texture_file);
void    wfobj_set_face_idx   (const WFobj *obj, const int face_idx);
void    wfobj_set_vtx_idx    (const WFobj *obj, const int vtx_idx);
//float   wfobj_get_vtx_coord  (const WFobj *obj, const int coord_idx);
//float   wfobj_get_norm_coord (const WFobj *obj, const int coord_idx);
//float   wfobj_get_text_coord (const WFobj *obj, const int coord_idx);
float   wfobj_get_vtx_coord  (const WFobj *obj, const int face_idx, const int vtx_idx, const int coord_idx);
float   wfobj_get_norm_coord (const WFobj *obj, const int face_idx, const int vtx_idx, const int coord_idx);
float   wfobj_get_text_coord (const WFobj *obj, const int face_idx, const int vtx_idx, const int coord_idx);
int     wfobj_get_num_of_faces (const WFobj *obj);
