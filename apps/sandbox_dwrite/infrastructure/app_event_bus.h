#ifndef APP_EVENT_BUS_H
#define APP_EVENT_BUS_H

#include "../domain/app_command.h"
#include "physics.h"

typedef struct {
    int valid;
    unsigned int frame_index;
    int running;
    int body_count;
    int constraint_count;
    int contact_count;
    float step_ms;
    unsigned int event_drop_count;
} AppRuntimeSnapshot;

typedef enum {
    APP_EVENT_NONE = 0,
    APP_EVENT_COMMAND_EXECUTED,
    APP_EVENT_RUNTIME_TICK,
    APP_EVENT_RUNTIME_STATE_CHANGED
} AppEventType;

typedef struct {
    AppEventType type;
    AppCommandType command_type;
    AppRuntimeSnapshot runtime_snapshot;
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

#endif
