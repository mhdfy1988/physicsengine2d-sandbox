#ifndef MODAL_SHELL_H
#define MODAL_SHELL_H

#include <d2d1.h>
#include <dwrite.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int show_value_input;
    int show_config_modal;
    int show_help_modal;
    const wchar_t* value_input_title;
    const wchar_t* value_input_buf;
    int value_input_len;
    int value_input_caret;
    float width;
    float height;
} ModalShellModel;

typedef struct {
    D2D1_RECT_F modal_rect;
    D2D1_RECT_F modal_close_rect;
    D2D1_RECT_F value_modal_rect;
} ModalShellOutput;

typedef struct {
    void (*draw_card_round)(D2D1_RECT_F rect, float radius, D2D1_COLOR_F fill, D2D1_COLOR_F border);
    void (*draw_text)(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color);
    float (*measure_text_width)(const wchar_t* text, IDWriteTextFormat* format);
    void (*set_brush_color)(float r, float g, float b, float a);
    void (*render_help_modal_content)(D2D1_RECT_F modal_rect);
    void (*render_config_modal_content)(D2D1_RECT_F modal_rect);
} ModalShellCallbacks;

void modal_shell_render(
    ID2D1HwndRenderTarget* target,
    ID2D1SolidColorBrush* brush,
    IDWriteTextFormat* fmt_ui,
    IDWriteTextFormat* fmt_mono,
    IDWriteTextFormat* fmt_info,
    const ModalShellModel* model,
    const ModalShellCallbacks* callbacks,
    ModalShellOutput* output);

#ifdef __cplusplus
}
#endif

#endif
