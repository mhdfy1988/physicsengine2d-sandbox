#ifndef EDITOR_EXTENSION_STATE_SERVICE_H
#define EDITOR_EXTENSION_STATE_SERVICE_H

#include "editor_plugin.hpp"
#include "prefab_semantics.hpp"
#include "project_workspace.hpp"
#include "session_recovery.hpp"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    EDITOR_EXTENSION_STARTUP_MAX_DIAGNOSTICS = 32,
    EDITOR_EXTENSION_STARTUP_MAX_STAGE = 48,
    EDITOR_EXTENSION_STARTUP_MAX_MESSAGE = 160
};

typedef enum {
    EDITOR_EXTENSION_STARTUP_SEVERITY_INFO = 0,
    EDITOR_EXTENSION_STARTUP_SEVERITY_WARNING = 1,
    EDITOR_EXTENSION_STARTUP_SEVERITY_ERROR = 2
} EditorExtensionStartupSeverity;

typedef struct {
    char plugin_id[EDITOR_PLUGIN_MAX_ID];
    char manifest_path[PROJECT_WORKSPACE_MAX_PATH];
    EditorPluginV1 implementation;
} EditorExtensionBuiltinPlugin;

typedef struct {
    const char* root_path;
    const char* workspace_path;
    const char* project_path;
    const char* package_path;
    const char* settings_path;
    const char* session_recovery_path;
    const char* packages_dir;
    const char* prefab_base_path;
    const char* prefab_nested_path;
    const char* prefab_variant_path;
    const EditorExtensionBuiltinPlugin* builtin_plugins;
    int builtin_plugin_count;
} EditorExtensionStartupConfig;

typedef struct {
    EditorExtensionStartupSeverity severity;
    char stage[EDITOR_EXTENSION_STARTUP_MAX_STAGE];
    char path[PROJECT_WORKSPACE_MAX_PATH];
    char message[EDITOR_EXTENSION_STARTUP_MAX_MESSAGE];
} EditorExtensionStartupDiagnostic;

typedef struct {
    int ok;
    int degraded;
    ProjectWorkspaceBootstrapReport bootstrap_report;
    WorkspaceDocument workspace_doc;
    ProjectDocument project_doc;
    PackageDocument package_doc;
    ProjectSettingsDocument project_settings_doc;
    SessionRecoveryState session_recovery_state;
    int session_recovery_loaded;
    EditorPluginRegistry plugin_registry;
    PrefabOverrideAnalysisReport prefab_analysis;
    int prefab_analysis_valid;
    PrefabSchemaDocument prefab_repaired;
    int prefab_repair_removed_count;
    int diagnostic_count;
    EditorExtensionStartupDiagnostic diagnostics[EDITOR_EXTENSION_STARTUP_MAX_DIAGNOSTICS];
} EditorExtensionStartupResult;

void editor_extension_startup_config_init(EditorExtensionStartupConfig* config);
void editor_extension_startup_result_init(EditorExtensionStartupResult* result);
int editor_extension_startup_run(const EditorExtensionStartupConfig* config, EditorExtensionStartupResult* out_result);
void editor_extension_startup_shutdown(EditorExtensionStartupResult* state);

#ifdef __cplusplus
}
#endif

#endif
