#pragma once

#include <ushader_public.h>

#include "bitmap.h"
#include "geometry.h"
//#include "geometry_fixpt.h"
#include "wavefront_obj.h"

#include <gpu_cfg.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>




Light light_turn_on  (Float3 dir, bool add_shadow_buf, gpu_cfg_t *cfg);
void  light_turn_off (Light *l);


void   set_screen_size   (gpu_cfg_t *cfg, size_t width, size_t height);
//size_t get_screen_width  (gpu_cfg_t *cfg);
//size_t get_screen_height (gpu_cfg_t *cfg);
//size_t get_screen_depth  (gpu_cfg_t *cfg);
//size_t get_tile_width    (gpu_cfg_t *cfg);
//size_t get_tile_height   (gpu_cfg_t *cfg);

void init_view             (fmat4 *m, Float3 *eye, Float3 *center, Float3 *up);
void init_perspective_proj (fmat4 *m, float left, float right, float top, float bot, float near, float far);
void init_ortho_proj       (fmat4 *m, float left, float right, float top, float bot, float near, float far);
void init_viewport         (fmat4 *m, int x, int y, int w, int h, int d);

void rotate_coords (fmat4 *in, fmat4 *out, float alpha_deg, axis axis);


//pixel_color_t set_color (uint8_t r, uint8_t g, uint8_t b, uint8_t a);

Object* obj_new (WaveFrontObj *wfobj, Bitmap *texture, Bitmap *normalmap, Bitmap *specularmap);
void obj_free            (Object *obj);
void obj_set_scale       (Object *obj, float x,     float y,     float z);
void obj_set_rotation    (Object *obj, float x_deg, float y_deg, float z_deg);
void obj_set_translation (Object *obj, float x,     float y,     float z);
void obj_init_model      (Object *obj);
//void obj_transform       (Object *obj, fmat4 *vpv, fmat4 *projview, float3 *light_dir);


