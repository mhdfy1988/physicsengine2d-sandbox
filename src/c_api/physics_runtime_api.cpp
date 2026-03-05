#include <stddef.h>
#include "physics_internal.h"

void physics_engine_set_gravity(PhysicsEngine* engine, Vec2 gravity) {
    if (engine == NULL) {
        return;
    }

    engine->gravity = gravity;
}

void physics_engine_set_time_step(PhysicsEngine* engine, float dt) {
    PhysicsRuntimeConfig cfg;
    if (engine == NULL || dt <= 0.0f) {
        return;
    }
    cfg.time_step = dt;
    cfg.substeps = engine->config.substeps;
    cfg.damping = engine->config.damping;
    physics_engine_set_runtime_config(engine, &cfg);
}

void physics_engine_set_iterations(PhysicsEngine* engine, int iterations) {
    PhysicsSolverConfig cfg;
    if (engine == NULL || iterations < 1) {
        return;
    }
    cfg.iterations = iterations;
    cfg.max_position_iterations_bias = engine->config.max_position_iterations_bias;
    physics_engine_set_solver_config(engine, &cfg);
}

void physics_engine_set_damping(PhysicsEngine* engine, float damping) {
    PhysicsRuntimeConfig cfg;
    if (engine == NULL) {
        return;
    }
    cfg.time_step = engine->config.time_step;
    cfg.substeps = engine->config.substeps;
    cfg.damping = damping;
    physics_engine_set_runtime_config(engine, &cfg);
}

void physics_engine_set_broadphase_cell_size(PhysicsEngine* engine, float cell_size) {
    PhysicsPipelineConfig cfg;
    if (engine == NULL || cell_size <= 0.0f) {
        return;
    }
    cfg.broadphase_cell_size = cell_size;
    cfg.broadphase_type = engine->config.broadphase_type;
    physics_engine_set_pipeline_config(engine, &cfg);
}

void physics_engine_set_broadphase_use_grid(PhysicsEngine* engine, int enable) {
    PhysicsPipelineConfig cfg;
    if (engine == NULL) {
        return;
    }
    cfg.broadphase_cell_size = engine->config.broadphase_cell_size;
    cfg.broadphase_type = enable ? PHYSICS_BROADPHASE_GRID : PHYSICS_BROADPHASE_BRUTE_FORCE;
    physics_engine_set_pipeline_config(engine, &cfg);
}

Vec2 physics_engine_get_gravity(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return vec2(0.0f, 0.0f);
    }
    return engine->gravity;
}

float physics_engine_get_time_step(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0.0f;
    }
    return engine->config.time_step;
}

int physics_engine_get_iterations(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->config.iterations;
}

float physics_engine_get_damping(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0.0f;
    }
    return engine->config.damping;
}

float physics_engine_get_broadphase_cell_size(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0.0f;
    }
    return engine->config.broadphase_cell_size;
}

int physics_engine_get_broadphase_use_grid(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->config.broadphase_type == PHYSICS_BROADPHASE_GRID ? 1 : 0;
}

void physics_engine_set_config(PhysicsEngine* engine, const PhysicsConfig* config) {
    PhysicsRuntimeConfig rcfg;
    PhysicsSolverConfig scfg;
    PhysicsPipelineConfig pcfg;
    if (engine == NULL || config == NULL) return;
    rcfg.time_step = config->time_step;
    rcfg.substeps = config->substeps;
    rcfg.damping = config->damping;
    scfg.iterations = config->iterations;
    scfg.max_position_iterations_bias = config->max_position_iterations_bias;
    pcfg.broadphase_cell_size = config->broadphase_cell_size;
    pcfg.broadphase_type = config->broadphase_type;
    physics_engine_set_runtime_config(engine, &rcfg);
    physics_engine_set_solver_config(engine, &scfg);
    physics_engine_set_pipeline_config(engine, &pcfg);
}

void physics_engine_get_config(const PhysicsEngine* engine, PhysicsConfig* out_config) {
    if (engine == NULL || out_config == NULL) return;
    *out_config = engine->config;
}

void physics_engine_update_velocities(PhysicsEngine* engine) {
    if (engine == NULL) return;
    physics_internal_update_velocities(engine, engine->config.time_step);
}

void physics_engine_update_positions(PhysicsEngine* engine) {
    if (engine == NULL) return;
    physics_internal_update_positions(engine, engine->config.time_step);
}

void physics_engine_detect_collisions(PhysicsEngine* engine) {
    if (engine != NULL) {
        engine->current_step_dt = engine->config.time_step;
    }
    physics_internal_detect_collisions(engine);
}

void physics_engine_resolve_collisions(PhysicsEngine* engine) {
    PhysicsSolverContext ctx;
    if (engine == NULL) return;
    ctx.velocity_iterations = engine->config.iterations;
    ctx.position_iterations = engine->config.iterations + engine->config.max_position_iterations_bias;
    ctx.dt = engine->config.time_step;
    physics_internal_resolve_collisions(engine, &ctx);
}

void physics_engine_clear_forces(PhysicsEngine* engine) {
    physics_internal_clear_forces(engine);
}

void physics_engine_step(PhysicsEngine* engine) {
    physics_internal_step(engine);
}

RigidBody* physics_engine_raycast(PhysicsEngine* engine, Vec2 start, Vec2 end) {
    return physics_internal_raycast(engine, start, end);
}

int physics_engine_get_bodies_in_area(PhysicsEngine* engine, Vec2 center, float radius,
                                      RigidBody** out_bodies, int max_bodies) {
    if (engine == NULL || out_bodies == NULL || max_bodies <= 0) {
        return 0;
    }

    int count = 0;

    for (int i = 0; i < engine->body_count && count < max_bodies; i++) {
        RigidBody* body = engine->bodies[i];
        if (body == NULL || body->shape == NULL) {
            continue;
        }

        Vec2 delta = vec2_sub(body->position, center);
        float dist = vec2_length(delta);

        if (body->shape->type == SHAPE_CIRCLE) {
            dist -= body->shape->data.circle.radius;
        }

        if (dist < radius) {
            out_bodies[count++] = body;
        }
    }

    return count;
}
