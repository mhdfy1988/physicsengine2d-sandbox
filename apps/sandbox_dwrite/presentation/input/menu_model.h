#ifndef MENU_MODEL_H
#define MENU_MODEL_H

typedef struct {
    int history_cursor;
    int history_top;
    int has_selected_dynamic;
    int clipboard_valid;
    int show_bottom_panel;
    int has_scene_snapshot;
    int has_autosave_snapshot;
    int draw_constraints;
    int draw_centers;
    int draw_contacts;
    int draw_velocity;
    int running;
    int show_left_panel;
    int show_right_panel;
    int theme_light;
} MenuUiState;

const wchar_t* menu_model_shortcut_text(int menu_id, int item_idx);
const wchar_t* menu_model_item_text(int menu_id, int item_idx);
int menu_model_item_enabled_state(int menu_id, int item_idx, const MenuUiState* state);
int menu_model_item_checked_state(int menu_id, int item_idx, const MenuUiState* state);
int menu_model_item_count_for_menu(int menu_id);
int menu_model_next_visible_id(int current_id, int dir);
int menu_model_find_enabled_from(int menu_id, int start, int dir, const MenuUiState* state);

#endif
