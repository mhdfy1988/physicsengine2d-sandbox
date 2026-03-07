#include "editor_extension_host_service.hpp"

#include <stdio.h>
#include <string.h>
#include <filesystem>

namespace {
static const char* kProjectRoot = "samples/physics_sandbox_project";
static const char* kSessionRecoveryPath = "samples/physics_sandbox_project/ProjectSettings/editor_session.physicssession";
static const char* kWorkspacePath = "samples/physics_sandbox_project/ProjectSettings/workspace.physicsworkspace";
static const char* kSceneInspectorPluginId = "builtin.scene_inspector";
static const char* kSceneInspectorManifestPath = "samples/physics_sandbox_project/Packages/builtin.scene_inspector.physicsplugin";
static const char* kFailingMenuPluginId = "builtin.failing_menu";
static const char* kFailingMenuManifestPath = "samples/physics_sandbox_project/Packages/builtin.failing_menu.physicsplugin";

static void host_copy_text(char* out, int out_capacity, const char* src) {
    if (out == NULL || out_capacity <= 0) return;
    out[0] = '\0';
    if (src == NULL) return;
    strncpy(out, src, (size_t)(out_capacity - 1));
    out[out_capacity - 1] = '\0';
}

static void host_init_builtin_plugins(EditorExtensionBuiltinPlugin* builtin_plugins,
                                      const EditorPluginV1* scene_inspector_impl,
                                      const EditorPluginV1* failing_plugin_impl) {
    memset(builtin_plugins, 0, sizeof(EditorExtensionBuiltinPlugin) * 2);
    host_copy_text(builtin_plugins[0].plugin_id, EDITOR_PLUGIN_MAX_ID, kSceneInspectorPluginId);
    host_copy_text(builtin_plugins[0].manifest_path, PROJECT_WORKSPACE_MAX_PATH, kSceneInspectorManifestPath);
    if (scene_inspector_impl != NULL) builtin_plugins[0].implementation = *scene_inspector_impl;

    host_copy_text(builtin_plugins[1].plugin_id, EDITOR_PLUGIN_MAX_ID, kFailingMenuPluginId);
    host_copy_text(builtin_plugins[1].manifest_path, PROJECT_WORKSPACE_MAX_PATH, kFailingMenuManifestPath);
    if (failing_plugin_impl != NULL) builtin_plugins[1].implementation = *failing_plugin_impl;
}

static void host_append_if_exists(DiagnosticBundleRequest* request, const char* source_path, const char* bundle_name) {
    if (request == NULL || source_path == NULL || bundle_name == NULL) return;
    if (request->file_count >= DIAGNOSTIC_BUNDLE_MAX_FILES) return;
    if (!std::filesystem::exists(source_path)) return;
    host_copy_text(request->files[request->file_count].source_path, DIAGNOSTIC_BUNDLE_MAX_PATH, source_path);
    host_copy_text(request->files[request->file_count].bundle_name, DIAGNOSTIC_BUNDLE_MAX_LABEL, bundle_name);
    request->file_count++;
}
}

int editor_extension_host_refresh(EditorExtensionStartupResult* state,
                                  int* builtin_enabled_flag,
                                  wchar_t* out_status_text,
                                  int status_text_capacity,
                                  int* out_startup_degraded,
                                  const EditorPluginV1* scene_inspector_impl,
                                  const EditorPluginV1* failing_plugin_impl) {
    EditorExtensionStartupConfig config;
    EditorExtensionBuiltinPlugin builtin_plugins[2];
    int startup_ok;
    int degraded;

    if (state == NULL) return 0;
    if (builtin_enabled_flag != NULL) *builtin_enabled_flag = 0;
    if (out_status_text != NULL && status_text_capacity > 0) out_status_text[0] = L'\0';
    if (out_startup_degraded != NULL) *out_startup_degraded = 0;

    editor_extension_startup_config_init(&config);
    host_init_builtin_plugins(builtin_plugins, scene_inspector_impl, failing_plugin_impl);
    config.builtin_plugins = builtin_plugins;
    config.builtin_plugin_count = 2;

    startup_ok = editor_extension_startup_run(&config, state);
    degraded = state->degraded || !startup_ok;
    if (out_startup_degraded != NULL) *out_startup_degraded = degraded;

    if (out_status_text != NULL && status_text_capacity > 0) {
        if (!startup_ok) {
            swprintf(out_status_text, (size_t)status_text_capacity, L"启动:失败(%d)", state->diagnostic_count);
        } else if (degraded) {
            swprintf(out_status_text, (size_t)status_text_capacity, L"启动:降级(%d)", state->diagnostic_count);
        } else {
            swprintf(out_status_text, (size_t)status_text_capacity, L"启动:正常");
        }
    }

    return startup_ok;
}

void editor_extension_host_shutdown(EditorExtensionStartupResult* state) {
    editor_extension_startup_shutdown(state);
}

const char* editor_extension_host_default_project_root(void) {
    return kProjectRoot;
}

const char* editor_extension_host_default_session_recovery_path(void) {
    return kSessionRecoveryPath;
}

const char* editor_extension_host_default_workspace_path(void) {
    return kWorkspacePath;
}

void editor_extension_host_append_default_diagnostic_files(DiagnosticBundleRequest* request,
                                                           const char* autosave_snapshot_path,
                                                           const char* scene_snapshot_path) {
    host_append_if_exists(request, autosave_snapshot_path, "autosave_snapshot.txt");
    host_append_if_exists(request, scene_snapshot_path, "scene_snapshot.txt");
    host_append_if_exists(request, editor_extension_host_default_session_recovery_path(), "editor_session.physicssession");
    host_append_if_exists(request, editor_extension_host_default_workspace_path(), "workspace.physicsworkspace");
}
