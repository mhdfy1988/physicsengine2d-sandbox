#ifndef PIE_LIFECYCLE_H
#define PIE_LIFECYCLE_H

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { PIE_SNAPSHOT_PATH_CAP = 260 };

typedef struct {
    int active;
    char snapshot_path[PIE_SNAPSHOT_PATH_CAP];
} PieLifecycle;

typedef struct {
    int (*save_snapshot)(const char* path, void* user);
    int (*load_snapshot)(const char* path, void* user);
    void (*log_text)(const wchar_t* text, void* user);
    void* user;
} PieLifecycleOps;

void pie_lifecycle_init(PieLifecycle* lifecycle, const char* snapshot_path);
int pie_lifecycle_enter(PieLifecycle* lifecycle, const PieLifecycleOps* ops);
int pie_lifecycle_exit(PieLifecycle* lifecycle, const PieLifecycleOps* ops);

#ifdef __cplusplus
}
#endif

#endif
