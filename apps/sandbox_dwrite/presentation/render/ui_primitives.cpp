#include <d2d1.h>

#include "ui_primitives.hpp"

static D2D1_RECT_F ui_rc(float l, float t, float r, float b) {
    D2D1_RECT_F v = {l, t, r, b};
    return v;
}

static D2D1_COLOR_F ui_rgba(float r, float g, float b, float a) {
    D2D1_COLOR_F v = {r, g, b, a};
    return v;
}

static void ui_set_brush_color(ID2D1SolidColorBrush* brush, D2D1_COLOR_F color) {
    ID2D1SolidColorBrush_SetColor(brush, &color);
}

void ui_draw_card_round(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                        D2D1_RECT_F r, float radius, D2D1_COLOR_F fill, D2D1_COLOR_F border) {
    D2D1_ROUNDED_RECT rr;
    rr.rect = r;
    rr.radiusX = radius;
    rr.radiusY = radius;
    ui_set_brush_color(brush, fill);
    ID2D1HwndRenderTarget_FillRoundedRectangle(target, &rr, (ID2D1Brush*)brush);
    ui_set_brush_color(brush, border);
    ID2D1HwndRenderTarget_DrawRoundedRectangle(target, &rr, (ID2D1Brush*)brush, 1.0f, NULL);
}

void ui_draw_panel_header_band(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                               D2D1_RECT_F panel_rect, float top_h, float inset) {
    D2D1_RECT_F hb = ui_rc(panel_rect.left + inset, panel_rect.top + 4.0f, panel_rect.right - inset, panel_rect.top + top_h + 2.0f);
    ui_draw_card_round(target, brush, hb, 7.0f, ui_rgba(0.14f, 0.15f, 0.18f, 1.0f), ui_rgba(0.24f, 0.26f, 0.31f, 1.0f));
}

void ui_draw_outer_shadow_rr(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                             D2D1_ROUNDED_RECT rr) {
    int i;
    for (i = 1; i <= 5; i++) {
        float d = (float)i;
        float a = 0.11f - d * 0.018f;
        D2D1_ROUNDED_RECT srr;
        if (a < 0.01f) a = 0.01f;
        srr.rect = ui_rc(rr.rect.left - d, rr.rect.top - d, rr.rect.right + d, rr.rect.bottom + d);
        srr.radiusX = rr.radiusX + d;
        srr.radiusY = rr.radiusY + d;
        ui_set_brush_color(brush, ui_rgba(0.05f, 0.07f, 0.10f, a));
        ID2D1HwndRenderTarget_DrawRoundedRectangle(target, &srr, (ID2D1Brush*)brush, 1.0f, NULL);
    }
}
