#pragma once 

#include "geometry.h"
#include "tgaimage.h"
#include <stddef.h>
#include <stdlib.h>

typedef union ObjData {
	int i;
	float f;
} objdata;

typedef struct DynArray {
	int end;
	int max;
	size_t elem_size;
	size_t expand_rate;
	objdata *data;
} DynArray;

DynArray * dyn_array_create (size_t elem_size, size_t initial_max);

int    dyn_array_push    (DynArray *a, objdata *el);
int    dyn_array_expand  (DynArray *a);
void   dyn_array_destroy (DynArray *a);

static inline void* dyn_array_get (DynArray *a, int i) {return &(a->data[i]);}
//static inline void * dyn_array_new (DynArray *a)        {return calloc(1, a->elem_size);}

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
} WFobj;

void init_obj (WFobj &obj, const char *obj_file, const char *texture_file);
int read_obj_file (const char *filename, WFobj &obj);
