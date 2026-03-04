#ifndef PROJECT_TREE_H
#define PROJECT_TREE_H

#include <windows.h>

enum { PROJECT_TREE_MAX_ITEMS = 512, PROJECT_TREE_LINE_MAX = 180 };

void project_tree_build(const wchar_t* root_path, int max_depth);
void project_tree_clear(void);

int project_tree_count(void);
const wchar_t* project_tree_line_at(int index);

int project_tree_dir_count(void);
int project_tree_file_count(void);

#endif
