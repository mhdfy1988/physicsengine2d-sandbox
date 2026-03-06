#include <stdlib.h>
#include <string.h>
#include "physics_internal.hpp"
#include "internal/physics_defaults.hpp"

static PhysicsConfig physics_default_config(void) {
    PhysicsConfig cfg;
    cfg.time_step = PHYSICS_DEFAULT_TIME_STEP;
    cfg.substeps = PHYSICS_DEFAULT_SUBSTEPS;
    cfg.iterations = PHYSICS_DEFAULT_ITERATIONS;
    cfg.damping = PHYSICS_DEFAULT_DAMPING;
    cfg.broadphase_cell_size = PHYSICS_DEFAULT_BROADPHASE_CELL_SIZE;
    cfg.broadphase_type = PHYSICS_BROADPHASE_GRID;
    cfg.max_position_iterations_bias = PHYSICS_DEFAULT_MAX_POSITION_ITERATION_BIAS;
    return cfg;
}

static PhysicsExperimentalConfig physics_default_experimental_config(void) {
    PhysicsExperimentalConfig cfg;
    cfg.ccd_enabled = PHYSICS_DEFAULT_EXPERIMENTAL_CCD;
    cfg.sleep_enabled = PHYSICS_DEFAULT_EXPERIMENTAL_SLEEP;
    cfg.threading_enabled = PHYSICS_DEFAULT_EXPERIMENTAL_THREADING;
    cfg.worker_count = PHYSICS_DEFAULT_EXPERIMENTAL_WORKERS;
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

    engine->gravity = vec2(PHYSICS_DEFAULT_GRAVITY_X, PHYSICS_DEFAULT_GRAVITY_Y);
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
