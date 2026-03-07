#ifndef EDITOR_SESSION_SERVICE_H
#define EDITOR_SESSION_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*init_state_defaults)(void);
    void (*init_editor_command_bus_callbacks)(void);
    void (*init_app_runtime_callbacks)(void);
    void (*refresh_editor_extension_state)(void);
    void (*init_hot_reload_pipeline)(void);
    void (*clear_debug_histories)(void);
    void (*clear_collision_events)(void);
    void (*reset_clipboard_body)(void);
    void (*load_ui_layout)(void);
    void (*apply_scene)(int scene_index);
    int (*session_recovery_should_restore)(void);
    int (*file_exists)(const char* path);
    int (*load_scene_snapshot)(const char* path);
    void (*history_reset_and_capture)(void);
    void (*session_recovery_persist)(const wchar_t* action);
    const char* autosave_snapshot_path;
} EditorSessionBootstrapArgs;

typedef struct {
    int autosave_snapshot_detected;
    int recovered_session;
} EditorSessionBootstrapResult;

typedef struct {
    int enabled;
    unsigned int now_ms;
    unsigned int autosave_interval_ms;
    unsigned int* last_autosave_ms;
    int (*save_scene_snapshot)(const char* path);
    void (*session_recovery_persist)(const wchar_t* action);
    const char* autosave_snapshot_path;
} EditorSessionAutosaveArgs;

void editor_session_bootstrap(const EditorSessionBootstrapArgs* args, EditorSessionBootstrapResult* out_result);
int editor_session_tick_autosave(const EditorSessionAutosaveArgs* args);

#ifdef __cplusplus
}
#endif

#endif
