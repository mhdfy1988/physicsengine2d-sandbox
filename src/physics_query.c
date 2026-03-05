#include <stddef.h>
#include "physics_internal.h"

PhysicsBodyHandle physics_engine_get_body_handle(const PhysicsEngine* engine, const RigidBody* body) {
    PhysicsBodyHandle h;
    int i;
    h.index = -1;
    h.epoch = 0u;
    if (engine == NULL || body == NULL) return h;
    h.epoch = engine->body_epoch;
    for (i = 0; i < engine->body_count; i++) {
        if (engine->bodies[i] == body) {
            h.index = i;
            break;
        }
    }
    return h;
}

RigidBody* physics_engine_resolve_body_handle(const PhysicsEngine* engine, PhysicsBodyHandle handle) {
    if (engine == NULL) return NULL;
    if (handle.index < 0 || handle.index >= engine->body_count) return NULL;
    if (handle.epoch != engine->body_epoch) return NULL;
    return engine->bodies[handle.index];
}

PhysicsConstraintHandle physics_engine_get_constraint_handle(const PhysicsEngine* engine, const Constraint* constraint) {
    PhysicsConstraintHandle h;
    int i;
    h.index = -1;
    h.epoch = 0u;
    if (engine == NULL || constraint == NULL) return h;
    h.epoch = engine->constraint_epoch;
    for (i = 0; i < engine->constraint_count; i++) {
        if (&engine->constraints[i] == constraint) {
            h.index = i;
            break;
        }
    }
    return h;
}

const Constraint* physics_engine_resolve_constraint_handle(const PhysicsEngine* engine, PhysicsConstraintHandle handle) {
    if (engine == NULL) return NULL;
    if (handle.index < 0 || handle.index >= engine->constraint_count) return NULL;
    if (handle.epoch != engine->constraint_epoch) return NULL;
    return &engine->constraints[handle.index];
}

void physics_engine_set_event_sink(PhysicsEngine* engine, PhysicsEventSinkFn sink, void* user) {
    if (engine == NULL) return;
    engine->event_sink = sink;
    engine->event_sink_user = user;
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
    return physics_internal_constraint_id_of(engine, constraint);
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
