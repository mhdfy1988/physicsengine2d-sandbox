#ifndef UI_LAYOUT_REPO_H
#define UI_LAYOUT_REPO_H

typedef struct {
    float left_ratio;
    float right_ratio;
    float bottom_open_h;
    int bottom_collapsed;
    int show_left;
    int show_right;
    int show_bottom;
    int theme_light;
    int preset;
} UiLayoutPrefs;

int ui_layout_repo_load(const wchar_t* path, UiLayoutPrefs* out_prefs);
int ui_layout_repo_save(const wchar_t* path, const UiLayoutPrefs* prefs);

#endif
