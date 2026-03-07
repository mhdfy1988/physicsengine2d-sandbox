#ifndef PHYSICS_WORLD_H
#define PHYSICS_WORLD_H

#include "physics_core/physics.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PhysicsWorld PhysicsWorld;
typedef PhysicsWorld PhysicsScene;

PhysicsWorld* physics_world_create(void);
void physics_world_free(PhysicsWorld* world);
void physics_world_step(PhysicsWorld* world);
PhysicsEngine* physics_world_engine(PhysicsWorld* world);
const PhysicsEngine* physics_world_engine_const(const PhysicsWorld* world);

#ifdef __cplusplus
}
#endif

#endif
