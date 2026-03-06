#include "runtime_snapshot_repo.hpp"

#include <stdio.h>

static float body_box_size_hint(const RigidBody* b) {
    int i;
    float max_x = 1.0f;
    float min_x = -1.0f;
    if (b == NULL || b->shape == NULL || b->shape->type != SHAPE_POLYGON) return 4.0f;
    for (i = 0; i < b->shape->data.polygon.vertex_count; i++) {
        float x = b->shape->data.polygon.vertices[i].x;
        if (x > max_x) max_x = x;
        if (x < min_x) min_x = x;
    }
    return max_f(2.0f, max_x - min_x);
}

int runtime_snapshot_repo_save(const PhysicsEngine* engine, const char* path) {
    FILE* fp;
    int i;
    int dyn_count = 0;
    int con_count = 0;
    if (engine == NULL || path == NULL) return 0;
    fp = fopen(path, "w");
    if (fp == NULL) return 0;
    for (i = 0; i < physics_engine_get_body_count(engine); i++) {
        RigidBody* b = physics_engine_get_body(engine, i);
        if (b != NULL && b->type == BODY_DYNAMIC) dyn_count++;
    }
    fprintf(fp, "BODIES %d\n", dyn_count);
    for (i = 0; i < physics_engine_get_body_count(engine); i++) {
        RigidBody* b = physics_engine_get_body(engine, i);
        if (b == NULL || b->type != BODY_DYNAMIC || b->shape == NULL) continue;
        if (b->shape->type == SHAPE_CIRCLE) {
            fprintf(fp, "b C %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f\n",
                    b->mass, b->position.x, b->position.y, b->velocity.x, b->velocity.y, b->angular_velocity,
                    b->shape->restitution, b->shape->friction, b->shape->data.circle.radius, b->damping);
        } else {
            fprintf(fp, "b B %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f\n",
                    b->mass, b->position.x, b->position.y, b->velocity.x, b->velocity.y, b->angular_velocity,
                    b->shape->restitution, b->shape->friction, body_box_size_hint(b), b->damping);
        }
    }
    for (i = 0; i < physics_engine_get_constraint_count(engine); i++) {
        const Constraint* c = physics_engine_get_constraint(engine, i);
        if (c->active && c->body_a != NULL && c->body_b != NULL && c->body_a->type == BODY_DYNAMIC && c->body_b->type == BODY_DYNAMIC) con_count++;
    }
    fprintf(fp, "CONSTRAINTS %d\n", con_count);
    {
        int j;
        for (i = 0; i < physics_engine_get_constraint_count(engine); i++) {
            const Constraint* c = physics_engine_get_constraint(engine, i);
            int ai = -1;
            int bi = -1;
            int k = 0;
            if (!c->active || c->body_a == NULL || c->body_b == NULL || c->body_a->type != BODY_DYNAMIC || c->body_b->type != BODY_DYNAMIC) continue;
            for (j = 0; j < physics_engine_get_body_count(engine); j++) {
                RigidBody* b = physics_engine_get_body(engine, j);
                if (b == NULL || b->type != BODY_DYNAMIC) continue;
                if (b == c->body_a) ai = k;
                if (b == c->body_b) bi = k;
                k++;
            }
            if (ai < 0 || bi < 0) continue;
            fprintf(fp, "c %d %d %d %.6f %.6f %.6f %.6f %d\n",
                    (int)c->type, ai, bi, c->rest_length, c->stiffness, c->damping, c->break_force, c->collide_connected);
        }
    }
    fclose(fp);
    return 1;
}

int runtime_snapshot_repo_load(PhysicsEngine* engine, const char* path) {
    FILE* fp;
    int body_n = 0;
    int con_n = 0;
    int i;
    RigidBody* saved[1024];
    int saved_n = 0;
    if (engine == NULL || path == NULL) return 0;
    fp = fopen(path, "r");
    if (fp == NULL) return 0;
    for (i = physics_engine_get_body_count(engine) - 1; i >= 0; i--) {
        RigidBody* b = physics_engine_get_body(engine, i);
        if (b != NULL && b->type == BODY_DYNAMIC) physics_engine_remove_body(engine, b);
    }
    physics_engine_clear_constraints(engine);
    if (fscanf(fp, "BODIES %d\n", &body_n) != 1) {
        fclose(fp);
        return 0;
    }
    for (i = 0; i < body_n && i < 1024; i++) {
        char kind = 0;
        float mass, px, py, vx, vy, ang, rest, fric, size, damp = 1.0f;
        Shape* sh;
        RigidBody* b;
        {
            char linebuf[256];
            int n = 0;
            if (fgets(linebuf, sizeof(linebuf), fp) == NULL) break;
            n = sscanf(linebuf, "b %c %f %f %f %f %f %f %f %f %f %f",
                       &kind, &mass, &px, &py, &vx, &vy, &ang, &rest, &fric, &size, &damp);
            if (n != 10 && n != 11) break;
            if (n == 10) damp = 1.0f;
        }
        if (kind == 'C') sh = shape_create_circle(size);
        else sh = shape_create_box(size, size);
        sh->restitution = rest;
        sh->friction = fric;
        b = body_create(px, py, mass, sh);
        b->velocity = vec2(vx, vy);
        b->angular_velocity = ang;
        if (damp < 0.0f) damp = 0.0f;
        if (damp > 1.0f) damp = 1.0f;
        b->damping = damp;
        physics_engine_add_body(engine, b);
        saved[saved_n++] = b;
    }
    if (fscanf(fp, "CONSTRAINTS %d\n", &con_n) == 1) {
        for (i = 0; i < con_n; i++) {
            int type, ai, bi, coll;
            float rl, st, dp, br;
            Constraint* c = NULL;
            if (fscanf(fp, "c %d %d %d %f %f %f %f %d\n", &type, &ai, &bi, &rl, &st, &dp, &br, &coll) != 8) break;
            if (ai < 0 || ai >= saved_n || bi < 0 || bi >= saved_n) continue;
            if (type == CONSTRAINT_DISTANCE) {
                c = physics_engine_add_distance_constraint(engine, saved[ai], saved[bi], saved[ai]->position, saved[bi]->position, st, coll);
                if (c != NULL) c->rest_length = rl;
            } else {
                c = physics_engine_add_spring_constraint(engine, saved[ai], saved[bi], saved[ai]->position, saved[bi]->position, rl, st, dp, coll);
            }
            if (c != NULL) c->break_force = br;
        }
    }
    fclose(fp);
    return 1;
}
