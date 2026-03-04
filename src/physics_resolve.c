#include <stddef.h>
#include "physics_internal.h"

void physics_internal_resolve_collisions(PhysicsEngine* engine) {
    int position_iters;
    int i;
    int iter;
    if (engine == NULL) {
        return;
    }
    position_iters = engine->iterations + 3;

    for (i = 0; i < engine->constraint_count; i++) {
        constraint_warm_start(&engine->constraints[i]);
    }

    for (iter = 0; iter < engine->iterations; iter++) {
        for (i = 0; i < engine->contact_count; i++) {
            collision_resolve_velocity(&engine->contacts[i]);
        }
        for (i = 0; i < engine->constraint_count; i++) {
            constraint_solve_velocity(&engine->constraints[i], engine->time_step);
        }
    }

    for (iter = 0; iter < position_iters; iter++) {
        for (i = 0; i < engine->contact_count; i++) {
            CollisionManifold* m = &engine->contacts[i];
            if (m->bodyA == NULL || m->bodyB == NULL) {
                continue;
            }

            {
                CollisionInfo refreshed = {0};
                if (!collision_detect(m->bodyA, m->bodyB, &refreshed)) {
                    continue;
                }

                m->info = refreshed;
                collision_resolve_position(m);
            }
        }
        for (i = 0; i < engine->constraint_count; i++) {
            constraint_solve_position(&engine->constraints[i]);
        }
    }
}
