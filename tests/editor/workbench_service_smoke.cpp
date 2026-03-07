#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>

#include "../../apps/sandbox_dwrite/application/workbench/workbench_service.hpp"

static int g_toggle_run_called = 0;
static int g_saved = 0;
static int g_loaded = 0;
static int g_history_reset = 0;
static int g_history_undo = 0;
static int g_history_redo = 0;
static int g_history_push = 0;
static int g_copy_ok = 1;
static int g_paste_ok = 1;
static int g_layout_next = 0;
static int g_layout_save = 0;
static int g_filter_input = 0;
static wchar_t g_last_log[128];

static void cb_toggle_run(void* user) {
    int* value = (int*)user;
    if (value != 0) (*value)++;
}

static void cb_noop(void* user) {
    (void)user;
}

static int smoke_save_snapshot(const char* path_utf8) {
    if (path_utf8 != 0 && path_utf8[0] != '\0') g_saved++;
    return 1;
}

static int smoke_load_snapshot(const char* path_utf8) {
    if (path_utf8 != 0 && path_utf8[0] != '\0') g_loaded++;
    return 1;
}

static void smoke_history_reset(void) { g_history_reset++; }
static void smoke_history_undo(void) { g_history_undo++; }
static void smoke_history_redo(void) { g_history_redo++; }
static void smoke_history_push(void) { g_history_push++; }
static int smoke_copy_selected(void) { return g_copy_ok; }
static int smoke_paste_selected(void) { return g_paste_ok; }
static void smoke_apply_layout_next(void) { g_layout_next++; }
static void smoke_save_layout(void) { g_layout_save++; }
static void smoke_begin_filter_input(void) { g_filter_input++; }

static void smoke_push_console_log(const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vswprintf(g_last_log, 128, fmt, args);
    va_end(args);
}

int main(void) {
    AppRuntime runtime;
    AppCommandCallbacks runtime_callbacks{};
    WorkbenchServiceCallbacks callbacks{};
    UiIntent intent{};
    int draw_centers = 0;
    int draw_contacts = 0;
    int draw_velocity = 0;
    int show_bottom = 0;
    int bottom_collapsed = 1;
    int bottom_tab = 1;
    int log_filter = 0;
    int log_scroll = 42;

    runtime_callbacks.toggle_run = cb_toggle_run;
    runtime_callbacks.step_once = cb_noop;
    runtime_callbacks.reset_scene = cb_noop;
    runtime_callbacks.spawn_circle = cb_noop;
    runtime_callbacks.spawn_box = cb_noop;
    runtime_callbacks.user = &g_toggle_run_called;
    app_runtime_init(&runtime, runtime_callbacks);

    callbacks.app_runtime = &runtime;
    callbacks.draw_centers = &draw_centers;
    callbacks.draw_contacts = &draw_contacts;
    callbacks.draw_velocity = &draw_velocity;
    callbacks.show_bottom_panel = &show_bottom;
    callbacks.bottom_panel_collapsed = &bottom_collapsed;
    callbacks.bottom_active_tab = &bottom_tab;
    callbacks.log_filter_mode = &log_filter;
    callbacks.log_scroll_offset = &log_scroll;
    callbacks.save_snapshot = smoke_save_snapshot;
    callbacks.load_snapshot = smoke_load_snapshot;
    callbacks.history_reset_and_capture = smoke_history_reset;
    callbacks.history_undo = smoke_history_undo;
    callbacks.history_redo = smoke_history_redo;
    callbacks.history_push_snapshot = smoke_history_push;
    callbacks.copy_selected_body_to_clipboard = smoke_copy_selected;
    callbacks.paste_body_from_clipboard = smoke_paste_selected;
    callbacks.apply_layout_preset_next = smoke_apply_layout_next;
    callbacks.save_ui_layout = smoke_save_layout;
    callbacks.begin_hierarchy_filter_input = smoke_begin_filter_input;
    callbacks.push_console_log = smoke_push_console_log;

    intent.type = UI_INTENT_APP_COMMAND;
    intent.app_command.type = APP_CMD_TOGGLE_RUN;
    if (!workbench_service_handle_intent(&callbacks, &intent) || g_toggle_run_called != 1) {
        printf("[FAIL] app command intent\n");
        return 1;
    }

    intent.type = UI_INTENT_SAVE_SNAPSHOT;
    intent.path_utf8 = "scene_snapshot.txt";
    if (!workbench_service_handle_intent(&callbacks, &intent) || g_saved != 1) {
        printf("[FAIL] save snapshot intent\n");
        return 2;
    }

    intent.type = UI_INTENT_LOAD_SNAPSHOT;
    intent.path_utf8 = "scene_snapshot.txt";
    intent.use_autosave = 0;
    if (!workbench_service_handle_intent(&callbacks, &intent) || g_loaded != 1 || g_history_reset != 1) {
        printf("[FAIL] load snapshot intent\n");
        return 3;
    }

    intent.type = UI_INTENT_HISTORY_UNDO;
    workbench_service_handle_intent(&callbacks, &intent);
    intent.type = UI_INTENT_HISTORY_REDO;
    workbench_service_handle_intent(&callbacks, &intent);
    if (g_history_undo != 1 || g_history_redo != 1) {
        printf("[FAIL] history intents\n");
        return 4;
    }

    intent.type = UI_INTENT_COPY_SELECTED;
    workbench_service_handle_intent(&callbacks, &intent);
    intent.type = UI_INTENT_PASTE_SELECTED;
    workbench_service_handle_intent(&callbacks, &intent);
    if (g_history_push != 1) {
        printf("[FAIL] paste should push history snapshot\n");
        return 5;
    }

    intent.type = UI_INTENT_FOCUS_CONSOLE_LOG;
    intent.log_filter_mode = 3;
    workbench_service_handle_intent(&callbacks, &intent);
    if (!show_bottom || bottom_collapsed || bottom_tab != 0 || log_filter != 3 || log_scroll != 0) {
        printf("[FAIL] focus console intent\n");
        return 6;
    }

    intent.type = UI_INTENT_TOGGLE_DRAW_CENTERS;
    workbench_service_handle_intent(&callbacks, &intent);
    intent.type = UI_INTENT_TOGGLE_DRAW_CONTACTS;
    workbench_service_handle_intent(&callbacks, &intent);
    intent.type = UI_INTENT_TOGGLE_DRAW_VELOCITY;
    workbench_service_handle_intent(&callbacks, &intent);
    if (!draw_centers || !draw_contacts || !draw_velocity) {
        printf("[FAIL] draw toggle intents\n");
        return 7;
    }

    intent.type = UI_INTENT_APPLY_NEXT_LAYOUT;
    workbench_service_handle_intent(&callbacks, &intent);
    intent.type = UI_INTENT_SAVE_LAYOUT;
    workbench_service_handle_intent(&callbacks, &intent);
    intent.type = UI_INTENT_BEGIN_HIERARCHY_FILTER_INPUT;
    workbench_service_handle_intent(&callbacks, &intent);
    if (g_layout_next != 1 || g_layout_save != 1 || g_filter_input != 1) {
        printf("[FAIL] layout/filter intents\n");
        return 8;
    }

    printf("[PASS] workbench service smoke\n");
    return 0;
}
