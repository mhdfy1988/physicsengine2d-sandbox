#include "workbench_service.hpp"

int workbench_service_handle_intent(const WorkbenchServiceCallbacks* callbacks, const UiIntent* intent) {
    if (callbacks == 0 || intent == 0) return 0;

    switch (intent->type) {
        case UI_INTENT_APP_COMMAND:
            if (callbacks->app_runtime == 0) return 0;
            return app_runtime_dispatch(callbacks->app_runtime, intent->app_command);
        case UI_INTENT_SAVE_SNAPSHOT:
            if (callbacks->save_snapshot == 0 || intent->path_utf8 == 0) return 0;
            if (callbacks->save_snapshot(intent->path_utf8)) callbacks->push_console_log(L"[cmd] saved %S", intent->path_utf8);
            else callbacks->push_console_log(L"[error] save %S failed", intent->path_utf8);
            return 1;
        case UI_INTENT_LOAD_SNAPSHOT:
            if (callbacks->load_snapshot == 0 || callbacks->history_reset_and_capture == 0 || intent->path_utf8 == 0) return 0;
            if (callbacks->load_snapshot(intent->path_utf8)) {
                callbacks->history_reset_and_capture();
                callbacks->push_console_log(intent->use_autosave ? L"[shortcut] restored autosave" : L"[shortcut] loaded scene snapshot");
            } else {
                callbacks->push_console_log(intent->use_autosave ? L"[error] restore autosave failed" : L"[error] load scene snapshot failed");
            }
            return 1;
        case UI_INTENT_HISTORY_RESET:
            if (callbacks->history_reset_and_capture == 0) return 0;
            callbacks->history_reset_and_capture();
            return 1;
        case UI_INTENT_HISTORY_UNDO:
            if (callbacks->history_undo == 0) return 0;
            callbacks->history_undo();
            return 1;
        case UI_INTENT_HISTORY_REDO:
            if (callbacks->history_redo == 0) return 0;
            callbacks->history_redo();
            return 1;
        case UI_INTENT_COPY_SELECTED:
            if (callbacks->copy_selected_body_to_clipboard == 0) return 0;
            if (callbacks->copy_selected_body_to_clipboard()) callbacks->push_console_log(L"[shortcut] copied selection");
            else callbacks->push_console_log(L"[hint] nothing to copy");
            return 1;
        case UI_INTENT_PASTE_SELECTED:
            if (callbacks->history_push_snapshot == 0 || callbacks->paste_body_from_clipboard == 0) return 0;
            callbacks->history_push_snapshot();
            if (callbacks->paste_body_from_clipboard()) callbacks->push_console_log(L"[shortcut] pasted selection");
            else callbacks->push_console_log(L"[hint] clipboard empty");
            return 1;
        case UI_INTENT_FOCUS_CONSOLE_LOG:
            if (callbacks->show_bottom_panel == 0 || callbacks->bottom_panel_collapsed == 0 || callbacks->bottom_active_tab == 0 ||
                callbacks->log_filter_mode == 0 || callbacks->log_scroll_offset == 0) return 0;
            *callbacks->show_bottom_panel = 1;
            *callbacks->bottom_panel_collapsed = 0;
            *callbacks->bottom_active_tab = 0;
            *callbacks->log_filter_mode = intent->log_filter_mode;
            *callbacks->log_scroll_offset = 0;
            return 1;
        case UI_INTENT_APPLY_NEXT_LAYOUT:
            if (callbacks->apply_layout_preset_next == 0) return 0;
            callbacks->apply_layout_preset_next();
            return 1;
        case UI_INTENT_SAVE_LAYOUT:
            if (callbacks->save_ui_layout == 0) return 0;
            callbacks->save_ui_layout();
            return 1;
        case UI_INTENT_TOGGLE_DRAW_CENTERS:
            if (callbacks->draw_centers == 0) return 0;
            *callbacks->draw_centers = !*callbacks->draw_centers;
            return 1;
        case UI_INTENT_TOGGLE_DRAW_CONTACTS:
            if (callbacks->draw_contacts == 0) return 0;
            *callbacks->draw_contacts = !*callbacks->draw_contacts;
            return 1;
        case UI_INTENT_TOGGLE_DRAW_VELOCITY:
            if (callbacks->draw_velocity == 0) return 0;
            *callbacks->draw_velocity = !*callbacks->draw_velocity;
            return 1;
        case UI_INTENT_BEGIN_HIERARCHY_FILTER_INPUT:
            if (callbacks->begin_hierarchy_filter_input == 0) return 0;
            callbacks->begin_hierarchy_filter_input();
            return 1;
        default:
            return 0;
    }
}
