#include "input_mapping.h"

int input_try_map_keydown_command(unsigned int key, AppCommand* out_cmd) {
    if (out_cmd == 0) return 0;
    if (key == '1') {
        out_cmd->type = APP_CMD_SPAWN_CIRCLE;
        return 1;
    }
    if (key == '2') {
        out_cmd->type = APP_CMD_SPAWN_BOX;
        return 1;
    }
    if (key == 0x20) { /* VK_SPACE */
        out_cmd->type = APP_CMD_TOGGLE_RUN;
        return 1;
    }
    if (key == 'N') {
        out_cmd->type = APP_CMD_STEP_ONCE;
        return 1;
    }
    if (key == 'R') {
        out_cmd->type = APP_CMD_RESET_SCENE;
        return 1;
    }
    return 0;
}

int input_try_map_toolbar_command(ToolbarAction action, AppCommand* out_cmd) {
    if (out_cmd == 0) return 0;
    if (action == TOOLBAR_ACTION_TOGGLE_RUN) {
        out_cmd->type = APP_CMD_TOGGLE_RUN;
        return 1;
    }
    if (action == TOOLBAR_ACTION_RESET_SCENE) {
        out_cmd->type = APP_CMD_RESET_SCENE;
        return 1;
    }
    return 0;
}

int input_try_map_log_paging(unsigned int key, int* out_delta) {
    if (out_delta == 0) return 0;
    if (key == 0x21) { /* VK_PRIOR */
        *out_delta = 3;
        return 1;
    }
    if (key == 0x22) { /* VK_NEXT */
        *out_delta = -3;
        return 1;
    }
    return 0;
}

int input_try_map_runtime_toggle(unsigned int key, RuntimeToggleAction* out_action) {
    if (out_action == 0) return 0;
    if (key == 'C') {
        *out_action = RUNTIME_TOGGLE_DRAW_CONTACTS;
        return 1;
    }
    if (key == 'V') {
        *out_action = RUNTIME_TOGGLE_DRAW_VELOCITY;
        return 1;
    }
    if (key == 'X') {
        *out_action = RUNTIME_TOGGLE_DRAW_CENTERS;
        return 1;
    }
    if (key == 'M') {
        *out_action = RUNTIME_TOGGLE_DRAW_CONSTRAINTS;
        return 1;
    }
    if (key == 0x79) { /* VK_F10 */
        *out_action = RUNTIME_TOGGLE_BOTTOM_PANEL;
        return 1;
    }
    if (key == 0x7A) { /* VK_F11 */
        *out_action = RUNTIME_TOGGLE_NEXT_LAYOUT;
        return 1;
    }
    return 0;
}

int input_try_map_scene_index(unsigned int key, int* out_scene_index) {
    if (out_scene_index == 0) return 0;
    if (key >= 0x70 && key <= 0x78) { /* VK_F1..VK_F9 */
        *out_scene_index = (int)(key - 0x70);
        return 1;
    }
    return 0;
}

int input_try_map_scene_step(unsigned int key, int* out_step) {
    if (out_step == 0) return 0;
    if (key == 0xDB) { /* VK_OEM_4 '[' */
        *out_step = -1;
        return 1;
    }
    if (key == 0xDD) { /* VK_OEM_6 ']' */
        *out_step = 1;
        return 1;
    }
    return 0;
}

int input_try_map_constraint_kind(unsigned int key, int* out_kind) {
    if (out_kind == 0) return 0;
    if (key == 'J') {
        *out_kind = 1;
        return 1;
    }
    if (key == 'K') {
        *out_kind = 2;
        return 1;
    }
    if (key == 'L') {
        *out_kind = 3;
        return 1;
    }
    if (key == 'P') {
        *out_kind = 4;
        return 1;
    }
    return 0;
}

int input_try_map_constraint_param_delta(unsigned int key, float* out_len_delta, float* out_stiffness_delta) {
    if (out_len_delta == 0 || out_stiffness_delta == 0) return 0;
    if (key == 'T') {
        *out_len_delta = 0.08f;
        *out_stiffness_delta = 0.0f;
        return 1;
    }
    if (key == 'G') {
        *out_len_delta = -0.08f;
        *out_stiffness_delta = 0.0f;
        return 1;
    }
    if (key == 'Y') {
        *out_len_delta = 0.0f;
        *out_stiffness_delta = 0.2f;
        return 1;
    }
    if (key == 'H') {
        *out_len_delta = 0.0f;
        *out_stiffness_delta = -0.2f;
        return 1;
    }
    return 0;
}
