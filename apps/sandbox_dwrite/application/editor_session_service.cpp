#include "editor_session_service.hpp"

#include <cstddef>

void editor_session_bootstrap(const EditorSessionBootstrapArgs* args, EditorSessionBootstrapResult* out_result) {
    int recovered_session = 0;
    int autosave_snapshot_detected = 0;
    if (out_result != NULL) {
        out_result->autosave_snapshot_detected = 0;
        out_result->recovered_session = 0;
    }
    if (args == NULL) return;

    if (args->init_state_defaults) args->init_state_defaults();
    if (args->init_editor_command_bus_callbacks) args->init_editor_command_bus_callbacks();
    if (args->init_app_runtime_callbacks) args->init_app_runtime_callbacks();
    if (args->refresh_editor_extension_state) args->refresh_editor_extension_state();
    if (args->init_hot_reload_pipeline) args->init_hot_reload_pipeline();
    if (args->clear_debug_histories) args->clear_debug_histories();
    if (args->clear_collision_events) args->clear_collision_events();
    if (args->reset_clipboard_body) args->reset_clipboard_body();
    if (args->load_ui_layout) args->load_ui_layout();

    if (args->file_exists && args->autosave_snapshot_path) {
        autosave_snapshot_detected = args->file_exists(args->autosave_snapshot_path);
    }
    if (args->apply_scene) args->apply_scene(0);
    if (args->session_recovery_should_restore &&
        args->session_recovery_should_restore() &&
        args->file_exists &&
        args->load_scene_snapshot &&
        args->autosave_snapshot_path &&
        args->file_exists(args->autosave_snapshot_path) &&
        args->load_scene_snapshot(args->autosave_snapshot_path)) {
        if (args->history_reset_and_capture) args->history_reset_and_capture();
        recovered_session = 1;
    }
    if (!recovered_session && args->session_recovery_persist) {
        args->session_recovery_persist(L"startup");
    }

    if (out_result != NULL) {
        out_result->autosave_snapshot_detected = autosave_snapshot_detected;
        out_result->recovered_session = recovered_session;
    }
}

int editor_session_tick_autosave(const EditorSessionAutosaveArgs* args) {
    if (args == NULL || !args->enabled || args->last_autosave_ms == NULL ||
        args->save_scene_snapshot == NULL || args->session_recovery_persist == NULL ||
        args->autosave_snapshot_path == NULL) {
        return 0;
    }
    if ((args->now_ms - *args->last_autosave_ms) < args->autosave_interval_ms) return 0;
    if (!args->save_scene_snapshot(args->autosave_snapshot_path)) return 0;
    *args->last_autosave_ms = args->now_ms;
    args->session_recovery_persist(L"autosave");
    return 1;
}
