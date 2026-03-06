#ifndef HISTORY_SERVICE_H
#define HISTORY_SERVICE_H

#include <stddef.h>

typedef struct {
    int cursor;
    int top;
    int count;
    int replaying;
} HistoryState;

typedef struct {
    int max_slots;
    int (*save_snapshot)(const char* path, void* user);
    int (*load_snapshot)(const char* path, void* user);
    void (*log_text)(const wchar_t* text, void* user);
    void* user;
} HistoryServiceOps;

void history_service_push_snapshot(HistoryState* state, const HistoryServiceOps* ops);
void history_service_reset_and_capture(HistoryState* state, const HistoryServiceOps* ops);
void history_service_undo(HistoryState* state, const HistoryServiceOps* ops);
void history_service_redo(HistoryState* state, const HistoryServiceOps* ops);

#endif
