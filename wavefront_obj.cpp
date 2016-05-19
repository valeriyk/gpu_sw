#include "wavefront_obj.h"
#include "dyn_array.h"
#include <stdio.h>
#include <stdlib.h>

// Parse Wavefront OBJ format

//int read_obj_file (const char *filename, DynArray *obj_vtx, DynArray *obj_norm, DynArray *obj_text, DynArray *obj_face) {
int read_obj_file (const char *filename, DynArray *obj_vtx, float3 *obj_norm, Point2Df *obj_text, Face *obj_face) {

    const int ALPHA_SIZE = 16;
    
    int  vtx_idx = 0;
    int text_idx = 0;
    int face_idx = 0;
    int norm_idx = 0;
    
    char alpha_num [ALPHA_SIZE];
    for (int i = 0; i < ALPHA_SIZE; i++) alpha_num[i] = '\0';
    int  alpha_idx = 0;
    
    obj_line_type     line_type  = EMPTY;
    obj_line_field    line_field = LINE_TYPE;
    obj_face_elem     face_elem  = VERTEX_IDX;
    
    FILE *fp = fopen (filename, "r");
    if (!fp) return 1;
    
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
    	char c = (char) ch;
		
		// for any line that is a comment we check only for end of line:
		if (COMMENT == line_type) {
			if (c == '\n') line_type = EMPTY;
		}
		else {
			// First detect line type
			if (LINE_TYPE == line_field) {
				if (V_DATA != line_type) {
					switch (c) {
						case '\n':
						case '\r':
						case '\t':
						case ' ':     line_type =   EMPTY; break;
						case 'g':
						case 's':
						case '#':     line_type = COMMENT; break;
						case 'f':     line_type =  F_DATA; break;
						case 'v':     line_type =  V_DATA; break;
						default:      return 2;
					}
					// Assuming that the line is not a comment,
					// we keep parsing the line type until we detect that:
					// - line is F_DATA
					// Detecting V_DATA is not sufficient to stop parsing
					// because it may turn into VT_DATA, VN_DATA or VP_DATA,
					// but we can know this only after looking at the
					// next character
					if (F_DATA == line_type) line_field = VALUE1; 
				}
				else {
					// here we finally can find out if our line type
					// was V_DATA, or it is VT_DATA/VN_DATA/VP_DATA
					switch (c) {
						case '\t':    line_type =  V_DATA; break;
						case ' ':     line_type =  V_DATA; break;
						case 't':     line_type = VT_DATA; break;
						case 'n':     line_type = VN_DATA; break;
						case 'p':     line_type = VP_DATA; break;
						default:      return 2;
					}
					// No need to further parse the line type, move
					// to parsing first value
					line_field = VALUE1;
				}
			}
			// After line type detected, read values VALUE1...VALUE4
			// accordingly
			else {
				if (((c >= '0') && (c <= '9')) || (c == '.') || (c == '-') || (c == 'e')) {
					alpha_num[alpha_idx++] = c;	
				}
				else if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '/') || (c == '\r')) {
					if (alpha_num[0] == '\0') {
						 // skip heading white spaces
						 // if unexpected delimeter is found then fail
						 if ((c != ' ') && (c != '\t')) return 2;
					}
					else {
						// convert string to number and save it
						if (V_DATA == line_type) {
							float af = (float) atof (alpha_num);
							if ((line_field == VALUE1) || (line_field == VALUE2) || (line_field == VALUE3)) {
								
								float *data = (float*) dyn_array_new(obj_vtx);
								*data = af;
								//printf ("obj_vtx = %f\n", obj_vtx->data);
								dyn_array_push (obj_vtx, data);
							}
							//if      (line_field == VALUE1) obj_vtx[vtx_idx][1] = af;
							//else if (line_field == VALUE2) obj_vtx[vtx_idx][1] = af;
							//else if (line_field == VALUE3) obj_vtx[vtx_idx][2] = af;
						}
						else if (F_DATA == line_type) {
							int ai = atoi (alpha_num);
							ai--; // decrement all indices because in OBJ they start at 1
							if (VERTEX_IDX == face_elem) {							
								if      (line_field == VALUE1) obj_face[face_idx].vtx_idx[0] = ai;
								else if (line_field == VALUE2) obj_face[face_idx].vtx_idx[1] = ai;
								else if (line_field == VALUE3) obj_face[face_idx].vtx_idx[2] = ai;
							}
							else if (TEXTURE_IDX == face_elem) {							
								if      (line_field == VALUE1) obj_face[face_idx].txt_idx[0] = ai;
								else if (line_field == VALUE2) obj_face[face_idx].txt_idx[1] = ai;
								else if (line_field == VALUE3) obj_face[face_idx].txt_idx[2] = ai;
							}
						}
						else if (VT_DATA == line_type) {
							float af = (float) atof (alpha_num);
							if      (line_field == VALUE1) obj_text[text_idx].u = af;
							else if (line_field == VALUE2) obj_text[text_idx].v = af;
						}
						else if (VN_DATA == line_type) {
							float af = (float) atof (alpha_num);
							if      (line_field == VALUE1) obj_norm[norm_idx][0] = af;
							else if (line_field == VALUE2) obj_norm[norm_idx][1] = af;
							else if (line_field == VALUE3) obj_norm[norm_idx][2] = af;
						}
						
						if (c == '/') {
							switch (face_elem) {
								case VERTEX_IDX:  face_elem = TEXTURE_IDX; break;
								case TEXTURE_IDX: face_elem =  NORMAL_IDX; break;
								case NORMAL_IDX:  return 2;
							}
						}
						else {
							switch (line_field) {
								case VALUE1:    line_field = VALUE2; break;
								case VALUE2:    line_field = VALUE3; break;
								case VALUE3:    line_field = VALUE4; break;
								case VALUE4:    break;
								case LINE_TYPE: return 2;
							}
							// reset face elem before moving to the next value
							face_elem = VERTEX_IDX;
						}
											
						// cleanup before moving on to the next value
						for (int i = 0; i < ALPHA_SIZE; i++) alpha_num[i] = '\0';
						alpha_idx = 0;
						
						if (c == '\n') {
							if      ( V_DATA == line_type)  vtx_idx++;
							else if (VT_DATA == line_type) text_idx++;
							else if (VN_DATA == line_type) norm_idx++;
							else if ( F_DATA == line_type) face_idx++;
							
							line_type  = EMPTY;
							line_field = LINE_TYPE;	
						}
					}
				}
				else return 2;
			}
		}
    }
    
    fclose (fp);
    return 0;
}

