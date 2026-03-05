#include <string.h>
#include "physics_internal.h"

int physics_engine_capture_snapshot(const PhysicsEngine* engine, PhysicsWorldSnapshot* out_snapshot) {
    int i;
    if (engine == NULL || out_snapshot == NULL) return 0;
    memset(out_snapshot, 0, sizeof(*out_snapshot));
    out_snapshot->version = PHYSICS_WORLD_SNAPSHOT_VERSION;
    out_snapshot->body_count = engine->body_count;
    if (out_snapshot->body_count > MAX_BODIES) out_snapshot->body_count = MAX_BODIES;
    for (i = 0; i < out_snapshot->body_count; i++) {
        const RigidBody* b = engine->bodies[i];
        if (b == NULL) continue;
        out_snapshot->bodies[i].active = b->active;
        out_snapshot->bodies[i].sleeping = b->sleeping;
        out_snapshot->bodies[i].type = b->type;
        out_snapshot->bodies[i].mass = b->mass;
        out_snapshot->bodies[i].inertia = b->inertia;
        out_snapshot->bodies[i].position = b->position;
        out_snapshot->bodies[i].angle = b->angle;
        out_snapshot->bodies[i].velocity = b->velocity;
        out_snapshot->bodies[i].angular_velocity = b->angular_velocity;
    }
    return 1;
}

int physics_engine_apply_snapshot(PhysicsEngine* engine, const PhysicsWorldSnapshot* snapshot) {
    int i;
    if (engine == NULL || snapshot == NULL) return 0;
    if (snapshot->version != PHYSICS_WORLD_SNAPSHOT_VERSION) return 0;
    if (snapshot->body_count > engine->body_count) return 0;
    for (i = 0; i < snapshot->body_count; i++) {
        RigidBody* b = engine->bodies[i];
        if (b == NULL) continue;
        b->active = snapshot->bodies[i].active ? 1 : 0;
        b->sleeping = snapshot->bodies[i].sleeping ? 1 : 0;
        b->type = snapshot->bodies[i].type;
        b->mass = snapshot->bodies[i].mass;
        b->inertia = snapshot->bodies[i].inertia;
        b->position = snapshot->bodies[i].position;
        b->angle = snapshot->bodies[i].angle;
        b->velocity = snapshot->bodies[i].velocity;
        b->angular_velocity = snapshot->bodies[i].angular_velocity;
        b->force = vec2(0.0f, 0.0f);
        b->torque = 0.0f;
        b->acceleration = vec2(0.0f, 0.0f);
        b->angular_acceleration = 0.0f;
        b->sleep_timer = 0.0f;
        if (b->type == BODY_STATIC || b->mass <= 0.0f) {
            b->inv_mass = 0.0f;
            b->inv_inertia = 0.0f;
        } else {
            b->inv_mass = 1.0f / b->mass;
            b->inv_inertia = (b->inertia > 0.0f) ? 1.0f / b->inertia : 0.0f;
        }
    }
    return 1;
}

int physics_engine_replay_from_snapshot(PhysicsEngine* engine, const PhysicsWorldSnapshot* snapshot, int steps) {
    int i;
    if (engine == NULL || snapshot == NULL || steps < 0) return 0;
    if (!physics_engine_apply_snapshot(engine, snapshot)) return 0;
    for (i = 0; i < steps; i++) {
        physics_engine_step(engine);
    }
    return 1;
}
