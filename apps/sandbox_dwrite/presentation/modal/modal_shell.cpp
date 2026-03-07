#include "modal_shell.hpp"

static D2D1_RECT_F modal_shell_rect(float left, float top, float right, float bottom) {
    D2D1_RECT_F rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    return rect;
}

void modal_shell_render(
    ID2D1HwndRenderTarget* target,
    ID2D1SolidColorBrush* brush,
    IDWriteTextFormat* fmt_ui,
    IDWriteTextFormat* fmt_mono,
    IDWriteTextFormat* fmt_info,
    const ModalShellModel* model,
    const ModalShellCallbacks* callbacks,
    ModalShellOutput* output) {
    if (target == 0 || brush == 0 || fmt_ui == 0 || fmt_mono == 0 || fmt_info == 0 || model == 0 || callbacks == 0 || output == 0) return;
    output->modal_rect = modal_shell_rect(0, 0, 0, 0);
    output->modal_close_rect = modal_shell_rect(0, 0, 0, 0);
    output->value_modal_rect = modal_shell_rect(0, 0, 0, 0);

    if (!(model->show_value_input || model->show_config_modal || model->show_help_modal)) return;

    {
        D2D1_RECT_F modal = modal_shell_rect(model->width * 0.13f, model->height * 0.12f, model->width * 0.87f, model->height * 0.88f);
        D2D1_RECT_F overlay = modal_shell_rect(0.0f, 0.0f, model->width, model->height);
        callbacks->set_brush_color(0.0f, 0.0f, 0.0f, 0.48f);
        ID2D1HwndRenderTarget_FillRectangle(target, &overlay, (ID2D1Brush*)brush);
        if (model->show_value_input) {
            wchar_t prefix[64];
            D2D1_RECT_F vm = modal_shell_rect(model->width * 0.34f, model->height * 0.40f, model->width * 0.66f, model->height * 0.58f);
            int i;
            int caret = model->value_input_caret;
            float cx;
            output->value_modal_rect = vm;
            callbacks->draw_card_round(vm, 10.0f, D2D1::ColorF(0.14f, 0.16f, 0.20f, 1.0f), D2D1::ColorF(0.30f, 0.34f, 0.41f, 1.0f));
            callbacks->draw_text(model->value_input_title, modal_shell_rect(vm.left + 14.0f, vm.top + 10.0f, vm.right - 14.0f, vm.top + 36.0f), fmt_ui, D2D1::ColorF(0.86f, 0.91f, 0.97f, 1.0f));
            callbacks->draw_card_round(modal_shell_rect(vm.left + 14.0f, vm.top + 44.0f, vm.right - 14.0f, vm.top + 78.0f), 6.0f, D2D1::ColorF(0.10f, 0.12f, 0.16f, 1.0f), D2D1::ColorF(0.34f, 0.40f, 0.50f, 1.0f));
            callbacks->draw_text(model->value_input_buf, modal_shell_rect(vm.left + 22.0f, vm.top + 50.0f, vm.right - 22.0f, vm.top + 72.0f), fmt_mono, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
            if (caret < 0) caret = 0;
            if (caret > model->value_input_len) caret = model->value_input_len;
            for (i = 0; i < caret && i < 63; i++) prefix[i] = model->value_input_buf[i];
            prefix[i] = L'\0';
            cx = vm.left + 22.0f + callbacks->measure_text_width(prefix, fmt_mono);
            if (cx > vm.right - 24.0f) cx = vm.right - 24.0f;
            callbacks->set_brush_color(0.45f, 0.62f, 0.84f, 1.0f);
            ID2D1HwndRenderTarget_DrawLine(target, D2D1::Point2F(cx, vm.top + 50.0f), D2D1::Point2F(cx, vm.top + 72.0f), (ID2D1Brush*)brush, 1.2f, NULL);
            callbacks->draw_text(L"Enter 确认  Esc 取消  Backspace 删除", modal_shell_rect(vm.left + 14.0f, vm.top + 88.0f, vm.right - 14.0f, vm.top + 112.0f), fmt_info, D2D1::ColorF(0.62f, 0.70f, 0.82f, 1.0f));
        } else {
            output->modal_rect = modal;
            output->modal_close_rect = modal_shell_rect(modal.right - 40.0f, modal.top + 12.0f, modal.right - 12.0f, modal.top + 40.0f);
            callbacks->draw_card_round(modal, 10.0f, D2D1::ColorF(0.14f, 0.16f, 0.20f, 1.0f), D2D1::ColorF(0.30f, 0.34f, 0.41f, 1.0f));
            callbacks->draw_text(L"x", output->modal_close_rect, fmt_ui, D2D1::ColorF(0.84f, 0.89f, 0.95f, 1.0f));
        }

        if (!model->show_value_input && model->show_help_modal) callbacks->render_help_modal_content(modal);
        if (!model->show_value_input && model->show_config_modal) callbacks->render_config_modal_content(modal);
    }
}
