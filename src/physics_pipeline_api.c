#include <string.h>
#include "physics_internal.h"

static void physics_set_error(PhysicsEngine* engine, PhysicsErrorCode error) {
    physics_internal_set_error(engine, error, NULL);
}

void physics_engine_set_callbacks(PhysicsEngine* engine, const PhysicsCallbacks* callbacks) {
    if (engine == NULL) return;
    if (callbacks == NULL) {
        memset(&engine->callbacks, 0, sizeof(engine->callbacks));
        return;
    }
    engine->callbacks = *callbacks;
}

void physics_engine_get_last_step_profile(const PhysicsEngine* engine, PhysicsStepProfile* out_profile) {
    if (engine == NULL || out_profile == NULL) return;
    *out_profile = engine->last_profile;
}

void physics_engine_set_broadphase_builder(PhysicsEngine* engine, PhysicsBroadphaseBuilder builder, void* user) {
    if (engine == NULL) return;
    if (builder == NULL) {
        engine->broadphase_ops.build_pairs = physics_internal_default_build_pairs;
        engine->broadphase_ops.user = NULL;
        return;
    }
    engine->broadphase_ops.build_pairs = builder;
    engine->broadphase_ops.user = user;
}

void physics_engine_set_narrowphase_builder(PhysicsEngine* engine, PhysicsNarrowphaseBuilder builder, void* user) {
    if (engine == NULL) return;
    if (builder == NULL) {
        engine->narrowphase_ops.build_contacts = physics_internal_default_build_contacts;
        engine->narrowphase_ops.user = NULL;
        return;
    }
    engine->narrowphase_ops.build_contacts = builder;
    engine->narrowphase_ops.user = user;
}

void physics_engine_reset_pipeline(PhysicsEngine* engine) {
    physics_internal_bind_default_pipeline(engine);
}

void physics_engine_clear_broadphase_pairs(PhysicsEngine* engine) {
    if (engine == NULL) return;
    engine->broadphase_pair_count = 0;
}

int physics_engine_add_broadphase_pair(PhysicsEngine* engine, int index_a, int index_b) {
    int i;
    if (engine == NULL) return 0;
    if (index_a < 0 || index_b < 0) {
        physics_set_error(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    if (index_a >= engine->body_count || index_b >= engine->body_count) {
        physics_set_error(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    if (index_a == index_b) {
        physics_set_error(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    if (index_a > index_b) {
        int t = index_a;
        index_a = index_b;
        index_b = t;
    }
    if (engine->broadphase_pair_count >= MAX_BROADPHASE_PAIRS) {
        physics_set_error(engine, PHYSICS_ERROR_CAPACITY_EXCEEDED);
        return 0;
    }

    for (i = 0; i < engine->broadphase_pair_count; i++) {
        BroadphasePair p = engine->broadphase_pairs[i];
        if (p.ia == index_a && p.ib == index_b) {
            physics_set_error(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
            return 0;
        }
    }
    engine->broadphase_pairs[engine->broadphase_pair_count].ia = index_a;
    engine->broadphase_pairs[engine->broadphase_pair_count].ib = index_b;
    engine->broadphase_pair_count++;
    physics_set_error(engine, PHYSICS_ERROR_NONE);
    return 1;
}

void physics_engine_clear_contacts(PhysicsEngine* engine) {
    if (engine == NULL) return;
    engine->contact_count = 0;
}

int physics_engine_add_contact(PhysicsEngine* engine, RigidBody* a, RigidBody* b, const CollisionInfo* info) {
    CollisionInfo local_info;
    if (engine == NULL || a == NULL || b == NULL) {
        physics_set_error(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    if (engine->contact_count >= MAX_CONTACTS) {
        physics_set_error(engine, PHYSICS_ERROR_CAPACITY_EXCEEDED);
        return 0;
    }
    if (info != NULL) {
        local_info = *info;
    } else {
        if (!collision_detect(a, b, &local_info)) {
            physics_set_error(engine, PHYSICS_ERROR_INVALID_ARGUMENT);
            return 0;
        }
    }
    engine->contacts[engine->contact_count].bodyA = a;
    engine->contacts[engine->contact_count].bodyB = b;
    engine->contacts[engine->contact_count].info = local_info;
    engine->contacts[engine->contact_count].cached_normal_impulse = 0.0f;
    engine->contacts[engine->contact_count].cached_tangent_impulse = 0.0f;
    physics_internal_emit_event(engine, PHYSICS_EVENT_CONTACT_CREATED, engine->contact_count, 0.0, NULL,
                                &engine->contacts[engine->contact_count]);
    engine->contact_count++;
    physics_set_error(engine, PHYSICS_ERROR_NONE);
    return 1;
}
