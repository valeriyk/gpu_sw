#pragma once 

#include "geometry.h"
#include "dyn_array.h"
#include "tgaimage.h"

typedef enum {V_DATA, VT_DATA, VN_DATA, VP_DATA, F_DATA, COMMENT, EMPTY} obj_line_type;
typedef enum {LINE_TYPE, VALUE1, VALUE2, VALUE3, VALUE4} obj_line_field;  
typedef enum {VERTEX_IDX, TEXTURE_IDX, NORMAL_IDX} obj_face_elem;

typedef struct WFobj {
	DynArray *vtx = NULL;
	DynArray *norm = NULL;
	DynArray *text = NULL;
	DynArray *face = NULL;
	TGAImage texture;
	int textw;
	int texth;
	float3 scale;
	float3 rotate;
	float3 tran;
} WFobj;

void init_obj (WFobj &obj, const char *obj_file, const char *texture_file);
int read_obj_file (const char *filename, WFobj &obj);
