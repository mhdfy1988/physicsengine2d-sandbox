#include "session_recovery.hpp"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {

void copy_text(char* out, int out_cap, const char* src) {
    if (out == nullptr || out_cap <= 0) return;
    if (src == nullptr) {
        out[0] = '\0';
        return;
    }
    strncpy(out, src, (size_t)(out_cap - 1));
    out[out_cap - 1] = '\0';
}

void trim_line(char* line) {
    size_t n;
    if (line == nullptr) return;
    n = strlen(line);
    while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r')) {
        line[n - 1] = '\0';
        n--;
    }
}

int split_pipe(char* line, char* tokens[], int max_tokens) {
    int count = 0;
    char* p = line;
    if (line == nullptr || tokens == nullptr || max_tokens <= 0) return 0;
    tokens[count++] = p;
    while (*p != '\0') {
        if (*p == '|') {
            *p = '\0';
            if (count < max_tokens) tokens[count++] = p + 1;
        }
        ++p;
    }
    return count;
}

int parse_int(const char* text, int* out_value) {
    char* endptr = nullptr;
    long value;
    if (text == nullptr || out_value == nullptr) return 0;
    errno = 0;
    value = strtol(text, &endptr, 10);
    if (errno != 0 || endptr == text || (endptr != nullptr && *endptr != '\0')) return 0;
    if (value < -2147483647L - 1L || value > 2147483647L) return 0;
    *out_value = (int)value;
    return 1;
}

}  // namespace

void session_recovery_state_init(SessionRecoveryState* state) {
    if (state == nullptr) return;
    memset(state, 0, sizeof(*state));
    state->schema_version = SESSION_RECOVERY_SCHEMA_VERSION_V1;
    state->reopen_last_workspace = 1;
    copy_text(state->workspace_manifest_path, SESSION_RECOVERY_MAX_PATH, "ProjectSettings/workspace.physicsworkspace");
    copy_text(state->last_snapshot_path, SESSION_RECOVERY_MAX_PATH, "autosave_snapshot.txt");
    copy_text(state->replay_capture_path, SESSION_RECOVERY_MAX_PATH, "autosave_snapshot.txt");
    copy_text(state->last_scene_guid, SESSION_RECOVERY_MAX_SCENE_GUID, "scene://default");
    copy_text(state->recent_action, SESSION_RECOVERY_MAX_ACTION, "startup");
}

int session_recovery_state_load_v1(const char* path, SessionRecoveryState* out_state) {
    FILE* fp;
    char line[512];
    SessionRecoveryState state;
    if (path == nullptr || out_state == nullptr) return 0;
    fp = fopen(path, "r");
    if (fp == nullptr) return 0;
    session_recovery_state_init(&state);
    while (fgets(line, sizeof(line), fp) != nullptr) {
        char* tokens[8];
        int token_count;
        trim_line(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        token_count = split_pipe(line, tokens, 8);
        if (token_count <= 0) {
            fclose(fp);
            return 0;
        }
        if (strcmp(tokens[0], "schema_version") == 0) {
            if (token_count != 2 || !parse_int(tokens[1], &state.schema_version) ||
                state.schema_version != SESSION_RECOVERY_SCHEMA_VERSION_V1) {
                fclose(fp);
                return 0;
            }
        } else if (strcmp(tokens[0], "reopen_last_workspace") == 0) {
            if (token_count != 2 || !parse_int(tokens[1], &state.reopen_last_workspace)) {
                fclose(fp);
                return 0;
            }
            state.reopen_last_workspace = state.reopen_last_workspace ? 1 : 0;
        } else if (strcmp(tokens[0], "unclean_shutdown") == 0) {
            if (token_count != 2 || !parse_int(tokens[1], &state.unclean_shutdown)) {
                fclose(fp);
                return 0;
            }
            state.unclean_shutdown = state.unclean_shutdown ? 1 : 0;
        } else if (strcmp(tokens[0], "workspace_manifest_path") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(state.workspace_manifest_path, SESSION_RECOVERY_MAX_PATH, tokens[1]);
        } else if (strcmp(tokens[0], "last_snapshot_path") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(state.last_snapshot_path, SESSION_RECOVERY_MAX_PATH, tokens[1]);
        } else if (strcmp(tokens[0], "replay_capture_path") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(state.replay_capture_path, SESSION_RECOVERY_MAX_PATH, tokens[1]);
        } else if (strcmp(tokens[0], "last_scene_guid") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(state.last_scene_guid, SESSION_RECOVERY_MAX_SCENE_GUID, tokens[1]);
        } else if (strcmp(tokens[0], "recent_action") == 0) {
            if (token_count != 2) {
                fclose(fp);
                return 0;
            }
            copy_text(state.recent_action, SESSION_RECOVERY_MAX_ACTION, tokens[1]);
        } else {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    *out_state = state;
    return 1;
}

int session_recovery_state_save_v1(const SessionRecoveryState* state, const char* path) {
    FILE* fp;
    if (state == nullptr || path == nullptr) return 0;
    fp = fopen(path, "w");
    if (fp == nullptr) return 0;
    fprintf(fp, "schema_version|%d\n", SESSION_RECOVERY_SCHEMA_VERSION_V1);
    fprintf(fp, "reopen_last_workspace|%d\n", state->reopen_last_workspace ? 1 : 0);
    fprintf(fp, "unclean_shutdown|%d\n", state->unclean_shutdown ? 1 : 0);
    fprintf(fp, "workspace_manifest_path|%s\n", state->workspace_manifest_path);
    fprintf(fp, "last_snapshot_path|%s\n", state->last_snapshot_path);
    fprintf(fp, "replay_capture_path|%s\n", state->replay_capture_path);
    fprintf(fp, "last_scene_guid|%s\n", state->last_scene_guid);
    fprintf(fp, "recent_action|%s\n", state->recent_action);
    return fclose(fp) == 0;
}

void session_recovery_mark_unclean(SessionRecoveryState* state,
                                   const char* snapshot_path,
                                   const char* replay_capture_path,
                                   const char* last_scene_guid,
                                   const char* recent_action) {
    if (state == nullptr) return;
    state->unclean_shutdown = 1;
    if (snapshot_path != nullptr) copy_text(state->last_snapshot_path, SESSION_RECOVERY_MAX_PATH, snapshot_path);
    if (replay_capture_path != nullptr) copy_text(state->replay_capture_path, SESSION_RECOVERY_MAX_PATH, replay_capture_path);
    if (last_scene_guid != nullptr) copy_text(state->last_scene_guid, SESSION_RECOVERY_MAX_SCENE_GUID, last_scene_guid);
    if (recent_action != nullptr) copy_text(state->recent_action, SESSION_RECOVERY_MAX_ACTION, recent_action);
}

void session_recovery_mark_clean(SessionRecoveryState* state, const char* recent_action) {
    if (state == nullptr) return;
    state->unclean_shutdown = 0;
    if (recent_action != nullptr) copy_text(state->recent_action, SESSION_RECOVERY_MAX_ACTION, recent_action);
}

int session_recovery_should_restore(const SessionRecoveryState* state) {
    if (state == nullptr) return 0;
    return state->reopen_last_workspace && state->unclean_shutdown && state->last_snapshot_path[0] != '\0';
}
