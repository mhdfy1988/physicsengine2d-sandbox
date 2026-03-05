#include <float.h>
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

void physics_internal_set_error(PhysicsEngine* engine, PhysicsErrorCode error, const char* message_override) {
    if (engine == NULL) return;
    engine->last_error = error;
    if (error != PHYSICS_ERROR_NONE) {
        const char* message = message_override != NULL ? message_override : physics_error_code_string(error);
        physics_internal_emit_event(engine, PHYSICS_EVENT_ERROR, (int)error, 0.0, message, NULL);
    }
}

static void physics_set_error(PhysicsEngine* engine, PhysicsErrorCode error) {
    physics_internal_set_error(engine, error, NULL);
}

static void physics_emit_trace_event(PhysicsEngine* engine, PhysicsEventType type, int ivalue, double dvalue,
                                     const char* message, const CollisionManifold* contact) {
#if PHYSICS_ENABLE_TRACE
    PhysicsTraceEvent event;
    memset(&event, 0, sizeof(event));
    event.type = type;
    event.step_index = engine->last_profile.step_index;
    switch (type) {
        case PHYSICS_EVENT_POST_BROADPHASE:
        case PHYSICS_EVENT_POST_NARROWPHASE:
            event.payload.stage.count = ivalue;
            event.payload.stage.elapsed_ms = dvalue;
            break;
        case PHYSICS_EVENT_STEP_END:
            event.payload.step_end.contact_count = ivalue;
            event.payload.step_end.total_ms = dvalue;
            break;
        case PHYSICS_EVENT_ERROR:
            event.payload.error.code = (PhysicsErrorCode)ivalue;
            event.payload.error.message = message;
            break;
        case PHYSICS_EVENT_CONTACT_CREATED:
            event.payload.contact.contact = contact;
            break;
        default:
            break;
    }
    if (engine->event_sink != NULL) {
        engine->event_sink(&event, engine->event_sink_user);
    }
#else
    (void)engine;
    (void)type;
    (void)ivalue;
    (void)dvalue;
    (void)message;
    (void)contact;
#endif
}

static void physics_emit_legacy_callbacks(PhysicsEngine* engine, PhysicsEventType type, int ivalue,
                                          const CollisionManifold* contact) {
    /* Compatibility bridge for legacy callbacks (always active for backward compatibility). */
    switch (type) {
        case PHYSICS_EVENT_STEP_BEGIN:
            if (engine->callbacks.on_pre_step != NULL) {
                engine->callbacks.on_pre_step(engine, engine->callbacks.user);
            }
            break;
        case PHYSICS_EVENT_POST_BROADPHASE:
            if (engine->callbacks.on_post_broadphase != NULL) {
                engine->callbacks.on_post_broadphase(engine, ivalue, engine->callbacks.user);
            }
            break;
        case PHYSICS_EVENT_POST_NARROWPHASE:
            if (engine->callbacks.on_post_narrowphase != NULL) {
                engine->callbacks.on_post_narrowphase(engine, ivalue, engine->callbacks.user);
            }
            break;
        case PHYSICS_EVENT_CONTACT_CREATED:
            if (engine->callbacks.on_contact_created != NULL && contact != NULL) {
                engine->callbacks.on_contact_created(engine, contact, engine->callbacks.user);
            }
            break;
        case PHYSICS_EVENT_STEP_END:
            if (engine->callbacks.on_post_step != NULL) {
                engine->callbacks.on_post_step(engine, &engine->last_profile, engine->callbacks.user);
            }
            break;
        default:
            break;
    }
}

