#include "wavefront_obj.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>





typedef struct VtxAttrIdx {
	uint32_t  coord_idx;
	uint32_t norm_idx;
	uint32_t text_idx;
} VtxAttrIdx;

typedef struct FaceAttrIdx {
	VtxAttrIdx vtx[3];
} FaceAttrIdx;


typedef enum {VX_DATA = 0, V_DATA, VT_DATA, VN_DATA, VP_DATA, F_DATA, COMMENT, EMPTY} wfobj_line_type;
typedef enum {TYPE = 0, DELIMETER01, VALUE1, DELIMETER12, VALUE2, DELIMETER23, VALUE3, DELIMETER34, VALUE4, TAIL} wfobj_line_field;  
typedef enum {VERTEX_IDX = 0, TEXTURE_IDX, NORMAL_IDX} wfobj_face_elem;



int  read_wfobj_file (const char *filename, WaveFrontObj *wfobj);




WaveFrontObj * wfobj_new (const char *wfobj_file) {
	
	WaveFrontObj *wfobj = (WaveFrontObj*) calloc (1, sizeof(WaveFrontObj));
	if (wfobj == NULL) return NULL;
	    
    read_wfobj_file (wfobj_file, wfobj);	        

    return wfobj;
}

void wfobj_free (WaveFrontObj *wfobj) {
	free (wfobj->vtx_attribs);
	free (wfobj);
}

VtxAttr wfobj_get_attribs  (const WaveFrontObj *wfobj, const int face_idx,  const int vtx_idx) {
	return wfobj->vtx_attribs[face_idx*3 + vtx_idx];
}

Float3 wfobj_get_vtx_coords  (const WaveFrontObj *wfobj, const int face_idx,  const int vtx_idx) {
	return wfobj->vtx_attribs[face_idx*3 + vtx_idx].vtx_coords;
}

Float3 wfobj_get_norm_coords (const WaveFrontObj *wfobj, const int face_idx, const int vtx_idx) {
	return wfobj->vtx_attribs[face_idx*3 + vtx_idx].norm_coords;
}

Float2 wfobj_get_texture_coords (const WaveFrontObj *wfobj, const int face_idx, const int vtx_idx) {
	return wfobj->vtx_attribs[face_idx*3 + vtx_idx].text_coords;
}


void store_vdata3 (Float3 *arr_ptr, wfobj_line_field line_field, char *alpha_num, uint32_t idx) {
	
	// convert string to number and save it
	float f = (float) atof (alpha_num);
	switch (line_field) {
		case VALUE1:  arr_ptr[idx].as_struct.x = f; break;
		case VALUE2:  arr_ptr[idx].as_struct.y = f; break;
		case VALUE3:  arr_ptr[idx].as_struct.z = f; break;
		default:      break;
	}
}

void store_vdata2 (Float2 *arr_ptr, wfobj_line_field line_field, char *alpha_num, uint32_t idx) {
	
	// convert string to number and save it
	float f = (float) atof (alpha_num);
	switch (line_field) {
		case VALUE1:  arr_ptr[idx].as_struct.u = f; break;
		case VALUE2:  arr_ptr[idx].as_struct.v = f; break;
		default:      break;
	}
}

void store_fdata (FaceAttrIdx *arr_ptr, wfobj_face_elem face_elem, char *alpha_num, uint32_t face_idx, uint32_t vtx_idx) {	
	
	// convert string to number and save it
	uint32_t i = atoi (alpha_num) - 1; // decrement all indices because in OBJ they start at 1
	switch (face_elem) {
		case VERTEX_IDX:  arr_ptr[face_idx].vtx[vtx_idx].coord_idx  = i; break;
		case TEXTURE_IDX: arr_ptr[face_idx].vtx[vtx_idx].text_idx = i; break;
		case NORMAL_IDX:  arr_ptr[face_idx].vtx[vtx_idx].norm_idx = i; break;
		default:          break;
	}
}

