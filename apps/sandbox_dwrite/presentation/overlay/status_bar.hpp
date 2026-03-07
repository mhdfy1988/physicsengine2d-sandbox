#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <d2d1.h>
#include <dwrite.h>

#include "../design_system/ui_theme.hpp"
#include "../status/status_presenter.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    D2D1_RECT_F status_rect;
    const wchar_t* hover_tip;
    StatusPresenterModel presenter_model;
} StatusBarRenderModel;

typedef struct {
    D2D1_RECT_F meta_rect;
} StatusBarRenderResult;

void status_bar_render(ID2D1HwndRenderTarget* target,
                       ID2D1SolidColorBrush* brush,
                       IDWriteTextFormat* text_format,
                       const UiTheme* theme,
                       const StatusBarRenderModel* model,
                       StatusBarRenderResult* out_result);

#ifdef __cplusplus
}
#endif

#endif
