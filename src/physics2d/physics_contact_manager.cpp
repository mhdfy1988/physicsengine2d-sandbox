#include <stddef.h>
#include "physics_internal.hpp"

static const CollisionManifold* find_persistent_contact(const PhysicsEngine* engine, const RigidBody* a, const RigidBody* b) {
    int i;
    if (engine == NULL || a == NULL || b == NULL) return NULL;
    for (i = 0; i < engine->persistent_contact_count; i++) {
        const CollisionManifold* c = &engine->persistent_contacts[i];
        if ((c->bodyA == a && c->bodyB == b) || (c->bodyA == b && c->bodyB == a)) {
            return c;
        }
    }
    return NULL;
}

extern "C" {

int physics_internal_default_build_contacts(PhysicsEngine* engine, void* user) {
    int i;
    (void)user;
    if (engine == NULL) return 0;
    for (i = 0; i < engine->broadphase_pair_count; i++) {
        int ia = engine->broadphase_pairs[i].ia;
        int ib = engine->broadphase_pairs[i].ib;
        RigidBody* a = physics_internal_body_from_id(engine, ia);
        RigidBody* b = physics_internal_body_from_id(engine, ib);
        CollisionInfo info = {};
        const CollisionManifold* persisted;
        if (a == NULL || b == NULL) continue;
        if (!collision_detect(a, b, &info)) continue;
        if (engine->contact_count >= MAX_CONTACTS) break;
        engine->contacts[engine->contact_count].bodyA = a;
        engine->contacts[engine->contact_count].bodyB = b;
        engine->contacts[engine->contact_count].info = info;
        engine->contacts[engine->contact_count].cached_normal_impulse = 0.0f;
        engine->contacts[engine->contact_count].cached_tangent_impulse = 0.0f;
        persisted = find_persistent_contact(engine, a, b);
        if (persisted != NULL) {
            engine->contacts[engine->contact_count].cached_normal_impulse = persisted->cached_normal_impulse;
            engine->contacts[engine->contact_count].cached_tangent_impulse = persisted->cached_tangent_impulse;
            engine->contacts[engine->contact_count].info.normal =
                vec2_normalize(vec2_add(info.normal, persisted->info.normal));
            engine->contacts[engine->contact_count].info.point =
                vec2_scale(vec2_add(info.point, persisted->info.point), 0.5f);
        }
        engine->contact_count++;
    }
    for (i = 1; i < engine->contact_count; i++) {
        CollisionManifold key = engine->contacts[i];
        int key_a = physics_internal_body_id_of(engine, key.bodyA);
        int key_b = physics_internal_body_id_of(engine, key.bodyB);
        int j = i - 1;
        if (key_a > key_b) {
            int t = key_a;
            key_a = key_b;
            key_b = t;
        }
        while (j >= 0) {
            int cur_a = physics_internal_body_id_of(engine, engine->contacts[j].bodyA);
            int cur_b = physics_internal_body_id_of(engine, engine->contacts[j].bodyB);
            if (cur_a > cur_b) {
                int t = cur_a;
                cur_a = cur_b;
                cur_b = t;
            }
            if (cur_a < key_a || (cur_a == key_a && cur_b <= key_b)) break;
            engine->contacts[j + 1] = engine->contacts[j];
            j--;
        }
        engine->contacts[j + 1] = key;
    }
    for (i = 0; i < engine->contact_count; i++) {
        physics_internal_emit_event(engine, PHYSICS_EVENT_CONTACT_CREATED, i, 0.0, NULL, &engine->contacts[i]);
    }
    return engine->contact_count;
}

}  // extern "C"
