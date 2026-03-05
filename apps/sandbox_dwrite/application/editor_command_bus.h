#ifndef EDITOR_COMMAND_BUS_H
#define EDITOR_COMMAND_BUS_H

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EDITOR_CMD_NONE = 0,
    EDITOR_CMD_SCENE_RENAME,
    EDITOR_CMD_SCENE_ORDER_MOVE,
    EDITOR_CMD_SCENE_ORDER_RESET,
    EDITOR_CMD_INSPECTOR_SET_VALUE
} EditorCommandType;

enum { EDITOR_COMMAND_TEXT_CAP = 64 };

typedef struct {
    EditorCommandType type;
    int arg_i0;
    int arg_i1;
    double arg_f0;
    wchar_t text[EDITOR_COMMAND_TEXT_CAP];
} EditorCommand;

typedef struct {
    int (*scene_rename)(int scene_index, const wchar_t* name, void* user);
    int (*scene_order_move)(int direction, void* user);
    int (*scene_order_reset)(void* user);
    int (*inspector_set_value)(int row, double value, int emit_log, void* user);
    void* user;
} EditorCommandCallbacks;

void editor_command_bus_init(EditorCommandCallbacks* callbacks);
int editor_command_bus_dispatch(const EditorCommandCallbacks* callbacks, const EditorCommand* command);

#ifdef __cplusplus
}
#endif

#endif
