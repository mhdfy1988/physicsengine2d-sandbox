#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>

#include "ui_text.hpp"

static D2D1_POINT_2F ui_pt(float x, float y) {
    D2D1_POINT_2F v = {x, y};
    return v;
}

static void ui_set_brush_color(ID2D1SolidColorBrush* brush, D2D1_COLOR_F color) {
    ID2D1SolidColorBrush_SetColor(brush, &color);
}

static void ui_release_unknown(IUnknown** p) {
    if (p != NULL && *p != NULL) {
        (*p)->Release();
        *p = NULL;
    }
}

void ui_draw_text(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                  const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    DWRITE_PARAGRAPH_ALIGNMENT old_align = IDWriteTextFormat_GetParagraphAlignment(fmt);
    IDWriteTextFormat_SetParagraphAlignment(fmt, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    ui_set_brush_color(brush, color);
    ID2D1HwndRenderTarget_DrawText(target, text, (UINT32)lstrlenW(text), fmt, &rect, (ID2D1Brush*)brush,
                                   D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT, DWRITE_MEASURING_MODE_NATURAL);
    IDWriteTextFormat_SetParagraphAlignment(fmt, old_align);
}

void ui_draw_text_vcenter(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush, IDWriteFactory* dwrite_factory,
                          const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    IDWriteTextLayout* layout = NULL;
    DWRITE_TEXT_METRICS m;
    float slot_h = rect.bottom - rect.top;
    float draw_h;
    float y;
    HRESULT hr;
    if (slot_h <= 0.0f) {
        ui_draw_text(target, brush, text, rect, fmt, color);
        return;
    }
    hr = IDWriteFactory_CreateTextLayout(
        dwrite_factory,
        text,
        (UINT32)lstrlenW(text),
        fmt,
        rect.right - rect.left,
        512.0f,
        &layout
    );
    if (FAILED(hr) || layout == NULL) {
        ui_draw_text(target, brush, text, rect, fmt, color);
        ui_release_unknown((IUnknown**)&layout);
        return;
    }
    if (FAILED(IDWriteTextLayout_GetMetrics(layout, &m))) {
        ui_draw_text(target, brush, text, rect, fmt, color);
        ui_release_unknown((IUnknown**)&layout);
        return;
    }
    draw_h = m.height;
    if (draw_h < 1.0f) draw_h = slot_h;
    if (draw_h > slot_h) draw_h = slot_h;
    y = rect.top + (slot_h - draw_h) * 0.5f;
    ui_set_brush_color(brush, color);
    ID2D1HwndRenderTarget_DrawTextLayout(target, ui_pt(rect.left, y), layout, (ID2D1Brush*)brush,
                                         D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
    ui_release_unknown((IUnknown**)&layout);
}

void ui_draw_text_hvcenter(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush,
                           const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    DWRITE_TEXT_ALIGNMENT old_text_align = IDWriteTextFormat_GetTextAlignment(fmt);
    DWRITE_PARAGRAPH_ALIGNMENT old_para_align = IDWriteTextFormat_GetParagraphAlignment(fmt);
    IDWriteTextFormat_SetTextAlignment(fmt, DWRITE_TEXT_ALIGNMENT_CENTER);
    IDWriteTextFormat_SetParagraphAlignment(fmt, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    ui_draw_text(target, brush, text, rect, fmt, color);
    IDWriteTextFormat_SetParagraphAlignment(fmt, old_para_align);
    IDWriteTextFormat_SetTextAlignment(fmt, old_text_align);
}

void ui_draw_text_right_vcenter(ID2D1HwndRenderTarget* target, ID2D1SolidColorBrush* brush, IDWriteFactory* dwrite_factory,
                                const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    DWRITE_TEXT_ALIGNMENT old_text_align = IDWriteTextFormat_GetTextAlignment(fmt);
    IDWriteTextFormat_SetTextAlignment(fmt, DWRITE_TEXT_ALIGNMENT_TRAILING);
    ui_draw_text_vcenter(target, brush, dwrite_factory, text, rect, fmt, color);
    IDWriteTextFormat_SetTextAlignment(fmt, old_text_align);
}

float ui_measure_text_width(IDWriteFactory* dwrite_factory, const wchar_t* text, IDWriteTextFormat* fmt) {
    IDWriteTextLayout* layout = NULL;
    DWRITE_TEXT_METRICS m;
    HRESULT hr;
    float w = 0.0f;
    if (text == NULL || text[0] == L'\0' || dwrite_factory == NULL || fmt == NULL) return 0.0f;
    hr = IDWriteFactory_CreateTextLayout(dwrite_factory, text, (UINT32)lstrlenW(text), fmt, 1200.0f, 64.0f, &layout);
    if (SUCCEEDED(hr) && layout != NULL) {
        if (SUCCEEDED(IDWriteTextLayout_GetMetrics(layout, &m))) {
            w = m.widthIncludingTrailingWhitespace;
        }
    }
    ui_release_unknown((IUnknown**)&layout);
    return w;
}
