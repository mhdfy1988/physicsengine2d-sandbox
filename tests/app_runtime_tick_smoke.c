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
    int saw_tick = 0;

    callbacks.toggle_run = cb_toggle_run;
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

    app_runtime_report_tick(&runtime, NULL, 1, 0.25f);
    while (app_runtime_pop_event(&runtime, &ev)) {
        if (ev.type == APP_EVENT_COMMAND_EXECUTED && ev.command_type == APP_CMD_TOGGLE_RUN) {
            saw_command = 1;
        } else if (ev.type == APP_EVENT_RUNTIME_TICK) {
            saw_tick = 1;
            if (!ev.runtime_snapshot.valid || ev.runtime_snapshot.frame_index == 0 || !ev.runtime_snapshot.running) {
                printf("[FAIL] runtime tick snapshot invalid\n");
                return 3;
            }
        }
    }
    if (!saw_command || !saw_tick) {
        printf("[FAIL] expected command and runtime tick events\n");
        return 4;
    }

    {
        const AppRuntimeSnapshot* snap = app_runtime_get_last_snapshot(&runtime);
        if (snap == NULL || !snap->valid || snap->frame_index == 0) {
            printf("[FAIL] last runtime snapshot unavailable\n");
            return 5;
        }
    }

    printf("[PASS] app runtime tick smoke\n");
    return 0;
}
