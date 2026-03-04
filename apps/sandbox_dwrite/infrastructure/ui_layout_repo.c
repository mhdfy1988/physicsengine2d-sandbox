#include <wchar.h>
#include <stdlib.h>
#include <windows.h>
#include "ui_layout_repo.h"

static float clampf_local(float v, float min_v, float max_v) {
    if (v < min_v) return min_v;
    if (v > max_v) return max_v;
    return v;
}

int ui_layout_repo_load(const wchar_t* path, UiLayoutPrefs* out_prefs) {
    wchar_t buf[64];
    if (path == NULL || out_prefs == NULL) return 0;
    GetPrivateProfileStringW(L"layout", L"left_ratio", L"0.18", buf, 64, path);
    out_prefs->left_ratio = (float)_wtof(buf);
    GetPrivateProfileStringW(L"layout", L"right_ratio", L"0.25", buf, 64, path);
    out_prefs->right_ratio = (float)_wtof(buf);
    GetPrivateProfileStringW(L"layout", L"bottom_open_h", L"132", buf, 64, path);
    out_prefs->bottom_open_h = (float)_wtof(buf);
    GetPrivateProfileStringW(L"layout", L"bottom_collapsed", L"1", buf, 64, path);
    out_prefs->bottom_collapsed = _wtoi(buf) ? 1 : 0;
    GetPrivateProfileStringW(L"layout", L"show_left", L"1", buf, 64, path);
    out_prefs->show_left = _wtoi(buf) ? 1 : 0;
    GetPrivateProfileStringW(L"layout", L"show_right", L"1", buf, 64, path);
    out_prefs->show_right = _wtoi(buf) ? 1 : 0;
    GetPrivateProfileStringW(L"layout", L"show_bottom", L"1", buf, 64, path);
    out_prefs->show_bottom = _wtoi(buf) ? 1 : 0;
    GetPrivateProfileStringW(L"layout", L"theme_light", L"0", buf, 64, path);
    out_prefs->theme_light = _wtoi(buf) ? 1 : 0;
    GetPrivateProfileStringW(L"layout", L"preset", L"0", buf, 64, path);
    out_prefs->preset = _wtoi(buf) % 3;

    out_prefs->left_ratio = clampf_local(out_prefs->left_ratio, 0.10f, 0.28f);
    out_prefs->right_ratio = clampf_local(out_prefs->right_ratio, 0.12f, 0.34f);
    out_prefs->bottom_open_h = clampf_local(out_prefs->bottom_open_h, 96.0f, 260.0f);
    return 1;
}

int ui_layout_repo_save(const wchar_t* path, const UiLayoutPrefs* prefs) {
    wchar_t buf[64];
    if (path == NULL || prefs == NULL) return 0;
    swprintf(buf, 64, L"%.4f", prefs->left_ratio);
    WritePrivateProfileStringW(L"layout", L"left_ratio", buf, path);
    swprintf(buf, 64, L"%.4f", prefs->right_ratio);
    WritePrivateProfileStringW(L"layout", L"right_ratio", buf, path);
    swprintf(buf, 64, L"%.1f", prefs->bottom_open_h);
    WritePrivateProfileStringW(L"layout", L"bottom_open_h", buf, path);
    swprintf(buf, 64, L"%d", prefs->bottom_collapsed ? 1 : 0);
    WritePrivateProfileStringW(L"layout", L"bottom_collapsed", buf, path);
    swprintf(buf, 64, L"%d", prefs->show_left ? 1 : 0);
    WritePrivateProfileStringW(L"layout", L"show_left", buf, path);
    swprintf(buf, 64, L"%d", prefs->show_right ? 1 : 0);
    WritePrivateProfileStringW(L"layout", L"show_right", buf, path);
    swprintf(buf, 64, L"%d", prefs->show_bottom ? 1 : 0);
    WritePrivateProfileStringW(L"layout", L"show_bottom", buf, path);
    swprintf(buf, 64, L"%d", prefs->theme_light ? 1 : 0);
    WritePrivateProfileStringW(L"layout", L"theme_light", buf, path);
    swprintf(buf, 64, L"%d", prefs->preset);
    WritePrivateProfileStringW(L"layout", L"preset", buf, path);
    return 1;
}