void physics_internal_emit_event(PhysicsEngine* engine, PhysicsEventType type, int ivalue, double dvalue,
                                 const char* message, const CollisionManifold* contact) {
    if (engine == NULL) return;
    physics_emit_trace_event(engine, type, ivalue, dvalue, message, contact);
    physics_emit_legacy_callbacks(engine, type, ivalue, contact);
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

void physics_engine_set_callbacks(PhysicsEngine* engine, const PhysicsCallbacks* callbacks) {
    if (engine == NULL) return;
    if (callbacks == NULL) {
        memset(&engine->callbacks, 0, sizeof(engine->callbacks));
        return;
    }
    engine->callbacks = *callbacks;
}

void physics_engine_get_last_step_profile(const PhysicsEngine* engine, PhysicsStepProfile* out_profile) {
    if (engine == NULL || out_profile == NULL) return;
    *out_profile = engine->last_profile;
}

void physics_engine_set_broadphase_builder(PhysicsEngine* engine, PhysicsBroadphaseBuilder builder, void* user) {
    if (engine == NULL) return;
    if (builder == NULL) {
        engine->broadphase_ops.build_pairs = physics_internal_default_build_pairs;
        engine->broadphase_ops.user = NULL;
        return;
    }
    engine->broadphase_ops.build_pairs = builder;
    engine->broadphase_ops.user = user;
}

void physics_engine_set_narrowphase_builder(PhysicsEngine* engine, PhysicsNarrowphaseBuilder builder, void* user) {
    if (engine == NULL) return;
    if (builder == NULL) {
        engine->narrowphase_ops.build_contacts = physics_internal_default_build_contacts;
        engine->narrowphase_ops.user = NULL;
        return;
    }
    engine->narrowphase_ops.build_contacts = builder;
    engine->narrowphase_ops.user = user;
}

void physics_engine_reset_pipeline(PhysicsEngine* engine) {
    physics_internal_bind_default_pipeline(engine);
}

void physics_engine_clear_broadphase_pairs(PhysicsEngine* engine) {
    if (engine == NULL) return;
    engine->broadphase_pair_count = 0;
}

int physics_engine_add_broadphase_pair(PhysicsEngine* engine, int index_a, int index_b) {
    int i;
    if (engine == NULL) return 0;
    if (index_a < 0 || index_b < 0) {
        physics_set_error(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    if (index_a >= engine->body_count || index_b >= engine->body_count) {
        physics_set_error(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    if (index_a == index_b) {
        physics_set_error(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    if (index_a > index_b) {
        int t = index_a;
        index_a = index_b;
        index_b = t;
    }
    if (engine->broadphase_pair_count >= MAX_BROADPHASE_PAIRS) {
        physics_set_error(engine, PHYSICS_ERROR_CAPACITY_EXCEEDED);
        return 0;
    }

    for (i = 0; i < engine->broadphase_pair_count; i++) {
        BroadphasePair p = engine->broadphase_pairs[i];
        if (p.ia == index_a && p.ib == index_b) {
            physics_set_error(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
            return 0;
        }
    }
    engine->broadphase_pairs[engine->broadphase_pair_count].ia = index_a;
    engine->broadphase_pairs[engine->broadphase_pair_count].ib = index_b;
    engine->broadphase_pair_count++;
    physics_set_error(engine, PHYSICS_ERROR_NONE);
    return 1;
}

void physics_engine_clear_contacts(PhysicsEngine* engine) {
    if (engine == NULL) return;
    engine->contact_count = 0;
}

int physics_engine_add_contact(PhysicsEngine* engine, RigidBody* a, RigidBody* b, const CollisionInfo* info) {
    CollisionInfo local_info;
    if (engine == NULL || a == NULL || b == NULL) {
        physics_set_error(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    if (engine->contact_count >= MAX_CONTACTS) {
        physics_set_error(engine, PHYSICS_ERROR_CAPACITY_EXCEEDED);
        return 0;
    }
    if (info != NULL) {
        local_info = *info;
    } else {
        if (!collision_detect(a, b, &local_info)) {
            physics_set_error(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
            return 0;
        }
    }
    engine->contacts[engine->contact_count].bodyA = a;
    engine->contacts[engine->contact_count].bodyB = b;
    engine->contacts[engine->contact_count].info = local_info;
    engine->contacts[engine->contact_count].cached_normal_impulse = 0.0f;
    engine->contacts[engine->contact_count].cached_tangent_impulse = 0.0f;
    physics_internal_emit_event(engine, PHYSICS_EVENT_CONTACT_CREATED, engine->contact_count, 0.0, NULL,
                                &engine->contacts[engine->contact_count]);
    engine->contact_count++;
    physics_set_error(engine, PHYSICS_ERROR_NONE);
    return 1;
}

void physics_engine_add_body(PhysicsEngine* engine, RigidBody* body) {
    if (engine == NULL || body == NULL) {
        return;
    }

    for (int i = 0; i < engine->body_count; i++) {
        if (engine->bodies[i] == body) {
            return;
        }
    }

    if (engine->body_count >= MAX_BODIES) {
        return;
    }

    engine->bodies[engine->body_count++] = body;
    engine->body_epoch++;
}

RigidBody* physics_engine_detach_body(PhysicsEngine* engine, RigidBody* body) {
    if (engine == NULL || body == NULL) {
        return NULL;
    }

    for (int i = 0; i < engine->body_count; i++) {
        if (engine->bodies[i] == body) {
            for (int ci = 0; ci < engine->constraint_count;) {
                Constraint* c = &engine->constraints[ci];
                if (c->body_a == body || c->body_b == body) {
                    for (int cj = ci; cj < engine->constraint_count - 1; cj++) {
                        engine->constraints[cj] = engine->constraints[cj + 1];
                    }
                    engine->constraint_count--;
                    engine->constraint_epoch++;
                    continue;
                }
                ci++;
            }

            RigidBody* detached = engine->bodies[i];
            for (int j = i; j < engine->body_count - 1; j++) {
                engine->bodies[j] = engine->bodies[j + 1];
            }
            engine->bodies[engine->body_count - 1] = NULL;
            engine->body_count--;
            engine->body_epoch++;
            return detached;
        }
    }

    return NULL;
}

Constraint* physics_engine_add_distance_constraint(PhysicsEngine* engine, RigidBody* a, RigidBody* b,
                                                   Vec2 world_anchor_a, Vec2 world_anchor_b,
                                                   float stiffness, int collide_connected) {
    if (engine == NULL || a == NULL || b == NULL) {
        return NULL;
    }
    if (engine->constraint_count >= MAX_CONSTRAINTS) {
        return NULL;
    }

    Constraint* c = &engine->constraints[engine->constraint_count++];
    constraint_init_distance(c, a, b, world_anchor_a, world_anchor_b, stiffness, collide_connected);
    engine->constraint_epoch++;
    return c;
}

Constraint* physics_engine_add_spring_constraint(PhysicsEngine* engine, RigidBody* a, RigidBody* b,
                                                 Vec2 world_anchor_a, Vec2 world_anchor_b,
                                                 float rest_length, float stiffness, float damping,
                                                 int collide_connected) {
    if (engine == NULL || a == NULL || b == NULL) {
        return NULL;
    }
    if (engine->constraint_count >= MAX_CONSTRAINTS) {
        return NULL;
    }

    Constraint* c = &engine->constraints[engine->constraint_count++];
    constraint_init_spring(c, a, b, world_anchor_a, world_anchor_b, rest_length, stiffness, damping, collide_connected);
    engine->constraint_epoch++;
    return c;
}

Constraint* physics_engine_add_rope_constraint(PhysicsEngine* engine, RigidBody* a, RigidBody* b,
                                               Vec2 world_anchor_a, Vec2 world_anchor_b,
                                               float max_length, float stiffness, int collide_connected) {
    if (engine == NULL || a == NULL || b == NULL) {
        return NULL;
    }
    if (engine->constraint_count >= MAX_CONSTRAINTS) {
        return NULL;
    }

    Constraint* c = &engine->constraints[engine->constraint_count++];
    constraint_init_rope(c, a, b, world_anchor_a, world_anchor_b, max_length, stiffness, collide_connected);
    engine->constraint_epoch++;
    return c;
}

void physics_engine_clear_constraints(PhysicsEngine* engine) {
    if (engine == NULL) {
        return;
    }

    engine->constraint_count = 0;
    engine->constraint_epoch++;
}

void physics_engine_remove_body(PhysicsEngine* engine, RigidBody* body) {
    RigidBody* detached = physics_engine_detach_body(engine, body);
    if (detached != NULL) {
        body_free(detached);
    }
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
