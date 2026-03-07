#include "editor_extension_state_service.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <filesystem>

#include "../infrastructure/app_path.hpp"
#include "physics_content/prefab_schema.hpp"

static const char* EDITOR_SAMPLE_PROJECT_ROOT = "samples/physics_sandbox_project";

static PrefabSchemaDocument* startup_alloc_prefab_documents(int count) {
    if (count <= 0) return NULL;
    return (PrefabSchemaDocument*)malloc((size_t)count * sizeof(PrefabSchemaDocument));
}

static void startup_copy_text(char* out, int out_cap, const char* src) {
    if (out == NULL || out_cap <= 0) return;
    out[0] = '\0';
    if (src == NULL) return;
    strncpy(out, src, (size_t)(out_cap - 1));
    out[out_cap - 1] = '\0';
}

static const char* startup_resolve_project_path(const char* relative_path) {
    static char resolved[12][PROJECT_WORKSPACE_MAX_PATH];
    static int next_slot = 0;
    char* slot;
    if (relative_path == NULL) return NULL;
    slot = resolved[next_slot];
    next_slot = (next_slot + 1) % 12;
    if (sandbox_app_path_find_from_exe_ancestors_utf8(relative_path, 4, slot, PROJECT_WORKSPACE_MAX_PATH)) return slot;
    startup_copy_text(slot, PROJECT_WORKSPACE_MAX_PATH, relative_path);
    return slot;
}

static void startup_push_diagnostic(EditorExtensionStartupResult* result,
                                    EditorExtensionStartupSeverity severity,
                                    const char* stage,
                                    const char* path,
                                    const char* message) {
    EditorExtensionStartupDiagnostic* diagnostic;
    if (result == NULL || result->diagnostic_count >= EDITOR_EXTENSION_STARTUP_MAX_DIAGNOSTICS) return;
    diagnostic = &result->diagnostics[result->diagnostic_count++];
    diagnostic->severity = severity;
    startup_copy_text(diagnostic->stage, EDITOR_EXTENSION_STARTUP_MAX_STAGE, stage);
    startup_copy_text(diagnostic->path, PROJECT_WORKSPACE_MAX_PATH, path);
    startup_copy_text(diagnostic->message, EDITOR_EXTENSION_STARTUP_MAX_MESSAGE, message);
    if (severity != EDITOR_EXTENSION_STARTUP_SEVERITY_INFO) result->degraded = 1;
}

static void startup_init_default_documents(const EditorExtensionStartupConfig* config,
                                           WorkspaceDocument* workspace,
                                           ProjectDocument* project,
                                           PackageDocument* package,
                                           ProjectSettingsDocument* settings) {
    int i;
    workspace_document_init(workspace);
    project_document_init(project);
    package_document_init(package);
    project_settings_document_init(settings);

    workspace->project_count = 1;
    startup_copy_text(workspace->active_project_guid, PROJECT_WORKSPACE_MAX_GUID, project->project_guid);
    startup_copy_text(workspace->projects[0].project_guid, PROJECT_WORKSPACE_MAX_GUID, project->project_guid);
    startup_copy_text(workspace->projects[0].project_name, PROJECT_WORKSPACE_MAX_NAME, project->project_name);
    startup_copy_text(workspace->projects[0].project_path, PROJECT_WORKSPACE_MAX_PATH, config->project_path);

    project->package_count = 1;
    startup_copy_text(project->packages[0].package_id, PROJECT_WORKSPACE_MAX_NAME, package->package_id);
    startup_copy_text(project->packages[0].package_path, PROJECT_WORKSPACE_MAX_PATH, config->package_path);
    project->packages[0].enabled = 1;

    package->plugin_count = config->builtin_plugin_count;
    if (package->plugin_count > PROJECT_WORKSPACE_MAX_PLUGINS) package->plugin_count = PROJECT_WORKSPACE_MAX_PLUGINS;
    for (i = 0; i < package->plugin_count; ++i) {
        startup_copy_text(package->plugins[i].plugin_id, PROJECT_WORKSPACE_MAX_NAME, config->builtin_plugins[i].plugin_id);
        startup_copy_text(package->plugins[i].plugin_manifest, PROJECT_WORKSPACE_MAX_PATH, config->builtin_plugins[i].manifest_path);
    }
}

