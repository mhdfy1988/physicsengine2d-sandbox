#define COBJMACROS

#include <d2d1.h>

#include "ui_icons.h"

static D2D1_POINT_2F ui_pt(float x, float y) {
    D2D1_POINT_2F v = {x, y};
    return v;
}

static void ui_set_brush_color(ID2D1SolidColorBrush* brush, D2D1_COLOR_F color) {
    ID2D1SolidColorBrush_SetColor(brush, &color);
}

void ui_draw_icon_minus(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                        D2D1_RECT_F r, D2D1_COLOR_F color, float thickness) {
    float cx = (r.left + r.right) * 0.5f;
    float cy = (r.top + r.bottom) * 0.5f;
    ui_set_brush_color(brush, color);
    ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - 4.5f, cy), ui_pt(cx + 4.5f, cy), (ID2D1Brush*)brush, thickness, NULL);
}

void ui_draw_icon_plus(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                       D2D1_RECT_F r, D2D1_COLOR_F color, float thickness) {
    float cx = (r.left + r.right) * 0.5f;
    float cy = (r.top + r.bottom) * 0.5f;
    ui_set_brush_color(brush, color);
    ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - 4.5f, cy), ui_pt(cx + 4.5f, cy), (ID2D1Brush*)brush, thickness, NULL);
    ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx, cy - 4.5f), ui_pt(cx, cy + 4.5f), (ID2D1Brush*)brush, thickness, NULL);
}

void ui_draw_icon_chevron(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                          D2D1_RECT_F r, int up, D2D1_COLOR_F color, float thickness) {
    float cx = (r.left + r.right) * 0.5f;
    float cy = (r.top + r.bottom) * 0.5f;
    float w = 4.0f;
    float h = 3.0f;
    ui_set_brush_color(brush, color);
    if (up) {
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - w, cy + h * 0.5f), ui_pt(cx, cy - h), (ID2D1Brush*)brush, thickness, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx, cy - h), ui_pt(cx + w, cy + h * 0.5f), (ID2D1Brush*)brush, thickness, NULL);
    } else {
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - w, cy - h * 0.5f), ui_pt(cx, cy + h), (ID2D1Brush*)brush, thickness, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx, cy + h), ui_pt(cx + w, cy - h * 0.5f), (ID2D1Brush*)brush, thickness, NULL);
    }
}

void ui_draw_icon_chevron_lr(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                             D2D1_RECT_F r, int left, D2D1_COLOR_F color, float thickness) {
    float cx = (r.left + r.right) * 0.5f;
    float cy = (r.top + r.bottom) * 0.5f;
    float w = 3.5f;
    float h = 4.5f;
    ui_set_brush_color(brush, color);
    if (left) {
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx + w * 0.5f, cy - h), ui_pt(cx - w, cy), (ID2D1Brush*)brush, thickness, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - w, cy), ui_pt(cx + w * 0.5f, cy + h), (ID2D1Brush*)brush, thickness, NULL);
    } else {
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - w * 0.5f, cy - h), ui_pt(cx + w, cy), (ID2D1Brush*)brush, thickness, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx + w, cy), ui_pt(cx - w * 0.5f, cy + h), (ID2D1Brush*)brush, thickness, NULL);
    }
}

void ui_draw_tree_disclosure_icon(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                                  D2D1_RECT_F r, int expanded, D2D1_COLOR_F color, float thickness) {
    if (expanded) ui_draw_icon_chevron(target, brush, r, 0, color, thickness);
    else ui_draw_icon_chevron_lr(target, brush, r, 0, color, thickness);
}
