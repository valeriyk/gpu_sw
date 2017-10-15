#include "gl.h"
#include "wavefront_obj.h"

//#include <tga.h>

#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
//#define STBI_TGA_ONLY
#include <stb_image.h>



// POSITIVE Z TOWARDS ME

/*
pixel_color_t set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	pixel_color_t pc;
	pc.r = r;
	pc.g = g;
	pc.b = b;
	//pc.a = a;
	return pc;
}
*/
void init_viewport (fmat4 *m, int x, int y, int w, int h, int d) {

	// X: map [-1:1] to [0:(SCREEN_WIDTH+HEIGTH)/2]
	// Y: map [-1:1] to [0:SCREEN_HEIGHT]
	// Z: map [-1:1] to [SCREEN_DEPTH:0]				
	// W: leave as is
				
	fmat4_identity (m);
	fmat4_set (m, 0, 0, h / 2.0f); //(w/2.0) * (h/w) = h/2.0 - adjust for screen aspect ratio
	fmat4_set (m, 0, 3, x + w / 2.0f);
	fmat4_set (m, 1, 1, h / 2.0f);
	fmat4_set (m, 1, 3, y + h / 2.0f);
	fmat4_set (m, 2, 2, -d / 2.0f); // minus sign because Z points in opposite directions in NDC and screen/clip
	fmat4_set (m, 2, 3,  d / 2.0f);
}

void set_screen_size (gpu_cfg_t *cfg, size_t width, size_t height) {
	
	cfg->screen_width  = width;//SCREEN_WIDTH;
	cfg->screen_height = height;//SCREEN_HEIGHT;
	cfg->num_of_tiles  = (width >> GPU_TILE_WIDTH_LOG2) * (height >> GPU_TILE_HEIGHT_LOG2);
	
	screenz_t depth = ~0; // all ones
	cfg->screen_depth = (size_t) depth;
}
/*
size_t get_screen_width  (gpu_cfg_t *cfg) {
	return cfg->screen_width;
}

size_t get_screen_height (gpu_cfg_t *cfg) {
	return cfg->screen_height;
}

size_t get_screen_depth  (gpu_cfg_t *cfg) {
	return cfg->screen_depth;
}
*/
Light light_turn_on (Float3 dir, bool add_shadow_buf, gpu_cfg_t *cfg) { //TBD add light_src
	Light l;
	l.enabled = true;
	l.dir = dir;
	l.src = Float3_set (-dir.as_struct.x, -dir.as_struct.y, -dir.as_struct.z);
	
	if (add_shadow_buf) {
		l.shadow_buf = calloc (get_screen_width(cfg) * get_screen_height(cfg), sizeof(screenz_t));
		if (l.shadow_buf != NULL) {
			l.has_shadow_buf = true;
		}
		else {
			l.has_shadow_buf = false;
		}
	}
	else {
		l.shadow_buf = NULL;
		l.has_shadow_buf = false;
	}
	return l;
}

void light_turn_off (Light *l) {
	l->enabled        = false;
	l->has_shadow_buf = false;
	
    if (l->shadow_buf != NULL) {
		free ((void *) l->shadow_buf);
	}
}

void init_perspective_proj (fmat4 *m, float left, float right, float top, float bot, float near, float far) {
	fmat4_identity (m);
	fmat4_set (m, 0, 0,       ( 2.0f * near) / (right - left));
	fmat4_set (m, 0, 2,       (right + left) / (right - left));
	fmat4_set (m, 1, 1,       ( 2.0f * near) / (  top -  bot));
	fmat4_set (m, 1, 2,       (  top +  bot) / (  top -  bot));
	fmat4_set (m, 2, 2,      -(  far + near) / (  far - near));
	fmat4_set (m, 2, 3, (-2.0f * far * near) / (  far - near));
	fmat4_set (m, 3, 2,  -1.0f);
	fmat4_set (m, 3, 3,   0.0f);
}

void init_ortho_proj (fmat4 *m, float left, float right, float top, float bot, float near, float far) {
	fmat4_identity (m);
	fmat4_set (m, 0, 0,            2.0f / (right - left));
	fmat4_set (m, 0, 3, -(right + left) / (right - left));
	fmat4_set (m, 1, 1,            2.0f / (  top -  bot));
	fmat4_set (m, 1, 3, -(  top +  bot) / (  top -  bot));
	fmat4_set (m, 2, 2,           -2.0f / (  far - near));
	fmat4_set (m, 2, 3, -(  far + near) / (  far - near));
	fmat4_set (m, 3, 3, 1.0f);
}

void init_view (fmat4 *m, Float3 *eye, Float3 *center, Float3 *up) {
	
	Float3 z = Float3_Float3_sub(eye, center);
	Float3_normalize (&z);
	Float3 x = Float3_Float3_crossprod(up, &z);
	Float3_normalize (&x);
	Float3 y = Float3_Float3_crossprod(&z, &x);
	Float3_normalize (&y);
	
	fmat4 Orient = FMAT4_IDENTITY;
	fmat4 Transl = FMAT4_IDENTITY;
	
	for (int i = 0; i < 3; i++)	{
		fmat4_set (&Orient, 0, i, x.as_array[i]);
		fmat4_set (&Orient, 1, i, y.as_array[i]);
		fmat4_set (&Orient, 2, i, z.as_array[i]);

		fmat4_set (&Transl, i, 3, -(eye->as_array[i]));
	}
	fmat4_fmat4_mult(&Orient, &Transl, m);
}

