#ifndef MENU_FILE_EDIT_ACTIONS_H
#define MENU_FILE_EDIT_ACTIONS_H

typedef struct {
    void (*new_project)(void* user);
    void (*open_project)(void* user);
    void (*new_scene)(void* user);
    int (*save_scene)(const char* path, void* user);
    int (*save_scene_as)(const char* path, void* user);
    void (*layout_settings)(void* user);
    void (*open_preferences)(void* user);
    void (*open_shortcuts)(void* user);
    void (*close_window)(void* user);
    void (*quit_app)(void* user);
    int (*save_snapshot)(const char* path, void* user);
    int (*load_snapshot)(const char* path, void* user);
    void (*history_reset)(void* user);
    void (*history_undo)(void* user);
    void (*history_redo)(void* user);
    void (*history_push)(void* user);
    int (*copy_selected)(void* user);
    int (*paste_selected)(void* user);
    void (*spawn_center_circle)(void* user);
    void (*spawn_center_box)(void* user);
    void (*clear_logs)(void* user);
    void (*log_text)(const wchar_t* text, void* user);
    int* draw_constraints;
    void* user;
} MenuFileEditOps;

int menu_file_edit_execute(int menu_id, int item_idx, const MenuFileEditOps* ops);

#endif
