#include <stddef.h>
#include "physics_internal.h"

void physics_internal_parallel_for(PhysicsEngine* engine, int count, int grain, PhysicsParallelForFn fn, void* user) {
    int i;
    int chunk;
    int workers;
    if (fn == NULL || count <= 0) return;
    if (grain <= 0) {
        grain = count;
    }
    if (engine != NULL && engine->job_system_installed && engine->job_system.parallel_for != NULL) {
        engine->job_system.parallel_for(count, grain, (PhysicsJobRangeFn)fn, user, engine->job_system.user);
        return;
    }
    workers = 1;
    if (engine != NULL && engine->experimental.threading_enabled) {
        workers = engine->experimental.worker_count;
        if (workers < 1) workers = 1;
    }

    /* Serial backend: keep chunked dispatch semantics for deterministic future thread backend swap. */
    chunk = grain;
    if (workers > 1) {
        int suggested = (count + workers - 1) / workers;
        if (suggested > chunk) {
            chunk = suggested;
        }
    }
    if (chunk < 1) chunk = 1;

    for (i = 0; i < count; i += chunk) {
        int end = i + chunk;
        if (end > count) end = count;
        fn(i, end, user);
    }
}
