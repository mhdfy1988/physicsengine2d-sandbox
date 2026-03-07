#ifndef SANDBOX_APP_PATH_H
#define SANDBOX_APP_PATH_H

#ifdef __cplusplus
extern "C" {
#endif

int sandbox_app_path_runtime_file_wide(const wchar_t* file_name, wchar_t* out_path, int out_capacity);
int sandbox_app_path_runtime_file_utf8(const char* file_name, char* out_path, int out_capacity);
int sandbox_app_path_find_from_exe_ancestors_wide(const wchar_t* relative_path, int max_parent_levels, wchar_t* out_path, int out_capacity);
int sandbox_app_path_find_from_exe_ancestors_utf8(const char* relative_path, int max_parent_levels, char* out_path, int out_capacity);

#ifdef __cplusplus
}
#endif

#endif
