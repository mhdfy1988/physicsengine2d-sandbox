#ifndef PHYSICS_INTERNAL_WORLD_STATE_H
#define PHYSICS_INTERNAL_WORLD_STATE_H

#include "../../include/physics.hpp"
#include "physics_ids.hpp"
#include "physics_memory.hpp"
#include "physics_pipeline.hpp"

typedef struct {
    PhysicsBodyId ia;
    PhysicsBodyId ib;
} BroadphasePair;

typedef struct {
    unsigned char* buffer;
    int capacity;
    int used;
} PhysicsScratchArena;

struct PhysicsEngine {
    Vec2 gravity;
    PhysicsConfig config;
    PhysicsExperimentalConfig experimental;

    RigidBody* bodies[MAX_BODIES];
    int body_count;
    unsigned int body_epoch;

    CollisionManifold contacts[MAX_CONTACTS];
    int contact_count;
    CollisionManifold persistent_contacts[MAX_CONTACTS];
    int persistent_contact_count;

    Constraint constraints[MAX_CONSTRAINTS];
    int constraint_count;
    unsigned int constraint_epoch;

    BroadphasePair broadphase_pairs[MAX_BROADPHASE_PAIRS];
    int broadphase_pair_count;
    unsigned short* pair_stamp;
    unsigned short pair_stamp_frame;
    AABB aabbs[MAX_BODIES];
    PhysicsScratchArena scratch;
    PhysicsBroadphaseOps broadphase_ops;
    PhysicsNarrowphaseOps narrowphase_ops;
    PhysicsPipelinePluginV1 plugin;
    int plugin_installed;
    PhysicsJobSystemV1 job_system;
    int job_system_installed;
    PhysicsErrorCode last_error;
    PhysicsCallbacks callbacks;
    PhysicsEventSinkFn event_sink;
    void* event_sink_user;
    PhysicsConfigSnapshot last_step_config_snapshot;
    PhysicsStepProfile last_profile;
    float current_step_dt;
};

#endif
