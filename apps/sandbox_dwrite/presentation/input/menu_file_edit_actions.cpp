#include <stddef.h>
#include "menu_file_edit_actions.hpp"

static void log_text(const MenuFileEditOps* ops, const wchar_t* text) {
    if (ops != NULL && ops->log_text != NULL && text != NULL) {
        ops->log_text(text, ops->user);
    }
}

int menu_file_edit_execute(int menu_id, int item_idx, const MenuFileEditOps* ops) {
    if (ops == NULL) return 0;
    if (menu_id == 1) {
        if (item_idx == 0) {
            if (ops->new_project != NULL) ops->new_project(ops->user);
        } else if (item_idx == 1) {
            if (ops->open_project != NULL) ops->open_project(ops->user);
        } else if (item_idx == 2) {
            if (ops->new_scene != NULL) ops->new_scene(ops->user);
        } else if (item_idx == 3) {
            if (ops->save_scene != NULL && ops->save_scene("scene_snapshot.txt", ops->user)) log_text(ops, L"[文件] 已保存 scene_snapshot.txt");
            else log_text(ops, L"[错误] 保存 scene_snapshot.txt 失败");
        } else if (item_idx == 4) {
            if (ops->save_scene_as != NULL && ops->save_scene_as("scene_snapshot_copy.txt", ops->user)) log_text(ops, L"[文件] 已另存为 scene_snapshot_copy.txt");
            else log_text(ops, L"[提示] 另存为暂未实现");
        } else if (item_idx == 5) {
            if (ops->layout_settings != NULL) ops->layout_settings(ops->user);
        } else if (item_idx == 6) {
            if (ops->open_preferences != NULL) ops->open_preferences(ops->user);
        } else if (item_idx == 7) {
            if (ops->open_shortcuts != NULL) ops->open_shortcuts(ops->user);
        } else if (item_idx == 8) {
            if (ops->close_window != NULL) ops->close_window(ops->user);
        } else if (item_idx == 9) {
            if (ops->quit_app != NULL) ops->quit_app(ops->user);
        }
        return 1;
    }
    if (menu_id == 2) {
        if (item_idx == 0) {
            if (ops->history_undo != NULL) ops->history_undo(ops->user);
        } else if (item_idx == 1) {
            if (ops->history_redo != NULL) ops->history_redo(ops->user);
        } else if (item_idx == 2) {
            if (ops->copy_selected != NULL && ops->copy_selected(ops->user)) log_text(ops, L"[编辑] 已复制选中对象");
            else log_text(ops, L"[提示] 当前无可复制对象");
        } else if (item_idx == 3) {
            if (ops->history_push != NULL) ops->history_push(ops->user);
            if (ops->paste_selected != NULL && ops->paste_selected(ops->user)) log_text(ops, L"[编辑] 已粘贴对象");
            else log_text(ops, L"[提示] 剪贴板为空");
        } else if (item_idx == 4) {
            if (ops->spawn_center_circle != NULL) ops->spawn_center_circle(ops->user);
        } else if (item_idx == 5) {
            if (ops->spawn_center_box != NULL) ops->spawn_center_box(ops->user);
        } else if (item_idx == 6) {
            if (ops->clear_logs != NULL) ops->clear_logs(ops->user);
        }
        return 1;
    }
    return 0;
}
