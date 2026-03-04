#include "app_command.h"

int app_command_execute(const AppCommand* cmd, const AppCommandCallbacks* callbacks) {
    if (cmd == 0 || callbacks == 0) return 0;
    switch (cmd->type) {
        case APP_CMD_TOGGLE_RUN:
            if (callbacks->toggle_run != 0) callbacks->toggle_run(callbacks->user);
            return 1;
        case APP_CMD_RESET_SCENE:
            if (callbacks->reset_scene != 0) callbacks->reset_scene(callbacks->user);
            return 1;
        case APP_CMD_SPAWN_CIRCLE:
            if (callbacks->spawn_circle != 0) callbacks->spawn_circle(callbacks->user);
            return 1;
        case APP_CMD_SPAWN_BOX:
            if (callbacks->spawn_box != 0) callbacks->spawn_box(callbacks->user);
            return 1;
        default:
            return 0;
    }
}
