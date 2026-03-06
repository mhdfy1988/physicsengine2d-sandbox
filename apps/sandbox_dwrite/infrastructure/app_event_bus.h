#ifndef APP_EVENT_BUS_H
#define APP_EVENT_BUS_H

#include "../domain/app_command.h"
#include "physics.h"
#include "asset_database.h"

#ifdef __cplusplus
extern "C" {
#endif

enum { APP_RUNTIME_MAX_ERRORS = 4 };

typedef enum {
    APP_RUNTIME_ERROR_WARNING = 1,
    APP_RUNTIME_ERROR_ERROR = 2
} AppRuntimeErrorSeverity;

typedef enum {
    APP_RUNTIME_ERROR_CODE_NONE = 0,
    APP_RUNTIME_ERROR_CODE_ENGINE_INVALID_ARGUMENT = PHYSICS_ERROR_INVALID_ARGUMENT,
    APP_RUNTIME_ERROR_CODE_ENGINE_API_VERSION_MISMATCH = PHYSICS_ERROR_API_VERSION_MISMATCH,
    APP_RUNTIME_ERROR_CODE_ENGINE_PLUGIN_INIT_FAILED = PHYSICS_ERROR_PLUGIN_INIT_FAILED,
    APP_RUNTIME_ERROR_CODE_ENGINE_CAPACITY_EXCEEDED = PHYSICS_ERROR_CAPACITY_EXCEEDED,
    APP_RUNTIME_ERROR_CODE_BRIDGE_MISSING_REVERSE = 1001,
    APP_RUNTIME_ERROR_CODE_BRIDGE_STALE_ENTITY = 1002,
    APP_RUNTIME_ERROR_CODE_BRIDGE_NULL_BODY = 1003,
    APP_RUNTIME_ERROR_CODE_BRIDGE_DUPLICATE_BODY = 1004,
    APP_RUNTIME_ERROR_CODE_BRIDGE_REFCOUNT_MISMATCH = 1005,
    APP_RUNTIME_ERROR_CODE_PIPELINE_MAPPING_ERRORS = 1006,
    APP_RUNTIME_ERROR_CODE_HOT_RELOAD_SCAN_FAILED = 1101,
    APP_RUNTIME_ERROR_CODE_HOT_RELOAD_IMPORT_FAILED = 1102,
    APP_RUNTIME_ERROR_CODE_HOT_RELOAD_BATCH_FAILED = 1103
} AppRuntimeErrorCode;

typedef struct {
    int code;
    int severity;
    int count;
} AppRuntimeErrorItem;

typedef struct {
    int valid;
    unsigned int frame_index;
    int running;
    int body_count;
    int constraint_count;
    int contact_count;
    int runtime_error_count;
    int runtime_error_code;
    int runtime_error_item_count;
    AppRuntimeErrorItem runtime_errors[APP_RUNTIME_MAX_ERRORS];
    float step_ms;
    unsigned int event_drop_count;
} AppRuntimeSnapshot;

enum { APP_HOT_RELOAD_MAX_IMPORTED = 16 };

typedef struct {
    int valid;
    int ready_batch_count;
    int affected_count;
    int imported_count;
    int failed_count;
    int pie_active;
    int rollback_retained;
    int imported_guid_count;
    char imported_guids[APP_HOT_RELOAD_MAX_IMPORTED][ASSET_DB_MAX_GUID];
} AppHotReloadSnapshot;

typedef enum {
    APP_EVENT_NONE = 0,
    APP_EVENT_COMMAND_EXECUTED,
    APP_EVENT_RUNTIME_TICK,
    APP_EVENT_RUNTIME_STATE_CHANGED,
    APP_EVENT_HOT_RELOAD_BATCH
} AppEventType;

typedef struct {
    AppEventType type;
    AppCommandType command_type;
    AppRuntimeSnapshot runtime_snapshot;
    AppHotReloadSnapshot hot_reload_snapshot;
} AppEvent;

enum { APP_EVENT_QUEUE_CAP = 128 };

typedef struct {
    AppEvent queue[APP_EVENT_QUEUE_CAP];
    int head;
    int count;
    unsigned int dropped_count;
} AppEventBus;

void app_event_bus_init(AppEventBus* bus);
int app_event_bus_publish(AppEventBus* bus, AppEvent event_data);
int app_event_bus_pop(AppEventBus* bus, AppEvent* out_event);
unsigned int app_event_bus_dropped_count(const AppEventBus* bus);

#ifdef __cplusplus
}
#endif

#endif
