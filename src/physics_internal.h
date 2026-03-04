#ifndef PHYSICS_INTERNAL_H
#define PHYSICS_INTERNAL_H

#include "../include/physics.h"
#include "internal/physics_world_state.h"

RigidBody* physics_internal_raycast(PhysicsEngine* engine, Vec2 start, Vec2 end);
void physics_internal_emit_event(PhysicsEngine* engine, PhysicsEventType type, int ivalue, double dvalue,
                                 const char* message, const CollisionManifold* contact);

#endif
