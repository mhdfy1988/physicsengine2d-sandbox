#ifndef EDITOR_EXTENSION_HOST_SERVICE_H
#define EDITOR_EXTENSION_HOST_SERVICE_H

#include <wchar.h>

#include "editor_extension_state_service.hpp"
#include "physics_content/diagnostic_bundle.hpp"

#ifdef __cplusplus
extern "C" {
#endif

int editor_extension_host_refresh(EditorExtensionStartupResult* state,
                                  int* builtin_enabled_flag,
                                  wchar_t* out_status_text,
                                  int status_text_capacity,
                                  int* out_startup_degraded,
                                  const EditorPluginV1* scene_inspector_impl,
                                  const EditorPluginV1* failing_plugin_impl);

void editor_extension_host_shutdown(EditorExtensionStartupResult* state);

const char* editor_extension_host_default_project_root(void);
const char* editor_extension_host_default_session_recovery_path(void);
const char* editor_extension_host_default_workspace_path(void);

void editor_extension_host_append_default_diagnostic_files(DiagnosticBundleRequest* request,
                                                           const char* autosave_snapshot_path,
                                                           const char* scene_snapshot_path);

#ifdef __cplusplus
}
#endif

#endif
