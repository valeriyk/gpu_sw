#include "dyn_array.h"

DynArray * dyn_array_create (size_t elem_size, size_t initial_max) {
	DynArray *a = (DynArray*) malloc(sizeof(DynArray));
	a->max = initial_max;
	a->data = (objdata*) calloc(initial_max, sizeof(objdata));
	a->end = 0;
	a->elem_size = elem_size;
	a->expand_rate = 256;
	
	return a;
}

int dyn_array_push (DynArray *a, objdata *el) {
	a->data[a->end] = *el;
	a->end++;
	if (a->end >= a->max) return dyn_array_expand(a);
	else return 0;
}

int dyn_array_expand  (DynArray *a) {
	a->max = a->max + a->expand_rate;
	a->data = (objdata*) realloc(a->data, a->max * sizeof (objdata));
	return 0;
}

void dyn_array_destroy (DynArray *a) {
	if (a) {
		if (a->data) free(a->data);
		free(a);
	}
}
