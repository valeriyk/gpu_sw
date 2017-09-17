#include "wavefront_obj.h"
#include "dynarray.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//#define NUM_OF_FACE_BINS NUM_OF_VSHADERS
//#define NUM_OF_FACE_BINS 1

typedef struct WFO_Private {
	DynArray *vtx;
	DynArray *norm;
	DynArray *text;
	DynArray *face;
	//DynArray *face[NUM_OF_FACE_BINS];
	//int face_offset;
	//int vtx_offset;
} WFO_Private;

typedef enum {VX_DATA = 0, V_DATA, VT_DATA, VN_DATA, VP_DATA, F_DATA, COMMENT, EMPTY} wfobj_line_type;
typedef enum {TYPE = 0, DELIMETER01, VALUE1, DELIMETER12, VALUE2, DELIMETER23, VALUE3, DELIMETER34, VALUE4} wfobj_line_field;  
typedef enum {VERTEX_IDX = 0, TEXTURE_IDX, NORMAL_IDX} wfobj_face_elem;



int  read_wfobj_file (const char *filename, WaveFrontObj *wfobj);


WaveFrontObj * wfobj_new (const char *wfobj_file) {
	
	WaveFrontObj *wfobj = (WaveFrontObj*) malloc (sizeof(WaveFrontObj));
	if (wfobj == NULL) return NULL;
	
	wfobj->private = (WFO_Private*) malloc (sizeof(WFO_Private));
	
	wfobj->private->vtx  = dyn_array_create (sizeof (float), 384);
    wfobj->private->norm = dyn_array_create (sizeof (float), 384);
	wfobj->private->text = dyn_array_create (sizeof (float), 256);
	//for (int i = 0; i < NUM_OF_FACE_BINS; i++) {
	//	wfobj->private->face[i] = dyn_array_create (sizeof   (int), 1152);
	//}
	wfobj->private->face = dyn_array_create (sizeof   (int), 1152);
	//wfobj->private->face_offset = 0;
    //wfobj->private->vtx_offset  = 0;
    
	read_wfobj_file (wfobj_file, wfobj);	        

    return wfobj;
}

void wfobj_free (WaveFrontObj *wfobj) {
	dyn_array_destroy (wfobj->private->vtx);
	dyn_array_destroy (wfobj->private->norm);
	dyn_array_destroy (wfobj->private->text);
	//for (int i = 0; i < NUM_OF_FACE_BINS; i++) {
	//	dyn_array_destroy (wfobj->private->face[i]);
	//}	
	dyn_array_destroy (wfobj->private->face);
	free (wfobj->private);
	free (wfobj);
}


int wfobj_get_num_of_faces (const WaveFrontObj *wfobj) {
	return wfobj->private->face->end / 9;
}

Float3 wfobj_get_vtx_coords  (const WaveFrontObj *wfobj, const int face_idx,  const int vtx_idx) {
	Float3 c;
	int vtx_coords_offset = *((int*) dyn_array_get (wfobj->private->face, face_idx*9 + vtx_idx*3));
	c.as_struct.x = *((float*) dyn_array_get (wfobj->private->vtx, vtx_coords_offset*3 + 0));
	c.as_struct.y = *((float*) dyn_array_get (wfobj->private->vtx, vtx_coords_offset*3 + 1));
	c.as_struct.z = *((float*) dyn_array_get (wfobj->private->vtx, vtx_coords_offset*3 + 2));
	return c;
}

Float2 wfobj_get_texture_coords (const WaveFrontObj *wfobj, const int face_idx, const int vtx_idx) {
	Float2 c;
	int text_coords_offset = *((int*) dyn_array_get (wfobj->private->face, face_idx*9 + vtx_idx*3 + 1));
	c.as_struct.u = *((float*) dyn_array_get (wfobj->private->text, text_coords_offset*2 + 0));
	c.as_struct.v = *((float*) dyn_array_get (wfobj->private->text, text_coords_offset*2 + 1));
	return c;
}