// Parse Wavefront OBJ format
int read_wfobj_file (const char *filename, WaveFrontObj *wfobj) {

    const int ALPHA_SIZE = 16;
    
    char alpha_num [ALPHA_SIZE];
    for (int i = 0; i < ALPHA_SIZE; i++) alpha_num[i] = '\0';
    int  alpha_idx = 0;
    
    FILE *fp = fopen (filename, "r");
    if (!fp) return 1;
    
    int ch;
    
    printf ("Reading Wavefront OBJ file\n");	
    
    // First pass - only count the number of faces, vertices, normals, and texture coordinates,
    // then allocate arrays for them.
    size_t num_of_faces        = 0;
    size_t num_of_vtx_entries  = 0;
    size_t num_of_norm_entries = 0;
    size_t num_of_text_entries = 0;
    
    wfobj_line_type     line_type  = EMPTY;
    wfobj_line_type     next_line_type = line_type;
    
    while ((ch = fgetc(fp)) != EOF) {
    	char c = (char) ch;
		
		switch (line_type) {
			case EMPTY:
				switch (c) {
					case '\n':
					case '\r':
					case '\t':
					case ' ':     next_line_type =   EMPTY;
					              break;
					              
					case 'g':
					case 's':
					case 'm':
					case '#':     next_line_type = COMMENT;
					              break;
					
					case 'f':     num_of_faces++;
					              next_line_type =  F_DATA;
					              break;
					              
					case 'v':     next_line_type = VX_DATA;
					              break;
					              
					default:      printf ("\tEMPTY: Unknown line type: %c\n", c);
					              return 2;
				}
				break;
				
			case VX_DATA:
				switch (c) {
					case '\t':    
					case ' ':     num_of_vtx_entries++;
					              line_type      =  V_DATA;
					              next_line_type =  V_DATA;
					              break;
					              
					case 't':     num_of_text_entries++;
					              next_line_type = VT_DATA;
					              break;
					              
					case 'n':     num_of_norm_entries++;
					              next_line_type = VN_DATA;
					              break;
					              
					case 'p':     next_line_type = VP_DATA;
					              break;
					              
					default:      printf ("\tVX_DATA: Unknown line type\n");
					              return 2;
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
		line_type  = next_line_type;
	}
	printf ("\tPass 1: %zu faces, %zu vertices, %zu vertex normals, %zu texture vertices detected\n", num_of_faces, num_of_vtx_entries, num_of_norm_entries, num_of_text_entries);	
    
    wfobj->num_of_faces      = num_of_faces;
    
    wfobj->vtx_attribs        = calloc (num_of_faces     * 3, sizeof (VtxAttr));
    FaceAttrIdx *face_arr_ptr = calloc (num_of_faces        , sizeof (FaceAttrIdx));
    Float3       *vtx_arr_ptr = calloc (num_of_vtx_entries  , sizeof (Float3));
    Float3      *norm_arr_ptr = calloc (num_of_norm_entries , sizeof (Float3));
    Float2      *text_arr_ptr = calloc (num_of_text_entries , sizeof (Float2));
    
    
    // Second pass - parse the OBJ file and fill the arrays
    rewind (fp);
    
    size_t face_idx = 0;
    size_t vtx_idx = 0;
    size_t coord_idx  = 0;
    size_t norm_idx = 0;
    size_t text_idx = 0;
    
    line_type         = EMPTY;
    next_line_type    = line_type;
    
    wfobj_line_field    line_field = TYPE;
    wfobj_line_field    next_line_field = line_field;
    
    wfobj_face_elem     face_elem  = VERTEX_IDX;
    wfobj_face_elem     next_face_elem = face_elem;
    
    while ((ch = fgetc(fp)) != EOF) {
    	char c = (char) ch;
		
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
					default:      printf ("\t\tEMPTY: Unknown line type\n"); return 2;
				}
				break;
			case VX_DATA:
				switch (c) {
					case '\t':    
					case ' ':     line_type      =  V_DATA;
					              next_line_type =  V_DATA;
					              break;
					case 't':     next_line_type = VT_DATA;
					              break;
					case 'n':     next_line_type = VN_DATA;
					              break;
					case 'p':     next_line_type = VP_DATA; break;
					default:      printf ("\t\tVX_DATA: Unknown line type\n");return 2;
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
				else if ((c != ' ')  && (c != '\t')) { next_line_field = VALUE1; }
				break;
			case VALUE1:
				if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r')) {
					switch (line_type) {
						case  F_DATA: store_fdata  (face_arr_ptr, face_elem,  alpha_num, face_idx, vtx_idx);
						              vtx_idx++;
						              break;
						case  V_DATA: store_vdata3 ( vtx_arr_ptr, line_field, alpha_num,  coord_idx); break;
						case VN_DATA: store_vdata3 (norm_arr_ptr, line_field, alpha_num, norm_idx); break;
						case VT_DATA: store_vdata2 (text_arr_ptr, line_field, alpha_num, text_idx); break;
						default:      break;
					}
					if ((c == ' ') || (c == '\t')) next_line_field = DELIMETER12;
					else next_line_field = TYPE;
				}
				break;
			case DELIMETER12:
				if      ((c == '\n') || (c == '\r')) { next_line_field = TYPE; }
				else if ((c != ' ')  && (c != '\t')) { next_line_field = VALUE2; }
				break;
			case VALUE2:
				if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r')) {
					switch (line_type) {
						case  F_DATA: store_fdata  (face_arr_ptr, face_elem,  alpha_num, face_idx, vtx_idx);
						              vtx_idx++;
						              break;
						case  V_DATA: store_vdata3 ( vtx_arr_ptr, line_field, alpha_num,  coord_idx); break;
						case VN_DATA: store_vdata3 (norm_arr_ptr, line_field, alpha_num, norm_idx); break;
						case VT_DATA: store_vdata2 (text_arr_ptr, line_field, alpha_num, text_idx); break;
						default:      break;
					}
					if ((c == ' ') || (c == '\t')) next_line_field = DELIMETER23;
					else next_line_field = TYPE;
				}
				break;
			case DELIMETER23:
				if      ((c == '\n') || (c == '\r')) { next_line_field = TYPE; }
				else if ((c != ' ')  && (c != '\t')) { next_line_field = VALUE3; }
				//if (line_type == F_DATA) face_idx++;
				break;
			case VALUE3:
				if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r')) {
					switch (line_type) {
						case  F_DATA: store_fdata  (face_arr_ptr, face_elem,  alpha_num, face_idx, vtx_idx);
						              vtx_idx = 0;
						              break;
						case  V_DATA: store_vdata3 ( vtx_arr_ptr, line_field, alpha_num,  coord_idx); break;
						case VN_DATA: store_vdata3 (norm_arr_ptr, line_field, alpha_num, norm_idx); break;
						//case VT_DATA: store_vdata2 (text_arr_ptr, line_field, alpha_num, text_idx); break;
						default:      break;
					}
					if ((c == ' ') || (c == '\t')) next_line_field = DELIMETER34;
					else next_line_field = TYPE;
				}
				break;
			case DELIMETER34:
				if      ((c == '\n') || (c == '\r')) { next_line_field = TYPE;  }
				else if ((c != ' ')  && (c != '\t')) { next_line_field = VALUE4; }
				break;
			case VALUE4:
				if      ((c == ' ')  && (c == '\t')) { next_line_field = TAIL; }
				else if ((c == '\n') || (c == '\r')) { next_line_field = TYPE; }
				break;
			case TAIL:
				if      ((c == '\n') || (c == '\r')) { next_line_field = TYPE; }
				break;
			default:
				return 2;
		}
		
		switch (face_elem) {
			case VERTEX_IDX:
				if (c == '/') { 
					store_fdata  (face_arr_ptr, face_elem,  alpha_num, face_idx, vtx_idx);
					next_face_elem = TEXTURE_IDX;
				}
				break;
			case TEXTURE_IDX:
				if (c == '/') {
					store_fdata  (face_arr_ptr, face_elem,  alpha_num, face_idx, vtx_idx);
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
				printf ("\t\tLine parsing error, line_type=%d, line_field=%d, character=%x\n", line_type, line_field, c);
				return 2;
			}
		}
		
		if (((line_field == VALUE2) || (line_field == DELIMETER23) || (line_field == VALUE3) || (line_field == DELIMETER34)) && (next_line_field == TYPE)) {
			switch (line_type) {
				case F_DATA:  face_idx++; break;
				case V_DATA:   coord_idx++; break;
				case VN_DATA: norm_idx++; break;
				case VT_DATA: text_idx++; break;
				default:                  break;
			}
		}
		
		line_type  = next_line_type;
		line_field = next_line_field;
		face_elem  = next_face_elem;
    }
    printf ("\tPass 2: %zu faces, %zu vertices, %zu vertex normals, %zu texture vertices processed\n", face_idx, coord_idx, norm_idx, text_idx);	
    printf ("\tPass 2: data read in correctly\n");
    fclose (fp);
    
    
    // Fill the Vertex Attributes array
    
    for (int i = 0; i < wfobj->num_of_faces; i++) {
		//printf ("face_arr_ptr[%d] indeces: vtx%d, norm%d, text%d", i, face_arr_ptr[i].coord_idx, face_arr_ptr[i].norm_idx, face_arr_ptr[i].text_idx);
		for (int j = 0; j < 3; j++) {
			wfobj->vtx_attribs[i * 3 + j].vtx_coords  =  vtx_arr_ptr[face_arr_ptr[i].vtx[j].coord_idx];
			wfobj->vtx_attribs[i * 3 + j].norm_coords = norm_arr_ptr[face_arr_ptr[i].vtx[j].norm_idx];
			wfobj->vtx_attribs[i * 3 + j].text_coords = text_arr_ptr[face_arr_ptr[i].vtx[j].text_idx];
		}
		//~ printf ("\tCombined vtx%d data: coord (%f:%f:%f) norm (%f:%f:%f) text (%f:%f)\n", i,
			//~ wfobj->vtx_attribs[i].vtx_coords.as_struct.x, wfobj->vtx_attribs[i].vtx_coords.as_struct.y, wfobj->vtx_attribs[i].vtx_coords.as_struct.z,
			//~ wfobj->vtx_attribs[i].norm_coords.as_struct.x, wfobj->vtx_attribs[i].norm_coords.as_struct.y, wfobj->vtx_attribs[i].norm_coords.as_struct.z,
			//~ wfobj->vtx_attribs[i].text_coords.as_struct.u, wfobj->vtx_attribs[i].text_coords.as_struct.v);
    }
    
    free (face_arr_ptr);
    free ( vtx_arr_ptr);
    free (norm_arr_ptr);
    free (text_arr_ptr);
    
    
    return 0;
}
