#ifndef PHYSICS_INTERNAL_MEMORY_H
#define PHYSICS_INTERNAL_MEMORY_H

void physics_internal_scratch_reset(PhysicsEngine* engine);
void* physics_internal_scratch_alloc(PhysicsEngine* engine, int bytes, int alignment);

#endif
