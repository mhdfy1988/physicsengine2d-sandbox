#ifndef INSPECTOR_RENDERER_H
#define INSPECTOR_RENDERER_H

#include <d2d1.h>
#include <dwrite.h>

#include "physics_core/physics.h"

#ifdef __cplusplus
extern "C" {
#endif

enum { INSPECTOR_RENDERER_MAX_ROWS = 12 };

typedef struct {
    int focused_row;
    int scroll_offset;
    int row_count;
    RigidBody* selected_body;
    const Constraint* selected_constraint;
} InspectorRendererState;

typedef struct {
    D2D1_RECT_F row_rects[INSPECTOR_RENDERER_MAX_ROWS];
    D2D1_RECT_F minus_rects[INSPECTOR_RENDERER_MAX_ROWS];
    D2D1_RECT_F plus_rects[INSPECTOR_RENDERER_MAX_ROWS];
    D2D1_RECT_F viewport_rect;
    int scroll_max;
} InspectorRendererOutput;

typedef struct {
    ID2D1HwndRenderTarget* target;
    ID2D1SolidColorBrush* brush;
    IDWriteTextFormat* fmt_ui;
    IDWriteTextFormat* fmt_info;
    D2D1_RECT_F inspector_rect;
    D2D1_RECT_F viewport_rect;
    D2D1_POINT_2F mouse_point;
    const InspectorRendererState* state;
    int (*spring_preset_for_constraint)(const Constraint* c);
    const wchar_t* (*inspector_row_hint_text)(void);
    void (*draw_panel_header_band)(D2D1_RECT_F panel_rect, float top_h, float inset);
    void (*draw_card_round)(D2D1_RECT_F rect, float radius, D2D1_COLOR_F fill, D2D1_COLOR_F border);
    void (*draw_text)(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color);
    void (*draw_text_vcenter)(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color);
} InspectorRendererContext;

void inspector_renderer_render(const InspectorRendererContext* context, InspectorRendererOutput* out_output);

#ifdef __cplusplus
}
#endif

#endif
