#pragma once 

#include "geometry.h"
#include "dyn_array.h"
#include "gl.h"

typedef enum {V_DATA, VT_DATA, VN_DATA, VP_DATA, F_DATA, COMMENT, EMPTY} obj_line_type;
typedef enum {LINE_TYPE, VALUE1, VALUE2, VALUE3, VALUE4} obj_line_field;  
typedef enum {VERTEX_IDX, TEXTURE_IDX, NORMAL_IDX} obj_face_elem;

//int read_obj_file (const char *filename, float3 *obj_vtx, float3 *obj_norm, Point2Df *obj_text, Face *obj_face);
//int read_obj_file (const char *filename, DynArray *obj_vtx, DynArray *obj_norm, DynArray *obj_text, Face *obj_face);
int read_obj_file (const char *filename, DynArray *obj_vtx, DynArray *obj_norm, DynArray *obj_text, DynArray *obj_face);
