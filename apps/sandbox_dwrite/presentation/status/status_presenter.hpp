#ifndef STATUS_PRESENTER_H
#define STATUS_PRESENTER_H

#include <windows.h>
#include <d2d1.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int body_count;
    int constraint_count;
    int contact_count;
    int recycled_count;
    const wchar_t* status_user;
    const wchar_t* startup_status_text;
    int startup_degraded;
    int hot_imported;
    int hot_failed;
    unsigned int hot_reload_last_event_ms;
    unsigned int now_ms;
    int runtime_bus_congested;
    unsigned int runtime_event_drop_count;
} StatusPresenterModel;

void status_presenter_init_context(wchar_t* out_project_path,
                                   int project_path_capacity,
                                   wchar_t* out_user,
                                   int user_capacity,
                                   wchar_t* out_startup_status,
                                   int startup_status_capacity,
                                   int* out_startup_degraded);
void status_presenter_format_counts(const StatusPresenterModel* model, wchar_t* out_text, int out_capacity);
void status_presenter_format_meta(const StatusPresenterModel* model, wchar_t* out_text, int out_capacity);
void status_presenter_format_startup(const StatusPresenterModel* model, wchar_t* out_text, int out_capacity);
void status_presenter_format_hot_reload(const StatusPresenterModel* model, wchar_t* out_text, int out_capacity, D2D1_COLOR_F* out_color);
void status_presenter_format_bus(const StatusPresenterModel* model, wchar_t* out_text, int out_capacity, D2D1_COLOR_F* out_color);

#ifdef __cplusplus
}
#endif

#endif
