#include "button.hpp"

void ui_icon_button_draw(ID2D1HwndRenderTarget* target,
                         ID2D1SolidColorBrush* brush,
                         D2D1_RECT_F rect,
                         ToolbarIconId icon,
                         int active,
                         int hovered) {
    ui_draw_toolbar_icon_button(target, brush, rect, icon, active, hovered);
}
