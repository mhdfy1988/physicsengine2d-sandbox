#include "console_perf.hpp"
#include <stdio.h>

static D2D1_RECT_F console_perf_rect(float left, float top, float right, float bottom) {
    D2D1_RECT_F rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    return rect;
}

static float console_perf_clamp(float value, float min_value, float max_value) {
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

static int console_perf_point_in_rect(float x, float y, D2D1_RECT_F rect) {
    return x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
}

void console_perf_render(
    ID2D1HwndRenderTarget* target,
    ID2D1SolidColorBrush* brush,
    IDWriteTextFormat* fmt_info,
    D2D1_RECT_F bottom_rect,
    const ConsolePerfModel* model,
    const ConsolePerfCallbacks* callbacks,
    ConsolePerfOutput* output) {
    D2D1_RECT_F graph;
    wchar_t line[128];
    int gi;
    if (target == 0 || brush == 0 || fmt_info == 0 || model == 0 || callbacks == 0 || output == 0) return;

    graph = console_perf_rect(bottom_rect.left + 280.0f, bottom_rect.top + 36.0f, bottom_rect.right - 12.0f, bottom_rect.bottom - 12.0f);
    output->perf_diag_export_rect = console_perf_rect(bottom_rect.right - 248.0f, bottom_rect.top + 6.0f, bottom_rect.right - 142.0f, bottom_rect.top + 28.0f);
    output->perf_export_rect = console_perf_rect(bottom_rect.right - 130.0f, bottom_rect.top + 6.0f, bottom_rect.right - 36.0f, bottom_rect.top + 28.0f);

    callbacks->draw_action_button(
        output->perf_diag_export_rect, L"导出诊断", 0,
        console_perf_point_in_rect(model->mouse_x, model->mouse_y, output->perf_diag_export_rect));
    callbacks->draw_action_button(
        output->perf_export_rect, L"导出CSV", 0,
        console_perf_point_in_rect(model->mouse_x, model->mouse_y, output->perf_export_rect));
    callbacks->draw_card_round(graph, 6.0f, D2D1::ColorF(0.14f, 0.18f, 0.24f, 1.0f), D2D1::ColorF(0.30f, 0.38f, 0.50f, 1.0f));

    swprintf(line, 128, L"FPS: %d", model->fps_display);
    callbacks->draw_text(line, console_perf_rect(bottom_rect.left + 12.0f, bottom_rect.top + 40.0f, bottom_rect.left + 180.0f, bottom_rect.top + 66.0f),
                         fmt_info, D2D1::ColorF(0.62f, 0.88f, 0.62f, 1.0f));
    swprintf(line, 128, L"物理耗时: %.2fms", model->physics_step_ms);
    callbacks->draw_text(line, console_perf_rect(bottom_rect.left + 12.0f, bottom_rect.top + 66.0f, bottom_rect.left + 260.0f, bottom_rect.top + 92.0f),
                         fmt_info, D2D1::ColorF(0.95f, 0.71f, 0.42f, 1.0f));
    swprintf(line, 128, L"粗检测: %ls", model->broadphase_use_grid ? L"网格" : L"暴力");
    callbacks->draw_text(line, console_perf_rect(bottom_rect.left + 12.0f, bottom_rect.top + 92.0f, bottom_rect.left + 260.0f, bottom_rect.top + 118.0f),
                         fmt_info, D2D1::ColorF(0.82f, 0.88f, 0.96f, 1.0f));

    if (model->perf_hist_count >= 2) {
        for (gi = 1; gi < model->perf_hist_count; gi++) {
            int idx0 = (model->perf_hist_head + gi - 1) % CONSOLE_PERF_HISTORY_CAP;
            int idx1 = (model->perf_hist_head + gi) % CONSOLE_PERF_HISTORY_CAP;
            float x0 = graph.left + (graph.right - graph.left) * ((float)(gi - 1) / (float)(model->perf_hist_count - 1));
            float x1 = graph.left + (graph.right - graph.left) * ((float)gi / (float)(model->perf_hist_count - 1));
            float y0_fps = graph.bottom - 4.0f - console_perf_clamp(model->fps_hist[idx0], 0.0f, 120.0f) / 120.0f * (graph.bottom - graph.top - 8.0f);
            float y1_fps = graph.bottom - 4.0f - console_perf_clamp(model->fps_hist[idx1], 0.0f, 120.0f) / 120.0f * (graph.bottom - graph.top - 8.0f);
            float y0_ms = graph.bottom - 4.0f - console_perf_clamp(model->step_hist[idx0], 0.0f, 16.0f) / 16.0f * (graph.bottom - graph.top - 8.0f);
            float y1_ms = graph.bottom - 4.0f - console_perf_clamp(model->step_hist[idx1], 0.0f, 16.0f) / 16.0f * (graph.bottom - graph.top - 8.0f);
            callbacks->set_brush_color(0.44f, 0.90f, 0.44f, 0.95f);
            ID2D1HwndRenderTarget_DrawLine(target, D2D1::Point2F(x0, y0_fps), D2D1::Point2F(x1, y1_fps), (ID2D1Brush*)brush, 1.3f, NULL);
            callbacks->set_brush_color(0.98f, 0.73f, 0.38f, 0.95f);
            ID2D1HwndRenderTarget_DrawLine(target, D2D1::Point2F(x0, y0_ms), D2D1::Point2F(x1, y1_ms), (ID2D1Brush*)brush, 1.3f, NULL);
        }
        callbacks->draw_text(L"绿:FPS  橙:耗时", console_perf_rect(graph.left + 6.0f, graph.top + 4.0f, graph.left + 140.0f, graph.top + 20.0f),
                             fmt_info, D2D1::ColorF(0.76f, 0.85f, 0.95f, 1.0f));
    }
}
