#include <stddef.h>
#include "app_runtime.h"

void app_runtime_init(AppRuntime* runtime, AppCommandCallbacks callbacks) {
    if (runtime == 0) return;
    app_event_bus_init(&runtime->event_bus);
    app_controller_init(&runtime->controller, callbacks, &runtime->event_bus);
    runtime->last_snapshot.valid = 0;
    runtime->frame_index = 0;
}

int app_runtime_dispatch(AppRuntime* runtime, AppCommand command_data) {
    if (runtime == 0) return 0;
    return app_controller_dispatch(&runtime->controller, command_data);
}

int app_runtime_pop_event(AppRuntime* runtime, AppEvent* out_event) {
    if (runtime == 0) return 0;
    return app_event_bus_pop(&runtime->event_bus, out_event);
}

void app_runtime_report_tick(AppRuntime* runtime, PhysicsEngine* engine, int running, float step_ms) {
    AppEvent event_data;
    int had_prev;
    int prev_running;
    if (runtime == 0) return;
    had_prev = runtime->last_snapshot.valid;
    prev_running = runtime->last_snapshot.running;
    runtime->frame_index++;
    runtime->last_snapshot.valid = 1;
    runtime->last_snapshot.frame_index = runtime->frame_index;
    runtime->last_snapshot.running = running ? 1 : 0;
    runtime->last_snapshot.body_count = (engine != NULL) ? physics_engine_get_body_count(engine) : 0;
    runtime->last_snapshot.constraint_count = (engine != NULL) ? physics_engine_get_constraint_count(engine) : 0;
    runtime->last_snapshot.contact_count = (engine != NULL) ? physics_engine_get_contact_count(engine) : 0;
    runtime->last_snapshot.step_ms = step_ms;

    event_data.type = APP_EVENT_RUNTIME_TICK;
    event_data.command_type = APP_CMD_NONE;
    event_data.runtime_snapshot = runtime->last_snapshot;
    app_event_bus_publish(&runtime->event_bus, event_data);

    if (had_prev && prev_running != runtime->last_snapshot.running) {
        event_data.type = APP_EVENT_RUNTIME_STATE_CHANGED;
        app_event_bus_publish(&runtime->event_bus, event_data);
    }
}

const AppRuntimeSnapshot* app_runtime_get_last_snapshot(const AppRuntime* runtime) {
    if (runtime == 0) return 0;
    return &runtime->last_snapshot;
}
