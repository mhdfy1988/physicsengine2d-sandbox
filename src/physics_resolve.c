#include <stddef.h>
#include "physics_internal.h"

void physics_internal_resolve_collisions(PhysicsEngine* engine, const PhysicsSolverContext* ctx) {
    PhysicsSolverWorldView view;
    PhysicsSolverContext local_ctx;
    if (engine == NULL) {
        return;
    }
    local_ctx.velocity_iterations = (ctx != NULL && ctx->velocity_iterations > 0) ? ctx->velocity_iterations : engine->config.iterations;
    local_ctx.position_iterations =
        (ctx != NULL && ctx->position_iterations > 0) ? ctx->position_iterations
                                                      : (engine->config.iterations + engine->config.max_position_iterations_bias);
    local_ctx.dt = (ctx != NULL && ctx->dt > 0.0f) ? ctx->dt : engine->config.time_step;

    view.contacts = engine->contacts;
    view.contact_count = engine->contact_count;
    view.constraints = engine->constraints;
    view.constraint_count = engine->constraint_count;
    view.engine_hint = engine;
    physics_internal_solve_world_view(&view, &local_ctx);
}
