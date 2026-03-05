#ifndef PHYSICS_INTERNAL_MEMORY_H
#define PHYSICS_INTERNAL_MEMORY_H

struct PhysicsEngine;

void physics_internal_scratch_reset(struct PhysicsEngine* engine);
void* physics_internal_scratch_alloc(struct PhysicsEngine* engine, int bytes, int alignment);

#endif
