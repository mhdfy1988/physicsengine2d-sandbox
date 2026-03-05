#include <stddef.h>
#include "app_runtime.h"

static int app_runtime_error_severity_from_code(int code) {
    if (code == PHYSICS_ERROR_NONE) return APP_RUNTIME_ERROR_WARNING;
    if (code == PHYSICS_ERROR_INVALID_ARGUMENT || code == PHYSICS_ERROR_CAPACITY_EXCEEDED) {
        return APP_RUNTIME_ERROR_ERROR;
    }
    return APP_RUNTIME_ERROR_WARNING;
}

void app_runtime_init(AppRuntime* runtime, AppCommandCallbacks callbacks) {
    if (runtime == 0) return;
    app_event_bus_init(&runtime->event_bus);
    app_controller_init(&runtime->controller, callbacks, &runtime->event_bus);
    runtime->last_snapshot.valid = 0;
    runtime->last_snapshot.runtime_error_count = 0;
    runtime->last_snapshot.runtime_error_code = PHYSICS_ERROR_NONE;
    runtime->last_snapshot.runtime_error_item_count = 0;
    runtime->last_snapshot.event_drop_count = 0;
    runtime->pending_error_count = 0;
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
    int tick_published;
    int state_published;
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
    runtime->last_snapshot.runtime_error_item_count = 0;
    if (runtime->pending_error_count > 0) {
        int i;
        runtime->last_snapshot.runtime_error_item_count = runtime->pending_error_count;
        runtime->last_snapshot.runtime_error_count = runtime->pending_error_count;
        runtime->last_snapshot.runtime_error_code = runtime->pending_errors[0].code;
        for (i = 0; i < runtime->pending_error_count; i++) {
            runtime->last_snapshot.runtime_errors[i] = runtime->pending_errors[i];
        }
        runtime->pending_error_count = 0;
    } else {
        runtime->last_snapshot.runtime_error_code = (engine != NULL) ? physics_engine_get_last_error(engine) : PHYSICS_ERROR_NONE;
        runtime->last_snapshot.runtime_error_count = (runtime->last_snapshot.runtime_error_code == PHYSICS_ERROR_NONE) ? 0 : 1;
        if (runtime->last_snapshot.runtime_error_count > 0) {
            runtime->last_snapshot.runtime_error_item_count = 1;
            runtime->last_snapshot.runtime_errors[0].code = runtime->last_snapshot.runtime_error_code;
            runtime->last_snapshot.runtime_errors[0].severity = app_runtime_error_severity_from_code(runtime->last_snapshot.runtime_error_code);
            runtime->last_snapshot.runtime_errors[0].count = 1;
        }
    }
    runtime->last_snapshot.step_ms = step_ms;
    runtime->last_snapshot.event_drop_count = app_event_bus_dropped_count(&runtime->event_bus);

    event_data.type = APP_EVENT_RUNTIME_TICK;
    event_data.command_type = APP_CMD_NONE;
    event_data.runtime_snapshot = runtime->last_snapshot;
    tick_published = app_event_bus_publish(&runtime->event_bus, event_data);
    if (!tick_published) {
        runtime->last_snapshot.event_drop_count = app_event_bus_dropped_count(&runtime->event_bus);
    }

    if (had_prev && prev_running != runtime->last_snapshot.running) {
        event_data.type = APP_EVENT_RUNTIME_STATE_CHANGED;
        event_data.runtime_snapshot = runtime->last_snapshot;
        state_published = app_event_bus_publish(&runtime->event_bus, event_data);
        if (!state_published) {
            runtime->last_snapshot.event_drop_count = app_event_bus_dropped_count(&runtime->event_bus);
        }
    }
}

const AppRuntimeSnapshot* app_runtime_get_last_snapshot(const AppRuntime* runtime) {
    if (runtime == 0) return 0;
    return &runtime->last_snapshot;
}

void app_runtime_set_runtime_errors(AppRuntime* runtime, const AppRuntimeErrorItem* errors, int error_count) {
    int i;
    if (runtime == 0) return;
    if (errors == 0 || error_count <= 0) {
        runtime->pending_error_count = 0;
        return;
    }
    if (error_count > APP_RUNTIME_MAX_ERRORS) error_count = APP_RUNTIME_MAX_ERRORS;
    runtime->pending_error_count = error_count;
    for (i = 0; i < error_count; i++) {
        runtime->pending_errors[i] = errors[i];
    }
}
