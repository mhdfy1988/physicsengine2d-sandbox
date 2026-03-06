#ifndef UI_PRIMITIVES_H
#define UI_PRIMITIVES_H

#include <d2d1.h>

void ui_draw_card_round(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                        D2D1_RECT_F r, float radius, D2D1_COLOR_F fill, D2D1_COLOR_F border);
void ui_draw_panel_header_band(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                               D2D1_RECT_F panel_rect, float top_h, float inset);
void ui_draw_outer_shadow_rr(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                             D2D1_ROUNDED_RECT rr);

#endif
