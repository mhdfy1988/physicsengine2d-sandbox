#include "inspector_debug_renderer.hpp"

#include <windows.h>
#include <stdio.h>

static D2D1_RECT_F debug_rect_make(float left, float top, float right, float bottom) {
    D2D1_RECT_F rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    return rect;
}

static int debug_point_in_rect(float x, float y, D2D1_RECT_F rect) {
    return x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
}

void inspector_debug_renderer_render(
    ID2D1HwndRenderTarget* target,
    IDWriteTextFormat* fmt_ui,
    IDWriteTextFormat* fmt_info,
    D2D1_RECT_F debug_rect,
    const InspectorDebugRendererModel* model,
    const InspectorDebugRendererCallbacks* callbacks,
    InspectorDebugRendererOutput* output) {
    wchar_t line[128];
    float debug_content_h;
    float debug_view_h;
    float debug_offset_y;
    float debug_content_right;
    int di;
    if (target == 0 || fmt_ui == 0 || fmt_info == 0 || model == 0 || callbacks == 0 || output == 0) return;

    output->viewport_rect = debug_rect_make(debug_rect.left + 8.0f, debug_rect.top + 40.0f, debug_rect.right - 14.0f, debug_rect.bottom - 8.0f);
    debug_content_right = output->viewport_rect.right - 2.0f;
    debug_view_h = output->viewport_rect.bottom - output->viewport_rect.top;
    debug_content_h = 1140.0f;
    output->scroll_max = 0;
    if (debug_content_h > debug_view_h) output->scroll_max = (int)(debug_content_h - debug_view_h + 0.5f);
    debug_offset_y = output->viewport_rect.top - (float)((model->scroll_offset < 0) ? 0 : (model->scroll_offset > output->scroll_max ? output->scroll_max : model->scroll_offset));
    output->runtime_tick_row_count = 0;
    output->runtime_state_row_count = 0;

    callbacks->draw_card_round(debug_rect, 10.0f, D2D1::ColorF(0.16f, 0.18f, 0.22f, 1.0f), D2D1::ColorF(0.29f, 0.33f, 0.40f, 1.0f));
    callbacks->draw_panel_header_band(debug_rect, 34.0f, 10.0f);
    callbacks->draw_text_vcenter(L"物理调试", debug_rect_make(debug_rect.left + 16.0f, debug_rect.top + 4.0f, debug_rect.right - 12.0f, debug_rect.top + 36.0f),
                                 fmt_ui, D2D1::ColorF(0.78f, 0.86f, 0.96f, 1.0f));

    ID2D1HwndRenderTarget_PushAxisAlignedClip(target, &output->viewport_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    callbacks->draw_text(L"模拟参数", debug_rect_make(debug_rect.left + 12.0f, debug_offset_y, debug_rect.left + 120.0f, debug_offset_y + 22.0f),
                         fmt_info, D2D1::ColorF(0.70f, 0.78f, 0.90f, 1.0f));
    {
        const wchar_t* labels[3] = {L"重力Y", L"时间步长", L"迭代"};
        int focused_row = model->focused_row;
        if (focused_row < 0) focused_row = 0;
        if (focused_row > 2) focused_row = 2;
        for (di = 0; di < 3; di++) {
            D2D1_RECT_F rr = debug_rect_make(debug_rect.left + 10.0f, debug_offset_y + 32.0f + di * 32.0f, debug_content_right, debug_offset_y + 58.0f + di * 32.0f);
            output->param_row_rect[di] = rr;
            output->param_minus_rect[di] = debug_rect_make(0, 0, 0, 0);
            output->param_plus_rect[di] = debug_rect_make(0, 0, 0, 0);
            callbacks->draw_card_round(rr, 5.0f,
                                       (di == focused_row) ? D2D1::ColorF(0.24f, 0.30f, 0.40f, 1.0f) : D2D1::ColorF(0.20f, 0.25f, 0.33f, 1.0f),
                                       (di == focused_row) ? D2D1::ColorF(0.41f, 0.50f, 0.64f, 1.0f) : D2D1::ColorF(0.33f, 0.40f, 0.52f, 1.0f));
            if (di == 0) swprintf(line, 128, L"%.2f", model->gravity_y);
            if (di == 1) swprintf(line, 128, L"%.4f", model->time_step);
            if (di == 2) swprintf(line, 128, L"%d", model->iterations);
            callbacks->draw_text(labels[di], debug_rect_make(rr.left + 6.0f, rr.top + 1.0f, rr.left + 110.0f, rr.bottom - 1.0f), fmt_info, D2D1::ColorF(0.74f, 0.81f, 0.91f, 1.0f));
            callbacks->draw_text(line, debug_rect_make(rr.left + 116.0f, rr.top + 1.0f, rr.right - 8.0f, rr.bottom - 1.0f), fmt_info, D2D1::ColorF(0.92f, 0.95f, 0.99f, 1.0f));
        }
    }
    callbacks->draw_text(L"运行时快照", debug_rect_make(debug_rect.left + 12.0f, debug_offset_y + 132.0f, debug_rect.left + 128.0f, debug_offset_y + 154.0f),
                         fmt_info, D2D1::ColorF(0.70f, 0.78f, 0.90f, 1.0f));
    {
        D2D1_RECT_F rt_rect = debug_rect_make(debug_rect.left + 10.0f, debug_offset_y + 162.0f, debug_content_right, debug_offset_y + 282.0f);
        callbacks->draw_card_round(rt_rect, 5.0f, D2D1::ColorF(0.20f, 0.25f, 0.33f, 1.0f), D2D1::ColorF(0.33f, 0.40f, 0.52f, 1.0f));
        swprintf(line, 128, L"帧序号: %u", model->runtime_frame_index);
        callbacks->draw_text(line, debug_rect_make(rt_rect.left + 8.0f, rt_rect.top + 4.0f, rt_rect.right - 8.0f, rt_rect.top + 24.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
        swprintf(line, 128, L"模拟状态: %ls", model->runtime_running ? L"运行" : L"暂停");
        callbacks->draw_text(line, debug_rect_make(rt_rect.left + 8.0f, rt_rect.top + 24.0f, rt_rect.right - 8.0f, rt_rect.top + 44.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
        swprintf(line, 128, L"对象: %d  约束: %d", model->runtime_body_count, model->runtime_constraint_count);
        callbacks->draw_text(line, debug_rect_make(rt_rect.left + 8.0f, rt_rect.top + 44.0f, rt_rect.right - 8.0f, rt_rect.top + 64.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
        swprintf(line, 128, L"接触: %d  单帧耗时: %.2fms", model->runtime_contact_count, model->physics_step_ms);
        callbacks->draw_text(line, debug_rect_make(rt_rect.left + 8.0f, rt_rect.top + 64.0f, rt_rect.right - 8.0f, rt_rect.top + 84.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
        swprintf(line, 128, L"状态变更事件: %d  错误: %d", model->runtime_state_change_count, model->runtime_error_count);
        callbacks->draw_text(line, debug_rect_make(rt_rect.left + 8.0f, rt_rect.top + 84.0f, rt_rect.right - 8.0f, rt_rect.top + 104.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
        if (model->runtime_error_item_count > 0) {
            swprintf(line, 128, L"首错: %d(%ls,%ls,x%d)  丢弃:%u",
                     model->runtime_errors[0].code,
                     callbacks->runtime_error_label(model->runtime_errors[0].code),
                     callbacks->runtime_error_severity_label(model->runtime_errors[0].severity),
                     model->runtime_errors[0].count,
                     model->runtime_event_drop_count);
        } else {
            swprintf(line, 128, L"错误码: %d(%ls)  事件丢弃累计: %u",
                     model->runtime_error_code, callbacks->runtime_error_label(model->runtime_error_code), model->runtime_event_drop_count);
        }
        callbacks->draw_text(line, debug_rect_make(rt_rect.left + 8.0f, rt_rect.top + 104.0f, rt_rect.right - 8.0f, rt_rect.top + 118.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
    }
    callbacks->draw_text(L"最近帧(最新3条)", debug_rect_make(debug_rect.left + 12.0f, debug_offset_y + 292.0f, debug_rect.left + 180.0f, debug_offset_y + 314.0f),
                         fmt_info, D2D1::ColorF(0.70f, 0.78f, 0.90f, 1.0f));
    {
        int i;
        int rows = (model->runtime_tick_history_count < 3) ? model->runtime_tick_history_count : 3;
        D2D1_RECT_F hist_rect = debug_rect_make(debug_rect.left + 10.0f, debug_offset_y + 320.0f, debug_content_right, debug_offset_y + 398.0f);
        callbacks->draw_card_round(hist_rect, 5.0f, D2D1::ColorF(0.20f, 0.25f, 0.33f, 1.0f), D2D1::ColorF(0.33f, 0.40f, 0.52f, 1.0f));
        if (rows <= 0) {
            callbacks->draw_text(L"暂无帧快照记录", debug_rect_make(hist_rect.left + 8.0f, hist_rect.top + 6.0f, hist_rect.right - 8.0f, hist_rect.top + 26.0f),
                                 fmt_info, D2D1::ColorF(0.66f, 0.74f, 0.84f, 1.0f));
        } else {
            for (i = 0; i < rows; i++) {
                int idx = (model->runtime_tick_history_head + model->runtime_tick_history_count - 1 - i + INSPECTOR_DEBUG_HISTORY_CAP) % INSPECTOR_DEBUG_HISTORY_CAP;
                InspectorDebugTickHistoryEntry entry = model->runtime_tick_history[idx];
                D2D1_RECT_F row_rect = debug_rect_make(hist_rect.left + 2.0f, hist_rect.top + 4.0f + i * 22.0f, hist_rect.right - 2.0f, hist_rect.top + 24.0f + i * 22.0f);
                output->runtime_tick_row_rect[i] = row_rect;
                output->runtime_tick_row_entry_index[i] = idx;
                output->runtime_tick_row_count = i + 1;
                callbacks->draw_card_round(row_rect, 3.0f,
                                           debug_point_in_rect(model->mouse_x, model->mouse_y, row_rect) ? D2D1::ColorF(0.26f, 0.32f, 0.42f, 1.0f) : D2D1::ColorF(0.20f, 0.25f, 0.33f, 0.0f),
                                           D2D1::ColorF(0.00f, 0.00f, 0.00f, 0.0f));
                swprintf(line, 128, L"#%u 对象:%d 约束:%d 接触:%d %.2fms", entry.frame_index, entry.body_count, entry.constraint_count, entry.contact_count, entry.step_ms);
                callbacks->draw_text(line, debug_rect_make(row_rect.left + 6.0f, row_rect.top + 1.0f, row_rect.right - 6.0f, row_rect.bottom - 1.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
            }
        }
    }
    callbacks->draw_text(L"状态时间线(最新3条)", debug_rect_make(debug_rect.left + 12.0f, debug_offset_y + 410.0f, debug_rect.left + 188.0f, debug_offset_y + 432.0f),
                         fmt_info, D2D1::ColorF(0.70f, 0.78f, 0.90f, 1.0f));
    {
        int i;
        int rows = (model->runtime_state_history_count < 3) ? model->runtime_state_history_count : 3;
        unsigned int now_ms = (unsigned int)GetTickCount();
        D2D1_RECT_F state_rect = debug_rect_make(debug_rect.left + 10.0f, debug_offset_y + 438.0f, debug_content_right, debug_offset_y + 516.0f);
        callbacks->draw_card_round(state_rect, 5.0f, D2D1::ColorF(0.20f, 0.25f, 0.33f, 1.0f), D2D1::ColorF(0.33f, 0.40f, 0.52f, 1.0f));
        if (rows <= 0) {
            callbacks->draw_text(L"暂无状态切换记录", debug_rect_make(state_rect.left + 8.0f, state_rect.top + 6.0f, state_rect.right - 8.0f, state_rect.top + 26.0f), fmt_info, D2D1::ColorF(0.66f, 0.74f, 0.84f, 1.0f));
        } else {
            for (i = 0; i < rows; i++) {
                int idx = (model->runtime_state_history_head + model->runtime_state_history_count - 1 - i + INSPECTOR_DEBUG_HISTORY_CAP) % INSPECTOR_DEBUG_HISTORY_CAP;
                InspectorDebugStateHistoryEntry entry = model->runtime_state_history[idx];
                unsigned int age_ms = now_ms - entry.tick_ms;
                D2D1_RECT_F row_rect = debug_rect_make(state_rect.left + 2.0f, state_rect.top + 4.0f + i * 22.0f, state_rect.right - 2.0f, state_rect.top + 24.0f + i * 22.0f);
                output->runtime_state_row_rect[i] = row_rect;
                output->runtime_state_row_entry_index[i] = idx;
                output->runtime_state_row_count = i + 1;
                callbacks->draw_card_round(row_rect, 3.0f,
                                           debug_point_in_rect(model->mouse_x, model->mouse_y, row_rect) ? D2D1::ColorF(0.26f, 0.32f, 0.42f, 1.0f) : D2D1::ColorF(0.20f, 0.25f, 0.33f, 0.0f),
                                           D2D1::ColorF(0.00f, 0.00f, 0.00f, 0.0f));
                swprintf(line, 128, L"#%u -> %ls (%ums前)", entry.frame_index, entry.running ? L"运行" : L"暂停", age_ms);
                callbacks->draw_text(line, debug_rect_make(row_rect.left + 6.0f, row_rect.top + 1.0f, row_rect.right - 6.0f, row_rect.bottom - 1.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
            }
        }
    }
    callbacks->draw_text(L"性能摘要", debug_rect_make(debug_rect.left + 12.0f, debug_offset_y + 528.0f, debug_rect.left + 128.0f, debug_offset_y + 550.0f), fmt_info, D2D1::ColorF(0.70f, 0.78f, 0.90f, 1.0f));
    {
        D2D1_RECT_F perf_rect = debug_rect_make(debug_rect.left + 10.0f, debug_offset_y + 556.0f, debug_content_right, debug_offset_y + 654.0f);
        float fps_avg = 0.0f;
        float fps_min = 9999.0f;
        float fps_max = 0.0f;
        float step_avg = 0.0f;
        float step_max = 0.0f;
        int i;
        callbacks->draw_card_round(perf_rect, 5.0f, D2D1::ColorF(0.20f, 0.25f, 0.33f, 1.0f), D2D1::ColorF(0.33f, 0.40f, 0.52f, 1.0f));
        if (model->perf_hist_count <= 0) {
            callbacks->draw_text(L"暂无性能历史，进入 PIE 后会逐帧采集", debug_rect_make(perf_rect.left + 8.0f, perf_rect.top + 8.0f, perf_rect.right - 8.0f, perf_rect.top + 28.0f), fmt_info, D2D1::ColorF(0.66f, 0.74f, 0.84f, 1.0f));
        } else {
            for (i = 0; i < model->perf_hist_count; i++) {
                int idx = (model->perf_hist_head + i) % 180;
                float fps_v = model->fps_hist[idx];
                float step_v = model->step_hist[idx];
                fps_avg += fps_v;
                step_avg += step_v;
                if (fps_v < fps_min) fps_min = fps_v;
                if (fps_v > fps_max) fps_max = fps_v;
                if (step_v > step_max) step_max = step_v;
            }
            fps_avg /= (float)model->perf_hist_count;
            step_avg /= (float)model->perf_hist_count;
            swprintf(line, 128, L"FPS 平均/最小/最大: %.1f / %.1f / %.1f", fps_avg, fps_min, fps_max);
            callbacks->draw_text(line, debug_rect_make(perf_rect.left + 8.0f, perf_rect.top + 6.0f, perf_rect.right - 8.0f, perf_rect.top + 26.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
            swprintf(line, 128, L"步进耗时 平均/峰值: %.2fms / %.2fms", step_avg, step_max);
            callbacks->draw_text(line, debug_rect_make(perf_rect.left + 8.0f, perf_rect.top + 28.0f, perf_rect.right - 8.0f, perf_rect.top + 48.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
            swprintf(line, 128, L"样本:%d  当前帧:%u  当前状态:%ls", model->perf_hist_count, model->runtime_frame_index, model->pie_active ? (model->runtime_running ? L"PIE运行" : L"PIE暂停") : L"编辑态");
            callbacks->draw_text(line, debug_rect_make(perf_rect.left + 8.0f, perf_rect.top + 50.0f, perf_rect.right - 8.0f, perf_rect.top + 70.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
            swprintf(line, 128, L"热重载累计 +%u / -%u  事件丢弃:%u", model->hot_reload_total_imported, model->hot_reload_total_failed, model->runtime_event_drop_count);
            callbacks->draw_text(line, debug_rect_make(perf_rect.left + 8.0f, perf_rect.top + 72.0f, perf_rect.right - 8.0f, perf_rect.top + 92.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
        }
    }
    callbacks->draw_text(L"最近事件流", debug_rect_make(debug_rect.left + 12.0f, debug_offset_y + 666.0f, debug_rect.left + 144.0f, debug_offset_y + 688.0f), fmt_info, D2D1::ColorF(0.70f, 0.78f, 0.90f, 1.0f));
    {
        D2D1_RECT_F event_rect = debug_rect_make(debug_rect.left + 10.0f, debug_offset_y + 694.0f, debug_content_right, debug_offset_y + 790.0f);
        int rows = (model->debug_event_history_count < 4) ? model->debug_event_history_count : 4;
        int i;
        unsigned int now_ms = (unsigned int)GetTickCount();
        callbacks->draw_card_round(event_rect, 5.0f, D2D1::ColorF(0.20f, 0.25f, 0.33f, 1.0f), D2D1::ColorF(0.33f, 0.40f, 0.52f, 1.0f));
        if (rows <= 0) {
            callbacks->draw_text(L"暂无事件流记录", debug_rect_make(event_rect.left + 8.0f, event_rect.top + 8.0f, event_rect.right - 8.0f, event_rect.top + 28.0f), fmt_info, D2D1::ColorF(0.66f, 0.74f, 0.84f, 1.0f));
        } else {
            for (i = 0; i < rows; i++) {
                int idx = (model->debug_event_history_head + model->debug_event_history_count - 1 - i + INSPECTOR_DEBUG_HISTORY_CAP) % INSPECTOR_DEBUG_HISTORY_CAP;
                unsigned int age_ms = now_ms - model->debug_event_history[idx].tick_ms;
                swprintf(line, 128, L"[%ums前] %ls", age_ms, model->debug_event_history[idx].text);
                callbacks->draw_text(line, debug_rect_make(event_rect.left + 8.0f, event_rect.top + 6.0f + i * 22.0f, event_rect.right - 8.0f, event_rect.top + 26.0f + i * 22.0f), fmt_info,
                                     i == 0 ? D2D1::ColorF(0.90f, 0.95f, 1.0f, 1.0f) : D2D1::ColorF(0.76f, 0.85f, 0.95f, 1.0f));
            }
        }
    }
    callbacks->draw_text(L"错误流", debug_rect_make(debug_rect.left + 12.0f, debug_offset_y + 802.0f, debug_rect.left + 112.0f, debug_offset_y + 824.0f), fmt_info, D2D1::ColorF(0.70f, 0.78f, 0.90f, 1.0f));
    {
        D2D1_RECT_F error_rect = debug_rect_make(debug_rect.left + 10.0f, debug_offset_y + 830.0f, debug_content_right, debug_offset_y + 926.0f);
        int rows = (model->debug_error_history_count < 4) ? model->debug_error_history_count : 4;
        int i;
        unsigned int now_ms = (unsigned int)GetTickCount();
        callbacks->draw_card_round(error_rect, 5.0f, D2D1::ColorF(0.20f, 0.25f, 0.33f, 1.0f), D2D1::ColorF(0.33f, 0.40f, 0.52f, 1.0f));
        if (rows <= 0) {
            callbacks->draw_text(L"暂无错误流记录", debug_rect_make(error_rect.left + 8.0f, error_rect.top + 8.0f, error_rect.right - 8.0f, error_rect.top + 28.0f), fmt_info, D2D1::ColorF(0.66f, 0.74f, 0.84f, 1.0f));
        } else {
            for (i = 0; i < rows; i++) {
                int idx = (model->debug_error_history_head + model->debug_error_history_count - 1 - i + INSPECTOR_DEBUG_HISTORY_CAP) % INSPECTOR_DEBUG_HISTORY_CAP;
                unsigned int age_ms = now_ms - model->debug_error_history[idx].tick_ms;
                swprintf(line, 128, L"[%ums前] %ls", age_ms, model->debug_error_history[idx].text);
                callbacks->draw_text(line, debug_rect_make(error_rect.left + 8.0f, error_rect.top + 6.0f + i * 22.0f, error_rect.right - 8.0f, error_rect.top + 26.0f + i * 22.0f), fmt_info,
                                     i == 0 ? D2D1::ColorF(0.99f, 0.85f, 0.80f, 1.0f) : D2D1::ColorF(0.93f, 0.73f, 0.68f, 1.0f));
            }
        }
    }
    callbacks->draw_text(L"热重载批次", debug_rect_make(debug_rect.left + 12.0f, debug_offset_y + 938.0f, debug_rect.left + 144.0f, debug_offset_y + 960.0f), fmt_info, D2D1::ColorF(0.70f, 0.78f, 0.90f, 1.0f));
    {
        D2D1_RECT_F hot_rect = debug_rect_make(debug_rect.left + 10.0f, debug_offset_y + 966.0f, debug_content_right, debug_offset_y + 1128.0f);
        int rows = (model->hot_reload_history_count < 2) ? model->hot_reload_history_count : 2;
        int i;
        float y = hot_rect.top + 6.0f;
        callbacks->draw_card_round(hot_rect, 5.0f, D2D1::ColorF(0.20f, 0.25f, 0.33f, 1.0f), D2D1::ColorF(0.33f, 0.40f, 0.52f, 1.0f));
        swprintf(line, 128, L"监听:%d  最近扫描变化:%d  就绪批次:%d", model->hot_reload_watch_count, model->hot_reload_scan_change_count, model->hot_reload_ready_batch_count);
        callbacks->draw_text(line, debug_rect_make(hot_rect.left + 8.0f, y, hot_rect.right - 8.0f, y + 20.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
        y += 22.0f;
        swprintf(line, 128, L"总计导入:%u  总计失败:%u  当前模式:%ls", model->hot_reload_total_imported, model->hot_reload_total_failed, model->pie_active ? L"PIE" : L"编辑态");
        callbacks->draw_text(line, debug_rect_make(hot_rect.left + 8.0f, y, hot_rect.right - 8.0f, y + 20.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
        y += 26.0f;
        if (rows <= 0) {
            callbacks->draw_text(L"暂无热重载批次历史", debug_rect_make(hot_rect.left + 8.0f, y, hot_rect.right - 8.0f, y + 20.0f), fmt_info, D2D1::ColorF(0.66f, 0.74f, 0.84f, 1.0f));
        } else {
            unsigned int now_ms = (unsigned int)GetTickCount();
            for (i = 0; i < rows; i++) {
                int idx = (model->hot_reload_history_head + model->hot_reload_history_count - 1 - i + INSPECTOR_DEBUG_HISTORY_CAP) % INSPECTOR_DEBUG_HISTORY_CAP;
                const InspectorDebugHotReloadHistoryEntry* entry = &model->hot_reload_history[idx];
                unsigned int age_ms = now_ms - entry->tick_ms;
                wchar_t guid_line[128];
                int wlen = 0;
                int j;
                swprintf(line, 128, L"[%ums前][%ls] 导入%d 失败%d 影响%d 就绪%d %ls", age_ms, entry->pie_active ? L"PIE" : L"编辑", entry->imported_count, entry->failed_count, entry->affected_count, entry->ready_batch_count, entry->rollback_retained ? L"回退:保留旧缓存" : L"");
                callbacks->draw_text(line, debug_rect_make(hot_rect.left + 8.0f, y, hot_rect.right - 8.0f, y + 20.0f), fmt_info, D2D1::ColorF(0.90f, 0.94f, 0.99f, 1.0f));
                y += 20.0f;
                guid_line[0] = L'\0';
                if (entry->imported_guid_count > 0) {
                    for (j = 0; j < entry->imported_guid_count && j < 2; j++) {
                        wchar_t guid_w[ASSET_DB_MAX_GUID];
                        int converted = MultiByteToWideChar(CP_UTF8, 0, entry->imported_guids[j], -1, guid_w, ASSET_DB_MAX_GUID);
                        if (converted <= 0) lstrcpynW(guid_w, L"(guid-decode-failed)", ASSET_DB_MAX_GUID);
                        if (wlen > 0 && wlen < 120) wlen += swprintf(guid_line + wlen, 128 - wlen, L", ");
                        if (wlen < 120) wlen += swprintf(guid_line + wlen, 128 - wlen, L"%ls", guid_w);
                    }
                    if (entry->imported_guid_count > 2 && wlen < 120) swprintf(guid_line + wlen, 128 - wlen, L" ... +%d", entry->imported_guid_count - 2);
                } else if (entry->rollback_retained) {
                    lstrcpynW(guid_line, L"失败批次未替换缓存，旧资源仍可继续使用", 128);
                } else {
                    lstrcpynW(guid_line, L"本批次没有导入 GUID 明细", 128);
                }
                callbacks->draw_text(guid_line, debug_rect_make(hot_rect.left + 20.0f, y, hot_rect.right - 8.0f, y + 18.0f), fmt_info, D2D1::ColorF(0.74f, 0.82f, 0.92f, 1.0f));
                y += 24.0f;
            }
        }
    }
    ID2D1HwndRenderTarget_PopAxisAlignedClip(target);
}
