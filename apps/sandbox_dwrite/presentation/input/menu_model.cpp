#include <stddef.h>
#include "menu_model.hpp"

const wchar_t* menu_model_shortcut_text(int menu_id, int item_idx) {
    if (menu_id == 1 && item_idx == 0) return L"Ctrl+S";
    if (menu_id == 1 && item_idx == 1) return L"Ctrl+O";
    if (menu_id == 1 && item_idx == 2) return L"Ctrl+Shift+O";
    if (menu_id == 2 && item_idx == 0) return L"Ctrl+Z";
    if (menu_id == 2 && item_idx == 1) return L"Ctrl+Y";
    if (menu_id == 2 && item_idx == 2) return L"Ctrl+C";
    if (menu_id == 2 && item_idx == 3) return L"Ctrl+V";
    if (menu_id == 2 && item_idx == 4) return L"1";
    if (menu_id == 2 && item_idx == 5) return L"2";
    if (menu_id == 3 && item_idx == 0) return L"X";
    if (menu_id == 3 && item_idx == 1) return L"C";
    if (menu_id == 3 && item_idx == 2) return L"V";
    if (menu_id == 5 && item_idx == 0) return L"Space";
    if (menu_id == 5 && item_idx == 1) return L"N";
    if (menu_id == 5 && item_idx == 2) return L"R";
    if (menu_id == 6 && item_idx == 0) return L"F11";
    if (menu_id == 6 && item_idx == 1) return L"Alt+1";
    if (menu_id == 6 && item_idx == 2) return L"Alt+2";
    if (menu_id == 6 && item_idx == 3) return L"Alt+3";
    if (menu_id == 6 && item_idx == 5) return L"Alt+T";
    return L"";
}

const wchar_t* menu_model_item_text(int menu_id, int item_idx) {
    if (menu_id == 1) {
        static const wchar_t* file_items[] = {L"保存快照", L"加载快照", L"恢复自动保存", L"约束调试开关"};
        if (item_idx >= 0 && item_idx < 4) return file_items[item_idx];
        return L"";
    }
    if (menu_id == 2) {
        static const wchar_t* edit_items[] = {L"撤销", L"重做", L"复制对象", L"粘贴对象", L"创建圆", L"创建方块", L"清空日志"};
        if (item_idx >= 0 && item_idx < 7) return edit_items[item_idx];
        return L"";
    }
    if (menu_id == 3) {
        static const wchar_t* view_items[] = {L"中心点显示", L"碰撞显示", L"速度向量显示"};
        if (item_idx >= 0 && item_idx < 3) return view_items[item_idx];
        return L"";
    }
    if (menu_id == 5) {
        static const wchar_t* physics_items[] = {L"运行/暂停", L"单步", L"重置场景"};
        if (item_idx >= 0 && item_idx < 3) return physics_items[item_idx];
        return L"";
    }
    if (menu_id == 6) {
        static const wchar_t* window_items[] = {L"切换布局预设", L"显示左侧栏", L"显示右侧栏", L"显示底部栏", L"切换控制台/性能", L"切换浅色主题"};
        if (item_idx >= 0 && item_idx < 6) return window_items[item_idx];
        return L"";
    }
    if (menu_id == 7) {
        static const wchar_t* help_items[] = {L"使用说明", L"物理调试指南", L"版本信息"};
        if (item_idx >= 0 && item_idx < 3) return help_items[item_idx];
        return L"";
    }
    return L"";
}

int menu_model_item_enabled_state(int menu_id, int item_idx, const MenuUiState* state) {
    if (state == NULL) return 1;
    if (menu_id == 1 && item_idx == 1) return state->has_scene_snapshot;
    if (menu_id == 1 && item_idx == 2) return state->has_autosave_snapshot;
    if (menu_id == 2 && item_idx == 0) return state->history_cursor > 0;
    if (menu_id == 2 && item_idx == 1) return state->history_cursor < state->history_top;
    if (menu_id == 2 && item_idx == 2) return state->has_selected_dynamic;
    if (menu_id == 2 && item_idx == 3) return state->clipboard_valid;
    if (menu_id == 6 && item_idx == 4) return state->show_bottom_panel;
    return 1;
}

int menu_model_item_checked_state(int menu_id, int item_idx, const MenuUiState* state) {
    if (state == NULL) return 0;
    if (menu_id == 1 && item_idx == 3) return state->draw_constraints;
    if (menu_id == 3 && item_idx == 0) return state->draw_centers;
    if (menu_id == 3 && item_idx == 1) return state->draw_contacts;
    if (menu_id == 3 && item_idx == 2) return state->draw_velocity;
    if (menu_id == 5 && item_idx == 0) return state->running;
    if (menu_id == 6 && item_idx == 1) return state->show_left_panel;
    if (menu_id == 6 && item_idx == 2) return state->show_right_panel;
    if (menu_id == 6 && item_idx == 3) return state->show_bottom_panel;
    if (menu_id == 6 && item_idx == 5) return state->theme_light;
    return 0;
}

int menu_model_item_count_for_menu(int menu_id) {
    if (menu_id == 1) return 4;
    if (menu_id == 2) return 7;
    if (menu_id == 3) return 3;
    if (menu_id == 4) return 0;
    if (menu_id == 5) return 3;
    if (menu_id == 6) return 6;
    if (menu_id == 7) return 3;
    return 0;
}

int menu_model_next_visible_id(int current_id, int dir) {
    static const int ids[] = {1, 2, 3, 5, 6, 7};
    int i;
    int idx = 0;
    int n = (int)(sizeof(ids) / sizeof(ids[0]));
    if (dir == 0) return current_id;
    for (i = 0; i < n; i++) {
        if (ids[i] == current_id) {
            idx = i;
            break;
        }
    }
    idx = (idx + (dir > 0 ? 1 : -1) + n) % n;
    return ids[idx];
}

int menu_model_find_enabled_from(int menu_id, int start, int dir, const MenuUiState* state) {
    int n = menu_model_item_count_for_menu(menu_id);
    int i;
    if (n <= 0) return 0;
    for (i = 0; i < n; i++) {
        int idx = (start + dir * i + n * 4) % n;
        if (menu_model_item_enabled_state(menu_id, idx, state)) return idx;
    }
    return 0;
}
