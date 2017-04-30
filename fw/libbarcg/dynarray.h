#pragma once


#include <stdlib.h>

typedef union DynArrayItem {
	size_t i;
	float  f;
} DynArrayItem;

typedef struct DynArray {
	int end;
	int max;
	size_t elem_size;
	size_t expand_rate;
	DynArrayItem *data;
} DynArray;



DynArray * dyn_array_create (size_t elem_size, size_t initial_max);

int    dyn_array_push    (DynArray *a, DynArrayItem *el);
int    dyn_array_expand  (DynArray *a);
int    dyn_array_get_end (DynArray *a);
void   dyn_array_destroy (DynArray *a);

static inline void* dyn_array_get (DynArray *a, int i) {
	return &(a->data[i]);
}
