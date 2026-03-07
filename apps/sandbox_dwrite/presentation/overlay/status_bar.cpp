#include "status_bar.hpp"

#include "../components/status_chip/status_chip.hpp"
#include "../render/ui_text.hpp"

static D2D1_RECT_F status_bar_rc(float l, float t, float r, float b) {
    D2D1_RECT_F v = {l, t, r, b};
    return v;
}

static D2D1_COLOR_F status_bar_rgba(float r, float g, float b, float a) {
    D2D1_COLOR_F v = {r, g, b, a};
    return v;
}

void status_bar_render(ID2D1HwndRenderTarget* target,
                       ID2D1SolidColorBrush* brush,
                       IDWriteTextFormat* text_format,
                       const UiTheme* theme,
                       const StatusBarRenderModel* model,
                       StatusBarRenderResult* out_result) {
    wchar_t line[128];
    wchar_t line_right[128];
    D2D1_COLOR_F bus_color;
    D2D1_COLOR_F hot_reload_color;
    D2D1_RECT_F startup_rect;
    D2D1_RECT_F hot_reload_rect;
    D2D1_RECT_F bus_rect;
    const wchar_t* hover_tip;

    if (target == 0 || brush == 0 || text_format == 0 || theme == 0 || model == 0) return;
    hover_tip = (model->hover_tip != 0) ? model->hover_tip : L"";
    if (out_result != 0) {
        out_result->meta_rect = status_bar_rc(0, 0, 0, 0);
    }

    status_presenter_format_counts(&model->presenter_model, line, 128);
    if (hover_tip[0] != L'\0') {
        ui_draw_text(target, brush, hover_tip,
                     status_bar_rc(model->status_rect.left + 330.0f, model->status_rect.top + 6.0f,
                                   model->status_rect.right - 630.0f, model->status_rect.bottom - 4.0f),
                     text_format, theme->components.status_bar.hint_text);
    }

    ui_draw_text(target, brush, line,
                 status_bar_rc(model->status_rect.left + 12.0f, model->status_rect.top + 6.0f,
                               model->status_rect.right - 200.0f, model->status_rect.bottom - 4.0f),
                 text_format, theme->components.status_bar.text);

    status_presenter_format_meta(&model->presenter_model, line, 128);
    if (out_result != 0) {
        out_result->meta_rect = status_bar_rc(model->status_rect.right - 620.0f, model->status_rect.top + 6.0f,
                                              model->status_rect.right - 392.0f, model->status_rect.bottom - 4.0f);
    }
    ui_draw_text(target, brush, line,
                 (out_result != 0) ? out_result->meta_rect
                                   : status_bar_rc(model->status_rect.right - 620.0f, model->status_rect.top + 6.0f,
                                                   model->status_rect.right - 392.0f, model->status_rect.bottom - 4.0f),
                 text_format, theme->components.status_bar.meta_text);

    status_presenter_format_startup(&model->presenter_model, line_right, 128);
    startup_rect = status_bar_rc(model->status_rect.right - 520.0f, model->status_rect.top + 5.0f,
                                 model->status_rect.right - 392.0f, model->status_rect.bottom - 5.0f);
    ui_status_chip_draw(target, brush, text_format, startup_rect, line_right,
                        model->presenter_model.startup_degraded ? status_bar_rgba(0.42f, 0.24f, 0.08f, 1.0f)
                                                                : status_bar_rgba(0.19f, 0.25f, 0.34f, 1.0f),
                        model->presenter_model.startup_degraded ? theme->colors.warning : theme->colors.status_border,
                        model->presenter_model.startup_degraded ? theme->colors.warning : theme->colors.text_muted);

    status_presenter_format_hot_reload(&model->presenter_model, line_right, 128, &hot_reload_color);
    hot_reload_rect = status_bar_rc(model->status_rect.right - 386.0f, model->status_rect.top + 5.0f,
                                    model->status_rect.right - 220.0f, model->status_rect.bottom - 5.0f);
    ui_status_chip_draw(target, brush, text_format, hot_reload_rect, line_right,
                        status_bar_rgba(0.16f, 0.20f, 0.28f, 1.0f), hot_reload_color, hot_reload_color);

    status_presenter_format_bus(&model->presenter_model, line_right, 128, &bus_color);
    bus_rect = status_bar_rc(model->status_rect.right - 216.0f, model->status_rect.top + 5.0f,
                             model->status_rect.right - 24.0f, model->status_rect.bottom - 5.0f);
    ui_status_chip_draw(target, brush, text_format, bus_rect, line_right,
                        status_bar_rgba(0.16f, 0.20f, 0.28f, 1.0f), bus_color, bus_color);
}
