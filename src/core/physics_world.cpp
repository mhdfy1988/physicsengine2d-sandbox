#include <stdlib.h>
#include "../include/physics_world.hpp"

struct PhysicsWorld {
    PhysicsEngine* engine;
};

extern "C" {

PhysicsWorld* physics_world_create(void) {
    PhysicsWorld* world = (PhysicsWorld*)malloc(sizeof(PhysicsWorld));
    if (world == NULL) return NULL;
    world->engine = physics_engine_create();
    if (world->engine == NULL) {
        free(world);
        return NULL;
    }
    return world;
}

void physics_world_free(PhysicsWorld* world) {
    if (world == NULL) return;
    physics_engine_free(world->engine);
    world->engine = NULL;
    free(world);
}

void physics_world_step(PhysicsWorld* world) {
    if (world == NULL || world->engine == NULL) return;
    physics_engine_step(world->engine);
}

PhysicsEngine* physics_world_engine(PhysicsWorld* world) {
    if (world == NULL) return NULL;
    return world->engine;
}

const PhysicsEngine* physics_world_engine_const(const PhysicsWorld* world) {
    if (world == NULL) return NULL;
    return world->engine;
}

}  // extern "C"
