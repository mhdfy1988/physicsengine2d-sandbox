#include <math.h>
#include <string.h>
#include "physics_internal.h"

#define GRID_BUCKETS 2048
#define GRID_MAX_ENTRIES 32768
#define GRID_MAX_SPAN 24

typedef struct {
    int body_index;
    int next;
} GridEntry;

typedef struct {
    int body_index;
    float min_x;
} SapProxy;

static int aabb_overlaps(AABB a, AABB b) {
    if (a.max.x < b.min.x || b.max.x < a.min.x) return 0;
    if (a.max.y < b.min.y || b.max.y < a.min.y) return 0;
    return 1;
}

static int pair_stamp_mark_if_new(PhysicsEngine* engine, int ia, int ib) {
    unsigned int idx;
    if (engine == NULL || engine->pair_stamp == NULL) return 0;
    if (ia > ib) {
        int t = ia;
        ia = ib;
        ib = t;
    }
    idx = (unsigned int)ia * (unsigned int)MAX_BODIES + (unsigned int)ib;
    if (engine->pair_stamp[idx] == engine->pair_stamp_frame) {
        return 0;
    }
    engine->pair_stamp[idx] = engine->pair_stamp_frame;
    return 1;
}

static void broadphase_mark_blocked_pairs(PhysicsEngine* engine) {
    int i;
    if (engine == NULL) return;
    for (i = 0; i < engine->constraint_count; i++) {
        const Constraint* c = &engine->constraints[i];
        int ia;
        int ib;
        if (!c->active || c->collide_connected) {
            continue;
        }
        ia = physics_internal_body_id_of(engine, c->body_a);
        ib = physics_internal_body_id_of(engine, c->body_b);
        if (ia < 0 || ib < 0 || ia == ib) continue;
        if (ia > ib) {
            int t = ia;
            ia = ib;
            ib = t;
        }
        (void)pair_stamp_mark_if_new(engine, ia, ib);
    }
}

static int broadphase_add_pair(PhysicsEngine* engine, int ia, int ib, const AABB* aabbs) {
    RigidBody* a;
    RigidBody* b;
    if (ia == ib) {
        return 0;
    }
    if (ia > ib) {
        int t = ia;
        ia = ib;
        ib = t;
    }
    if (!pair_stamp_mark_if_new(engine, ia, ib)) {
        return 0;
    }

    a = engine->bodies[ia];
    b = engine->bodies[ib];
    if (a == NULL || b == NULL || !a->active || !b->active || a->shape == NULL || b->shape == NULL) {
        return 0;
    }
    if (a->type == BODY_STATIC && b->type == BODY_STATIC) {
        return 0;
    }
    if (!aabb_overlaps(aabbs[ia], aabbs[ib])) {
        return 0;
    }
    if (engine->broadphase_pair_count >= MAX_BROADPHASE_PAIRS) {
        return 0;
    }

    (void)a;
    (void)b;
    engine->broadphase_pairs[engine->broadphase_pair_count].ia = ia;
    engine->broadphase_pairs[engine->broadphase_pair_count].ib = ib;
    engine->broadphase_pair_count++;
    return 1;
}

static int grid_hash(int x, int y) {
    unsigned int hx = (unsigned int)(x * 73856093);
    unsigned int hy = (unsigned int)(y * 19349663);
    return (int)((hx ^ hy) % GRID_BUCKETS);
}

static void broadphase_build_bruteforce(PhysicsEngine* engine, const AABB* aabbs) {
    int i;
    int j;
    for (i = 0; i < engine->body_count; i++) {
        for (j = i + 1; j < engine->body_count; j++) {
            broadphase_add_pair(engine, i, j, aabbs);
        }
    }
}

static void broadphase_build_grid(PhysicsEngine* engine, const AABB* aabbs, float cell_size) {
    int* bucket_heads;
    GridEntry* entries;
    int entry_count = 0;
    int i;
    bucket_heads = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * GRID_BUCKETS), 16);
    entries = (GridEntry*)physics_internal_scratch_alloc(engine, (int)(sizeof(GridEntry) * GRID_MAX_ENTRIES), 16);
    if (bucket_heads == NULL || entries == NULL) {
        broadphase_build_bruteforce(engine, aabbs);
        return;
    }
    memset(bucket_heads, 0xFF, sizeof(int) * (size_t)GRID_BUCKETS);

    for (i = 0; i < engine->body_count; i++) {
        RigidBody* body = engine->bodies[i];
        int min_cx;
        int max_cx;
        int min_cy;
        int max_cy;
        int cx;
        int cy;
        if (body == NULL || !body->active || body->shape == NULL) {
            continue;
        }

        min_cx = (int)floorf(aabbs[i].min.x / cell_size);
        max_cx = (int)floorf(aabbs[i].max.x / cell_size);
        min_cy = (int)floorf(aabbs[i].min.y / cell_size);
        max_cy = (int)floorf(aabbs[i].max.y / cell_size);

        if ((max_cx - min_cx) > GRID_MAX_SPAN || (max_cy - min_cy) > GRID_MAX_SPAN) {
            broadphase_build_bruteforce(engine, aabbs);
            return;
        }

        for (cy = min_cy; cy <= max_cy; cy++) {
            for (cx = min_cx; cx <= max_cx; cx++) {
                int h = grid_hash(cx, cy);
                if (entry_count >= GRID_MAX_ENTRIES) {
                    broadphase_build_bruteforce(engine, aabbs);
                    return;
                }

                entries[entry_count].body_index = i;
                entries[entry_count].next = bucket_heads[h];
                bucket_heads[h] = entry_count;
                entry_count++;
            }
        }
    }

    for (i = 0; i < GRID_BUCKETS; i++) {
        int ei;
        for (ei = bucket_heads[i]; ei != -1; ei = entries[ei].next) {
            int ej;
            for (ej = entries[ei].next; ej != -1; ej = entries[ej].next) {
                int ia = entries[ei].body_index;
                int ib = entries[ej].body_index;
                broadphase_add_pair(engine, ia, ib, aabbs);
            }
        }
    }
}

