#include <stddef.h>
#include <string.h>
#include "physics_internal.hpp"

static int is_island_body(const RigidBody* body) {
    return (body != NULL && body->active && body->type != BODY_STATIC);
}

static int uf_find(int* parent, int x) {
    int root = x;
    while (parent[root] != root) {
        root = parent[root];
    }
    while (parent[x] != x) {
        int next = parent[x];
        parent[x] = root;
        x = next;
    }
    return root;
}

static void uf_union(int* parent, unsigned char* rank, int a, int b) {
    int ra;
    int rb;
    if (a < 0 || b < 0) return;
    ra = uf_find(parent, a);
    rb = uf_find(parent, b);
    if (ra == rb) return;
    if (rank[ra] < rank[rb]) {
        parent[ra] = rb;
    } else if (rank[ra] > rank[rb]) {
        parent[rb] = ra;
    } else {
        parent[rb] = ra;
        rank[ra]++;
    }
}

static void solve_monolithic(PhysicsEngine* engine, const PhysicsSolverContext* ctx) {
    PhysicsSolverWorldView view;
    view.contacts = engine->contacts;
    view.contact_count = engine->contact_count;
    view.contact_ids = NULL;
    view.constraints = engine->constraints;
    view.constraint_count = engine->constraint_count;
    view.constraint_ids = NULL;
    view.engine_hint = engine;
    physics_internal_solve_world_view(&view, ctx);
}

