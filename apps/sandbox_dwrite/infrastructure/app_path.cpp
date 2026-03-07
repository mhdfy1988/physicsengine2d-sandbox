#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include "app_path.hpp"

#include <windows.h>
#include <string.h>

static int sandbox_app_path_copy_wide(wchar_t* out_path, int out_capacity, const wchar_t* src) {
    if (out_path == NULL || out_capacity <= 0 || src == NULL) return 0;
    if (lstrlenW(src) >= out_capacity) return 0;
    lstrcpynW(out_path, src, out_capacity);
    return 1;
}

static int sandbox_app_path_copy_utf8(char* out_path, int out_capacity, const char* src) {
    if (out_path == NULL || out_capacity <= 0 || src == NULL) return 0;
    if ((int)strlen(src) >= out_capacity) return 0;
    strncpy(out_path, src, (size_t)(out_capacity - 1));
    out_path[out_capacity - 1] = '\0';
    return 1;
}

static int sandbox_app_path_get_exe_dir_wide(wchar_t* out_dir, int out_capacity) {
    DWORD len;
    int idx;
    if (out_dir == NULL || out_capacity <= 0) return 0;
    len = GetModuleFileNameW(NULL, out_dir, (DWORD)out_capacity);
    if (len == 0 || len >= (DWORD)out_capacity) return 0;
    for (idx = (int)len - 1; idx >= 0; --idx) {
        if (out_dir[idx] == L'\\' || out_dir[idx] == L'/') {
            out_dir[idx] = L'\0';
            return 1;
        }
    }
    return 0;
}

static int sandbox_app_path_join_wide(const wchar_t* left, const wchar_t* right, wchar_t* out_path, int out_capacity) {
    int left_len;
    int right_len;
    int need_sep;
    if (left == NULL || right == NULL || out_path == NULL || out_capacity <= 0) return 0;
    left_len = lstrlenW(left);
    right_len = lstrlenW(right);
    need_sep = left_len > 0 && left[left_len - 1] != L'\\' && left[left_len - 1] != L'/';
    if (left_len + right_len + (need_sep ? 1 : 0) + 1 > out_capacity) return 0;
    lstrcpynW(out_path, left, out_capacity);
    if (need_sep) lstrcatW(out_path, L"\\");
    lstrcatW(out_path, right);
    return 1;
}

static int sandbox_app_path_parent_dir_wide(wchar_t* path) {
    int idx;
    if (path == NULL) return 0;
    for (idx = lstrlenW(path) - 1; idx >= 0; --idx) {
        if (path[idx] == L'\\' || path[idx] == L'/') {
            path[idx] = L'\0';
            return 1;
        }
    }
    return 0;
}

static int sandbox_app_path_wide_to_utf8(const wchar_t* src, char* out_path, int out_capacity) {
    return WideCharToMultiByte(CP_UTF8, 0, src, -1, out_path, out_capacity, NULL, NULL) > 0 ? 1 : 0;
}

static int sandbox_app_path_utf8_to_wide(const char* src, wchar_t* out_path, int out_capacity) {
    return MultiByteToWideChar(CP_UTF8, 0, src, -1, out_path, out_capacity) > 0 ? 1 : 0;
}

int sandbox_app_path_runtime_file_wide(const wchar_t* file_name, wchar_t* out_path, int out_capacity) {
    wchar_t exe_dir[MAX_PATH];
    if (file_name == NULL || out_path == NULL || out_capacity <= 0) return 0;
    if (!sandbox_app_path_get_exe_dir_wide(exe_dir, MAX_PATH)) return 0;
    return sandbox_app_path_join_wide(exe_dir, file_name, out_path, out_capacity);
}

int sandbox_app_path_runtime_file_utf8(const char* file_name, char* out_path, int out_capacity) {
    wchar_t wide_name[MAX_PATH];
    wchar_t wide_path[MAX_PATH];
    if (!sandbox_app_path_utf8_to_wide(file_name, wide_name, MAX_PATH)) {
        return sandbox_app_path_copy_utf8(out_path, out_capacity, file_name);
    }
    if (!sandbox_app_path_runtime_file_wide(wide_name, wide_path, MAX_PATH)) return 0;
    return sandbox_app_path_wide_to_utf8(wide_path, out_path, out_capacity);
}

int sandbox_app_path_find_from_exe_ancestors_wide(const wchar_t* relative_path, int max_parent_levels, wchar_t* out_path, int out_capacity) {
    wchar_t current_dir[MAX_PATH];
    int level;
    if (relative_path == NULL || out_path == NULL || out_capacity <= 0) return 0;
    if (!sandbox_app_path_get_exe_dir_wide(current_dir, MAX_PATH)) return 0;
    for (level = 0; level <= max_parent_levels; ++level) {
        wchar_t candidate[MAX_PATH];
        if (sandbox_app_path_join_wide(current_dir, relative_path, candidate, MAX_PATH) &&
            GetFileAttributesW(candidate) != INVALID_FILE_ATTRIBUTES) {
            return sandbox_app_path_copy_wide(out_path, out_capacity, candidate);
        }
        if (!sandbox_app_path_parent_dir_wide(current_dir)) break;
    }
    return 0;
}

int sandbox_app_path_find_from_exe_ancestors_utf8(const char* relative_path, int max_parent_levels, char* out_path, int out_capacity) {
    wchar_t wide_relative[MAX_PATH];
    wchar_t wide_path[MAX_PATH];
    if (!sandbox_app_path_utf8_to_wide(relative_path, wide_relative, MAX_PATH)) {
        return sandbox_app_path_copy_utf8(out_path, out_capacity, relative_path);
    }
    if (!sandbox_app_path_find_from_exe_ancestors_wide(wide_relative, max_parent_levels, wide_path, MAX_PATH)) return 0;
    return sandbox_app_path_wide_to_utf8(wide_path, out_path, out_capacity);
}
