#include "panel.hpp"

#include "../../render/ui_primitives.hpp"

void ui_panel_draw(ID2D1HwndRenderTarget* target,
                   ID2D1SolidColorBrush* brush,
                   D2D1_RECT_F rect,
                   const PanelStyle* style) {
    if (target == 0 || brush == 0 || style == 0) return;
    ui_draw_card_round(target, brush, rect, style->radius, style->fill, style->border);
}

void ui_panel_draw_with_shadow(ID2D1HwndRenderTarget* target,
                               ID2D1SolidColorBrush* brush,
                               D2D1_RECT_F rect,
                               const PanelStyle* style) {
    D2D1_ROUNDED_RECT rounded_rect;
    if (target == 0 || brush == 0 || style == 0) return;
    rounded_rect.rect = rect;
    rounded_rect.radiusX = style->radius;
    rounded_rect.radiusY = style->radius;
    ui_draw_outer_shadow_rr(target, brush, rounded_rect);
    ui_draw_card_round(target, brush, rect, style->radius, style->fill, style->border);
}
