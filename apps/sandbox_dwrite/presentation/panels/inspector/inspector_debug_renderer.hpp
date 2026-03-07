#ifndef INSPECTOR_DEBUG_RENDERER_H
#define INSPECTOR_DEBUG_RENDERER_H

#include <d2d1.h>
#include <dwrite.h>

#include "physics_content/asset_database.hpp"

#ifdef __cplusplus
extern "C" {
#endif

enum { INSPECTOR_DEBUG_HISTORY_CAP = 8 };
enum { INSPECTOR_DEBUG_PARAM_ROWS = 3 };
enum { INSPECTOR_DEBUG_RUNTIME_ROWS = 3 };
enum { INSPECTOR_DEBUG_HOT_RELOAD_MAX_IMPORTED = 16 };

typedef struct {
    unsigned int frame_index;
    int body_count;
    int constraint_count;
    int contact_count;
    float step_ms;
} InspectorDebugTickHistoryEntry;

typedef struct {
    unsigned int frame_index;
    unsigned int tick_ms;
    int running;
} InspectorDebugStateHistoryEntry;

typedef struct {
    unsigned int tick_ms;
    wchar_t text[160];
} InspectorDebugTextHistoryEntry;

typedef struct {
    unsigned int tick_ms;
    int pie_active;
    int ready_batch_count;
    int affected_count;
    int imported_count;
    int failed_count;
    int rollback_retained;
    int imported_guid_count;
    char imported_guids[INSPECTOR_DEBUG_HOT_RELOAD_MAX_IMPORTED][ASSET_DB_MAX_GUID];
} InspectorDebugHotReloadHistoryEntry;

typedef struct {
    int code;
    int severity;
    int count;
} InspectorDebugRuntimeErrorEntry;

typedef struct {
    int focused_row;
    int scroll_offset;
    float mouse_x;
    float mouse_y;
    float gravity_y;
    float time_step;
    int iterations;
    unsigned int runtime_frame_index;
    int runtime_running;
    int runtime_body_count;
    int runtime_constraint_count;
    int runtime_contact_count;
    float physics_step_ms;
    int runtime_state_change_count;
    int runtime_error_count;
    int runtime_error_item_count;
    InspectorDebugRuntimeErrorEntry runtime_errors[4];
    int runtime_error_code;
    unsigned int runtime_event_drop_count;
    InspectorDebugTickHistoryEntry runtime_tick_history[INSPECTOR_DEBUG_HISTORY_CAP];
    int runtime_tick_history_head;
    int runtime_tick_history_count;
    InspectorDebugStateHistoryEntry runtime_state_history[INSPECTOR_DEBUG_HISTORY_CAP];
    int runtime_state_history_head;
    int runtime_state_history_count;
    int perf_hist_count;
    int perf_hist_head;
    const float* fps_hist;
    const float* step_hist;
    int pie_active;
    unsigned int hot_reload_total_imported;
    unsigned int hot_reload_total_failed;
    int hot_reload_watch_count;
    int hot_reload_scan_change_count;
    int hot_reload_ready_batch_count;
    InspectorDebugTextHistoryEntry debug_event_history[INSPECTOR_DEBUG_HISTORY_CAP];
    int debug_event_history_head;
    int debug_event_history_count;
    InspectorDebugTextHistoryEntry debug_error_history[INSPECTOR_DEBUG_HISTORY_CAP];
    int debug_error_history_head;
    int debug_error_history_count;
    InspectorDebugHotReloadHistoryEntry hot_reload_history[INSPECTOR_DEBUG_HISTORY_CAP];
    int hot_reload_history_head;
    int hot_reload_history_count;
} InspectorDebugRendererModel;

typedef struct {
    D2D1_RECT_F viewport_rect;
    D2D1_RECT_F param_row_rect[INSPECTOR_DEBUG_PARAM_ROWS];
    D2D1_RECT_F param_minus_rect[INSPECTOR_DEBUG_PARAM_ROWS];
    D2D1_RECT_F param_plus_rect[INSPECTOR_DEBUG_PARAM_ROWS];
    D2D1_RECT_F runtime_tick_row_rect[INSPECTOR_DEBUG_RUNTIME_ROWS];
    int runtime_tick_row_entry_index[INSPECTOR_DEBUG_RUNTIME_ROWS];
    int runtime_tick_row_count;
    D2D1_RECT_F runtime_state_row_rect[INSPECTOR_DEBUG_RUNTIME_ROWS];
    int runtime_state_row_entry_index[INSPECTOR_DEBUG_RUNTIME_ROWS];
    int runtime_state_row_count;
    int scroll_max;
} InspectorDebugRendererOutput;

typedef struct {
    void (*draw_card_round)(D2D1_RECT_F rect, float radius, D2D1_COLOR_F fill, D2D1_COLOR_F border);
    void (*draw_text)(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color);
    void (*draw_text_vcenter)(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color);
    void (*draw_panel_header_band)(D2D1_RECT_F rect, float header_height, float radius);
    const wchar_t* (*runtime_error_label)(int code);
    const wchar_t* (*runtime_error_severity_label)(int severity);
} InspectorDebugRendererCallbacks;

void inspector_debug_renderer_render(
    ID2D1HwndRenderTarget* target,
    IDWriteTextFormat* fmt_ui,
    IDWriteTextFormat* fmt_info,
    D2D1_RECT_F debug_rect,
    const InspectorDebugRendererModel* model,
    const InspectorDebugRendererCallbacks* callbacks,
    InspectorDebugRendererOutput* output);

#ifdef __cplusplus
}
#endif

#endif
