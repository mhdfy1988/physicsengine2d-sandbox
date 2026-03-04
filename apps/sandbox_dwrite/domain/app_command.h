#ifndef APP_COMMAND_H
#define APP_COMMAND_H

typedef enum {
    APP_CMD_NONE = 0,
    APP_CMD_TOGGLE_RUN,
    APP_CMD_RESET_SCENE,
    APP_CMD_SPAWN_CIRCLE,
    APP_CMD_SPAWN_BOX
} AppCommandType;

typedef struct {
    AppCommandType type;
} AppCommand;

typedef struct {
    void (*toggle_run)(void* user);
    void (*reset_scene)(void* user);
    void (*spawn_circle)(void* user);
    void (*spawn_box)(void* user);
    void* user;
} AppCommandCallbacks;

int app_command_execute(const AppCommand* cmd, const AppCommandCallbacks* callbacks);

#endif
