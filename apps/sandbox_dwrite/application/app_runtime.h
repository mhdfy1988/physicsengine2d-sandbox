#ifndef APP_RUNTIME_H
#define APP_RUNTIME_H

#include "app_controller.h"

typedef struct {
    AppEventBus event_bus;
    AppController controller;
    AppRuntimeSnapshot last_snapshot;
    unsigned int frame_index;
} AppRuntime;

void app_runtime_init(AppRuntime* runtime, AppCommandCallbacks callbacks);
int app_runtime_dispatch(AppRuntime* runtime, AppCommand command_data);
int app_runtime_pop_event(AppRuntime* runtime, AppEvent* out_event);
void app_runtime_report_tick(AppRuntime* runtime, PhysicsEngine* engine, int running, float step_ms);
const AppRuntimeSnapshot* app_runtime_get_last_snapshot(const AppRuntime* runtime);

#endif
