#include <stdlib.h>
#include <string.h>
#include "physics_internal.h"

static PhysicsConfig physics_default_config(void) {
    PhysicsConfig cfg;
    cfg.time_step = 1.0f / 60.0f;
    cfg.substeps = 3;
    cfg.iterations = 5;
    cfg.damping = 0.99f;
    cfg.broadphase_cell_size = 10.0f;
    cfg.broadphase_type = PHYSICS_BROADPHASE_GRID;
    cfg.max_position_iterations_bias = 3;
    return cfg;
}

static PhysicsExperimentalConfig physics_default_experimental_config(void) {
    PhysicsExperimentalConfig cfg;
    cfg.ccd_enabled = 0;
    cfg.sleep_enabled = 0;
    cfg.threading_enabled = 0;
    cfg.worker_count = 1;
    return cfg;
}

PhysicsEngine* physics_engine_create(void) {
    PhysicsEngine* engine = (PhysicsEngine*)malloc(sizeof(PhysicsEngine));
    unsigned char* scratch;
    unsigned short* pair_stamp;
    int scratch_cap;
    if (engine == NULL) {
        return NULL;
    }
    memset(engine, 0, sizeof(*engine));

    engine->gravity = vec2(0, 9.8f);
    engine->config = physics_default_config();
    engine->experimental = physics_default_experimental_config();
    physics_internal_sanitize_config(&engine->config);
    physics_internal_sanitize_experimental_config(&engine->experimental);
    engine->body_count = 0;
    engine->contact_count = 0;
    engine->constraint_count = 0;
    engine->broadphase_pair_count = 0;
    scratch_cap = (MAX_BODIES * MAX_BODIES) + (MAX_BODIES * (int)sizeof(AABB)) + 65536;
    scratch = (unsigned char*)malloc((size_t)scratch_cap);
    if (scratch == NULL) {
        free(engine);
        return NULL;
    }
    pair_stamp = (unsigned short*)calloc((size_t)(MAX_BODIES * MAX_BODIES), sizeof(unsigned short));
    if (pair_stamp == NULL) {
        free(scratch);
        free(engine);
        return NULL;
    }
    engine->scratch.buffer = scratch;
    engine->scratch.capacity = scratch_cap;
    engine->scratch.used = 0;
    engine->pair_stamp = pair_stamp;
    engine->pair_stamp_frame = 1;
    physics_internal_bind_default_pipeline(engine);
    engine->current_step_dt = engine->config.time_step;

    return engine;
}

void physics_engine_free(PhysicsEngine* engine) {
    if (engine == NULL) {
        return;
    }

    for (int i = 0; i < engine->body_count; i++) {
        body_free(engine->bodies[i]);
        engine->bodies[i] = NULL;
    }
    free(engine->scratch.buffer);
    engine->scratch.buffer = NULL;
    free(engine->pair_stamp);
    engine->pair_stamp = NULL;

    free(engine);
}
