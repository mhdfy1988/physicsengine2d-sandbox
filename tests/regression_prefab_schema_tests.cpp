#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "../include/prefab_schema.hpp"

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

static int prefab_doc_equals(const PrefabSchemaDocument* a, const PrefabSchemaDocument* b) {
    if (a == NULL || b == NULL) return 0;
    return memcmp(a, b, sizeof(*a)) == 0;
}

static void build_prefab_doc(PrefabSchemaDocument* doc,
                             const char* prefab_guid,
                             const char* base_guid,
                             int id_seed,
                             int entity_count,
                             int override_count) {
    int i;
    prefab_schema_document_init(doc);
    strcpy(doc->prefab_guid, prefab_guid);
    strcpy(doc->base_prefab_guid, base_guid);
    doc->is_variant = (base_guid != NULL && strcmp(base_guid, "none") != 0) ? 1 : 0;
    doc->entity_count = entity_count;
    doc->override_count = override_count;

    for (i = 0; i < entity_count; ++i) {
        PrefabSchemaEntity* entity = &doc->entities[i];
        entity->entity_id = id_seed + (entity_count - i);
        snprintf(entity->name, sizeof(entity->name), "entity_%d_%d", id_seed, i);
        entity->shape = (i % 2 == 0) ? PREFAB_SCHEMA_ENTITY_BOX : PREFAB_SCHEMA_ENTITY_CIRCLE;
        entity->mass = 1.0f + (float)i;
        entity->position_x = (float)(i * 2);
        entity->position_y = (float)(i * -3);
        entity->velocity_x = 0.25f * (float)i;
        entity->velocity_y = -0.5f * (float)i;
        entity->angular_velocity = 0.1f * (float)(i + 1);
        entity->restitution = 0.05f * (float)((i % 5) + 1);
        entity->friction = 0.1f * (float)((i % 4) + 1);
        entity->size = 0.5f + 0.25f * (float)i;
        entity->damping = 0.9f + 0.001f * (float)i;
    }

    for (i = 0; i < override_count; ++i) {
        PrefabSchemaOverride* override_entry = &doc->overrides[i];
        override_entry->override_id = id_seed + (override_count - i);
        override_entry->entity_id = doc->entities[i % entity_count].entity_id;
        snprintf(override_entry->field, sizeof(override_entry->field), "field_%d", i);
        snprintf(override_entry->value, sizeof(override_entry->value), "value_%d_%d", id_seed, i);
    }
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

int regression_test_prefab_schema_reentrant_io(void) {
    const char* source_a = "_tmp_prefab_schema_reentrant_source_a.txt";
    const char* source_b = "_tmp_prefab_schema_reentrant_source_b.txt";
    const char* save_a = "_tmp_prefab_schema_reentrant_save_a.txt";
    const char* save_b = "_tmp_prefab_schema_reentrant_save_b.txt";
    std::unique_ptr<PrefabSchemaDocument> input_a(new PrefabSchemaDocument());
    std::unique_ptr<PrefabSchemaDocument> input_b(new PrefabSchemaDocument());
    std::unique_ptr<PrefabSchemaDocument> expected_a(new PrefabSchemaDocument());
    std::unique_ptr<PrefabSchemaDocument> expected_b(new PrefabSchemaDocument());
    std::atomic<int> failures{0};
    std::atomic<int> ready{0};
    std::atomic<int> phase{0};

    if (!input_a || !input_b || !expected_a || !expected_b) {
        printf("[FAIL] failed to allocate prefab reentrancy fixtures\n");
        return 0;
    }

    build_prefab_doc(input_a.get(), "prefab://thread_a", "none", 100, 32, 16);
    build_prefab_doc(input_b.get(), "prefab://thread_b", "prefab://base_b", 200, 48, 24);

    if (!prefab_schema_save_v1(input_a.get(), source_a) || !prefab_schema_save_v1(input_b.get(), source_b)) {
        printf("[FAIL] failed to prepare prefab reentrancy fixtures\n");
        remove(source_a);
        remove(source_b);
        return 0;
    }
    if (!prefab_schema_load_v1(source_a, expected_a.get()) || !prefab_schema_load_v1(source_b, expected_b.get())) {
        printf("[FAIL] failed to load prefab reentrancy fixtures\n");
        remove(source_a);
        remove(source_b);
        return 0;
    }

    auto wait_for_phase = [&](int expected_phase) {
        ready.fetch_add(1, std::memory_order_relaxed);
        while (phase.load(std::memory_order_acquire) != expected_phase) {
            std::this_thread::yield();
        }
    };

    auto load_worker = [&](const char* path, const PrefabSchemaDocument* expected) {
        wait_for_phase(1);
        for (int i = 0; i < 128; ++i) {
            std::unique_ptr<PrefabSchemaDocument> loaded(new PrefabSchemaDocument());
            if (!loaded || !prefab_schema_load_v1(path, loaded.get()) || !prefab_doc_equals(loaded.get(), expected)) {
                failures.fetch_add(1, std::memory_order_relaxed);
                break;
            }
        }
    };

    auto save_worker = [&](const PrefabSchemaDocument* doc, const char* output_path, const PrefabSchemaDocument* expected) {
        wait_for_phase(1);
        for (int i = 0; i < 128; ++i) {
            std::unique_ptr<PrefabSchemaDocument> loaded(new PrefabSchemaDocument());
            if (!prefab_schema_save_v1(doc, output_path) ||
                !loaded ||
                !prefab_schema_load_v1(output_path, loaded.get()) ||
                !prefab_doc_equals(loaded.get(), expected)) {
                failures.fetch_add(1, std::memory_order_relaxed);
                break;
            }
        }
    };

    std::thread t1(load_worker, source_a, expected_a.get());
    std::thread t2(load_worker, source_b, expected_b.get());
    std::thread t3(save_worker, input_a.get(), save_a, expected_a.get());
    std::thread t4(save_worker, input_b.get(), save_b, expected_b.get());

    while (ready.load(std::memory_order_acquire) != 4) {
        std::this_thread::yield();
    }
    phase.store(1, std::memory_order_release);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    remove(source_a);
    remove(source_b);
    remove(save_a);
    remove(save_b);

    if (failures.load(std::memory_order_relaxed) != 0) {
        printf("[FAIL] prefab schema reentrant io mismatch\n");
        return 0;
    }

    printf("[PASS] prefab schema reentrant io\n");
    return 1;
}
