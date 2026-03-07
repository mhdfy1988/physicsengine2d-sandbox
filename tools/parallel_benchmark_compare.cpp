#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "../include/physics_core/physics.hpp"

static double now_ms(void) {
#ifdef _WIN32
    static LARGE_INTEGER freq{};
    LARGE_INTEGER now;
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart * 1000.0 / (double)freq.QuadPart;
#elif defined(CLOCK_MONOTONIC)
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
    }
    return (double)clock() * 1000.0 / (double)CLOCKS_PER_SEC;
#else
    return (double)clock() * 1000.0 / (double)CLOCKS_PER_SEC;
#endif
}

static int env_int_or_default(const char* name, int fallback) {
    const char* text = getenv(name);
    int value;
    if (text == NULL || text[0] == '\0') return fallback;
    value = atoi(text);
    return (value > 0) ? value : fallback;
}

static PhysicsEngine* build_engine(int threading_enabled, int worker_count, int body_count) {
    PhysicsEngine* engine = physics_engine_create();
    PhysicsExperimentalConfig ecfg;
    int i;
    if (engine == NULL) return NULL;
    physics_engine_set_gravity(engine, vec2(0.0f, 9.8f));
    physics_engine_set_time_step(engine, 1.0f / 120.0f);
    physics_engine_set_iterations(engine, 12);
    physics_engine_set_broadphase_use_grid(engine, 1);
    ecfg.ccd_enabled = 1;
    ecfg.sleep_enabled = 0;
    ecfg.threading_enabled = threading_enabled ? 1 : 0;
    ecfg.worker_count = worker_count;
    physics_engine_set_experimental_config(engine, &ecfg);
    for (i = 0; i < body_count; i++) {
        float x = (float)(i % 20) * 1.6f;
        float y = (float)(i / 20) * 1.6f;
        RigidBody* b = body_create(x, y, 1.0f, shape_create_circle(0.55f));
        if (b != NULL) physics_engine_add_body(engine, b);
    }
    {
        RigidBody* floor = body_create(16.0f, 32.0f, 1.0f, shape_create_box(72.0f, 2.0f));
        if (floor != NULL) {
            body_set_type(floor, BODY_STATIC);
            physics_engine_add_body(engine, floor);
        }
    }
    return engine;
}

static double run_average_step_ms(PhysicsEngine* engine, int warmup_steps, int measure_steps) {
    double t0;
    int i;
    if (engine == NULL || measure_steps <= 0) return 0.0;
    for (i = 0; i < warmup_steps; i++) {
        physics_engine_step(engine);
    }
    t0 = now_ms();
    for (i = 0; i < measure_steps; i++) {
        physics_engine_step(engine);
    }
    return (now_ms() - t0) / (double)measure_steps;
}

int main(void) {
    const int body_count = env_int_or_default("PHYSICS_PARALLEL_BENCH_BODIES", 960);
    const int warmup_steps = env_int_or_default("PHYSICS_PARALLEL_BENCH_WARMUP", 120);
    const int measure_steps = env_int_or_default("PHYSICS_PARALLEL_BENCH_STEPS", 480);
    PhysicsEngine* seq = build_engine(0, 1, body_count);
    PhysicsEngine* par2 = build_engine(1, 2, body_count);
    PhysicsEngine* par4 = build_engine(1, 4, body_count);
    double seq_ms;
    double par2_ms;
    double par4_ms;
    if (seq == NULL || par2 == NULL || par4 == NULL) {
        printf("[FAIL] parallel benchmark compare: engine create failed\n");
        physics_engine_free(seq);
        physics_engine_free(par2);
        physics_engine_free(par4);
        return 1;
    }
    seq_ms = run_average_step_ms(seq, warmup_steps, measure_steps);
    par2_ms = run_average_step_ms(par2, warmup_steps, measure_steps);
    par4_ms = run_average_step_ms(par4, warmup_steps, measure_steps);
    printf("[INFO] parallel benchmark bodies=%d warmup=%d steps=%d\n", body_count, warmup_steps, measure_steps);
    printf("[INFO] parallel benchmark sequential_avg_step_ms=%.4f\n", seq_ms);
    printf("[INFO] parallel benchmark threaded_avg_step_ms=%.4f workers=2 speedup=%.3fx\n",
           par2_ms,
           (par2_ms > 0.0) ? (seq_ms / par2_ms) : 0.0);
    printf("[INFO] parallel benchmark threaded_avg_step_ms=%.4f workers=4 speedup=%.3fx\n",
           par4_ms,
           (par4_ms > 0.0) ? (seq_ms / par4_ms) : 0.0);
    physics_engine_free(seq);
    physics_engine_free(par2);
    physics_engine_free(par4);
    printf("[PASS] parallel benchmark compare\n");
    return 0;
}
