#ifndef PHYSICS_INTERNAL_IDS_H
#define PHYSICS_INTERNAL_IDS_H

#include "../../include/body.hpp"
#include "../../include/collision.hpp"
#include "../../include/constraint.hpp"

struct PhysicsEngine;

#ifdef __cplusplus
extern "C" {
#endif

typedef int PhysicsBodyId;
typedef int PhysicsConstraintId;
typedef int PhysicsContactId;

enum {
    PHYSICS_BODY_ID_INVALID = -1,
    PHYSICS_CONSTRAINT_ID_INVALID = -1,
    PHYSICS_CONTACT_ID_INVALID = -1
};

PhysicsBodyId physics_internal_body_id_of(const struct PhysicsEngine* engine, const RigidBody* body);
RigidBody* physics_internal_body_from_id(const struct PhysicsEngine* engine, PhysicsBodyId id);
PhysicsConstraintId physics_internal_constraint_id_of(const struct PhysicsEngine* engine, const Constraint* c);
Constraint* physics_internal_constraint_from_id(struct PhysicsEngine* engine, PhysicsConstraintId id);
const Constraint* physics_internal_constraint_from_id_const(const struct PhysicsEngine* engine, PhysicsConstraintId id);
CollisionManifold* physics_internal_contact_from_id(struct PhysicsEngine* engine, PhysicsContactId id);
const CollisionManifold* physics_internal_contact_from_id_const(const struct PhysicsEngine* engine, PhysicsContactId id);

#ifdef __cplusplus
}
#endif

#endif
