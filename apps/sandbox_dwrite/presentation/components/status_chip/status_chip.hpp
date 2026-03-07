#ifndef UI_STATUS_CHIP_H
#define UI_STATUS_CHIP_H

#include <d2d1.h>
#include <dwrite.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_status_chip_draw(ID2D1HwndRenderTarget* target,
                         ID2D1SolidColorBrush* brush,
                         IDWriteTextFormat* text_format,
                         D2D1_RECT_F rect,
                         const wchar_t* text,
                         D2D1_COLOR_F fill,
                         D2D1_COLOR_F border,
                         D2D1_COLOR_F text_color);

#ifdef __cplusplus
}
#endif

#endif
