#include <stddef.h>
#include "physics_internal.h"

extern "C" {

PhysicsBodyId physics_internal_body_id_of(const PhysicsEngine* engine, const RigidBody* body) {
    int i;
    if (engine == NULL || body == NULL) return PHYSICS_BODY_ID_INVALID;
    for (i = 0; i < engine->body_count; i++) {
        if (engine->bodies[i] == body) return i;
    }
    return PHYSICS_BODY_ID_INVALID;
}

RigidBody* physics_internal_body_from_id(const PhysicsEngine* engine, PhysicsBodyId id) {
    if (engine == NULL) return NULL;
    if (id < 0 || id >= engine->body_count) return NULL;
    return engine->bodies[id];
}

PhysicsConstraintId physics_internal_constraint_id_of(const PhysicsEngine* engine, const Constraint* c) {
    int i;
    if (engine == NULL || c == NULL) return PHYSICS_CONSTRAINT_ID_INVALID;
    for (i = 0; i < engine->constraint_count; i++) {
        if (&engine->constraints[i] == c) return i;
    }
    return PHYSICS_CONSTRAINT_ID_INVALID;
}

Constraint* physics_internal_constraint_from_id(PhysicsEngine* engine, PhysicsConstraintId id) {
    if (engine == NULL) return NULL;
    if (id < 0 || id >= engine->constraint_count) return NULL;
    return &engine->constraints[id];
}

const Constraint* physics_internal_constraint_from_id_const(const PhysicsEngine* engine, PhysicsConstraintId id) {
    if (engine == NULL) return NULL;
    if (id < 0 || id >= engine->constraint_count) return NULL;
    return &engine->constraints[id];
}

CollisionManifold* physics_internal_contact_from_id(PhysicsEngine* engine, PhysicsContactId id) {
    if (engine == NULL) return NULL;
    if (id < 0 || id >= engine->contact_count) return NULL;
    return &engine->contacts[id];
}

const CollisionManifold* physics_internal_contact_from_id_const(const PhysicsEngine* engine, PhysicsContactId id) {
    if (engine == NULL) return NULL;
    if (id < 0 || id >= engine->contact_count) return NULL;
    return &engine->contacts[id];
}

}  // extern "C"
