#ifndef APP_RUNTIME_H
#define APP_RUNTIME_H

#include "app_controller.h"

typedef struct {
    AppEventBus event_bus;
    AppController controller;
} AppRuntime;

void app_runtime_init(AppRuntime* runtime, AppCommandCallbacks callbacks);
int app_runtime_dispatch(AppRuntime* runtime, AppCommand command_data);
int app_runtime_pop_event(AppRuntime* runtime, AppEvent* out_event);

#endif
