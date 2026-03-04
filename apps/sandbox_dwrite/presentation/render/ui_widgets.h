#ifndef UI_WIDGETS_H
#define UI_WIDGETS_H

#include <d2d1.h>
#include <dwrite.h>

typedef enum {
    TB_ICON_SAVE = 0,
    TB_ICON_UNDO,
    TB_ICON_REDO,
    TB_ICON_GRID,
    TB_ICON_COLLISION,
    TB_ICON_VELOCITY,
    TB_ICON_CONSTRAINT,
    TB_ICON_SPRING,
    TB_ICON_CHAIN,
    TB_ICON_ROPE,
    TB_ICON_RUN,
    TB_ICON_PAUSE,
    TB_ICON_STEP,
    TB_ICON_RESET
} ToolbarIconId;

void ui_draw_action_button(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush, IDWriteTextFormat* fmt_button,
                           D2D1_RECT_F r, const wchar_t* label, int active, int hovered);
void ui_draw_toolbar_icon_button(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                                 D2D1_RECT_F r, ToolbarIconId icon, int active, int hovered);

#endif
