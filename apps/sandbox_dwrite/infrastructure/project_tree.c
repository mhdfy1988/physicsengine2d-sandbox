#define COBJMACROS

#include <windows.h>
#include <wchar.h>
#include <stdio.h>

#include "project_tree.h"

static wchar_t g_project_tree_items[PROJECT_TREE_MAX_ITEMS][PROJECT_TREE_LINE_MAX];
static int g_project_tree_count;
static int g_project_tree_dir_count;
static int g_project_tree_file_count;

static int project_tree_append_line(const wchar_t* line) {
    if (line == NULL) return 0;
    if (g_project_tree_count >= PROJECT_TREE_MAX_ITEMS) return 0;
    lstrcpynW(g_project_tree_items[g_project_tree_count], line, PROJECT_TREE_LINE_MAX);
    g_project_tree_count++;
    return 1;
}

static void project_tree_make_indent(int depth, wchar_t* out, int cap) {
    int i;
    int n = 0;
    if (out == NULL || cap <= 0) return;
    for (i = 0; i < depth * 2 && n < cap - 1; i++) {
        out[n++] = L' ';
    }
    out[n] = L'\0';
}

static int project_tree_should_descend(const wchar_t* name, int depth, int max_depth) {
    if (name == NULL) return 0;
    if (depth >= max_depth) return 0;
    if (_wcsicmp(name, L".git") == 0) return 0;
    if (_wcsicmp(name, L"backups") == 0) return 0;
    if (_wcsicmp(name, L"bin") == 0) return 0;
    if (_wcsicmp(name, L"obj") == 0) return 0;
    if (_wcsicmp(name, L"dist") == 0) return 0;
    return 1;
}

static void project_tree_add_entry_line(int depth, int is_dir, const wchar_t* name) {
    wchar_t indent[64];
    wchar_t line[PROJECT_TREE_LINE_MAX];
    if (name == NULL || name[0] == L'\0') return;
    project_tree_make_indent(depth, indent, 64);
    if (is_dir) {
        swprintf(line, PROJECT_TREE_LINE_MAX, L"%ls[D] %ls/", indent, name);
        g_project_tree_dir_count++;
    } else {
        swprintf(line, PROJECT_TREE_LINE_MAX, L"%ls%ls", indent, name);
        g_project_tree_file_count++;
    }
    project_tree_append_line(line);
}

static void project_tree_build_recursive(const wchar_t* base_path, int depth, int max_depth) {
    int pass;
    if (base_path == NULL) return;
    for (pass = 0; pass < 2 && g_project_tree_count < PROJECT_TREE_MAX_ITEMS; pass++) {
        wchar_t pattern[520];
        WIN32_FIND_DATAW fd;
        HANDLE hFind;
        swprintf(pattern, 520, L"%ls\\*", base_path);
        hFind = FindFirstFileW(pattern, &fd);
        if (hFind == INVALID_HANDLE_VALUE) continue;
        do {
            int is_dir;
            wchar_t child_path[520];
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;
            is_dir = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
            if (pass == 0 && !is_dir) continue;
            if (pass == 1 && is_dir) continue;
            project_tree_add_entry_line(depth, is_dir, fd.cFileName);
            if (g_project_tree_count >= PROJECT_TREE_MAX_ITEMS) break;
            if (is_dir && project_tree_should_descend(fd.cFileName, depth, max_depth)) {
                swprintf(child_path, 520, L"%ls\\%ls", base_path, fd.cFileName);
                project_tree_build_recursive(child_path, depth + 1, max_depth);
            }
        } while (FindNextFileW(hFind, &fd) && g_project_tree_count < PROJECT_TREE_MAX_ITEMS);
        FindClose(hFind);
    }
}

void project_tree_build(const wchar_t* root_path, int max_depth) {
    project_tree_clear();
    project_tree_build_recursive(root_path != NULL ? root_path : L".", 0, max_depth);
    if (g_project_tree_count <= 0) {
        project_tree_append_line(L"(空)");
    }
}

void project_tree_clear(void) {
    g_project_tree_count = 0;
    g_project_tree_dir_count = 0;
    g_project_tree_file_count = 0;
}

int project_tree_count(void) {
    return g_project_tree_count;
}

const wchar_t* project_tree_line_at(int index) {
    if (index < 0 || index >= g_project_tree_count) return L"";
    return g_project_tree_items[index];
}

int project_tree_dir_count(void) {
    return g_project_tree_dir_count;
}

int project_tree_file_count(void) {
    return g_project_tree_file_count;
}
