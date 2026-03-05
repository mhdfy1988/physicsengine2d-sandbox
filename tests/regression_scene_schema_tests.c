#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/scene_schema.h"

static int file_content_equals(const char* a_path, const char* b_path) {
    FILE* fa = fopen(a_path, "r");
    FILE* fb = fopen(b_path, "r");
    char la[512];
    char lb[512];
    int ok = 1;
    if (fa == NULL || fb == NULL) {
        if (fa != NULL) fclose(fa);
        if (fb != NULL) fclose(fb);
        return 0;
    }
    for (;;) {
        char* pa = fgets(la, sizeof(la), fa);
        char* pb = fgets(lb, sizeof(lb), fb);
        size_t na;
        size_t nb;
        if (pa == NULL && pb == NULL) break;
        if (pa == NULL || pb == NULL) {
            ok = 0;
            break;
        }
        na = strlen(la);
        while (na > 0 && (la[na - 1] == '\n' || la[na - 1] == '\r')) {
            la[na - 1] = '\0';
            na--;
        }
        nb = strlen(lb);
        while (nb > 0 && (lb[nb - 1] == '\n' || lb[nb - 1] == '\r')) {
            lb[nb - 1] = '\0';
            nb--;
        }
        if (strcmp(la, lb) != 0) {
            ok = 0;
            break;
        }
    }
    fclose(fa);
    fclose(fb);
    return ok;
}

int regression_test_scene_schema_roundtrip(void) {
    const char* file_a = "_tmp_scene_schema_roundtrip_a.txt";
    const char* file_b = "_tmp_scene_schema_roundtrip_b.txt";
    SceneSchemaDocument input;
    SceneSchemaDocument loaded;

    scene_schema_document_init(&input);
    strcpy(input.scene_guid, "scene://roundtrip_demo");
    input.entity_count = 2;

    input.entities[0].entity_id = 2;
    strcpy(input.entities[0].name, "crate");
    input.entities[0].shape = SCENE_SCHEMA_ENTITY_BOX;
    input.entities[0].mass = 2.0f;
    input.entities[0].position_x = 4.0f;
    input.entities[0].position_y = 1.0f;
    input.entities[0].velocity_x = -0.2f;
    input.entities[0].velocity_y = 0.0f;
    input.entities[0].angular_velocity = 0.1f;
    input.entities[0].restitution = 0.5f;
    input.entities[0].friction = 0.4f;
    input.entities[0].size = 3.0f;
    input.entities[0].damping = 0.98f;

    input.entities[1].entity_id = 1;
    strcpy(input.entities[1].name, "ball");
    input.entities[1].shape = SCENE_SCHEMA_ENTITY_CIRCLE;
    input.entities[1].mass = 1.0f;
    input.entities[1].position_x = 0.0f;
    input.entities[1].position_y = 0.0f;
    input.entities[1].velocity_x = 1.0f;
    input.entities[1].velocity_y = 0.0f;
    input.entities[1].angular_velocity = 0.0f;
    input.entities[1].restitution = 0.8f;
    input.entities[1].friction = 0.3f;
    input.entities[1].size = 2.0f;
    input.entities[1].damping = 0.99f;

    input.constraint_count = 1;
    input.constraints[0].constraint_id = 10;
    input.constraints[0].type = 0;
    input.constraints[0].entity_id_a = 1;
    input.constraints[0].entity_id_b = 2;
    input.constraints[0].rest_length = 3.5f;
    input.constraints[0].stiffness = 0.7f;
    input.constraints[0].damping = 0.1f;
    input.constraints[0].break_force = 100.0f;
    input.constraints[0].collide_connected = 1;

    if (!scene_schema_save_v1(&input, file_a)) {
        printf("[FAIL] failed to write scene schema roundtrip source\n");
        return 0;
    }
    if (!scene_schema_load_v1(file_a, &loaded)) {
        printf("[FAIL] failed to read scene schema roundtrip source\n");
        remove(file_a);
        return 0;
    }
    if (loaded.schema_version != SCENE_SCHEMA_VERSION_V1 || strcmp(loaded.scene_guid, "scene://roundtrip_demo") != 0) {
        printf("[FAIL] loaded schema header mismatch\n");
        remove(file_a);
        return 0;
    }
    if (loaded.entity_count != 2 || loaded.entities[0].entity_id != 1 || loaded.entities[1].entity_id != 2) {
        printf("[FAIL] loaded entity ordering mismatch\n");
        remove(file_a);
        return 0;
    }
    if (!scene_schema_save_v1(&loaded, file_b)) {
        printf("[FAIL] failed to write scene schema roundtrip target\n");
        remove(file_a);
        return 0;
    }
    if (!file_content_equals(file_a, file_b)) {
        printf("[FAIL] roundtrip text is not deterministic\n");
        remove(file_a);
        remove(file_b);
        return 0;
    }

    remove(file_a);
    remove(file_b);
    printf("[PASS] scene schema roundtrip determinism\n");
    return 1;
}

int regression_test_scene_schema_migration_golden(void) {
    const char* legacy_path = "tests/data/scene_snapshot_v0.txt";
    const char* golden_path = "tests/data/scene_schema_v1_golden.txt";
    const char* out_path = "_tmp_scene_schema_migrated.txt";
    SceneSchemaMigrationOptions options;
    SceneSchemaMigrationReport report;

    options.dry_run = 1;
    if (!scene_schema_migrate_legacy_snapshot_to_v1(legacy_path, NULL, &options, &report)) {
        printf("[FAIL] migration dry-run failed\n");
        return 0;
    }
    if (report.source_version != 0 || report.target_version != 1 || report.entity_count != 3 || report.constraint_count != 2 ||
        report.warning_count != 0 || report.failure_count != 0) {
        printf("[FAIL] migration dry-run report mismatch\n");
        return 0;
    }

    options.dry_run = 0;
    if (!scene_schema_migrate_legacy_snapshot_to_v1(legacy_path, out_path, &options, &report)) {
        printf("[FAIL] migration write failed\n");
        return 0;
    }
    if (!file_content_equals(out_path, golden_path)) {
        printf("[FAIL] migration golden output mismatch\n");
        remove(out_path);
        return 0;
    }

    remove(out_path);
    printf("[PASS] scene schema migration golden\n");
    return 1;
}
