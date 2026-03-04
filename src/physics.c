#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "physics_internal.h"

PhysicsEngine* physics_engine_create(void) {
    PhysicsEngine* engine = (PhysicsEngine*)malloc(sizeof(PhysicsEngine));
    if (engine == NULL) {
        return NULL;
    }

    engine->gravity = vec2(0, 9.8f);
    engine->damping = 0.99f;
    engine->time_step = 1.0f / 60.0f;
    engine->iterations = 5;
    engine->body_count = 0;
    engine->contact_count = 0;
    engine->constraint_count = 0;
    engine->broadphase_pair_count = 0;
    engine->broadphase_cell_size = 10.0f;
    engine->broadphase_use_grid = 1;

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

    free(engine);
}

void physics_engine_set_gravity(PhysicsEngine* engine, Vec2 gravity) {
    if (engine == NULL) {
        return;
    }

    engine->gravity = gravity;
}

void physics_engine_set_time_step(PhysicsEngine* engine, float dt) {
    if (engine == NULL || dt <= 0.0f) {
        return;
    }

    engine->time_step = dt;
}

void physics_engine_set_iterations(PhysicsEngine* engine, int iterations) {
    if (engine == NULL || iterations < 1) {
        return;
    }

    engine->iterations = iterations;
}

void physics_engine_set_damping(PhysicsEngine* engine, float damping) {
    if (engine == NULL) {
        return;
    }

    engine->damping = clamp(damping, 0.0f, 1.0f);
}

void physics_engine_set_broadphase_cell_size(PhysicsEngine* engine, float cell_size) {
    if (engine == NULL || cell_size <= 0.0f) {
        return;
    }

    engine->broadphase_cell_size = cell_size;
}

void physics_engine_set_broadphase_use_grid(PhysicsEngine* engine, int enable) {
    if (engine == NULL) {
        return;
    }

    engine->broadphase_use_grid = enable ? 1 : 0;
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
    return engine->time_step;
}

int physics_engine_get_iterations(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->iterations;
}

float physics_engine_get_damping(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0.0f;
    }
    return engine->damping;
}

float physics_engine_get_broadphase_cell_size(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0.0f;
    }
    return engine->broadphase_cell_size;
}

int physics_engine_get_broadphase_use_grid(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->broadphase_use_grid;
}

int physics_engine_get_body_count(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->body_count;
}

RigidBody* physics_engine_get_body(const PhysicsEngine* engine, int index) {
    if (engine == NULL || index < 0 || index >= engine->body_count) {
        return NULL;
    }
    return engine->bodies[index];
}

int physics_engine_get_contact_count(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->contact_count;
}

const CollisionManifold* physics_engine_get_contact(const PhysicsEngine* engine, int index) {
    if (engine == NULL || index < 0 || index >= engine->contact_count) {
        return NULL;
    }
    return &engine->contacts[index];
}

int physics_engine_get_constraint_count(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->constraint_count;
}

const Constraint* physics_engine_get_constraint(const PhysicsEngine* engine, int index) {
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return NULL;
    }
    return &engine->constraints[index];
}

int physics_engine_find_constraint_index(const PhysicsEngine* engine, const Constraint* constraint) {
    int i;
    if (engine == NULL || constraint == NULL) {
        return -1;
    }
    for (i = 0; i < engine->constraint_count; i++) {
        if (&engine->constraints[i] == constraint) {
            return i;
        }
    }
    return -1;
}

int physics_engine_constraint_is_active(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0;
    }
    return c->active;
}

ConstraintType physics_engine_constraint_get_type(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return CONSTRAINT_DISTANCE;
    }
    return c->type;
}

float physics_engine_constraint_get_rest_length(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0.0f;
    }
    return c->rest_length;
}

float physics_engine_constraint_get_stiffness(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0.0f;
    }
    return c->stiffness;
}

float physics_engine_constraint_get_damping(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0.0f;
    }
    return c->damping;
}

float physics_engine_constraint_get_break_force(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0.0f;
    }
    return c->break_force;
}

float physics_engine_constraint_get_last_force(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0.0f;
    }
    return c->last_force;
}

int physics_engine_constraint_get_collide_connected(const PhysicsEngine* engine, int index) {
    const Constraint* c = physics_engine_get_constraint(engine, index);
    if (c == NULL) {
        return 0;
    }
    return c->collide_connected;
}

void physics_engine_constraint_set_active(PhysicsEngine* engine, int index, int active) {
    Constraint* c;
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return;
    }
    c = &engine->constraints[index];
    c->active = active ? 1 : 0;
}

void physics_engine_constraint_set_rest_length(PhysicsEngine* engine, int index, float rest_length) {
    Constraint* c;
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return;
    }
    c = &engine->constraints[index];
    c->rest_length = max_f(rest_length, 0.0f);
}

void physics_engine_constraint_set_stiffness(PhysicsEngine* engine, int index, float stiffness) {
    Constraint* c;
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return;
    }
    c = &engine->constraints[index];
    c->stiffness = max_f(stiffness, 0.0f);
}

void physics_engine_constraint_set_damping(PhysicsEngine* engine, int index, float damping) {
    Constraint* c;
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return;
    }
    c = &engine->constraints[index];
    c->damping = max_f(damping, 0.0f);
}

void physics_engine_constraint_set_break_force(PhysicsEngine* engine, int index, float break_force) {
    Constraint* c;
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return;
    }
    c = &engine->constraints[index];
    c->break_force = max_f(break_force, 0.0f);
}

void physics_engine_constraint_set_collide_connected(PhysicsEngine* engine, int index, int collide_connected) {
    Constraint* c;
    if (engine == NULL || index < 0 || index >= engine->constraint_count) {
        return;
    }
    c = &engine->constraints[index];
    c->collide_connected = collide_connected ? 1 : 0;
}

int physics_engine_get_broadphase_pair_count(const PhysicsEngine* engine) {
    if (engine == NULL) {
        return 0;
    }
    return engine->broadphase_pair_count;
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
    return c;
}

void physics_engine_clear_constraints(PhysicsEngine* engine) {
    if (engine == NULL) {
        return;
    }

    engine->constraint_count = 0;
}

void physics_engine_remove_body(PhysicsEngine* engine, RigidBody* body) {
    RigidBody* detached = physics_engine_detach_body(engine, body);
    if (detached != NULL) {
        body_free(detached);
    }
}

void physics_engine_update_velocities(PhysicsEngine* engine) {
    physics_internal_update_velocities(engine);
}

void physics_engine_update_positions(PhysicsEngine* engine) {
    physics_internal_update_positions(engine);
}

void physics_engine_detect_collisions(PhysicsEngine* engine) {
    physics_internal_detect_collisions(engine);
}

void physics_engine_resolve_collisions(PhysicsEngine* engine) {
    physics_internal_resolve_collisions(engine);
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
