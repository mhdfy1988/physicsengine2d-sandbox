#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/prefab_schema.h"

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

int regression_test_prefab_schema_roundtrip(void) {
    const char* file_a = "_tmp_prefab_schema_roundtrip_a.txt";
    const char* file_b = "_tmp_prefab_schema_roundtrip_b.txt";
    PrefabSchemaDocument input;
    PrefabSchemaDocument loaded;

    prefab_schema_document_init(&input);
    strcpy(input.prefab_guid, "prefab://enemy_base");
    strcpy(input.base_prefab_guid, "none");
    input.is_variant = 0;
    input.entity_count = 2;

    input.entities[0].entity_id = 5;
    strcpy(input.entities[0].name, "body");
    input.entities[0].shape = PREFAB_SCHEMA_ENTITY_BOX;
    input.entities[0].mass = 4.0f;
    input.entities[0].position_x = 2.0f;
    input.entities[0].position_y = 1.0f;
    input.entities[0].velocity_x = 0.0f;
    input.entities[0].velocity_y = 0.0f;
    input.entities[0].angular_velocity = 0.0f;
    input.entities[0].restitution = 0.1f;
    input.entities[0].friction = 0.6f;
    input.entities[0].size = 2.5f;
    input.entities[0].damping = 0.97f;

    input.entities[1].entity_id = 2;
    strcpy(input.entities[1].name, "sensor");
    input.entities[1].shape = PREFAB_SCHEMA_ENTITY_CIRCLE;
    input.entities[1].mass = 0.5f;
    input.entities[1].position_x = 0.0f;
    input.entities[1].position_y = 1.5f;
    input.entities[1].velocity_x = 0.0f;
    input.entities[1].velocity_y = 0.0f;
    input.entities[1].angular_velocity = 0.0f;
    input.entities[1].restitution = 0.8f;
    input.entities[1].friction = 0.2f;
    input.entities[1].size = 0.7f;
    input.entities[1].damping = 1.0f;

    input.override_count = 2;
    input.overrides[0].override_id = 9;
    input.overrides[0].entity_id = 5;
    strcpy(input.overrides[0].field, "restitution");
    strcpy(input.overrides[0].value, "0.25");
    input.overrides[1].override_id = 3;
    input.overrides[1].entity_id = 2;
    strcpy(input.overrides[1].field, "size");
    strcpy(input.overrides[1].value, "0.85");

    if (!prefab_schema_save_v1(&input, file_a)) {
        printf("[FAIL] failed to write prefab schema roundtrip source\n");
        return 0;
    }
    if (!prefab_schema_load_v1(file_a, &loaded)) {
        printf("[FAIL] failed to read prefab schema roundtrip source\n");
        remove(file_a);
        return 0;
    }
    if (loaded.schema_version != PREFAB_SCHEMA_VERSION_V1 ||
        strcmp(loaded.prefab_guid, "prefab://enemy_base") != 0 ||
        loaded.entity_count != 2 ||
        loaded.entities[0].entity_id != 2 ||
        loaded.entities[1].entity_id != 5 ||
        loaded.override_count != 2 ||
        loaded.overrides[0].override_id != 3 ||
        loaded.overrides[1].override_id != 9) {
        printf("[FAIL] loaded prefab schema mismatch\n");
        remove(file_a);
        return 0;
    }
    if (!prefab_schema_save_v1(&loaded, file_b)) {
        printf("[FAIL] failed to write prefab schema roundtrip target\n");
        remove(file_a);
        return 0;
    }
    if (!file_content_equals(file_a, file_b)) {
        printf("[FAIL] prefab schema roundtrip text is not deterministic\n");
        remove(file_a);
        remove(file_b);
        return 0;
    }

    remove(file_a);
    remove(file_b);
    printf("[PASS] prefab schema roundtrip determinism\n");
    return 1;
}

int regression_test_prefab_schema_variant_parse(void) {
    PrefabSchemaDocument doc;
    if (!prefab_schema_load_v1("tests/data/prefab_schema_v1_variant.txt", &doc)) {
        printf("[FAIL] failed to load prefab variant schema fixture\n");
        return 0;
    }
    if (doc.schema_version != 1 ||
        strcmp(doc.prefab_guid, "prefab://enemy_variant_fast") != 0 ||
        strcmp(doc.base_prefab_guid, "prefab://enemy_base") != 0 ||
        doc.is_variant != 1 ||
        doc.entity_count != 1 ||
        doc.override_count != 2) {
        printf("[FAIL] prefab variant header mismatch\n");
        return 0;
    }
    if (doc.overrides[0].override_id != 1 ||
        strcmp(doc.overrides[0].field, "mass") != 0 ||
        strcmp(doc.overrides[0].value, "3.5") != 0 ||
        doc.overrides[1].override_id != 2 ||
        strcmp(doc.overrides[1].field, "damping") != 0 ||
        strcmp(doc.overrides[1].value, "0.94") != 0) {
        printf("[FAIL] prefab variant override mismatch\n");
        return 0;
    }
    printf("[PASS] prefab schema variant parsing\n");
    return 1;
}
