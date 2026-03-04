#ifndef APP_EVENT_BUS_H
#define APP_EVENT_BUS_H

#include "../domain/app_command.h"

typedef enum {
    APP_EVENT_NONE = 0,
    APP_EVENT_COMMAND_EXECUTED
} AppEventType;

typedef struct {
    AppEventType type;
    AppCommandType command_type;
} AppEvent;

enum { APP_EVENT_QUEUE_CAP = 128 };

typedef struct {
    AppEvent queue[APP_EVENT_QUEUE_CAP];
    int head;
    int count;
} AppEventBus;

void app_event_bus_init(AppEventBus* bus);
int app_event_bus_publish(AppEventBus* bus, AppEvent event_data);
int app_event_bus_pop(AppEventBus* bus, AppEvent* out_event);

#endif