static void startup_load_documents(const EditorExtensionStartupConfig* config, EditorExtensionStartupResult* result) {
    if (!workspace_document_load_v1(config->workspace_path, &result->workspace_doc)) {
        startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_ERROR, "load_workspace", config->workspace_path,
                                "failed to load workspace document");
    }
    if (!project_document_load_v1(config->project_path, &result->project_doc)) {
        startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_ERROR, "load_project", config->project_path,
                                "failed to load project document");
    }
    if (!package_document_load_v1(config->package_path, &result->package_doc)) {
        startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_ERROR, "load_package", config->package_path,
                                "failed to load package document");
    }
    if (!project_settings_document_load_v1(config->settings_path, &result->project_settings_doc)) {
        startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_ERROR, "load_settings", config->settings_path,
                                "failed to load project settings");
    }
}

static void startup_attach_builtin_plugins(const EditorExtensionStartupConfig* config, EditorExtensionStartupResult* result) {
    int i;
    for (i = 0; i < config->builtin_plugin_count; ++i) {
        const EditorExtensionBuiltinPlugin* builtin = &config->builtin_plugins[i];
        if (!editor_plugin_registry_attach(&result->plugin_registry, builtin->plugin_id, &builtin->implementation)) {
            if (result->plugin_registry.plugin_count < EDITOR_PLUGIN_MAX_COUNT) {
                EditorPluginRecord* record = &result->plugin_registry.plugins[result->plugin_registry.plugin_count++];
                memset(record, 0, sizeof(*record));
                if (!editor_plugin_manifest_load_v1(builtin->manifest_path, &record->manifest)) {
                    record->manifest.api_version = EDITOR_PLUGIN_API_VERSION_1;
                    startup_copy_text(record->manifest.plugin_id, EDITOR_PLUGIN_MAX_ID, builtin->plugin_id);
                }
                startup_copy_text(record->manifest_path, EDITOR_PLUGIN_MAX_PATH, builtin->manifest_path);
                record->implementation = builtin->implementation;
                record->state = EDITOR_PLUGIN_STATE_DISCOVERED;
            } else {
                startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_WARNING, "attach_plugin",
                                        builtin->plugin_id, "failed to attach builtin plugin");
            }
        }
    }
}

static void startup_load_session_recovery(const EditorExtensionStartupConfig* config, EditorExtensionStartupResult* result) {
    if (config->session_recovery_path == NULL || config->session_recovery_path[0] == '\0') return;
    session_recovery_state_init(&result->session_recovery_state);
    if (!std::filesystem::exists(config->session_recovery_path)) return;
    if (!session_recovery_state_load_v1(config->session_recovery_path, &result->session_recovery_state)) {
        startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_WARNING, "load_session_recovery",
                                config->session_recovery_path, "failed to load session recovery state");
        return;
    }
    result->session_recovery_loaded = 1;
}

static void startup_collect_plugin_diagnostics(EditorExtensionStartupResult* result) {
    int i;
    for (i = 0; i < result->plugin_registry.plugin_count; ++i) {
        const EditorPluginRecord* record = &result->plugin_registry.plugins[i];
        if (record->state == EDITOR_PLUGIN_STATE_FAILED) {
            startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_WARNING, "init_plugin",
                                    record->manifest.plugin_id,
                                    record->disable_reason[0] ? record->disable_reason : "plugin init failed");
        } else if (record->state == EDITOR_PLUGIN_STATE_DISABLED) {
            startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_WARNING, "bind_plugin",
                                    record->manifest.plugin_id,
                                    record->disable_reason[0] ? record->disable_reason : "plugin disabled");
        }
    }
}

static void startup_run_prefab_analysis(const EditorExtensionStartupConfig* config, EditorExtensionStartupResult* result) {
    PrefabSchemaDocument* layers = startup_alloc_prefab_documents(3);
    if (layers == NULL) {
        startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_ERROR, "prefab_analysis", config->prefab_variant_path,
                                "failed to allocate prefab analysis scratch");
        return;
    }
    if (!prefab_schema_load_v1(config->prefab_base_path, &layers[0])) {
        startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_WARNING, "load_prefab", config->prefab_base_path,
                                "failed to load base prefab");
        free(layers);
        return;
    }
    if (!prefab_schema_load_v1(config->prefab_nested_path, &layers[1])) {
        startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_WARNING, "load_prefab", config->prefab_nested_path,
                                "failed to load nested prefab");
        free(layers);
        return;
    }
    if (!prefab_schema_load_v1(config->prefab_variant_path, &layers[2])) {
        startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_WARNING, "load_prefab", config->prefab_variant_path,
                                "failed to load variant prefab");
        free(layers);
        return;
    }
    if (!prefab_semantics_analyze_variant_stack(layers, 3, &result->prefab_analysis)) {
        startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_WARNING, "prefab_analysis", config->prefab_variant_path,
                                "failed to analyze prefab variant stack");
        free(layers);
        return;
    }
    result->prefab_analysis_valid = 1;
    if (!prefab_semantics_repair_variant_overrides(&layers[2], &result->prefab_analysis,
                                                   &result->prefab_repaired, &result->prefab_repair_removed_count)) {
        startup_push_diagnostic(result, EDITOR_EXTENSION_STARTUP_SEVERITY_WARNING, "prefab_repair", config->prefab_variant_path,
                                "failed to repair prefab overrides");
    }
    free(layers);
}

