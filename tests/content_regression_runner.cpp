#include <stdio.h>

typedef int (*RegressionTestFn)(void);

typedef struct {
    const char* name;
    RegressionTestFn fn;
} RegressionTestEntry;

int regression_test_scene_schema_roundtrip(void);
int regression_test_scene_schema_migration_golden(void);
int regression_test_prefab_schema_roundtrip(void);
int regression_test_prefab_schema_variant_parse(void);
int regression_test_prefab_schema_reentrant_io(void);
int regression_test_project_workspace_roundtrip(void);
int regression_test_project_workspace_bootstrap(void);
int regression_test_prefab_semantics_conflict_resolution(void);
int regression_test_prefab_semantics_dangling_override_repair(void);
int regression_test_editor_plugin_registry_lifecycle(void);
int regression_test_editor_plugin_registry_duplicate_rejected(void);
int regression_test_session_recovery_roundtrip(void);
int regression_test_session_recovery_pending_restore(void);
int regression_test_diagnostic_bundle_export(void);
int regression_test_asset_meta_roundtrip(void);
int regression_test_asset_dependency_graph(void);
int regression_test_asset_importer_smoke(void);
int regression_test_asset_importer_parameterized_settings(void);
int regression_test_asset_invalidation_propagation(void);
int regression_test_asset_pipeline_incremental_reimport(void);
int regression_test_asset_pipeline_meta_parameter_deserialize(void);
int regression_test_asset_pipeline_missing_source_failure(void);
int regression_test_asset_watch_debounce_batch(void);
int regression_test_asset_hot_reload_service_pipeline_bridge(void);
int regression_test_asset_fs_poll_source_bridge(void);
int regression_test_asset_hot_reload_missing_source_failure(void);
int regression_test_asset_hot_reload_failure_keeps_previous_artifact(void);
int regression_test_asset_fs_poll_rewatch_does_not_swallow_change(void);

int main(void) {
    int passed = 0;
    int total = 0;
    int i;
    static const RegressionTestEntry kTests[] = {
        {"regression_test_scene_schema_roundtrip", regression_test_scene_schema_roundtrip},
        {"regression_test_scene_schema_migration_golden", regression_test_scene_schema_migration_golden},
        {"regression_test_prefab_schema_roundtrip", regression_test_prefab_schema_roundtrip},
        {"regression_test_prefab_schema_variant_parse", regression_test_prefab_schema_variant_parse},
        {"regression_test_prefab_schema_reentrant_io", regression_test_prefab_schema_reentrant_io},
        {"regression_test_project_workspace_roundtrip", regression_test_project_workspace_roundtrip},
        {"regression_test_project_workspace_bootstrap", regression_test_project_workspace_bootstrap},
        {"regression_test_prefab_semantics_conflict_resolution", regression_test_prefab_semantics_conflict_resolution},
        {"regression_test_prefab_semantics_dangling_override_repair", regression_test_prefab_semantics_dangling_override_repair},
        {"regression_test_editor_plugin_registry_lifecycle", regression_test_editor_plugin_registry_lifecycle},
        {"regression_test_editor_plugin_registry_duplicate_rejected", regression_test_editor_plugin_registry_duplicate_rejected},
        {"regression_test_session_recovery_roundtrip", regression_test_session_recovery_roundtrip},
        {"regression_test_session_recovery_pending_restore", regression_test_session_recovery_pending_restore},
        {"regression_test_diagnostic_bundle_export", regression_test_diagnostic_bundle_export},
        {"regression_test_asset_meta_roundtrip", regression_test_asset_meta_roundtrip},
        {"regression_test_asset_dependency_graph", regression_test_asset_dependency_graph},
        {"regression_test_asset_importer_smoke", regression_test_asset_importer_smoke},
        {"regression_test_asset_importer_parameterized_settings", regression_test_asset_importer_parameterized_settings},
        {"regression_test_asset_invalidation_propagation", regression_test_asset_invalidation_propagation},
        {"regression_test_asset_pipeline_incremental_reimport", regression_test_asset_pipeline_incremental_reimport},
        {"regression_test_asset_pipeline_meta_parameter_deserialize", regression_test_asset_pipeline_meta_parameter_deserialize},
        {"regression_test_asset_pipeline_missing_source_failure", regression_test_asset_pipeline_missing_source_failure},
        {"regression_test_asset_watch_debounce_batch", regression_test_asset_watch_debounce_batch},
        {"regression_test_asset_hot_reload_service_pipeline_bridge", regression_test_asset_hot_reload_service_pipeline_bridge},
        {"regression_test_asset_fs_poll_source_bridge", regression_test_asset_fs_poll_source_bridge},
        {"regression_test_asset_hot_reload_missing_source_failure", regression_test_asset_hot_reload_missing_source_failure},
        {"regression_test_asset_hot_reload_failure_keeps_previous_artifact", regression_test_asset_hot_reload_failure_keeps_previous_artifact},
        {"regression_test_asset_fs_poll_rewatch_does_not_swallow_change", regression_test_asset_fs_poll_rewatch_does_not_swallow_change},
    };

    for (i = 0; i < (int)(sizeof(kTests) / sizeof(kTests[0])); ++i) {
        total++;
        if (!kTests[i].fn()) {
            printf("\nResult: FAIL (%d/%d) at %s\n", passed, total, kTests[i].name);
            return 1;
        }
        passed++;
    }

    printf("\nResult: PASS (%d/%d)\n", passed, total);
    return 0;
}
