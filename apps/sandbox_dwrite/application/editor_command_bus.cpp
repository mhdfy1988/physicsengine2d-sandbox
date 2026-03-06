#include <string.h>
#include "editor_command_bus.hpp"

void editor_command_bus_init(EditorCommandCallbacks* callbacks) {
    if (callbacks == NULL) return;
    memset(callbacks, 0, sizeof(*callbacks));
}

int editor_command_bus_dispatch(const EditorCommandCallbacks* callbacks, const EditorCommand* command) {
    if (callbacks == NULL || command == NULL) return 0;
    switch (command->type) {
        case EDITOR_CMD_SCENE_RENAME:
            if (callbacks->scene_rename == NULL) return 0;
            return callbacks->scene_rename(command->arg_i0, command->text, callbacks->user);
        case EDITOR_CMD_SCENE_ORDER_MOVE:
            if (callbacks->scene_order_move == NULL) return 0;
            return callbacks->scene_order_move(command->arg_i0, callbacks->user);
        case EDITOR_CMD_SCENE_ORDER_RESET:
            if (callbacks->scene_order_reset == NULL) return 0;
            return callbacks->scene_order_reset(callbacks->user);
        case EDITOR_CMD_SCENE_ASSET_REF_SET:
            if (callbacks->scene_asset_ref_set == NULL) return 0;
            return callbacks->scene_asset_ref_set(command->arg_i0, command->text, callbacks->user);
        case EDITOR_CMD_INSPECTOR_SET_VALUE:
            if (callbacks->inspector_set_value == NULL) return 0;
            return callbacks->inspector_set_value(command->arg_i0, command->arg_f0, command->arg_i1, callbacks->user);
        default:
            return 0;
    }
}
