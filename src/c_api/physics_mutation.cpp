#include <stddef.h>
#include "physics_internal.h"

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