void editor_extension_startup_config_init(EditorExtensionStartupConfig* config) {
    if (config == NULL) return;
    memset(config, 0, sizeof(*config));
    config->root_path = startup_resolve_project_path(EDITOR_SAMPLE_PROJECT_ROOT);
    config->project_root = startup_resolve_project_path(EDITOR_SAMPLE_PROJECT_ROOT);
    config->workspace_path = startup_resolve_project_path("samples/physics_sandbox_project/ProjectSettings/workspace.physicsworkspace");
    config->project_path = startup_resolve_project_path("samples/physics_sandbox_project/ProjectSettings/project.physicsproject");
    config->package_path = startup_resolve_project_path("samples/physics_sandbox_project/Packages/core_tools.physicspackage");
    config->settings_path = startup_resolve_project_path("samples/physics_sandbox_project/ProjectSettings/editor.physicssettings");
    config->session_recovery_path = startup_resolve_project_path("samples/physics_sandbox_project/ProjectSettings/editor_session.physicssession");
    config->packages_dir = startup_resolve_project_path("samples/physics_sandbox_project/Packages");
    config->prefab_base_path = startup_resolve_project_path("samples/physics_sandbox_project/Prefabs/phase_g_base.prefab");
    config->prefab_nested_path = startup_resolve_project_path("samples/physics_sandbox_project/Prefabs/phase_g_nested.prefab");
    config->prefab_variant_path = startup_resolve_project_path("samples/physics_sandbox_project/Prefabs/phase_g_variant.prefab");
}

void editor_extension_startup_result_init(EditorExtensionStartupResult* result) {
    if (result == NULL) return;
    memset(result, 0, sizeof(*result));
    workspace_document_init(&result->workspace_doc);
    project_document_init(&result->project_doc);
    package_document_init(&result->package_doc);
    project_settings_document_init(&result->project_settings_doc);
    session_recovery_state_init(&result->session_recovery_state);
    editor_plugin_registry_init(&result->plugin_registry);
    prefab_override_analysis_init(&result->prefab_analysis);
}

int editor_extension_startup_run(const EditorExtensionStartupConfig* config, EditorExtensionStartupResult* out_result) {
    WorkspaceDocument bootstrap_workspace;
    ProjectDocument bootstrap_project;
    PackageDocument bootstrap_package;
    ProjectSettingsDocument bootstrap_settings;
    if (config == NULL || out_result == NULL) return 0;

    editor_extension_startup_result_init(out_result);
    startup_init_default_documents(config, &bootstrap_workspace, &bootstrap_project, &bootstrap_package, &bootstrap_settings);

    if (!project_workspace_bootstrap_v1(config->root_path, &bootstrap_workspace, &bootstrap_project, &bootstrap_package,
                                        &bootstrap_settings, &out_result->bootstrap_report)) {
        startup_push_diagnostic(out_result, EDITOR_EXTENSION_STARTUP_SEVERITY_ERROR, "bootstrap", config->root_path,
                                "failed to bootstrap workspace layout");
    }

    startup_load_documents(config, out_result);
    startup_load_session_recovery(config, out_result);

    if (!editor_plugin_registry_scan_v1(config->packages_dir, &out_result->plugin_registry)) {
        startup_push_diagnostic(out_result, EDITOR_EXTENSION_STARTUP_SEVERITY_WARNING, "scan_plugins", config->packages_dir,
                                "failed to scan plugin manifests");
    }
    startup_attach_builtin_plugins(config, out_result);
    editor_plugin_registry_initialize_all(&out_result->plugin_registry);
    startup_collect_plugin_diagnostics(out_result);

    startup_run_prefab_analysis(config, out_result);

    out_result->ok = 1;
    for (int i = 0; i < out_result->diagnostic_count; ++i) {
        if (out_result->diagnostics[i].severity == EDITOR_EXTENSION_STARTUP_SEVERITY_ERROR) {
            out_result->ok = 0;
            break;
        }
    }
    return out_result->ok;
}

void editor_extension_startup_shutdown(EditorExtensionStartupResult* state) {
    if (state == NULL) return;
    editor_plugin_registry_shutdown(&state->plugin_registry);
}
