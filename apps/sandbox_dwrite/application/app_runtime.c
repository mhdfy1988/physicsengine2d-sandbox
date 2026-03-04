#include "app_runtime.h"

void app_runtime_init(AppRuntime* runtime, AppCommandCallbacks callbacks) {
    if (runtime == 0) return;
    app_event_bus_init(&runtime->event_bus);
    app_controller_init(&runtime->controller, callbacks, &runtime->event_bus);
}

int app_runtime_dispatch(AppRuntime* runtime, AppCommand command_data) {
    if (runtime == 0) return 0;
    return app_controller_dispatch(&runtime->controller, command_data);
}

int app_runtime_pop_event(AppRuntime* runtime, AppEvent* out_event) {
    if (runtime == 0) return 0;
    return app_event_bus_pop(&runtime->event_bus, out_event);
}
