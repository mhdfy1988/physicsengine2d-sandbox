#ifndef APP_COMMAND_H
#define APP_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_CMD_NONE = 0,
    APP_CMD_TOGGLE_RUN,
    APP_CMD_STEP_ONCE,
    APP_CMD_RESET_SCENE,
    APP_CMD_SPAWN_CIRCLE,
    APP_CMD_SPAWN_BOX
} AppCommandType;

typedef struct {
    AppCommandType type;
} AppCommand;

typedef struct {
    void (*toggle_run)(void* user);
    void (*step_once)(void* user);
    void (*reset_scene)(void* user);
    void (*spawn_circle)(void* user);
    void (*spawn_box)(void* user);
    void* user;
} AppCommandCallbacks;

int app_command_execute(const AppCommand* cmd, const AppCommandCallbacks* callbacks);

#ifdef __cplusplus
}
#endif

#endif
