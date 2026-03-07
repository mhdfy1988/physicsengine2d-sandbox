#include "console_tabs.hpp"

static D2D1_RECT_F console_tabs_rect(float left, float top, float right, float bottom) {
    D2D1_RECT_F rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    return rect;
}

static int console_tabs_point_in_rect(float x, float y, D2D1_RECT_F rect) {
    return x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
}

void console_tabs_render(
    ID2D1HwndRenderTarget* target,
    ID2D1SolidColorBrush* brush,
    IDWriteTextFormat* fmt_info,
    D2D1_RECT_F bottom_rect,
    const ConsoleTabsModel* model,
    const ConsoleTabsCallbacks* callbacks,
    ConsoleTabsOutput* output) {
    D2D1_RECT_F active_tab_indicator;
    int tab_console_active;
    int tab_perf_active;
    int show_bottom_tabs;
    if (model == 0 || callbacks == 0 || output == 0) return;
    output->bottom_fold_rect = console_tabs_rect(0, 0, 0, 0);
    output->tab_console_rect = console_tabs_rect(0, 0, 0, 0);
    output->tab_perf_rect = console_tabs_rect(0, 0, 0, 0);
    if (!model->show_bottom_panel) return;

    show_bottom_tabs = !model->bottom_panel_collapsed;
    output->bottom_fold_rect = console_tabs_rect(bottom_rect.right - 28.0f, bottom_rect.top + 6.0f, bottom_rect.right - 8.0f, bottom_rect.top + 24.0f);
    if (show_bottom_tabs) {
        output->tab_console_rect = console_tabs_rect(bottom_rect.left + 14.0f, bottom_rect.top + 7.0f, bottom_rect.left + 94.0f, bottom_rect.top + 29.0f);
        output->tab_perf_rect = console_tabs_rect(bottom_rect.left + 96.0f, bottom_rect.top + 7.0f, bottom_rect.left + 176.0f, bottom_rect.top + 29.0f);
        tab_console_active = (model->bottom_active_tab == 0);
        tab_perf_active = (model->bottom_active_tab == 1);
        callbacks->draw_text_hvcenter(
            L"控制台", output->tab_console_rect, fmt_info,
            tab_console_active
                ? D2D1::ColorF(0.94f, 0.97f, 1.0f, 1.0f)
                : D2D1::ColorF(0.70f, 0.78f, 0.90f, 1.0f));
        callbacks->draw_text_hvcenter(
            L"性能", output->tab_perf_rect, fmt_info,
            tab_perf_active
                ? D2D1::ColorF(0.94f, 0.97f, 1.0f, 1.0f)
                : D2D1::ColorF(0.70f, 0.78f, 0.90f, 1.0f));
        active_tab_indicator = tab_console_active
                                   ? console_tabs_rect(output->tab_console_rect.left + 12.0f, output->tab_console_rect.bottom - 2.0f,
                                                       output->tab_console_rect.right - 12.0f, output->tab_console_rect.bottom)
                                   : console_tabs_rect(output->tab_perf_rect.left + 12.0f, output->tab_perf_rect.bottom - 2.0f,
                                                       output->tab_perf_rect.right - 12.0f, output->tab_perf_rect.bottom);
        callbacks->draw_card_round(
            active_tab_indicator,
            1.0f,
            D2D1::ColorF(0.56f, 0.73f, 0.95f, 1.0f),
            D2D1::ColorF(0.56f, 0.73f, 0.95f, 1.0f));
    }
    callbacks->draw_icon_chevron(
        target,
        brush,
        output->bottom_fold_rect,
        model->bottom_panel_collapsed ? 1 : 0,
        console_tabs_point_in_rect(model->mouse_x, model->mouse_y, output->bottom_fold_rect)
            ? D2D1::ColorF(0.96f, 0.98f, 1.0f, 1.0f)
            : D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f),
        1.5f);
}
