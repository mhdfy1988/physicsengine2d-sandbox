#ifndef CONSOLE_RENDERER_H
#define CONSOLE_RENDERER_H

#include <d2d1.h>
#include <dwrite.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { CONSOLE_RENDERER_LINE_MAX = 180 };
enum { CONSOLE_RENDERER_EVENT_ROWS_MAX = 24 };

typedef struct {
    int log_filter_mode;
    int log_scroll_offset;
    int collision_event_filter_selected_only;
    int log_search_len;
    const wchar_t* log_search_buf;
    int console_log_count;
    int console_log_head;
    const wchar_t (*console_logs)[CONSOLE_RENDERER_LINE_MAX];
    int collision_event_count;
    int collision_event_head;
    float mouse_x;
    float mouse_y;
} ConsoleRendererModel;

typedef struct {
    D2D1_RECT_F log_search_clear_rect;
    D2D1_RECT_F log_search_rect;
    D2D1_RECT_F log_clear_rect;
    D2D1_RECT_F log_filter_warn_rect;
    D2D1_RECT_F log_filter_collision_rect;
    D2D1_RECT_F log_filter_physics_rect;
    D2D1_RECT_F log_filter_state_rect;
    D2D1_RECT_F log_filter_all_rect;
    D2D1_RECT_F dbg_collision_filter_rect;
    D2D1_RECT_F log_viewport_rect;
    D2D1_RECT_F log_scroll_track_rect;
    D2D1_RECT_F log_scroll_thumb_rect;
    D2D1_RECT_F dbg_collision_row_rect[CONSOLE_RENDERER_EVENT_ROWS_MAX];
    int dbg_collision_row_event_index[CONSOLE_RENDERER_EVENT_ROWS_MAX];
    int dbg_collision_row_count;
    int log_scroll_max;
} ConsoleRendererOutput;

typedef struct {
    void (*draw_text_tab_button)(D2D1_RECT_F rect, const wchar_t* text, int active, int hover);
    void (*draw_action_button)(D2D1_RECT_F rect, const wchar_t* text, int active, int hover);
    void (*draw_text)(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* format, D2D1_COLOR_F color);
    void (*draw_card_round)(D2D1_RECT_F rect, float radius, D2D1_COLOR_F fill, D2D1_COLOR_F border);
    int (*log_match_filter)(const wchar_t* text, int mode);
    int (*collision_event_involves_selected)(int event_index);
    int (*format_collision_event_line)(int event_index, wchar_t* buffer, int buffer_count);
} ConsoleRendererCallbacks;

void console_renderer_render(
    ID2D1HwndRenderTarget* target,
    IDWriteTextFormat* fmt_info,
    D2D1_RECT_F bottom_rect,
    const ConsoleRendererModel* model,
    const ConsoleRendererCallbacks* callbacks,
    ConsoleRendererOutput* output);

#ifdef __cplusplus
}
#endif

#endif
