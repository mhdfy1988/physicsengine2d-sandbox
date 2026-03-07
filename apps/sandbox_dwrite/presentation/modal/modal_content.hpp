#ifndef MODAL_CONTENT_H
#define MODAL_CONTENT_H

#include <d2d1.h>
#include <dwrite.h>

#include "../../application/scene_catalog.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int help_modal_page;
} HelpModalContentModel;

typedef struct {
    const SceneConfig* config;
    int focused_param;
} ConfigModalContentModel;

typedef struct {
    D2D1_RECT_F row_rect[8];
    D2D1_RECT_F minus_rect[8];
    D2D1_RECT_F plus_rect[8];
} ConfigModalContentOutput;

typedef struct {
    void (*draw_text)(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color);
    void (*draw_card_round)(D2D1_RECT_F rect, float radius, D2D1_COLOR_F fill, D2D1_COLOR_F border);
    void (*draw_outer_shadow_rr)(D2D1_ROUNDED_RECT rr);
    int (*can_adjust_param)(const SceneConfig* cfg, int param_index, int sign);
} ModalContentCallbacks;

void help_modal_content_render(
    D2D1_RECT_F modal_rect,
    IDWriteTextFormat* fmt_title,
    IDWriteTextFormat* fmt_mono,
    const HelpModalContentModel* model,
    const ModalContentCallbacks* callbacks);

void config_modal_content_render(
    D2D1_RECT_F modal_rect,
    IDWriteTextFormat* fmt_title,
    IDWriteTextFormat* fmt_mono,
    const ConfigModalContentModel* model,
    const ModalContentCallbacks* callbacks,
    ConfigModalContentOutput* output);

#ifdef __cplusplus
}
#endif

#endif