extern "C" {

void physics_internal_resolve_collisions(PhysicsEngine* engine, const PhysicsSolverContext* ctx) {
    PhysicsSolverContext local_ctx;
    int body_count;
    int contact_count;
    int constraint_count;
    int* parent;
    unsigned char* rank;
    int* body_island;
    int* root_island;
    int* contact_island;
    int* constraint_island;
    int* island_contact_counts;
    int* island_constraint_counts;
    int* island_contact_offsets;
    int* island_constraint_offsets;
    int* island_contact_cursor;
    int* island_constraint_cursor;
    int* island_contact_ids;
    int* island_constraint_ids;
    int solvable_contact_count;
    int solvable_constraint_count;
    int island_count;
    int i;
    if (engine == NULL) {
        return;
    }
    local_ctx.velocity_iterations = (ctx != NULL && ctx->velocity_iterations > 0) ? ctx->velocity_iterations : engine->config.iterations;
    local_ctx.position_iterations =
        (ctx != NULL && ctx->position_iterations > 0) ? ctx->position_iterations
                                                      : (engine->config.iterations + engine->config.max_position_iterations_bias);
    local_ctx.dt = (ctx != NULL && ctx->dt > 0.0f) ? ctx->dt : engine->config.time_step;
    body_count = engine->body_count;
    contact_count = engine->contact_count;
    constraint_count = engine->constraint_count;
    if (body_count <= 0 || (contact_count <= 0 && constraint_count <= 0)) {
        return;
    }

    parent = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)body_count), 16);
    rank = (unsigned char*)physics_internal_scratch_alloc(engine, (int)(sizeof(unsigned char) * (size_t)body_count), 16);
    body_island = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)body_count), 16);
    root_island = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)body_count), 16);
    contact_island = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)contact_count), 16);
    constraint_island = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)constraint_count), 16);
    island_contact_counts = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)body_count), 16);
    island_constraint_counts = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)body_count), 16);
    island_contact_offsets = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)body_count), 16);
    island_constraint_offsets = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)body_count), 16);
    island_contact_cursor = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)body_count), 16);
    island_constraint_cursor = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)body_count), 16);
    if (parent == NULL || rank == NULL || body_island == NULL || root_island == NULL ||
        contact_island == NULL || constraint_island == NULL || island_contact_counts == NULL ||
        island_constraint_counts == NULL || island_contact_offsets == NULL || island_constraint_offsets == NULL ||
        island_contact_cursor == NULL || island_constraint_cursor == NULL) {
        solve_monolithic(engine, &local_ctx);
        return;
    }

    memset(rank, 0, (size_t)body_count);
    for (i = 0; i < body_count; i++) {
        if (is_island_body(engine->bodies[i])) {
            parent[i] = i;
        } else {
            parent[i] = -1;
        }
        body_island[i] = -1;
        root_island[i] = -1;
        island_contact_counts[i] = 0;
        island_constraint_counts[i] = 0;
        island_contact_offsets[i] = 0;
        island_constraint_offsets[i] = 0;
        island_contact_cursor[i] = 0;
        island_constraint_cursor[i] = 0;
    }

    for (i = 0; i < contact_count; i++) {
        const CollisionManifold* m = &engine->contacts[i];
        int ia = physics_internal_body_id_of(engine, m->bodyA);
        int ib = physics_internal_body_id_of(engine, m->bodyB);
        contact_island[i] = -1;
        if (ia >= 0 && ia < body_count && ib >= 0 && ib < body_count &&
            parent[ia] >= 0 && parent[ib] >= 0) {
            uf_union(parent, rank, ia, ib);
        }
    }

    for (i = 0; i < constraint_count; i++) {
        const Constraint* c = &engine->constraints[i];
        int ia = physics_internal_body_id_of(engine, c->body_a);
        int ib = physics_internal_body_id_of(engine, c->body_b);
        constraint_island[i] = -1;
        if (!c->active) continue;
        if (ia >= 0 && ia < body_count && ib >= 0 && ib < body_count &&
            parent[ia] >= 0 && parent[ib] >= 0) {
            uf_union(parent, rank, ia, ib);
        }
    }

    island_count = 0;
    for (i = 0; i < body_count; i++) {
        if (parent[i] < 0) continue;
        {
            int root = uf_find(parent, i);
            if (root_island[root] < 0) {
                root_island[root] = island_count++;
            }
            body_island[i] = root_island[root];
        }
    }
    if (island_count <= 1) {
        solve_monolithic(engine, &local_ctx);
        return;
    }

    solvable_contact_count = 0;
    for (i = 0; i < contact_count; i++) {
        const CollisionManifold* m = &engine->contacts[i];
        int ia = physics_internal_body_id_of(engine, m->bodyA);
        int ib = physics_internal_body_id_of(engine, m->bodyB);
        int island = -1;
        if (ia >= 0 && ia < body_count && body_island[ia] >= 0) island = body_island[ia];
        if (ib >= 0 && ib < body_count && body_island[ib] >= 0) {
            if (island < 0) island = body_island[ib];
        }
        contact_island[i] = island;
        if (island >= 0) {
            island_contact_counts[island]++;
            solvable_contact_count++;
        }
    }

    solvable_constraint_count = 0;
    for (i = 0; i < constraint_count; i++) {
        const Constraint* c = &engine->constraints[i];
        int ia = physics_internal_body_id_of(engine, c->body_a);
        int ib = physics_internal_body_id_of(engine, c->body_b);
        int island = -1;
        if (!c->active) {
            constraint_island[i] = -1;
            continue;
        }
        if (ia >= 0 && ia < body_count && body_island[ia] >= 0) island = body_island[ia];
        if (ib >= 0 && ib < body_count && body_island[ib] >= 0) {
            if (island < 0) island = body_island[ib];
        }
        constraint_island[i] = island;
        if (island >= 0) {
            island_constraint_counts[island]++;
            solvable_constraint_count++;
        }
    }
    if (solvable_contact_count <= 0 && solvable_constraint_count <= 0) {
        return;
    }

    island_contact_ids = (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)solvable_contact_count), 16);
    island_constraint_ids =
        (int*)physics_internal_scratch_alloc(engine, (int)(sizeof(int) * (size_t)solvable_constraint_count), 16);
    if ((solvable_contact_count > 0 && island_contact_ids == NULL) ||
        (solvable_constraint_count > 0 && island_constraint_ids == NULL)) {
        solve_monolithic(engine, &local_ctx);
        return;
    }

    {
        int offset = 0;
        for (i = 0; i < island_count; i++) {
            island_contact_offsets[i] = offset;
            island_contact_cursor[i] = offset;
            offset += island_contact_counts[i];
        }
    }
    {
        int offset = 0;
        for (i = 0; i < island_count; i++) {
            island_constraint_offsets[i] = offset;
            island_constraint_cursor[i] = offset;
            offset += island_constraint_counts[i];
        }
    }

    for (i = 0; i < contact_count; i++) {
        int island = contact_island[i];
        if (island < 0) continue;
        island_contact_ids[island_contact_cursor[island]++] = i;
    }
    for (i = 0; i < constraint_count; i++) {
        int island = constraint_island[i];
        if (island < 0) continue;
        island_constraint_ids[island_constraint_cursor[island]++] = i;
    }

    for (i = 0; i < island_count; i++) {
        PhysicsSolverWorldView view;
        view.contacts = engine->contacts;
        view.contact_count = island_contact_counts[i];
        view.contact_ids = (view.contact_count > 0) ? &island_contact_ids[island_contact_offsets[i]] : NULL;
        view.constraints = engine->constraints;
        view.constraint_count = island_constraint_counts[i];
        view.constraint_ids = (view.constraint_count > 0) ? &island_constraint_ids[island_constraint_offsets[i]] : NULL;
        view.engine_hint = engine;
        if (view.contact_count > 0 || view.constraint_count > 0) {
            physics_internal_solve_world_view(&view, &local_ctx);
        }
    }
}

}  // extern "C"
