#ifndef CONSOLE_PERF_H
#define CONSOLE_PERF_H

#include <d2d1.h>
#include <dwrite.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { CONSOLE_PERF_HISTORY_CAP = 180 };

typedef struct {
    int fps_display;
    float physics_step_ms;
    int broadphase_use_grid;
    int perf_hist_count;
    int perf_hist_head;
    const float* fps_hist;
    const float* step_hist;
    float mouse_x;
    float mouse_y;
} ConsolePerfModel;

typedef struct {
    D2D1_RECT_F perf_diag_export_rect;
    D2D1_RECT_F perf_export_rect;
} ConsolePerfOutput;

typedef struct {
    void (*draw_action_button)(D2D1_RECT_F rect, const wchar_t* text, int active, int hover);
    void (*draw_card_round)(D2D1_RECT_F rect, float radius, D2D1_COLOR_F fill, D2D1_COLOR_F border);
    void (*draw_text)(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color);
    void (*set_brush_color)(float r, float g, float b, float a);
} ConsolePerfCallbacks;

void console_perf_render(
    ID2D1HwndRenderTarget* target,
    ID2D1SolidColorBrush* brush,
    IDWriteTextFormat* fmt_info,
    D2D1_RECT_F bottom_rect,
    const ConsolePerfModel* model,
    const ConsolePerfCallbacks* callbacks,
    ConsolePerfOutput* output);

#ifdef __cplusplus
}
#endif

#endif