void rotate_coords (fmat4 *in, fmat4 *out, float alpha_deg, axis a) {
	float rad = alpha_deg * 0.01745f; // degrees to rad conversion
	float sin_alpha = sinf(rad);
	float cos_alpha = cosf(rad);
	
	fmat4 r = FMAT4_IDENTITY;
	
	fmat4_set (&r, 0, 0, (a == X) ? 1.0f : cos_alpha);
	fmat4_set (&r, 1, 1, (a == Y) ? 1.0f : cos_alpha);
	fmat4_set (&r, 2, 2, (a == Z) ? 1.0f : cos_alpha);
	
	fmat4_set (&r, 0, 1, (a == Z) ? -sin_alpha : 0.0f);
	fmat4_set (&r, 0, 2, (a == Y) ? -sin_alpha : 0.0f);
	fmat4_set (&r, 1, 2, (a == X) ? -sin_alpha : 0.0f);
	
	fmat4_set (&r, 1, 0, (a == Z) ?  sin_alpha : 0.0f);
	fmat4_set (&r, 2, 0, (a == Y) ?  sin_alpha : 0.0f);
	fmat4_set (&r, 2, 1, (a == X) ?  sin_alpha : 0.0f);
	
	fmat4_fmat4_mult (in, &r, out);
}

Object *obj_new (WaveFrontObj *wfobj, Bitmap *texture, Bitmap *normalmap, Bitmap *specularmap) {
	Object *obj = (Object *) malloc (sizeof(Object));
	obj->wfobj = wfobj;
	for (int i = 0; i < 3; i++) {
		obj->scale[i]  = 1.f;
		obj->rotate[i] = 0.f;
		obj->tran[i]   = 0.f;
	}
	fmat4_identity (&(obj->mvp));
	for (int i = 0; i < GPU_MAX_LIGHTS; i++) {
		fmat4_identity (&(obj->shadow_mvp[i]));
	}
	
	obj->texture     = texture;
	obj->normalmap   = normalmap;
	obj->specularmap = specularmap;
	
	return obj;
}

void obj_free (Object *obj) {
	free (obj);
}


void obj_set_scale (Object *obj, float x, float y, float z) {
	obj->scale[0] = x;
	obj->scale[1] = y;
	obj->scale[2] = z;
}

void obj_set_rotation (Object *obj, float x_deg, float y_deg, float z_deg) {
	obj->rotate[0] = x_deg;
	obj->rotate[1] = y_deg;
	obj->rotate[2] = z_deg;
}

void obj_set_translation (Object *obj, float x, float y, float z) {
	obj->tran[0] = x;
	obj->tran[1] = y;
	obj->tran[2] = z;
}

void obj_init_model (Object *obj) {
	
	// 1. translate	
	fmat4 t = FMAT4_IDENTITY;
	fmat4_set (&t, 0, 3, obj->tran[X]);
	fmat4_set (&t, 1, 3, obj->tran[Y]);
	fmat4_set (&t, 2, 3, obj->tran[Z]);
	
	// 2. rotate
	fmat4 rot_x, rot_xy, rot_xyz;
	rotate_coords (&t,      &rot_x,   obj->rotate[X], X);
	rotate_coords (&rot_x,  &rot_xy,  obj->rotate[Y], Y);
	rotate_coords (&rot_xy, &rot_xyz, obj->rotate[Z], Z);
	
	// 3. scale	
	fmat4 s = FMAT4_IDENTITY;
	fmat4_set (&s, 0, 0, obj->scale[X]);
	fmat4_set (&s, 1, 1, obj->scale[Y]);
	fmat4_set (&s, 2, 2, obj->scale[Z]);
	fmat4_fmat4_mult (&rot_xyz, &s, &(obj->model));
}


//~ Bitmap * bitmap_new () {
	//~ Bitmap *bmp = (Bitmap*) malloc (sizeof(Bitmap));
	//~ return bmp;
//~ }

void bitmap_free (Bitmap *bmp) {
	if (bmp->data != NULL) free (bmp->data);
	if (bmp       != NULL) free (bmp);
}

Bitmap *new_bitmap_from_file (const char *filename, const int bytespp) {
    
    //Bitmap *bmp = bitmap_new ();
    Bitmap *bmp = (Bitmap*) malloc (sizeof(Bitmap));
    if (bmp == NULL) return NULL;
    
    int w = 0;
    int h = 0;
    int ch_num = 0;
    bmp->data = (uint8_t *) stbi_load(filename, &w, &h, &ch_num, bytespp);
    if (bmp->data == NULL) {
		printf ("Error: could not load TGA\n");
		return NULL;
	}
	bmp->w       = (uint32_t) w;
	bmp->h       = (uint32_t) h;
	bmp->bytespp = bytespp;
	
	return bmp;
}
