#include "console_renderer.hpp"

static D2D1_RECT_F console_renderer_rect(float left, float top, float right, float bottom) {
    D2D1_RECT_F rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    return rect;
}

static int console_renderer_point_in_rect(float x, float y, D2D1_RECT_F rect) {
    return x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
}

void console_renderer_render(
    ID2D1HwndRenderTarget* target,
    IDWriteTextFormat* fmt_info,
    D2D1_RECT_F bottom_rect,
    const ConsoleRendererModel* model,
    const ConsoleRendererCallbacks* callbacks,
    ConsoleRendererOutput* output) {
    int li;
    int shown = 0;
    int skip;
    int total_lines = 0;
    int max_lines;
    wchar_t line[128];
    float controls_right = bottom_rect.right - 36.0f;
    float x0;
    float gap = 8.0f;
    float line_h = 24.0f;
    float viewport_top;
    float viewport_bottom;
    float viewport_h;
    if (target == 0 || fmt_info == 0 || model == 0 || callbacks == 0 || output == 0) return;

    output->log_search_clear_rect = console_renderer_rect(0, 0, 0, 0);
    output->log_search_rect = console_renderer_rect(0, 0, 0, 0);
    output->dbg_collision_filter_rect = console_renderer_rect(0, 0, 0, 0);
    output->log_viewport_rect = console_renderer_rect(0, 0, 0, 0);
    output->log_scroll_track_rect = console_renderer_rect(0, 0, 0, 0);
    output->log_scroll_thumb_rect = console_renderer_rect(0, 0, 0, 0);
    output->dbg_collision_row_count = 0;
    output->log_scroll_max = 0;

    x0 = controls_right;
    output->log_clear_rect = console_renderer_rect(x0 - 58.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);
    x0 = output->log_clear_rect.left - gap;
    output->log_filter_warn_rect = console_renderer_rect(x0 - 60.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);
    x0 = output->log_filter_warn_rect.left - gap;
    output->log_filter_collision_rect = console_renderer_rect(x0 - 60.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);
    x0 = output->log_filter_collision_rect.left - gap;
    output->log_filter_physics_rect = console_renderer_rect(x0 - 60.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);
    x0 = output->log_filter_physics_rect.left - gap;
    output->log_filter_state_rect = console_renderer_rect(x0 - 60.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);
    x0 = output->log_filter_state_rect.left - gap;
    output->log_filter_all_rect = console_renderer_rect(x0 - 46.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);

    callbacks->draw_text_tab_button(output->log_filter_all_rect, L"全部", model->log_filter_mode == 0,
                                    console_renderer_point_in_rect(model->mouse_x, model->mouse_y, output->log_filter_all_rect));
    callbacks->draw_text_tab_button(output->log_filter_state_rect, L"状态", model->log_filter_mode == 1,
                                    console_renderer_point_in_rect(model->mouse_x, model->mouse_y, output->log_filter_state_rect));
    callbacks->draw_text_tab_button(output->log_filter_physics_rect, L"物理", model->log_filter_mode == 2,
                                    console_renderer_point_in_rect(model->mouse_x, model->mouse_y, output->log_filter_physics_rect));
    callbacks->draw_text_tab_button(output->log_filter_collision_rect, L"碰撞", model->log_filter_mode == 4,
                                    console_renderer_point_in_rect(model->mouse_x, model->mouse_y, output->log_filter_collision_rect));
    callbacks->draw_text_tab_button(output->log_filter_warn_rect, L"警告", model->log_filter_mode == 3,
                                    console_renderer_point_in_rect(model->mouse_x, model->mouse_y, output->log_filter_warn_rect));
    callbacks->draw_text_tab_button(output->log_clear_rect, L"清空", 0,
                                    console_renderer_point_in_rect(model->mouse_x, model->mouse_y, output->log_clear_rect));

    viewport_top = bottom_rect.top + 38.0f;
    viewport_bottom = bottom_rect.bottom - 10.0f;
    if (model->log_filter_mode == 4) {
        output->dbg_collision_filter_rect = console_renderer_rect(bottom_rect.left + 12.0f, bottom_rect.bottom - 32.0f, bottom_rect.left + 136.0f, bottom_rect.bottom - 10.0f);
        callbacks->draw_action_button(output->dbg_collision_filter_rect, L"仅选中相关", model->collision_event_filter_selected_only,
                                      console_renderer_point_in_rect(model->mouse_x, model->mouse_y, output->dbg_collision_filter_rect));
        viewport_bottom = output->dbg_collision_filter_rect.top - 6.0f;
    }

    viewport_h = viewport_bottom - viewport_top;
    if (viewport_h < line_h * 2.0f) viewport_h = line_h * 2.0f;
    output->log_viewport_rect = console_renderer_rect(bottom_rect.left + 10.0f, viewport_top, bottom_rect.right - 18.0f, viewport_top + viewport_h);
    max_lines = (int)(viewport_h / line_h);
    if (max_lines < 1) max_lines = 1;
    skip = model->log_scroll_offset;

    if (model->log_filter_mode == 4) {
        int i;
        for (i = model->collision_event_count - 1; i >= 0; i--) {
            int idx = (model->collision_event_head + i) % 120;
            if (!callbacks->format_collision_event_line(idx, line, 128)) continue;
            if (model->collision_event_filter_selected_only && !callbacks->collision_event_involves_selected(idx)) continue;
            if (model->log_search_len > 0 && wcsstr(line, model->log_search_buf) == NULL) continue;
            total_lines++;
        }
        output->log_scroll_max = (total_lines > max_lines) ? (total_lines - max_lines) : 0;
        if (skip < 0) skip = 0;
        if (skip > output->log_scroll_max) skip = output->log_scroll_max;
        ID2D1HwndRenderTarget_PushAxisAlignedClip(target, &output->log_viewport_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        for (i = model->collision_event_count - 1; i >= 0 && shown < max_lines; i--) {
            int idx = (model->collision_event_head + i) % 120;
            if (!callbacks->format_collision_event_line(idx, line, 128)) continue;
            if (model->collision_event_filter_selected_only && !callbacks->collision_event_involves_selected(idx)) continue;
            if (model->log_search_len > 0 && wcsstr(line, model->log_search_buf) == NULL) continue;
            if (skip > 0) { skip--; continue; }
            if (shown < CONSOLE_RENDERER_EVENT_ROWS_MAX) {
                output->dbg_collision_row_rect[shown] = console_renderer_rect(output->log_viewport_rect.left, output->log_viewport_rect.top + shown * line_h,
                                                                              output->log_viewport_rect.right, output->log_viewport_rect.top + (shown + 1) * line_h);
                output->dbg_collision_row_event_index[shown] = idx;
                output->dbg_collision_row_count = shown + 1;
            }
            callbacks->draw_text(line,
                                 console_renderer_rect(output->log_viewport_rect.left + 2.0f, output->log_viewport_rect.top + shown * line_h,
                                                       output->log_viewport_rect.right - 2.0f, output->log_viewport_rect.top + (shown + 1) * line_h),
                                 fmt_info, shown == 0 ? D2D1::ColorF(0.90f, 0.95f, 1.0f, 1.0f) : D2D1::ColorF(0.76f, 0.85f, 0.95f, 1.0f));
            shown++;
        }
        if (shown == 0) {
            callbacks->draw_text(model->collision_event_filter_selected_only ? L"无选中对象相关碰撞" : L"暂无碰撞记录",
                                 console_renderer_rect(output->log_viewport_rect.left + 2.0f, output->log_viewport_rect.top + 4.0f,
                                                       output->log_viewport_rect.right - 2.0f, output->log_viewport_rect.top + line_h + 4.0f),
                                 fmt_info, D2D1::ColorF(0.65f, 0.74f, 0.86f, 1.0f));
        }
        ID2D1HwndRenderTarget_PopAxisAlignedClip(target);
    } else {
        for (li = model->console_log_count - 1; li >= 0; li--) {
            int idx = (model->console_log_head + li) % 200;
            if (!callbacks->log_match_filter(model->console_logs[idx], model->log_filter_mode)) continue;
            total_lines++;
        }
        output->log_scroll_max = (total_lines > max_lines) ? (total_lines - max_lines) : 0;
        if (skip < 0) skip = 0;
        if (skip > output->log_scroll_max) skip = output->log_scroll_max;
        ID2D1HwndRenderTarget_PushAxisAlignedClip(target, &output->log_viewport_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        for (li = model->console_log_count - 1; li >= 0 && shown < max_lines; li--) {
            int idx = (model->console_log_head + li) % 200;
            if (!callbacks->log_match_filter(model->console_logs[idx], model->log_filter_mode)) continue;
            if (skip > 0) { skip--; continue; }
            callbacks->draw_text(model->console_logs[idx],
                                 console_renderer_rect(output->log_viewport_rect.left + 2.0f, output->log_viewport_rect.top + shown * line_h,
                                                       output->log_viewport_rect.right - 2.0f, output->log_viewport_rect.top + (shown + 1) * line_h),
                                 fmt_info, shown == 0 ? D2D1::ColorF(0.90f, 0.95f, 1.0f, 1.0f) : D2D1::ColorF(0.76f, 0.85f, 0.95f, 1.0f));
            shown++;
        }
        ID2D1HwndRenderTarget_PopAxisAlignedClip(target);
    }

    if (output->log_scroll_max > 0) {
        float track_h;
        float thumb_h;
        float travel;
        float ratio;
        float thumb_top;
        output->log_scroll_track_rect = console_renderer_rect(output->log_viewport_rect.right + 2.0f, output->log_viewport_rect.top,
                                                              output->log_viewport_rect.right + 6.0f, output->log_viewport_rect.bottom);
        track_h = output->log_scroll_track_rect.bottom - output->log_scroll_track_rect.top;
        thumb_h = ((float)max_lines / (float)total_lines) * track_h;
        if (thumb_h < 20.0f) thumb_h = 20.0f;
        if (thumb_h > track_h) thumb_h = track_h;
        travel = track_h - thumb_h;
        ratio = (output->log_scroll_max > 0) ? ((float)((model->log_scroll_offset < 0) ? 0 : model->log_scroll_offset) / (float)output->log_scroll_max) : 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
        thumb_top = output->log_scroll_track_rect.top + travel * ratio;
        output->log_scroll_thumb_rect = console_renderer_rect(output->log_scroll_track_rect.left, thumb_top,
                                                              output->log_scroll_track_rect.right, thumb_top + thumb_h);
        callbacks->draw_card_round(output->log_scroll_track_rect, 2.0f, D2D1::ColorF(0.13f, 0.15f, 0.19f, 1.0f), D2D1::ColorF(0.24f, 0.28f, 0.35f, 1.0f));
        callbacks->draw_card_round(output->log_scroll_thumb_rect, 2.0f,
                                   console_renderer_point_in_rect(model->mouse_x, model->mouse_y, output->log_scroll_thumb_rect)
                                       ? D2D1::ColorF(0.40f, 0.52f, 0.69f, 1.0f)
                                       : D2D1::ColorF(0.33f, 0.43f, 0.57f, 1.0f),
                                   D2D1::ColorF(0.52f, 0.65f, 0.82f, 1.0f));
    }

    if (model->log_scroll_offset > 0) {
        callbacks->draw_text(L"滚动查看更多日志",
                             console_renderer_rect(bottom_rect.right - 170.0f, bottom_rect.top + 8.0f, bottom_rect.right - 34.0f, bottom_rect.top + 26.0f),
                             fmt_info, D2D1::ColorF(0.78f, 0.86f, 0.96f, 1.0f));
    }
}
