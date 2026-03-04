#ifndef UI_ICONS_H
#define UI_ICONS_H

#include <d2d1.h>

void ui_draw_icon_minus(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                        D2D1_RECT_F r, D2D1_COLOR_F color, float thickness);
void ui_draw_icon_plus(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                       D2D1_RECT_F r, D2D1_COLOR_F color, float thickness);
void ui_draw_icon_chevron(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                          D2D1_RECT_F r, int up, D2D1_COLOR_F color, float thickness);
void ui_draw_icon_chevron_lr(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                             D2D1_RECT_F r, int left, D2D1_COLOR_F color, float thickness);
void ui_draw_tree_disclosure_icon(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                                  D2D1_RECT_F r, int expanded, D2D1_COLOR_F color, float thickness);

#endif
