#ifndef MENU_VIEW_PHYSICS_WINDOW_ACTIONS_H
#define MENU_VIEW_PHYSICS_WINDOW_ACTIONS_H

typedef struct {
    int* draw_centers;
    int* draw_contacts;
    int* draw_velocity;
    int* show_left;
    int* show_right;
    int* show_bottom;
    int* bottom_active_tab;
    int* theme_light;
    int* ui_layout_preset;
    void (*toggle_run)(void* user);
    void (*step_once)(void* user);
    void (*reset_scene)(void* user);
    void (*apply_next_layout)(void* user);
    void (*save_layout)(void* user);
    void* user;
} MenuViewPhysicsWindowOps;

int menu_view_physics_window_execute(int menu_id, int item_idx, const MenuViewPhysicsWindowOps* ops);

#endif
