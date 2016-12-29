#include "wavefront_obj.h"
#include "dynarray.h"

#include <stdio.h>
#include <stdlib.h>



typedef struct WFobjPrivate {
	DynArray *vtx;
	DynArray *norm;
	DynArray *text;
	DynArray *face;
	int face_offset;
	int vtx_offset;
} WFobjPrivate;

typedef enum {V_DATA, VT_DATA, VN_DATA, VP_DATA, F_DATA, COMMENT, EMPTY} obj_line_type;
typedef enum {LINE_TYPE, VALUE1, VALUE2, VALUE3, VALUE4} obj_line_field;  
typedef enum {VERTEX_IDX, TEXTURE_IDX, NORMAL_IDX} obj_face_elem;



int  read_obj_file (const char *filename, WFobj *obj);

void wfobj_get_bitmap_rgb (const Bitmap *bmp, const int u, const int v, uint8_t *r, uint8_t *g, uint8_t *b);
void wfobj_get_bitmap_xyz (const Bitmap *bmp, const int u, const int v, float *x, float *y, float *z);
int  wfobj_get_bitmap_int (const Bitmap *bmp, const int u, const int v);


WFobj * wfobj_new (const char *obj_file, Bitmap *texture, Bitmap *normalmap, Bitmap *specularmap) {
	
	WFobj *obj = (WFobj*) malloc (sizeof(WFobj));
	if (obj == NULL) return NULL;
	
	obj->priv = (WFobjPrivate*) malloc (sizeof(WFobjPrivate));
	
	obj->priv->vtx  = dyn_array_create (sizeof (float), 384);
    obj->priv->norm = dyn_array_create (sizeof (float), 384);
	obj->priv->text = dyn_array_create (sizeof (float), 256);
	obj->priv->face = dyn_array_create (sizeof   (int), 1152);
	obj->priv->face_offset = 0;
    obj->priv->vtx_offset  = 0;
    
	read_obj_file (obj_file, obj);	        
    
	obj->texture     = texture;
	obj->normalmap   = normalmap;
	obj->specularmap = specularmap;
	
    return obj;
}

void wfobj_free (WFobj *obj) {
	dyn_array_destroy (obj->priv->vtx);
	dyn_array_destroy (obj->priv->norm);
	dyn_array_destroy (obj->priv->text);
	dyn_array_destroy (obj->priv->face);
	
	free (obj->priv);
	free (obj->texture);
	free (obj->normalmap);
	free (obj->specularmap);
	free (obj);
}

/*void wfobj_set_face_idx   (const WFobj *obj, const int face_idx) {
	obj->face_offset = face_idx*9;
}

void wfobj_set_vtx_idx    (const WFobj *obj, const int vtx_idx) {
	obj->vtx_offset = obj->face_offset + vtx_idx*3;
}*/

Float3 wfobj_get_vtx_coords  (const WFobj *obj, const int face_idx,  const int vtx_idx) {
	Float3 c;
	int vtx_coords_offset = *((int*) dyn_array_get (obj->priv->face, face_idx*9 + vtx_idx*3));
	c.as_struct.x = *((float*) dyn_array_get (obj->priv->vtx, vtx_coords_offset*3 + 0));
	c.as_struct.y = *((float*) dyn_array_get (obj->priv->vtx, vtx_coords_offset*3 + 1));
	c.as_struct.z = *((float*) dyn_array_get (obj->priv->vtx, vtx_coords_offset*3 + 2));
	return c;
}

Float2 wfobj_get_texture_coords (const WFobj *obj, const int face_idx, const int vtx_idx) {
	Float2 c;
	int text_coords_offset = *((int*) dyn_array_get (obj->priv->face, face_idx*9 + vtx_idx*3 + 1));
	c.as_struct.u = *((float*) dyn_array_get (obj->priv->text, text_coords_offset*2 + 0)) * (float) obj->texture->w;
	c.as_struct.v = *((float*) dyn_array_get (obj->priv->text, text_coords_offset*2 + 1)) * (float) obj->texture->h;
	return c;
}

Float3 wfobj_get_norm_coords (const WFobj *obj, const int face_idx, const int vtx_idx) {
	Float3 c;
	int norm_coords_offset = *((int*) dyn_array_get (obj->priv->face, face_idx*9 + vtx_idx*3 + 2));
	c.as_struct.x = *((float*) dyn_array_get (obj->priv->norm, norm_coords_offset*3 + 0));
	c.as_struct.y = *((float*) dyn_array_get (obj->priv->norm, norm_coords_offset*3 + 1));
	c.as_struct.z = *((float*) dyn_array_get (obj->priv->norm, norm_coords_offset*3 + 2));
	return c;
}

