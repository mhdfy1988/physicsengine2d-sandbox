#ifndef WORKBENCH_SERVICE_H
#define WORKBENCH_SERVICE_H

#include "ui_intent.hpp"
#include "../app_runtime.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    AppRuntime* app_runtime;
    int* draw_centers;
    int* draw_contacts;
    int* draw_velocity;
    int* show_bottom_panel;
    int* bottom_panel_collapsed;
    int* bottom_active_tab;
    int* log_filter_mode;
    int* log_scroll_offset;
    int (*save_snapshot)(const char* path_utf8);
    int (*load_snapshot)(const char* path_utf8);
    void (*history_reset_and_capture)(void);
    void (*history_undo)(void);
    void (*history_redo)(void);
    void (*history_push_snapshot)(void);
    int (*copy_selected_body_to_clipboard)(void);
    int (*paste_body_from_clipboard)(void);
    void (*apply_layout_preset_next)(void);
    void (*save_ui_layout)(void);
    void (*begin_hierarchy_filter_input)(void);
    void (*push_console_log)(const wchar_t* fmt, ...);
} WorkbenchServiceCallbacks;

int workbench_service_handle_intent(const WorkbenchServiceCallbacks* callbacks, const UiIntent* intent);

#ifdef __cplusplus
}
#endif

#endif
