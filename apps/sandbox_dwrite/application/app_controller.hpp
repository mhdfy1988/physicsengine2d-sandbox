#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include "../domain/app_command.hpp"
#include "../infrastructure/app_event_bus.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    AppCommandCallbacks callbacks;
    AppEventBus* event_bus;
} AppController;

void app_controller_init(AppController* controller, AppCommandCallbacks callbacks, AppEventBus* event_bus);
int app_controller_dispatch(AppController* controller, AppCommand command_data);

#ifdef __cplusplus
}
#endif

#endif
