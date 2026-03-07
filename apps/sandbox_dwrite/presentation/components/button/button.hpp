#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include <d2d1.h>

#include "../../render/ui_widgets.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void ui_icon_button_draw(ID2D1HwndRenderTarget* target,
                         ID2D1SolidColorBrush* brush,
                         D2D1_RECT_F rect,
                         ToolbarIconId icon,
                         int active,
                         int hovered);

#ifdef __cplusplus
}
#endif

#endif
