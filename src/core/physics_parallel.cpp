#include <stddef.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "physics_internal.hpp"

#ifdef _WIN32
typedef struct {
    int count;
    int chunk;
    volatile LONG next_begin;
    volatile LONG pending_workers;
    HANDLE done_event;
    PhysicsParallelForFn fn;
    void* user;
} ParallelDispatchCtx;

static void parallel_worker_run(ParallelDispatchCtx* ctx) {
    for (;;) {
        LONG begin = InterlockedExchangeAdd(&ctx->next_begin, (LONG)ctx->chunk);
        int end;
        if ((int)begin >= ctx->count) break;
        end = (int)begin + ctx->chunk;
        if (end > ctx->count) end = ctx->count;
        ctx->fn((int)begin, end, ctx->user);
    }
}

static DWORD WINAPI parallel_worker_proc(LPVOID param) {
    ParallelDispatchCtx* ctx = (ParallelDispatchCtx*)param;
    parallel_worker_run(ctx);
    if (ctx->done_event != NULL && InterlockedDecrement(&ctx->pending_workers) == 0) {
        SetEvent(ctx->done_event);
    }
    return 0;
}
#endif

extern "C" {

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
        int max_useful_workers;
        int suggested = (count + workers - 1) / workers;
        if (suggested > chunk) {
            chunk = suggested;
        }
        max_useful_workers = (count + chunk - 1) / chunk;
        if (max_useful_workers < 1) max_useful_workers = 1;
        if (workers > max_useful_workers) {
            workers = max_useful_workers;
        }
        if (workers < 1) workers = 1;
    }
    if (chunk < 1) chunk = 1;

#ifdef _WIN32
    if (workers > 1) {
        int worker_threads = workers - 1;
        ParallelDispatchCtx ctx;
        HANDLE done_event;
        ctx.count = count;
        ctx.chunk = chunk;
        ctx.next_begin = 0;
        ctx.pending_workers = 0;
        ctx.done_event = NULL;
        ctx.fn = fn;
        ctx.user = user;
        done_event = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (done_event == NULL) {
            for (i = 0; i < count; i += chunk) {
                int end = i + chunk;
                if (end > count) end = count;
                fn(i, end, user);
            }
            return;
        }
        ctx.done_event = done_event;
        for (i = 0; i < worker_threads; i++) {
            InterlockedIncrement(&ctx.pending_workers);
            if (!QueueUserWorkItem(parallel_worker_proc, &ctx, WT_EXECUTEDEFAULT)) {
                if (InterlockedDecrement(&ctx.pending_workers) == 0) {
                    SetEvent(done_event);
                }
            }
        }
        parallel_worker_run(&ctx);
        if (ctx.pending_workers > 0) {
            WaitForSingleObject(done_event, INFINITE);
        }
        CloseHandle(done_event);
        return;
    }
#endif

    for (i = 0; i < count; i += chunk) {
        int end = i + chunk;
        if (end > count) end = count;
        fn(i, end, user);
    }
}

}  // extern "C"
