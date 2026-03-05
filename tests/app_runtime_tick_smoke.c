#include <stdio.h>
#include "../apps/sandbox_dwrite/application/app_runtime.h"

static int g_toggle_called = 0;

static void cb_toggle_run(void* user) {
    int* p = (int*)user;
    if (p != NULL) {
        (*p)++;
    }
    g_toggle_called++;
}

static void cb_noop(void* user) {
    (void)user;
}

int main(void) {
    AppRuntime runtime;
    AppCommandCallbacks callbacks;
    AppCommand cmd;
    AppEvent ev;
    int saw_command = 0;
    int tick_count = 0;
    unsigned int prev_frame_index = 0;
    int state_change_count = 0;
    int saw_running_state = 0;
    int saw_paused_state = 0;

    callbacks.toggle_run = cb_toggle_run;
    callbacks.step_once = cb_noop;
    callbacks.reset_scene = cb_noop;
    callbacks.spawn_circle = cb_noop;
    callbacks.spawn_box = cb_noop;
    callbacks.user = &g_toggle_called;
    app_runtime_init(&runtime, callbacks);

    cmd.type = APP_CMD_TOGGLE_RUN;
    if (!app_runtime_dispatch(&runtime, cmd)) {
        printf("[FAIL] app_runtime_dispatch failed\n");
        return 1;
    }
    if (g_toggle_called <= 0) {
        printf("[FAIL] toggle callback not invoked\n");
        return 2;
    }

    app_runtime_report_tick(&runtime, NULL, 0, 0.25f);
    app_runtime_report_tick(&runtime, NULL, 1, 0.20f);
    app_runtime_report_tick(&runtime, NULL, 0, 0.18f);
    while (app_runtime_pop_event(&runtime, &ev)) {
        if (ev.type == APP_EVENT_COMMAND_EXECUTED && ev.command_type == APP_CMD_TOGGLE_RUN) {
            saw_command = 1;
        } else if (ev.type == APP_EVENT_RUNTIME_TICK) {
            tick_count++;
            if (!ev.runtime_snapshot.valid || ev.runtime_snapshot.frame_index == 0) {
                printf("[FAIL] runtime tick snapshot invalid\n");
                return 3;
            }
            if (ev.runtime_snapshot.runtime_error_count != 0 || ev.runtime_snapshot.runtime_error_code != PHYSICS_ERROR_NONE) {
                printf("[FAIL] unexpected runtime error fields in null-engine tick\n");
                return 8;
            }
            if (ev.runtime_snapshot.runtime_error_item_count != 0) {
                printf("[FAIL] unexpected runtime error item list in null-engine tick\n");
                return 10;
            }
            if (ev.runtime_snapshot.frame_index <= prev_frame_index) {
                printf("[FAIL] runtime tick frame index not increasing\n");
                return 6;
            }
            prev_frame_index = ev.runtime_snapshot.frame_index;
        } else if (ev.type == APP_EVENT_RUNTIME_STATE_CHANGED) {
            state_change_count++;
            if (ev.runtime_snapshot.running) saw_running_state = 1;
            if (!ev.runtime_snapshot.running) saw_paused_state = 1;
        }
    }
    if (!saw_command || tick_count != 3 || state_change_count != 2 || !saw_running_state || !saw_paused_state) {
        printf("[FAIL] expected command/tick/state-change event sequence\n");
        return 4;
    }

    {
        const AppRuntimeSnapshot* snap = app_runtime_get_last_snapshot(&runtime);
        if (snap == NULL || !snap->valid || snap->frame_index == 0) {
            printf("[FAIL] last runtime snapshot unavailable\n");
            return 5;
        }
        if (snap->runtime_error_count != 0 || snap->runtime_error_code != PHYSICS_ERROR_NONE) {
            printf("[FAIL] last runtime snapshot error fields unexpected\n");
            return 9;
        }
        if (snap->runtime_error_item_count != 0) {
            printf("[FAIL] last runtime snapshot error item list unexpected\n");
            return 11;
        }
    }
    {
        int i;
        const AppRuntimeSnapshot* snap;
        for (i = 0; i < 300; i++) {
            app_runtime_report_tick(&runtime, NULL, (i & 1) ? 1 : 0, 0.1f);
        }
        snap = app_runtime_get_last_snapshot(&runtime);
        if (snap == NULL || snap->event_drop_count == 0) {
            printf("[FAIL] runtime event drop counter unavailable\n");
            return 7;
        }
    }
    {
        AppRuntime runtime2;
        AppCommandCallbacks callbacks2;
        AppRuntimeErrorItem errs[2];
        AppEvent ev2;
        int found_tick = 0;
        callbacks2.toggle_run = cb_noop;
        callbacks2.step_once = cb_noop;
        callbacks2.reset_scene = cb_noop;
        callbacks2.spawn_circle = cb_noop;
        callbacks2.spawn_box = cb_noop;
        callbacks2.user = 0;
        app_runtime_init(&runtime2, callbacks2);
        errs[0].code = PHYSICS_ERROR_INVALID_ARGUMENT;
        errs[0].severity = APP_RUNTIME_ERROR_ERROR;
        errs[0].count = 2;
        errs[1].code = PHYSICS_ERROR_CAPACITY_EXCEEDED;
        errs[1].severity = APP_RUNTIME_ERROR_WARNING;
        errs[1].count = 5;
        app_runtime_set_runtime_errors(&runtime2, errs, 2);
        app_runtime_report_tick(&runtime2, NULL, 0, 0.3f);
        while (app_runtime_pop_event(&runtime2, &ev2)) {
            if (ev2.type == APP_EVENT_RUNTIME_TICK) {
                found_tick = 1;
                if (ev2.runtime_snapshot.runtime_error_item_count != 2 ||
                    ev2.runtime_snapshot.runtime_error_count != 2 ||
                    ev2.runtime_snapshot.runtime_errors[0].code != PHYSICS_ERROR_INVALID_ARGUMENT ||
                    ev2.runtime_snapshot.runtime_errors[1].code != PHYSICS_ERROR_CAPACITY_EXCEEDED) {
                    printf("[FAIL] injected runtime error list not propagated\n");
                    return 12;
                }
            }
        }
        if (!found_tick) {
            printf("[FAIL] no runtime tick after injected errors\n");
            return 13;
        }
    }

    printf("[PASS] app runtime tick smoke\n");
    return 0;
}
