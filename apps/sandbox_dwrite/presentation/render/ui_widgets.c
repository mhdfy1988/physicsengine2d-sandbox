#define COBJMACROS

#include <d2d1.h>
#include <dwrite.h>

#include "ui_text.h"
#include "ui_widgets.h"

static D2D1_RECT_F ui_rc(float l, float t, float r, float b) {
    D2D1_RECT_F v = {l, t, r, b};
    return v;
}

static D2D1_COLOR_F ui_rgba(float r, float g, float b, float a) {
    D2D1_COLOR_F v = {r, g, b, a};
    return v;
}

static D2D1_POINT_2F ui_pt(float x, float y) {
    D2D1_POINT_2F v = {x, y};
    return v;
}

static void ui_set_brush_color(ID2D1SolidColorBrush* brush, D2D1_COLOR_F color) {
    ID2D1SolidColorBrush_SetColor(brush, &color);
}

void ui_draw_action_button(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush, IDWriteTextFormat* fmt_button,
                           D2D1_RECT_F r, const wchar_t* label, int active, int hovered) {
    D2D1_ROUNDED_RECT rr;
    D2D1_ROUNDED_RECT sr;
    D2D1_RECT_F shadow = ui_rc(r.left, r.top + 1.5f, r.right, r.bottom + 1.5f);
    D2D1_COLOR_F bg;
    D2D1_COLOR_F bd;

    rr.rect = r;
    rr.radiusX = 10.0f;
    rr.radiusY = 10.0f;
    sr.rect = shadow;
    sr.radiusX = 10.0f;
    sr.radiusY = 10.0f;

    if (active) {
        bg = hovered ? ui_rgba(0.25f, 0.41f, 0.63f, 1.0f) : ui_rgba(0.22f, 0.36f, 0.56f, 1.0f);
        bd = ui_rgba(0.39f, 0.56f, 0.76f, 1.0f);
    } else {
        bg = hovered ? ui_rgba(0.20f, 0.23f, 0.28f, 1.0f) : ui_rgba(0.17f, 0.19f, 0.24f, 1.0f);
        bd = hovered ? ui_rgba(0.35f, 0.39f, 0.47f, 1.0f) : ui_rgba(0.30f, 0.34f, 0.42f, 1.0f);
    }

    ui_set_brush_color(brush, ui_rgba(0.08f, 0.09f, 0.12f, hovered ? 0.34f : 0.24f));
    ID2D1HwndRenderTarget_FillRoundedRectangle(target, &sr, (ID2D1Brush*)brush);
    ui_set_brush_color(brush, bg);
    ID2D1HwndRenderTarget_FillRoundedRectangle(target, &rr, (ID2D1Brush*)brush);
    ui_set_brush_color(brush, bd);
    ID2D1HwndRenderTarget_DrawRoundedRectangle(target, &rr, (ID2D1Brush*)brush, 1.0f, NULL);

    ui_draw_text(target, brush, label, r, fmt_button, active ? ui_rgba(0.97f, 0.98f, 1.0f, 1.0f) : ui_rgba(0.86f, 0.90f, 0.96f, 1.0f));
}

