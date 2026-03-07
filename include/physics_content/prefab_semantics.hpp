#ifndef PREFAB_SEMANTICS_H
#define PREFAB_SEMANTICS_H

#include "physics_content/prefab_schema.hpp"

enum {
    PREFAB_SEMANTICS_MAX_RESULTS = PREFAB_SCHEMA_MAX_OVERRIDES
};

typedef enum {
    PREFAB_OVERRIDE_STATUS_APPLIED = 0,
    PREFAB_OVERRIDE_STATUS_CONFLICT = 1,
    PREFAB_OVERRIDE_STATUS_DANGLING = 2
} PrefabOverrideAnalysisStatus;

typedef struct {
    int layer_index;
    int override_id;
    int entity_id;
    char field[PREFAB_SCHEMA_MAX_FIELD];
    char value[PREFAB_SCHEMA_MAX_VALUE];
    PrefabOverrideAnalysisStatus status;
    int shadowed_by_override_id;
} PrefabOverrideAnalysisEntry;

typedef struct {
    int entry_count;
    int applied_count;
    int conflict_count;
    int dangling_count;
    int repairable_count;
    PrefabOverrideAnalysisEntry entries[PREFAB_SEMANTICS_MAX_RESULTS];
} PrefabOverrideAnalysisReport;

void prefab_override_analysis_init(PrefabOverrideAnalysisReport* report);
int prefab_semantics_analyze_variant_stack(const PrefabSchemaDocument* layers,
                                           int layer_count,
                                           PrefabOverrideAnalysisReport* out_report);
int prefab_semantics_repair_variant_overrides(const PrefabSchemaDocument* source,
                                              const PrefabOverrideAnalysisReport* analysis,
                                              PrefabSchemaDocument* out_repaired,
                                              int* out_removed_count);

#endif
