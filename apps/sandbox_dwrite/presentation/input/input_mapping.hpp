#ifndef INPUT_MAPPING_H
#define INPUT_MAPPING_H

#include "../../domain/app_command.hpp"

typedef enum {
    TOOLBAR_ACTION_NONE = 0,
    TOOLBAR_ACTION_TOGGLE_RUN,
    TOOLBAR_ACTION_RESET_SCENE
} ToolbarAction;

typedef enum {
    RUNTIME_TOGGLE_NONE = 0,
    RUNTIME_TOGGLE_DRAW_CONTACTS,
    RUNTIME_TOGGLE_DRAW_VELOCITY,
    RUNTIME_TOGGLE_DRAW_CENTERS,
    RUNTIME_TOGGLE_DRAW_CONSTRAINTS,
    RUNTIME_TOGGLE_BOTTOM_PANEL,
    RUNTIME_TOGGLE_NEXT_LAYOUT
} RuntimeToggleAction;

int input_try_map_keydown_command(unsigned int key, AppCommand* out_cmd);
int input_try_map_toolbar_command(ToolbarAction action, AppCommand* out_cmd);
int input_try_map_log_paging(unsigned int key, int* out_delta);
int input_try_map_runtime_toggle(unsigned int key, RuntimeToggleAction* out_action);
int input_try_map_scene_index(unsigned int key, int* out_scene_index);
int input_try_map_scene_step(unsigned int key, int* out_step);
int input_try_map_constraint_kind(unsigned int key, int* out_kind);
int input_try_map_constraint_param_delta(unsigned int key, float* out_len_delta, float* out_stiffness_delta);

#endif
