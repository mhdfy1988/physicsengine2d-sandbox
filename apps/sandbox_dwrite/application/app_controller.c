#include "app_controller.h"
#include <string.h>

void app_controller_init(AppController* controller, AppCommandCallbacks callbacks, AppEventBus* event_bus) {
    if (controller == 0) return;
    controller->callbacks = callbacks;
    controller->event_bus = event_bus;
}

int app_controller_dispatch(AppController* controller, AppCommand command_data) {
    AppEvent event_data;
    int ok;
    if (controller == 0) return 0;
    ok = app_command_execute(&command_data, &controller->callbacks);
    if (!ok) return 0;
    if (controller->event_bus != 0) {
        memset(&event_data, 0, sizeof(event_data));
        event_data.type = APP_EVENT_COMMAND_EXECUTED;
        event_data.command_type = command_data.type;
        app_event_bus_publish(controller->event_bus, event_data);
    }
    return 1;
}