Float3 wfobj_get_norm_coords (const WaveFrontObj *wfobj, const int face_idx, const int vtx_idx) {
	Float3 c;
	int norm_coords_offset = *((int*) dyn_array_get (wfobj->private->face, face_idx*9 + vtx_idx*3 + 2));
	c.as_struct.x = *((float*) dyn_array_get (wfobj->private->norm, norm_coords_offset*3 + 0));
	c.as_struct.y = *((float*) dyn_array_get (wfobj->private->norm, norm_coords_offset*3 + 1));
	c.as_struct.z = *((float*) dyn_array_get (wfobj->private->norm, norm_coords_offset*3 + 2));
	return c;
}

void push_data (WaveFrontObj *wfobj, wfobj_line_type line_type, wfobj_line_field line_field, wfobj_face_elem face_elem, char *alpha_num) {
	DynArrayItem data;
	//static uint32_t bin_num = 0;
	// convert string to number and save it
	if ((line_type == V_DATA) && ((line_field == VALUE1) || (line_field == VALUE2) || (line_field == VALUE3))) {
		data.f = (float) atof (alpha_num);
		dyn_array_push (wfobj->private->vtx, &data);
	}
	else if ((line_type == F_DATA) && ((face_elem == VERTEX_IDX) || (face_elem == TEXTURE_IDX) || (face_elem == NORMAL_IDX))) {
		data.i = atoi (alpha_num) - 1; // decrement all indices because in OBJ they start at 1
		//dyn_array_push (wfobj->private->face[bin_num++], &data);
		//if (bin_num >= NUM_OF_FACE_BINS) {
		//	bin_num = 0; // wrap
		//}
		dyn_array_push (wfobj->private->face, &data);
	}
	else if ((line_type == VT_DATA) && ((line_field == VALUE1) || (line_field == VALUE2))) {
		data.f = (float) atof (alpha_num);
		dyn_array_push (wfobj->private->text, &data);
	}
	else if ((line_type == VN_DATA) && ((line_field == VALUE1) || (line_field == VALUE2) || (line_field == VALUE3))) {
		data.f = (float) atof (alpha_num);
		dyn_array_push (wfobj->private->norm, &data);
	}
}

