#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "physics.hpp"
#include "snapshot_repo.hpp"

static int parse_int_arg(const char* text, int fallback) {
    char* endptr = NULL;
    long value;
    if (text == NULL || text[0] == '\0') return fallback;
    value = strtol(text, &endptr, 10);
    if (endptr == text || (endptr != NULL && *endptr != '\0')) return fallback;
    return (int)value;
}

int main(int argc, char** argv) {
    const char* snapshot_path = NULL;
    int steps = 0;
    int i;
    PhysicsEngine* engine = physics_engine_create();
    if (engine == NULL) {
        printf("[FAIL] unable to create engine\n");
        return 1;
    }

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--snapshot") == 0 && i + 1 < argc) {
            snapshot_path = argv[++i];
        } else if (strcmp(argv[i], "--steps") == 0 && i + 1 < argc) {
            steps = parse_int_arg(argv[++i], steps);
        }
    }

    if (snapshot_path != NULL) {
        if (!snapshot_repo_load(engine, snapshot_path)) {
            printf("[FAIL] unable to load snapshot: %s\n", snapshot_path);
            physics_engine_free(engine);
            return 1;
        }
    }

    for (i = 0; i < steps; ++i) {
        physics_engine_step(engine);
    }

    printf("[INFO] runtime_cli bodies=%d constraints=%d contacts=%d steps=%d\n",
           physics_engine_get_body_count(engine),
           physics_engine_get_constraint_count(engine),
           physics_engine_get_contact_count(engine),
           steps);
    if (physics_engine_get_body_count(engine) > 0) {
        const RigidBody* body = physics_engine_get_body(engine, 0);
        if (body != NULL) {
            printf("[INFO] runtime_cli body0_pos=(%.4f,%.4f) vel=(%.4f,%.4f)\n",
                   body->position.x, body->position.y, body->velocity.x, body->velocity.y);
        }
    }
    printf("[PASS] runtime cli replay\n");
    physics_engine_free(engine);
    return 0;
}
