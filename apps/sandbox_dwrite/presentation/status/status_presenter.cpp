#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include "status_presenter.hpp"

#include <stdio.h>
#include <string.h>

static D2D1_COLOR_F make_color(float r, float g, float b, float a) {
    D2D1_COLOR_F color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}

static void status_presenter_clear(wchar_t* out_text, int out_capacity) {
    if (out_text == NULL || out_capacity <= 0) return;
    out_text[0] = L'\0';
}

void status_presenter_init_context(wchar_t* out_project_path,
                                   int project_path_capacity,
                                   wchar_t* out_user,
                                   int user_capacity,
                                   wchar_t* out_startup_status,
                                   int startup_status_capacity,
                                   int* out_startup_degraded) {
    DWORD uname_len = (DWORD)(user_capacity > 0 ? (user_capacity - 1) : 0);
    status_presenter_clear(out_project_path, project_path_capacity);
    status_presenter_clear(out_user, user_capacity);
    status_presenter_clear(out_startup_status, startup_status_capacity);
    if (out_startup_degraded != NULL) *out_startup_degraded = 0;
    if (out_project_path != NULL && project_path_capacity > 0) {
        GetCurrentDirectoryW((DWORD)project_path_capacity, out_project_path);
    }
    if (out_user != NULL && user_capacity > 0) {
        if (!GetUserNameW(out_user, &uname_len)) out_user[0] = L'\0';
    }
}

void status_presenter_format_counts(const StatusPresenterModel* model, wchar_t* out_text, int out_capacity) {
    if (model == NULL || out_text == NULL || out_capacity <= 0) return;
    swprintf(out_text, (size_t)out_capacity, L"对象:%d  约束:%d  接触:%d  回收:%d",
             model->body_count, model->constraint_count, model->contact_count, model->recycled_count);
}

void status_presenter_format_meta(const StatusPresenterModel* model, wchar_t* out_text, int out_capacity) {
    const wchar_t* user = (model != NULL && model->status_user != NULL && model->status_user[0] != L'\0')
                              ? model->status_user
                              : L"unknown";
    if (out_text == NULL || out_capacity <= 0) return;
    swprintf(out_text, (size_t)out_capacity, L"用户:%ls | v0.3", user);
}

void status_presenter_format_startup(const StatusPresenterModel* model, wchar_t* out_text, int out_capacity) {
    const wchar_t* status = (model != NULL && model->startup_status_text != NULL && model->startup_status_text[0] != L'\0')
                                ? model->startup_status_text
                                : L"启动:未知";
    if (out_text == NULL || out_capacity <= 0) return;
    swprintf(out_text, (size_t)out_capacity, L"%ls", status);
}

void status_presenter_format_hot_reload(const StatusPresenterModel* model, wchar_t* out_text, int out_capacity, D2D1_COLOR_F* out_color) {
    if (out_text == NULL || out_capacity <= 0 || model == NULL) return;
    if (out_color != NULL) *out_color = make_color(0.63f, 0.72f, 0.84f, 1.0f);
    if (model->hot_failed > 0) {
        if (out_color != NULL) *out_color = make_color(0.98f, 0.47f, 0.43f, 1.0f);
    } else if (model->hot_imported > 0 && model->hot_reload_last_event_ms > 0 &&
               (model->now_ms - model->hot_reload_last_event_ms) <= 3000) {
        if (out_color != NULL) *out_color = make_color(0.68f, 0.90f, 0.70f, 1.0f);
    }
    swprintf(out_text, (size_t)out_capacity, L"热重载:+%d/-%d", model->hot_imported, model->hot_failed);
}

void status_presenter_format_bus(const StatusPresenterModel* model, wchar_t* out_text, int out_capacity, D2D1_COLOR_F* out_color) {
    if (out_text == NULL || out_capacity <= 0 || model == NULL) return;
    if (out_color != NULL) *out_color = make_color(0.66f, 0.88f, 0.66f, 1.0f);
    if (model->runtime_bus_congested) {
        if (out_color != NULL) {
            *out_color = (model->runtime_event_drop_count >= 100)
                             ? make_color(0.98f, 0.42f, 0.38f, 1.0f)
                             : make_color(0.97f, 0.78f, 0.42f, 1.0f);
        }
        swprintf(out_text, (size_t)out_capacity, L"事件总线:拥塞(%u)", model->runtime_event_drop_count);
        return;
    }
    swprintf(out_text, (size_t)out_capacity, L"事件总线:健康");
}
