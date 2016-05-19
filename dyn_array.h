#pragma once

typedef struct DynArray {
	int end;
	int max;
	size_t elem_size;
	size_t expand_rate;
	void **data;
} DynArray;

DynArray * dyn_array_create (size_t elem_size, size_t initial_max);

static inline void * dyn_array_get (DynArray *a, int i) {return a->data[i];}

int    dyn_array_push    (DynArray *a, void *el);
int    dyn_array_expand  (DynArray *a);
void   dyn_array_destroy (DynArray *a);
