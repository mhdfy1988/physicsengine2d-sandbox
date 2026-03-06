#ifndef PHYSICS_INTERNAL_H
#define PHYSICS_INTERNAL_H

#include <stddef.h>
#include "internal/physics_world_state.hpp"

#ifdef __cplusplus
extern "C" {
#endif

RigidBody* physics_internal_raycast(PhysicsEngine* engine, Vec2 start, Vec2 end);
void physics_internal_emit_event(PhysicsEngine* engine, PhysicsEventType type, int ivalue, double dvalue,
                                 const char* message, const CollisionManifold* contact);
void physics_internal_set_error(PhysicsEngine* engine, PhysicsErrorCode error, const char* message_override);
static inline void physics_internal_set_error_code(PhysicsEngine* engine, PhysicsErrorCode error) {
    physics_internal_set_error(engine, error, NULL);
}
void physics_internal_sanitize_config(PhysicsConfig* config);
void physics_internal_sanitize_experimental_config(PhysicsExperimentalConfig* config);

#ifdef __cplusplus
}
#endif

#endif
