#include "app_event_bus.h"

void app_event_bus_init(AppEventBus* bus) {
    if (bus == 0) return;
    bus->head = 0;
    bus->count = 0;
}

int app_event_bus_publish(AppEventBus* bus, AppEvent event_data) {
    int slot;
    if (bus == 0) return 0;
    if (bus->count >= APP_EVENT_QUEUE_CAP) return 0;
    slot = (bus->head + bus->count) % APP_EVENT_QUEUE_CAP;
    bus->queue[slot] = event_data;
    bus->count++;
    return 1;
}

int app_event_bus_pop(AppEventBus* bus, AppEvent* out_event) {
    if (bus == 0 || out_event == 0) return 0;
    if (bus->count <= 0) return 0;
    *out_event = bus->queue[bus->head];
    bus->head = (bus->head + 1) % APP_EVENT_QUEUE_CAP;
    bus->count--;
    return 1;
}
