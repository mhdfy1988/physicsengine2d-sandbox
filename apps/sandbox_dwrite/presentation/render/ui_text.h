#ifndef UI_TEXT_H
#define UI_TEXT_H

#include <d2d1.h>
#include <dwrite.h>

void ui_draw_text(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                  const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color);
void ui_draw_text_vcenter(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush, IDWriteFactory* dwrite_factory,
                          const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color);
void ui_draw_text_hvcenter(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                           const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color);
void ui_draw_text_right_vcenter(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush, IDWriteFactory* dwrite_factory,
                                const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color);
float ui_measure_text_width(IDWriteFactory* dwrite_factory, const wchar_t* text, IDWriteTextFormat* fmt);

#endif
