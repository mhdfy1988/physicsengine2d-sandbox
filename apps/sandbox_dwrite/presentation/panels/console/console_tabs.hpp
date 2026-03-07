#ifndef CONSOLE_TABS_H
#define CONSOLE_TABS_H

#include <d2d1.h>
#include <dwrite.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int show_bottom_panel;
    int bottom_panel_collapsed;
    int bottom_active_tab;
    float mouse_x;
    float mouse_y;
} ConsoleTabsModel;

typedef struct {
    D2D1_RECT_F bottom_fold_rect;
    D2D1_RECT_F tab_console_rect;
    D2D1_RECT_F tab_perf_rect;
} ConsoleTabsOutput;

typedef struct {
    void (*draw_text_hvcenter)(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color);
    void (*draw_card_round)(D2D1_RECT_F rect, float radius, D2D1_COLOR_F fill, D2D1_COLOR_F border);
    void (*draw_icon_chevron)(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush, D2D1_RECT_F rect, int direction, D2D1_COLOR_F color, float stroke_width);
} ConsoleTabsCallbacks;

void console_tabs_render(
    ID2D1HwndRenderTarget* target,
    ID2D1SolidColorBrush* brush,
    IDWriteTextFormat* fmt_info,
    D2D1_RECT_F bottom_rect,
    const ConsoleTabsModel* model,
    const ConsoleTabsCallbacks* callbacks,
    ConsoleTabsOutput* output);

#ifdef __cplusplus
}
#endif

#endif
