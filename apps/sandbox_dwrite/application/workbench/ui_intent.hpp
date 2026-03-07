#ifndef WORKBENCH_UI_INTENT_H
#define WORKBENCH_UI_INTENT_H

#include "../../domain/app_command.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UI_INTENT_NONE = 0,
    UI_INTENT_APP_COMMAND,
    UI_INTENT_SAVE_SNAPSHOT,
    UI_INTENT_LOAD_SNAPSHOT,
    UI_INTENT_HISTORY_RESET,
    UI_INTENT_HISTORY_UNDO,
    UI_INTENT_HISTORY_REDO,
    UI_INTENT_COPY_SELECTED,
    UI_INTENT_PASTE_SELECTED,
    UI_INTENT_FOCUS_CONSOLE_LOG,
    UI_INTENT_APPLY_NEXT_LAYOUT,
    UI_INTENT_SAVE_LAYOUT,
    UI_INTENT_TOGGLE_DRAW_CENTERS,
    UI_INTENT_TOGGLE_DRAW_CONTACTS,
    UI_INTENT_TOGGLE_DRAW_VELOCITY,
    UI_INTENT_BEGIN_HIERARCHY_FILTER_INPUT
} UiIntentType;

typedef struct {
    UiIntentType type;
    AppCommand app_command;
    const char* path_utf8;
    int use_autosave;
    int log_filter_mode;
} UiIntent;

#ifdef __cplusplus
}
#endif

#endif