static void broadphase_build_sap(PhysicsEngine* engine, const AABB* aabbs) {
    SapProxy* proxies;
    int* active;
    int proxy_count = 0;
    int i;
    proxies = (SapProxy*)physics_internal_scratch_alloc(engine, (int)(sizeof(SapProxy) * (size_t)engine->body_count), 16);
    active = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)engine->body_count), 16);
    if (proxies == NULL || active == NULL) {
        broadphase_build_bruteforce(engine, aabbs);
        return;
    }

    for (i = 0; i < engine->body_count; i++) {
        RigidBody* b = engine->bodies[i];
        if (b == NULL || !b->active || b->shape == NULL) continue;
        proxies[proxy_count].body_index = i;
        proxies[proxy_count].min_x = aabbs[i].min.x;
        proxy_count++;
    }

    for (i = 1; i < proxy_count; i++) {
        SapProxy key = proxies[i];
        int j = i - 1;
        while (j >= 0 && (proxies[j].min_x > key.min_x ||
               (proxies[j].min_x == key.min_x && proxies[j].body_index > key.body_index))) {
            proxies[j + 1] = proxies[j];
            j--;
        }
        proxies[j + 1] = key;
    }

    {
        int active_count = 0;
        for (i = 0; i < proxy_count; i++) {
            int bi = proxies[i].body_index;
            int k = 0;
            int j;
            for (j = 0; j < active_count; j++) {
                int aj = active[j];
                if (aabbs[aj].max.x >= aabbs[bi].min.x) {
                    active[k++] = aj;
                }
            }
            active_count = k;
            for (j = 0; j < active_count; j++) {
                broadphase_add_pair(engine, active[j], bi, aabbs);
            }
            if (active_count < engine->body_count) {
                active[active_count++] = bi;
            }
        }
    }
}

static void broadphase_sort_pairs(PhysicsEngine* engine) {
    int i;
    if (engine == NULL) return;
    for (i = 1; i < engine->broadphase_pair_count; i++) {
        BroadphasePair key = engine->broadphase_pairs[i];
        int j = i - 1;
        while (j >= 0) {
            BroadphasePair cur = engine->broadphase_pairs[j];
            if (cur.ia < key.ia) break;
            if (cur.ia == key.ia && cur.ib <= key.ib) break;
            engine->broadphase_pairs[j + 1] = cur;
            j--;
        }
        engine->broadphase_pairs[j + 1] = key;
    }
}

int physics_internal_default_build_pairs(PhysicsEngine* engine, void* user) {
    const AABB* aabbs;
    (void)user;
    if (engine == NULL) {
        return 0;
    }
    if (engine->pair_stamp == NULL) {
        return 0;
    }
    engine->pair_stamp_frame++;
    if (engine->pair_stamp_frame == 0) {
        memset(engine->pair_stamp, 0, sizeof(unsigned short) * (size_t)(MAX_BODIES * MAX_BODIES));
        engine->pair_stamp_frame = 1;
    }
    aabbs = engine->aabbs;
    broadphase_mark_blocked_pairs(engine);
    if (engine->config.broadphase_type == PHYSICS_BROADPHASE_GRID && engine->config.broadphase_cell_size > 1e-6f) {
        broadphase_build_grid(engine, aabbs, engine->config.broadphase_cell_size);
    } else if (engine->config.broadphase_type == PHYSICS_BROADPHASE_SAP) {
        broadphase_build_sap(engine, aabbs);
    } else if (engine->config.broadphase_type == PHYSICS_BROADPHASE_BVH) {
        physics_internal_set_error(engine, PHYSICS_ERROR_INVALID_ARGUMENT,
                                   "broadphase_bvh_unsupported_fallback_bruteforce");
        broadphase_build_bruteforce(engine, aabbs);
    } else {
        broadphase_build_bruteforce(engine, aabbs);
    }
    broadphase_sort_pairs(engine);
    return engine->broadphase_pair_count;
}