// Parse Wavefront OBJ format
int read_wfobj_file (const char *filename, WaveFrontObj *wfobj) {

    const int ALPHA_SIZE = 16;
    
    char alpha_num [ALPHA_SIZE];
    for (int i = 0; i < ALPHA_SIZE; i++) alpha_num[i] = '\0';
    int  alpha_idx = 0;
    
    wfobj_line_type     line_type  = EMPTY;
    wfobj_line_field    line_field = TYPE;
    wfobj_face_elem     face_elem  = VERTEX_IDX;
    
    wfobj_line_type     next_line_type = line_type;
    wfobj_line_field    next_line_field = line_field;
    wfobj_face_elem     next_face_elem = face_elem;
    
    FILE *fp = fopen (filename, "r");
    if (!fp) return 1;
    
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
    	char c = (char) ch;
		
		//putchar (c);
		
		switch (line_type) {
			case EMPTY:
				switch (c) {
					case '\n':
					case '\r':
					case '\t':
					case ' ':     next_line_type =   EMPTY; break;
					case 'g':
					case 's':
					case 'm':
					case '#':     next_line_type = COMMENT; break;
					case 'f':     next_line_type =  F_DATA; break;
					case 'v':     next_line_type = VX_DATA; break;
					default:      printf ("EMPTY: Unknown line type\n"); return 2;
				}
				break;
			case VX_DATA:
				switch (c) {
					case '\t':    
					case ' ':     line_type      =  V_DATA;
					              next_line_type =  V_DATA;
					              break;
					case 't':     next_line_type = VT_DATA; break;
					case 'n':     next_line_type = VN_DATA; break;
					case 'p':     next_line_type = VP_DATA; break;
					default:      printf ("VX_DATA: Unknown line type\n");return 2;
				}
				break;
			case V_DATA:
			case VT_DATA:
			case VN_DATA:
			case VP_DATA:
			case F_DATA:
			case COMMENT:
			default:
				if ((c == '\n') || (c == '\r')) {
					next_line_type = EMPTY;
				}
				break;
		}
		
		switch (line_field) {
			case TYPE:
				if ((line_type == V_DATA) || (line_type == VT_DATA) || (line_type == VN_DATA) || (line_type == F_DATA)) {
					if      ((c == ' ')  || (c == '\t')) { next_line_field = DELIMETER01; }
					else if ((c == '\n') || (c == '\r')) { next_line_field = TYPE; }
				}
				break;
			case DELIMETER01:
				if      ((c == '\n') || (c == '\r')) { next_line_field = TYPE; }
				else if ((c != ' ')  && (c != '\t')) next_line_field = VALUE1;
				break;
			case VALUE1:
				if      ((c == ' ')  || (c == '\t')) { push_data (wfobj, line_type, line_field, face_elem, alpha_num); next_line_field = DELIMETER12; }
				else if ((c == '\n') || (c == '\r')) { push_data (wfobj, line_type, line_field, face_elem, alpha_num); next_line_field = TYPE; }
				break;
			case DELIMETER12:
				if      ((c == '\n') || (c == '\r')) { next_line_field = TYPE; }
				else if ((c != ' ')  && (c != '\t')) next_line_field = VALUE2;
				break;
			case VALUE2:
				if      ((c == ' ')  || (c == '\t')) { push_data (wfobj, line_type, line_field, face_elem, alpha_num); next_line_field = DELIMETER23; }
				else if ((c == '\n') || (c == '\r')) { push_data (wfobj, line_type, line_field, face_elem, alpha_num); next_line_field = TYPE; }
				break;
			case DELIMETER23:
				if      ((c == '\n') || (c == '\r')) { next_line_field = TYPE; }
				else if ((c != ' ')  && (c != '\t')) next_line_field = VALUE3;
				break;
			case VALUE3:
				if      ((c == ' ')  || (c == '\t')) { push_data (wfobj, line_type, line_field, face_elem, alpha_num); next_line_field = DELIMETER34;  }
				else if ((c == '\n') || (c == '\r')) { push_data (wfobj, line_type, line_field, face_elem, alpha_num); next_line_field = TYPE;  }
				break;
			case DELIMETER34:
				if      ((c == '\n') || (c == '\r')) { next_line_field = TYPE;  }
				else if ((c != ' ')  && (c != '\t')) next_line_field = VALUE4;
				break;
			case VALUE4:
				if      ((c == '\n') || (c == '\r')) { push_data (wfobj, line_type, line_field, face_elem, alpha_num); next_line_field = TYPE; }
				break;
			default:
				return 2;
		}
		
		switch (face_elem) {
			case VERTEX_IDX:
				if (c == '/') { 
					push_data (wfobj, line_type, line_field, face_elem, alpha_num);
					next_face_elem = TEXTURE_IDX;
				}
				break;
			case TEXTURE_IDX:
				if (c == '/') {
					push_data (wfobj, line_type, line_field, face_elem, alpha_num);
					next_face_elem = NORMAL_IDX;
				}
				else if ((c == ' ')  || (c == '\t') || (c == '\n') || (c == '\r')) {
					next_face_elem = VERTEX_IDX;
				}
				break;
			case NORMAL_IDX:			
				if ((c == ' ')  || (c == '\t') || (c == '\n') || (c == '\r')) {
					next_face_elem = VERTEX_IDX;
				}
		}
		
		if ((line_type != COMMENT) && (line_field != TYPE)) {
			
			// number can be represented as -0.1e2, check for all the characters:
			if (((c >= '0') && (c <= '9')) || (c == '.') || (c == '-') || (c == 'e')) {			
				assert (alpha_idx < ALPHA_SIZE);
				alpha_num[alpha_idx++] = c;	
			}
			else if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '/') || (c == '\r')) {
				// cleanup before moving on to the next value
				for (int i = 0; i < ALPHA_SIZE; i++) alpha_num[i] = '\0';
				alpha_idx = 0;
			}	
			else {
				printf ("Line parsing error, line_type=%d, line_field=%d, character=%x\n", line_type, line_field, c);
				return 2;
			}
		}
		
		line_type  = next_line_type;
		line_field = next_line_field;
		face_elem  = next_face_elem;
    }
    
    fclose (fp);
    return 0;
}
