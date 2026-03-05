#include <stddef.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "physics_internal.h"

#ifdef _WIN32
typedef struct {
    int count;
    int chunk;
    volatile LONG next_begin;
    PhysicsParallelForFn fn;
    void* user;
} ParallelDispatchCtx;

static DWORD WINAPI parallel_worker_proc(LPVOID param) {
    ParallelDispatchCtx* ctx = (ParallelDispatchCtx*)param;
    for (;;) {
        LONG begin = InterlockedExchangeAdd(&ctx->next_begin, (LONG)ctx->chunk);
        int end;
        if ((int)begin >= ctx->count) break;
        end = (int)begin + ctx->chunk;
        if (end > ctx->count) end = ctx->count;
        ctx->fn((int)begin, end, ctx->user);
    }
    return 0;
}
#endif

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

    chunk = grain;
    if (workers > 1) {
        int suggested = (count + workers - 1) / workers;
        if (suggested > chunk) {
            chunk = suggested;
        }
    }
    if (chunk < 1) chunk = 1;

#ifdef _WIN32
    if (workers > 1) {
        int worker_threads = workers - 1;
        HANDLE handles[63];
        ParallelDispatchCtx ctx;
        int created = 0;
        if (worker_threads > 63) worker_threads = 63;
        ctx.count = count;
        ctx.chunk = chunk;
        ctx.next_begin = 0;
        ctx.fn = fn;
        ctx.user = user;
        for (i = 0; i < worker_threads; i++) {
            handles[created] = CreateThread(NULL, 0, parallel_worker_proc, &ctx, 0, NULL);
            if (handles[created] != NULL) {
                created++;
            }
        }
        parallel_worker_proc(&ctx);
        if (created > 0) {
            WaitForMultipleObjects((DWORD)created, handles, TRUE, INFINITE);
            for (i = 0; i < created; i++) {
                CloseHandle(handles[i]);
            }
        }
        return;
    }
#endif

    for (i = 0; i < count; i += chunk) {
        int end = i + chunk;
        if (end > count) end = count;
        fn(i, end, user);
    }
}
