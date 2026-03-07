#include "physics_content/prefab_semantics.hpp"

#include <string.h>

namespace {

void copy_text(char* out, int out_cap, const char* src) {
    if (out == nullptr || out_cap <= 0) return;
    if (src == nullptr) {
        out[0] = '\0';
        return;
    }
    strncpy(out, src, (size_t)(out_cap - 1));
    out[out_cap - 1] = '\0';
}

int entity_exists_in_layers(const PrefabSchemaDocument* layers, int layer_count, int entity_id) {
    int layer_index;
    for (layer_index = 0; layer_index < layer_count; ++layer_index) {
        int entity_index;
        const PrefabSchemaDocument* layer = &layers[layer_index];
        for (entity_index = 0; entity_index < layer->entity_count; ++entity_index) {
            if (layer->entities[entity_index].entity_id == entity_id) return 1;
        }
    }
    return 0;
}

}  // namespace

void prefab_override_analysis_init(PrefabOverrideAnalysisReport* report) {
    if (report == nullptr) return;
    memset(report, 0, sizeof(*report));
}

int prefab_semantics_analyze_variant_stack(const PrefabSchemaDocument* layers,
                                           int layer_count,
                                           PrefabOverrideAnalysisReport* out_report) {
    int total_entries = 0;
    int layer_index;
    if (layers == nullptr || out_report == nullptr || layer_count <= 0) return 0;
    prefab_override_analysis_init(out_report);

    for (layer_index = 0; layer_index < layer_count; ++layer_index) {
        const PrefabSchemaDocument* layer = &layers[layer_index];
        int override_index;
        for (override_index = 0; override_index < layer->override_count; ++override_index) {
            const PrefabSchemaOverride* source = &layer->overrides[override_index];
            PrefabOverrideAnalysisEntry* entry;
            int next_layer;
            int shadowed_by = 0;

            if (total_entries >= PREFAB_SEMANTICS_MAX_RESULTS) return 0;
            entry = &out_report->entries[total_entries++];
            memset(entry, 0, sizeof(*entry));
            entry->layer_index = layer_index;
            entry->override_id = source->override_id;
            entry->entity_id = source->entity_id;
            copy_text(entry->field, PREFAB_SCHEMA_MAX_FIELD, source->field);
            copy_text(entry->value, PREFAB_SCHEMA_MAX_VALUE, source->value);

            if (!entity_exists_in_layers(layers, layer_count, source->entity_id)) {
                entry->status = PREFAB_OVERRIDE_STATUS_DANGLING;
                out_report->dangling_count++;
                out_report->repairable_count++;
                continue;
            }

            for (next_layer = layer_index + 1; next_layer < layer_count; ++next_layer) {
                int probe_index;
                const PrefabSchemaDocument* probe_layer = &layers[next_layer];
                for (probe_index = 0; probe_index < probe_layer->override_count; ++probe_index) {
                    const PrefabSchemaOverride* probe = &probe_layer->overrides[probe_index];
                    if (probe->entity_id == source->entity_id && strcmp(probe->field, source->field) == 0) {
                        shadowed_by = probe->override_id;
                        break;
                    }
                }
                if (shadowed_by != 0) break;
            }

            if (shadowed_by != 0) {
                entry->status = PREFAB_OVERRIDE_STATUS_CONFLICT;
                entry->shadowed_by_override_id = shadowed_by;
                out_report->conflict_count++;
            } else {
                entry->status = PREFAB_OVERRIDE_STATUS_APPLIED;
                out_report->applied_count++;
            }
        }
    }

    out_report->entry_count = total_entries;
    return 1;
}

int prefab_semantics_repair_variant_overrides(const PrefabSchemaDocument* source,
                                              const PrefabOverrideAnalysisReport* analysis,
                                              PrefabSchemaDocument* out_repaired,
                                              int* out_removed_count) {
    int override_index;
    int write_index = 0;
    int removed_count = 0;
    if (source == nullptr || analysis == nullptr || out_repaired == nullptr) return 0;
    *out_repaired = *source;
    out_repaired->override_count = 0;
    for (override_index = 0; override_index < source->override_count; ++override_index) {
        const PrefabSchemaOverride* current = &source->overrides[override_index];
        int keep = 1;
        int analysis_index;
        for (analysis_index = 0; analysis_index < analysis->entry_count; ++analysis_index) {
            const PrefabOverrideAnalysisEntry* entry = &analysis->entries[analysis_index];
            if (entry->override_id == current->override_id &&
                entry->entity_id == current->entity_id &&
                strcmp(entry->field, current->field) == 0 &&
                entry->status == PREFAB_OVERRIDE_STATUS_DANGLING) {
                keep = 0;
                break;
            }
        }
        if (keep) {
            out_repaired->overrides[write_index++] = *current;
        } else {
            removed_count++;
        }
    }
    out_repaired->override_count = write_index;
    if (out_removed_count != nullptr) *out_removed_count = removed_count;
    return 1;
}
