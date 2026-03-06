#include <stddef.h>
#include <stdint.h>
#include "physics_internal.hpp"

extern "C" {

void physics_internal_scratch_reset(PhysicsEngine* engine) {
    if (engine == NULL) return;
    engine->scratch.used = 0;
}

void* physics_internal_scratch_alloc(PhysicsEngine* engine, int bytes, int alignment) {
    int aligned_used;
    int mask;
    if (engine == NULL || engine->scratch.buffer == NULL || bytes <= 0) return NULL;
    if (alignment <= 0) alignment = (int)sizeof(void*);
    mask = alignment - 1;
    if ((alignment & mask) != 0) return NULL;
    aligned_used = (engine->scratch.used + mask) & ~mask;
    if (aligned_used > engine->scratch.capacity - bytes) return NULL;
    engine->scratch.used = aligned_used + bytes;
    return (void*)(engine->scratch.buffer + aligned_used);
}

}  // extern "C"
