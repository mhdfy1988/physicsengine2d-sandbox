#ifndef PHYSICS_INTERNAL_IDS_H
#define PHYSICS_INTERNAL_IDS_H

#include "../../include/body.h"
#include "../../include/collision.h"
#include "../../include/constraint.h"

typedef int PhysicsBodyId;
typedef int PhysicsConstraintId;
typedef int PhysicsContactId;

enum {
    PHYSICS_BODY_ID_INVALID = -1,
    PHYSICS_CONSTRAINT_ID_INVALID = -1,
    PHYSICS_CONTACT_ID_INVALID = -1
};

PhysicsBodyId physics_internal_body_id_of(const PhysicsEngine* engine, const RigidBody* body);
RigidBody* physics_internal_body_from_id(const PhysicsEngine* engine, PhysicsBodyId id);
PhysicsConstraintId physics_internal_constraint_id_of(const PhysicsEngine* engine, const Constraint* c);
Constraint* physics_internal_constraint_from_id(PhysicsEngine* engine, PhysicsConstraintId id);
const Constraint* physics_internal_constraint_from_id_const(const PhysicsEngine* engine, PhysicsConstraintId id);
CollisionManifold* physics_internal_contact_from_id(PhysicsEngine* engine, PhysicsContactId id);
const CollisionManifold* physics_internal_contact_from_id_const(const PhysicsEngine* engine, PhysicsContactId id);

#endif
