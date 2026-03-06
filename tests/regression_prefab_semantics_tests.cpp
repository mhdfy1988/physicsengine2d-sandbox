#include <stdio.h>
#include <string.h>

#include "../include/prefab_schema.hpp"
#include "../include/prefab_semantics.hpp"

static void init_entity(PrefabSchemaEntity* entity, int entity_id, const char* name) {
    memset(entity, 0, sizeof(*entity));
    entity->entity_id = entity_id;
    strcpy(entity->name, name);
    entity->shape = PREFAB_SCHEMA_ENTITY_BOX;
    entity->damping = 1.0f;
}

static void init_override(PrefabSchemaOverride* item, int override_id, int entity_id, const char* field, const char* value) {
    memset(item, 0, sizeof(*item));
    item->override_id = override_id;
    item->entity_id = entity_id;
    strcpy(item->field, field);
    strcpy(item->value, value);
}

int regression_test_prefab_semantics_conflict_resolution(void) {
    PrefabSchemaDocument layers[3];
    PrefabOverrideAnalysisReport report;

    prefab_schema_document_init(&layers[0]);
    prefab_schema_document_init(&layers[1]);
    prefab_schema_document_init(&layers[2]);

    layers[0].entity_count = 1;
    init_entity(&layers[0].entities[0], 1, "body");
    layers[1].override_count = 1;
    init_override(&layers[1].overrides[0], 11, 1, "mass", "2.0");
    layers[2].override_count = 2;
    init_override(&layers[2].overrides[0], 21, 1, "mass", "3.0");
    init_override(&layers[2].overrides[1], 22, 1, "damping", "0.95");

    if (!prefab_semantics_analyze_variant_stack(layers, 3, &report)) {
        printf("[FAIL] prefab semantics analysis failed\n");
        return 0;
    }
    if (report.entry_count != 3 || report.applied_count != 2 || report.conflict_count != 1 || report.dangling_count != 0) {
        printf("[FAIL] prefab semantics aggregate mismatch\n");
        return 0;
    }
    if (report.entries[0].status != PREFAB_OVERRIDE_STATUS_CONFLICT ||
        report.entries[0].shadowed_by_override_id != 21 ||
        report.entries[1].status != PREFAB_OVERRIDE_STATUS_APPLIED ||
        report.entries[2].status != PREFAB_OVERRIDE_STATUS_APPLIED) {
        printf("[FAIL] prefab semantics nearest-layer resolution mismatch\n");
        return 0;
    }

    printf("[PASS] prefab semantics nearest-layer conflict resolution\n");
    return 1;
}

int regression_test_prefab_semantics_dangling_override_repair(void) {
    PrefabSchemaDocument layers[2];
    PrefabOverrideAnalysisReport report;
    PrefabSchemaDocument repaired;
    int removed_count = 0;

    prefab_schema_document_init(&layers[0]);
    prefab_schema_document_init(&layers[1]);
    layers[0].entity_count = 1;
    init_entity(&layers[0].entities[0], 1, "body");
    layers[1].override_count = 2;
    init_override(&layers[1].overrides[0], 31, 99, "mass", "4.0");
    init_override(&layers[1].overrides[1], 32, 1, "friction", "0.5");

    if (!prefab_semantics_analyze_variant_stack(layers, 2, &report)) {
        printf("[FAIL] prefab semantics dangling analysis failed\n");
        return 0;
    }
    if (report.dangling_count != 1 || report.repairable_count != 1) {
        printf("[FAIL] prefab semantics dangling analysis mismatch\n");
        return 0;
    }
    if (!prefab_semantics_repair_variant_overrides(&layers[1], &report, &repaired, &removed_count)) {
        printf("[FAIL] prefab semantics repair failed\n");
        return 0;
    }
    if (removed_count != 1 || repaired.override_count != 1 || repaired.overrides[0].override_id != 32) {
        printf("[FAIL] prefab semantics repair result mismatch\n");
        return 0;
    }

    printf("[PASS] prefab semantics dangling override repair\n");
    return 1;
}
