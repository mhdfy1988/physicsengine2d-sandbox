#include <stddef.h>
#include "menu_view_physics_window_actions.hpp"

int menu_view_physics_window_execute(int menu_id, int item_idx, const MenuViewPhysicsWindowOps* ops) {
    if (ops == NULL) return 0;
    if (menu_id == 3) {
        if (item_idx == 0 && ops->draw_centers != NULL) *ops->draw_centers = !(*ops->draw_centers);
        else if (item_idx == 1 && ops->draw_contacts != NULL) *ops->draw_contacts = !(*ops->draw_contacts);
        else if (item_idx == 2 && ops->draw_velocity != NULL) *ops->draw_velocity = !(*ops->draw_velocity);
        return 1;
    }
    if (menu_id == 5) {
        if (item_idx == 0 && ops->toggle_run != NULL) ops->toggle_run(ops->user);
        else if (item_idx == 1 && ops->step_once != NULL) ops->step_once(ops->user);
        else if (item_idx == 2 && ops->reset_scene != NULL) ops->reset_scene(ops->user);
        return 1;
    }
    if (menu_id == 6) {
        if (item_idx == 0 && ops->apply_next_layout != NULL) {
            ops->apply_next_layout(ops->user);
        } else if (item_idx == 1 && ops->show_left != NULL) {
            *ops->show_left = !(*ops->show_left);
        } else if (item_idx == 2 && ops->show_right != NULL) {
            *ops->show_right = !(*ops->show_right);
        } else if (item_idx == 3 && ops->show_bottom != NULL) {
            *ops->show_bottom = !(*ops->show_bottom);
        } else if (item_idx == 4 && ops->show_bottom != NULL && *ops->show_bottom && ops->bottom_active_tab != NULL) {
            *ops->bottom_active_tab = (*ops->bottom_active_tab + 1) % 2;
        } else if (item_idx == 5 && ops->theme_light != NULL) {
            *ops->theme_light = !(*ops->theme_light);
        }
        if (ops->save_layout != NULL) ops->save_layout(ops->user);
        return 1;
    }
    return 0;
}