int wfobj_get_num_of_faces (const WFobj *obj) {
	return obj->priv->face->end / 9;
}

void wfobj_get_bitmap_rgb (const Bitmap *bmp, const int u, const int v, uint8_t *r, uint8_t *g, uint8_t *b) {
	if (bmp->data != NULL) {
		*r = *(bmp->data + (u + bmp->w * v) * (bmp->bytespp) + 0);
		*g = *(bmp->data + (u + bmp->w * v) * (bmp->bytespp) + 1);
		*b = *(bmp->data + (u + bmp->w * v) * (bmp->bytespp) + 2);
	}
	else {
		*r = *g = *b = 0;
	}
}

void wfobj_get_bitmap_xyz (const Bitmap *bmp, const int u, const int v, float *x, float *y, float *z) {
	if (bmp->data != NULL) {
		*x = *(bmp->data + (u + bmp->w * v) * (bmp->bytespp) + 0) / 255.f * 2.f - 1.f;
		*y = *(bmp->data + (u + bmp->w * v) * (bmp->bytespp) + 1) / 255.f * 2.f - 1.f;
		*z = *(bmp->data + (u + bmp->w * v) * (bmp->bytespp) + 2) / 255.f * 2.f - 1.f;
	}
	else {
		*x = *y = *z = 0.f;
	}
}

int wfobj_get_bitmap_int (const Bitmap *bmp, const int u, const int v) {
	if (bmp->data != NULL)
		return (int) *(bmp->data + (u + bmp->w * v) * (bmp->bytespp));
	else
		return 0;
}

void wfobj_get_rgb_from_texture     (const WFobj *obj, const int u, const int v, uint8_t *r, uint8_t *g, uint8_t *b) {
	wfobj_get_bitmap_rgb (obj->texture, u, v, r, g, b);
}

Float3 wfobj_get_normal_from_map     (const WFobj *obj, const int u, const int v) {
	Float3 n;
	wfobj_get_bitmap_xyz (obj->normalmap, u, v, &n.as_struct.x, &n.as_struct.y, &n.as_struct.z);
	return n;
}

int  wfobj_get_specularity_from_map (const WFobj *obj, const int u, const int v) {
	return wfobj_get_bitmap_int (obj->specularmap, u, v);
}

// Parse Wavefront OBJ format
int read_obj_file (const char *filename, WFobj *obj) {

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
		if (line_type == COMMENT) {
			if (c == '\n') line_type = EMPTY;
		}
		else {
			// First detect line type
			if (line_field == LINE_TYPE) {
				if (line_type != V_DATA) {
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
					if (line_type == F_DATA) line_field = VALUE1; 
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
			// After line type detected, read values VALUE1...VALUE4 accordingly
			else {
				if (((c >= '0') && (c <= '9')) || (c == '.') || (c == '-') || (c == 'e')) {
					alpha_num[alpha_idx++] = c;	
				}
				else if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '/') || (c == '\r')) {
					if (alpha_num[0] == '\0') {
						 // skip heading white spaces, if unexpected delimeter is found then fail
						 if ((c != ' ') && (c != '\t')) return 2;
					}
					else {
						DynArrayItem data;
						// convert string to number and save it
						if ((line_type == V_DATA) && ((line_field == VALUE1) || (line_field == VALUE2) || (line_field == VALUE3))) {
							data.f = (float) atof (alpha_num);
							dyn_array_push (obj->priv->vtx, &data);
						}
						else if ((line_type == F_DATA) && ((face_elem == VERTEX_IDX) || (face_elem == TEXTURE_IDX) || (face_elem == NORMAL_IDX))) {
							data.i = atoi (alpha_num) - 1; // decrement all indices because in OBJ they start at 1
							dyn_array_push (obj->priv->face, &data);
						}
						else if ((line_type == VT_DATA) && ((line_field == VALUE1) || (line_field == VALUE2))) {
							data.f = (float) atof (alpha_num);
							dyn_array_push (obj->priv->text, &data);
						}
						else if ((line_type == VN_DATA) && ((line_field == VALUE1) || (line_field == VALUE2) || (line_field == VALUE3))) {
							data.f = (float) atof (alpha_num);
							dyn_array_push (obj->priv->norm, &data);
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
						for (int i = 0; i < ALPHA_SIZE; i++) {
							alpha_num[i] = '\0';
						}
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
