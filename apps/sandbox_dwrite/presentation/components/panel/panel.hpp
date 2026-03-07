#ifndef UI_PANEL_H
#define UI_PANEL_H

#include <d2d1.h>

#include "../../design_system/ui_component_style.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void ui_panel_draw(ID2D1HwndRenderTarget* target,
                   ID2D1SolidColorBrush* brush,
                   D2D1_RECT_F rect,
                   const PanelStyle* style);
void ui_panel_draw_with_shadow(ID2D1HwndRenderTarget* target,
                               ID2D1SolidColorBrush* brush,
                               D2D1_RECT_F rect,
                               const PanelStyle* style);

#ifdef __cplusplus
}
#endif

#endif
