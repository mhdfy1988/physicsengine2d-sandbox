#ifndef SCENE_CATALOG_H
#define SCENE_CATALOG_H

#include <stddef.h>

typedef struct {
    float gravity_y;
    float time_step;
    float damping;
    int iterations;
    float ball_mass;
    float box_mass;
    float spawn_circle_radius;
    float spawn_box_size;
    float ball_restitution;
    float box_restitution;
} SceneConfig;

enum { SCENE_COUNT = 9 };

const wchar_t* scene_catalog_name(int scene_index);
const SceneConfig* scene_catalog_default(int scene_index);
void scene_catalog_copy_defaults(SceneConfig* out_configs, int max_count);

#endif
