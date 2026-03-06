#ifndef APP_RUNTIME_H
#define APP_RUNTIME_H

#include "app_controller.hpp"
#include "asset_hot_reload.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    AppEventBus event_bus;
    AppController controller;
    AppRuntimeSnapshot last_snapshot;
    AppHotReloadSnapshot last_hot_reload;
    AppRuntimeErrorItem pending_errors[APP_RUNTIME_MAX_ERRORS];
    int pending_error_count;
    unsigned int frame_index;
} AppRuntime;

void app_runtime_init(AppRuntime* runtime, AppCommandCallbacks callbacks);
int app_runtime_dispatch(AppRuntime* runtime, AppCommand command_data);
int app_runtime_pop_event(AppRuntime* runtime, AppEvent* out_event);
void app_runtime_report_tick(AppRuntime* runtime, PhysicsEngine* engine, int running, float step_ms);
void app_runtime_report_hot_reload(AppRuntime* runtime, const AssetHotReloadTickReport* report, int pie_active);
const AppRuntimeSnapshot* app_runtime_get_last_snapshot(const AppRuntime* runtime);
const AppHotReloadSnapshot* app_runtime_get_last_hot_reload(const AppRuntime* runtime);
void app_runtime_set_runtime_errors(AppRuntime* runtime, const AppRuntimeErrorItem* errors, int error_count);

#ifdef __cplusplus
}
#endif

#endif
