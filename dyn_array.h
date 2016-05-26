#pragma once

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
