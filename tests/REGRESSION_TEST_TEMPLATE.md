# Regression Test Template

Use this template when adding a new core regression test in `tests/regression_tests.c`.

## Pattern

```c
static int test_name_here(void) {
    PhysicsEngine* engine = physics_engine_create();
    if (engine == NULL) {
        printf("[FAIL] create engine failed\n");
        return 0;
    }

    /* Arrange */
    physics_engine_set_gravity(engine, vec2(0.0f, 9.8f));
    physics_engine_set_time_step(engine, 1.0f / 60.0f);

    /* Act */
    for (int i = 0; i < 120; i++) {
        physics_engine_step(engine);
    }

    /* Assert */
    if (/* failure condition */) {
        physics_engine_free(engine);
        printf("[FAIL] reason\n");
        return 0;
    }

    physics_engine_free(engine);
    printf("[PASS] test name\n");
    return 1;
}
```

## Rules

- Always free the engine on all return paths after creation.
- Use only public APIs from `include/physics.hpp` (no internal struct access).
- Prefer numeric tolerances (`EPS`) over exact float equality.
- Keep each test focused on one behavior/regression.

## Registration

Add the test to `main()` using:

```c
RUN_TEST_OR_FAIL(test_name_here);
```
