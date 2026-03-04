#include <stddef.h>
#include "physics_internal.h"

int physics_internal_default_build_contacts(PhysicsEngine* engine, void* user) {
    int i;
    (void)user;
    if (engine == NULL) return 0;
    for (i = 0; i < engine->broadphase_pair_count; i++) {
        int ia = engine->broadphase_pairs[i].ia;
        int ib = engine->broadphase_pairs[i].ib;
        RigidBody* a = physics_internal_body_from_id(engine, ia);
        RigidBody* b = physics_internal_body_from_id(engine, ib);
        CollisionInfo info = {0};
        if (a == NULL || b == NULL) continue;
        if (!collision_detect(a, b, &info)) continue;
        if (engine->contact_count >= MAX_CONTACTS) break;
        engine->contacts[engine->contact_count].bodyA = a;
        engine->contacts[engine->contact_count].bodyB = b;
        engine->contacts[engine->contact_count].info = info;
        physics_internal_emit_event(engine, PHYSICS_EVENT_CONTACT_CREATED, engine->contact_count, 0.0, NULL,
                                    &engine->contacts[engine->contact_count]);
        engine->contact_count++;
    }
    return engine->contact_count;
}