void ui_draw_toolbar_icon_button(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                                 D2D1_RECT_F r, ToolbarIconId icon, int active, int hovered) {
    D2D1_ROUNDED_RECT rr;
    D2D1_COLOR_F bg;
    D2D1_COLOR_F bd;
    D2D1_COLOR_F fg = active ? ui_rgba(0.97f, 0.98f, 1.0f, 1.0f) : ui_rgba(0.86f, 0.90f, 0.96f, 1.0f);
    float cx = (r.left + r.right) * 0.5f;
    float cy = (r.top + r.bottom) * 0.5f;
    float x0 = cx - 7.0f;
    float x1 = cx + 7.0f;
    float y0 = cy - 7.0f;
    float y1 = cy + 7.0f;

    rr.rect = r;
    rr.radiusX = 10.0f;
    rr.radiusY = 10.0f;
    if (active) {
        bg = hovered ? ui_rgba(0.25f, 0.41f, 0.63f, 1.0f) : ui_rgba(0.22f, 0.36f, 0.56f, 1.0f);
        bd = ui_rgba(0.39f, 0.56f, 0.76f, 1.0f);
    } else {
        bg = hovered ? ui_rgba(0.20f, 0.23f, 0.28f, 1.0f) : ui_rgba(0.17f, 0.19f, 0.24f, 1.0f);
        bd = hovered ? ui_rgba(0.35f, 0.39f, 0.47f, 1.0f) : ui_rgba(0.30f, 0.34f, 0.42f, 1.0f);
    }
    ui_set_brush_color(brush, bg);
    ID2D1HwndRenderTarget_FillRoundedRectangle(target, &rr, (ID2D1Brush*)brush);
    ui_set_brush_color(brush, bd);
    ID2D1HwndRenderTarget_DrawRoundedRectangle(target, &rr, (ID2D1Brush*)brush, 1.0f, NULL);
    ui_set_brush_color(brush, fg);

    if (icon == TB_ICON_SAVE) {
        D2D1_RECT_F fr = ui_rc(x0, y0, x1, y1);
        D2D1_RECT_F sl = ui_rc(x0 + 2.0f, y0 + 2.0f, x1 - 2.0f, cy - 1.0f);
        ID2D1HwndRenderTarget_DrawRectangle(target, &fr, (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawRectangle(target, &sl, (ID2D1Brush*)brush, 1.0f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 2.0f, cy + 2.0f), ui_pt(x1 - 2.0f, cy + 2.0f), (ID2D1Brush*)brush, 1.2f, NULL);
    } else if (icon == TB_ICON_UNDO || icon == TB_ICON_REDO) {
        int dir = (icon == TB_ICON_UNDO) ? -1 : 1;
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx + dir * 5.0f, cy - 4.0f), ui_pt(cx - dir * 4.0f, cy - 4.0f), (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - dir * 4.0f, cy - 4.0f), ui_pt(cx - dir * 4.0f, cy + 3.0f), (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - dir * 4.0f, cy + 3.0f), ui_pt(cx + dir * 5.0f, cy + 3.0f), (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - dir * 7.0f, cy - 4.0f), ui_pt(cx - dir * 4.0f, cy - 7.0f), (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - dir * 7.0f, cy - 4.0f), ui_pt(cx - dir * 4.0f, cy - 1.0f), (ID2D1Brush*)brush, 1.2f, NULL);
    } else if (icon == TB_ICON_GRID) {
        int gi;
        for (gi = 0; gi < 3; gi++) {
            float gx = x0 + 1.0f + gi * 6.0f;
            float gy = y0 + 1.0f + gi * 6.0f;
            ID2D1HwndRenderTarget_DrawLine(target, ui_pt(gx, y0 + 1.0f), ui_pt(gx, y1 - 1.0f), (ID2D1Brush*)brush, 1.0f, NULL);
            ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 1.0f, gy), ui_pt(x1 - 1.0f, gy), (ID2D1Brush*)brush, 1.0f, NULL);
        }
    } else if (icon == TB_ICON_COLLISION) {
        D2D1_ELLIPSE e1 = {ui_pt(cx - 3.0f, cy), 4.0f, 4.0f};
        D2D1_ELLIPSE e2 = {ui_pt(cx + 3.0f, cy), 4.0f, 4.0f};
        ID2D1HwndRenderTarget_DrawEllipse(target, &e1, (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawEllipse(target, &e2, (ID2D1Brush*)brush, 1.2f, NULL);
    } else if (icon == TB_ICON_VELOCITY) {
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0, cy), ui_pt(x1 - 3.0f, cy), (ID2D1Brush*)brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x1 - 3.0f, cy), ui_pt(x1 - 7.0f, cy - 3.0f), (ID2D1Brush*)brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x1 - 3.0f, cy), ui_pt(x1 - 7.0f, cy + 3.0f), (ID2D1Brush*)brush, 1.4f, NULL);
    } else if (icon == TB_ICON_CONSTRAINT) {
        D2D1_ELLIPSE e1 = {ui_pt(cx - 3.6f, cy), 3.0f, 2.3f};
        D2D1_ELLIPSE e2 = {ui_pt(cx + 3.6f, cy), 3.0f, 2.3f};
        ID2D1HwndRenderTarget_DrawEllipse(target, &e1, (ID2D1Brush*)brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawEllipse(target, &e2, (ID2D1Brush*)brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - 0.8f, cy), ui_pt(cx + 0.8f, cy), (ID2D1Brush*)brush, 1.4f, NULL);
    } else if (icon == TB_ICON_SPRING) {
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 0.5f, cy), ui_pt(x0 + 3.0f, cy), (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 3.0f, cy), ui_pt(x0 + 5.0f, cy - 3.0f), (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 5.0f, cy - 3.0f), ui_pt(x0 + 7.0f, cy + 3.0f), (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 7.0f, cy + 3.0f), ui_pt(x0 + 9.0f, cy - 3.0f), (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 9.0f, cy - 3.0f), ui_pt(x0 + 11.0f, cy + 3.0f), (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 11.0f, cy + 3.0f), ui_pt(x1 - 0.5f, cy), (ID2D1Brush*)brush, 1.2f, NULL);
    } else if (icon == TB_ICON_CHAIN) {
        D2D1_ELLIPSE e1 = {ui_pt(cx - 5.2f, cy), 2.4f, 1.9f};
        D2D1_ELLIPSE e2 = {ui_pt(cx, cy), 2.4f, 1.9f};
        D2D1_ELLIPSE e3 = {ui_pt(cx + 5.2f, cy), 2.4f, 1.9f};
        ID2D1HwndRenderTarget_DrawEllipse(target, &e1, (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawEllipse(target, &e2, (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawEllipse(target, &e3, (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - 2.8f, cy), ui_pt(cx - 2.2f, cy), (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx + 2.2f, cy), ui_pt(cx + 2.8f, cy), (ID2D1Brush*)brush, 1.2f, NULL);
    } else if (icon == TB_ICON_ROPE) {
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 0.5f, cy + 1.5f), ui_pt(x0 + 2.8f, cy + 1.5f), (ID2D1Brush*)brush, 1.1f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 2.8f, cy + 1.5f), ui_pt(x0 + 4.8f, cy - 2.0f), (ID2D1Brush*)brush, 1.1f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 4.8f, cy - 2.0f), ui_pt(x0 + 6.8f, cy + 2.0f), (ID2D1Brush*)brush, 1.1f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 6.8f, cy + 2.0f), ui_pt(x0 + 8.8f, cy - 2.0f), (ID2D1Brush*)brush, 1.1f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 8.8f, cy - 2.0f), ui_pt(x0 + 10.8f, cy + 2.0f), (ID2D1Brush*)brush, 1.1f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(x0 + 10.8f, cy + 2.0f), ui_pt(x1 - 0.5f, cy + 1.5f), (ID2D1Brush*)brush, 1.1f, NULL);
    } else if (icon == TB_ICON_RUN) {
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - 3.0f, cy - 5.0f), ui_pt(cx + 5.0f, cy), (ID2D1Brush*)brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx + 5.0f, cy), ui_pt(cx - 3.0f, cy + 5.0f), (ID2D1Brush*)brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - 3.0f, cy + 5.0f), ui_pt(cx - 3.0f, cy - 5.0f), (ID2D1Brush*)brush, 1.4f, NULL);
    } else if (icon == TB_ICON_PAUSE) {
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - 3.5f, cy - 5.0f), ui_pt(cx - 3.5f, cy + 5.0f), (ID2D1Brush*)brush, 2.0f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx + 3.5f, cy - 5.0f), ui_pt(cx + 3.5f, cy + 5.0f), (ID2D1Brush*)brush, 2.0f, NULL);
    } else if (icon == TB_ICON_STEP) {
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - 6.0f, cy - 6.0f), ui_pt(cx - 6.0f, cy + 6.0f), (ID2D1Brush*)brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - 2.0f, cy - 5.0f), ui_pt(cx + 5.0f, cy), (ID2D1Brush*)brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx + 5.0f, cy), ui_pt(cx - 2.0f, cy + 5.0f), (ID2D1Brush*)brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - 2.0f, cy + 5.0f), ui_pt(cx - 2.0f, cy - 5.0f), (ID2D1Brush*)brush, 1.4f, NULL);
    } else if (icon == TB_ICON_RESET) {
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - 6.0f, cy + 3.0f), ui_pt(cx - 1.0f, cy - 3.0f), (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx - 1.0f, cy - 3.0f), ui_pt(cx + 5.0f, cy + 1.0f), (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx + 5.0f, cy + 1.0f), ui_pt(cx + 2.0f, cy + 1.0f), (ID2D1Brush*)brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(target, ui_pt(cx + 5.0f, cy + 1.0f), ui_pt(cx + 5.0f, cy - 2.0f), (ID2D1Brush*)brush, 1.2f, NULL);
    }
}
