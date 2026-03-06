#ifndef SESSION_RECOVERY_H
#define SESSION_RECOVERY_H

enum {
    SESSION_RECOVERY_SCHEMA_VERSION_V1 = 1,
    SESSION_RECOVERY_MAX_PATH = 260,
    SESSION_RECOVERY_MAX_SCENE_GUID = 96,
    SESSION_RECOVERY_MAX_ACTION = 128
};

typedef struct {
    int schema_version;
    int reopen_last_workspace;
    int unclean_shutdown;
    char workspace_manifest_path[SESSION_RECOVERY_MAX_PATH];
    char last_snapshot_path[SESSION_RECOVERY_MAX_PATH];
    char replay_capture_path[SESSION_RECOVERY_MAX_PATH];
    char last_scene_guid[SESSION_RECOVERY_MAX_SCENE_GUID];
    char recent_action[SESSION_RECOVERY_MAX_ACTION];
} SessionRecoveryState;

void session_recovery_state_init(SessionRecoveryState* state);
int session_recovery_state_load_v1(const char* path, SessionRecoveryState* out_state);
int session_recovery_state_save_v1(const SessionRecoveryState* state, const char* path);
void session_recovery_mark_unclean(SessionRecoveryState* state,
                                   const char* snapshot_path,
                                   const char* replay_capture_path,
                                   const char* last_scene_guid,
                                   const char* recent_action);
void session_recovery_mark_clean(SessionRecoveryState* state, const char* recent_action);
int session_recovery_should_restore(const SessionRecoveryState* state);

#endif
