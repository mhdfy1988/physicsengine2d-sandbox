#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define COBJMACROS

#include <windows.h>
#include <windowsx.h>
#include <initguid.h>
#include <d2d1.h>
#include <dwrite.h>
#include <dxgiformat.h>
#include <wincodec.h>
#include <objbase.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#include "physics.h"
#include "infrastructure/project_tree.h"
#include "infrastructure/snapshot_repo.h"
#include "infrastructure/ui_layout_repo.h"
#include "presentation/render/ui_icons.h"
#include "presentation/render/ui_primitives.h"
#include "presentation/render/ui_text.h"
#include "presentation/render/ui_widgets.h"
#include "domain/app_command.h"
#include "application/app_runtime.h"
#include "application/scene_catalog.h"
#include "application/scene_builder.h"
#include "application/history_service.h"
#include "application/runtime_param_service.h"
#include "presentation/input/input_mapping.h"
#include "presentation/input/menu_file_edit_actions.h"
#include "presentation/input/menu_view_physics_window_actions.h"
#include "presentation/input/menu_help_actions.h"
#include "presentation/input/menu_model.h"

#define IDI_APP_MAIN 1001

typedef struct {
    ID2D1Factory* d2d_factory;
    IDWriteFactory* dwrite_factory;
    IWICImagingFactory* wic_factory;
    ID2D1HwndRenderTarget* target;
    ID2D1Bitmap* app_icon_bitmap;
    ID2D1SolidColorBrush* brush;
    IDWriteTextFormat* fmt_title;
    IDWriteTextFormat* fmt_value;
    IDWriteTextFormat* fmt_ui;
    IDWriteTextFormat* fmt_mono;
    IDWriteTextFormat* fmt_info;
    IDWriteTextFormat* fmt_button;
    IDWriteTextFormat* fmt_icon;
    IDWriteTextFormat* fmt_value_center;
    IDWriteTextFormat* fmt_mono_center;
} UiApp;

typedef struct {
    float x;
    float y;
} ScreenPt;

typedef enum {
    INSPECTOR_NONE = 0,
    INSPECTOR_BODY,
    INSPECTOR_CONSTRAINT
} InspectorTargetType;

typedef struct {
    int type;
    float position_x;
    float position_y;
    float velocity_x;
    float velocity_y;
    float angular_velocity;
    float mass;
} BodyDraft;

typedef struct {
    float rest_length;
    float stiffness;
    float damping;
    float break_force;
    int collide_connected;
    int active;
} ConstraintDraft;

typedef struct {
    unsigned int tick_ms;
    int scene_index;
    int body_a_index;
    int body_b_index;
    Vec2 point;
    float rel_speed;
    float penetration;
} CollisionEvent;

typedef struct {
    int valid;
    int is_circle;
    float mass;
    float restitution;
    float friction;
    float damping;
    float size;
    Vec2 velocity;
    float angular_velocity;
} BodyClipboard;

typedef struct {
    PhysicsEngine* engine;
    SceneConfig scenes[SCENE_COUNT];
    int scene_index;
    int running;
    int draw_contacts;
    int draw_velocity;
    int draw_centers;
    int draw_constraints;
    RigidBody* selected;
    int selected_constraint_index;
    RigidBody* constraint_seed_body;
    int constraint_create_mode;
    float constraint_break_force_default;
    int spring_preset_mode;
    int dragging;
    int show_config_modal;
    int show_help_modal;
    int help_modal_page;
    int show_inspector_modal;
    int show_value_input;
    int value_input_target;
    int value_input_row;
    wchar_t value_input_buf[64];
    int value_input_len;
    int value_input_caret;
    int open_menu_id;
    int open_menu_focus_index;
    int bottom_panel_collapsed;
    int bottom_active_tab;
    int ui_show_left_panel;
    int ui_show_right_panel;
    int ui_show_bottom_panel;
    int ui_theme_light;
    int fps_display;
    float physics_step_ms;
    unsigned int fps_last_tick_ms;
    int fps_accum_frames;
    int last_contact_count;
    unsigned int last_contact_log_ms;
    unsigned int last_collision_capture_ms;
    unsigned int last_autosave_ms;
    float ui_left_ratio;
    float ui_right_ratio;
    float ui_bottom_open_h;
    int ui_drag_splitter;
    int ui_layout_preset;
    int log_filter_mode;
    int log_scroll_offset;
    int hierarchy_scroll_offset;
    int hierarchy_scroll_max;
    int hierarchy_scroll_dragging;
    float hierarchy_scroll_drag_start_y;
    int hierarchy_scroll_offset_start;
    int tree_scene_expanded;
    int tree_bodies_expanded;
    int tree_constraints_expanded;
    int tree_project_expanded;
    int tree_body_circle_expanded;
    int tree_body_polygon_expanded;
    int tree_constraint_distance_expanded;
    int tree_constraint_spring_expanded;
    int project_scroll_offset;
    int project_scroll_max;
    unsigned int project_tree_last_scan_ms;
    int inspector_scroll_offset;
    int inspector_scroll_max;
    int debug_scroll_offset;
    int debug_scroll_max;
    int history_cursor;
    int history_top;
    int history_count;
    int history_replaying;
    wchar_t log_search_buf[48];
    int log_search_len;
    wchar_t hierarchy_filter_buf[32];
    int hierarchy_filter_len;
    float fps_hist[180];
    float step_hist[180];
    int perf_hist_head;
    int perf_hist_count;
    int focused_param;
    int inspector_focused_row;
    int debug_focused_row;
    int keyboard_focus_area;
    InspectorTargetType inspector_target_type;
    RigidBody* inspector_body_target;
    Constraint* inspector_constraint_target;
    BodyDraft inspector_body_draft;
    ConstraintDraft inspector_constraint_draft;
    int recycled_count;
    float stage_left;
    float stage_top;
    float stage_right;
    float stage_bottom;
    int scene_needs_stage_fit;
    ScreenPt mouse_screen;
} SandboxState;

static UiApp g_ui = {0};
static SandboxState g_state = {0};
static HWND g_app_hwnd = NULL;
static AppRuntime g_app_runtime;
static const float WORLD_SCALE = 12.0f;
static const float WORLD_ORIGIN_X = 20.0f;
static const float WORLD_ORIGIN_Y = 20.0f;
enum { EXPLORER_MAX_ITEMS = 512 };
enum { DEBUG_EVENT_ROWS_MAX = 24 };
enum { INSPECTOR_MAX_ROWS = 12 };

typedef struct {
    D2D1_RECT_F modal_rect;
    D2D1_RECT_F modal_close_rect;
    D2D1_RECT_F value_modal_rect;
    D2D1_RECT_F cfg_row_rect[8];
    D2D1_RECT_F cfg_minus_rect[8];
    D2D1_RECT_F cfg_plus_rect[8];
    D2D1_RECT_F top_run_rect;
    D2D1_RECT_F top_step_rect;
    D2D1_RECT_F top_reset_rect;
    D2D1_RECT_F top_save_rect;
    D2D1_RECT_F top_undo_rect;
    D2D1_RECT_F top_redo_rect;
    D2D1_RECT_F top_grid_rect;
    D2D1_RECT_F top_collision_rect;
    D2D1_RECT_F top_velocity_rect;
    D2D1_RECT_F top_constraint_rect;
    D2D1_RECT_F top_spring_rect;
    D2D1_RECT_F top_chain_rect;
    D2D1_RECT_F top_rope_rect;
    D2D1_RECT_F menu_file_rect;
    D2D1_RECT_F menu_edit_rect;
    D2D1_RECT_F menu_view_rect;
    D2D1_RECT_F menu_gameobject_rect;
    D2D1_RECT_F menu_component_rect;
    D2D1_RECT_F menu_physics_rect;
    D2D1_RECT_F menu_window_rect;
    D2D1_RECT_F menu_help_word_rect;
    D2D1_RECT_F menu_app_icon_rect;
    D2D1_RECT_F menu_bar_drag_rect;
    D2D1_RECT_F win_min_rect;
    D2D1_RECT_F win_max_rect;
    D2D1_RECT_F win_close_rect;
    D2D1_RECT_F menu_dropdown_rect;
    D2D1_RECT_F menu_item_rect[8];
    int menu_item_enabled[8];
    int menu_item_count;
    D2D1_RECT_F explorer_scene_rect;
    D2D1_RECT_F tree_scene_header_rect;
    D2D1_RECT_F tree_bodies_header_rect;
    D2D1_RECT_F tree_constraints_header_rect;
    D2D1_RECT_F tree_body_circle_header_rect;
    D2D1_RECT_F tree_body_polygon_header_rect;
    D2D1_RECT_F tree_constraint_distance_header_rect;
    D2D1_RECT_F tree_constraint_spring_header_rect;
    D2D1_RECT_F hierarchy_search_rect;
    D2D1_RECT_F hierarchy_search_clear_rect;
    D2D1_RECT_F hierarchy_viewport_rect;
    D2D1_RECT_F hierarchy_scroll_track_rect;
    D2D1_RECT_F hierarchy_scroll_thumb_rect;
    D2D1_RECT_F project_viewport_rect;
    D2D1_RECT_F tree_project_root_rect;
    D2D1_RECT_F project_scroll_track_rect;
    D2D1_RECT_F project_scroll_thumb_rect;
    D2D1_RECT_F explorer_body_rect[EXPLORER_MAX_ITEMS];
    D2D1_RECT_F explorer_constraint_rect[EXPLORER_MAX_ITEMS];
    RigidBody* explorer_body_ptr[EXPLORER_MAX_ITEMS];
    int explorer_constraint_index[EXPLORER_MAX_ITEMS];
    int explorer_body_count;
    int explorer_constraint_count;
    D2D1_RECT_F bottom_tab_console_rect;
    D2D1_RECT_F bottom_tab_perf_rect;
    D2D1_RECT_F bottom_fold_rect;
    D2D1_RECT_F left_fold_rect;
    D2D1_RECT_F right_fold_rect;
    D2D1_RECT_F log_filter_all_rect;
    D2D1_RECT_F log_filter_state_rect;
    D2D1_RECT_F log_filter_physics_rect;
    D2D1_RECT_F log_filter_collision_rect;
    D2D1_RECT_F log_filter_warn_rect;
    D2D1_RECT_F log_clear_rect;
    D2D1_RECT_F log_search_rect;
    D2D1_RECT_F log_search_clear_rect;
    D2D1_RECT_F log_viewport_rect;
    D2D1_RECT_F log_scroll_track_rect;
    D2D1_RECT_F log_scroll_thumb_rect;
    D2D1_RECT_F inspector_viewport_rect;
    D2D1_RECT_F inspector_scroll_track_rect;
    D2D1_RECT_F inspector_scroll_thumb_rect;
    D2D1_RECT_F debug_viewport_rect;
    D2D1_RECT_F debug_scroll_track_rect;
    D2D1_RECT_F debug_scroll_thumb_rect;
    D2D1_RECT_F perf_export_rect;
    D2D1_RECT_F dbg_row_rect[3];
    D2D1_RECT_F dbg_minus_rect[3];
    D2D1_RECT_F dbg_plus_rect[3];
    D2D1_RECT_F dbg_collision_filter_rect;
    D2D1_RECT_F dbg_collision_row_rect[DEBUG_EVENT_ROWS_MAX];
    int dbg_collision_row_event_index[DEBUG_EVENT_ROWS_MAX];
    int dbg_collision_row_count;
    int log_scroll_max;
    D2D1_RECT_F splitter_left_rect;
    D2D1_RECT_F splitter_right_rect;
    D2D1_RECT_F splitter_bottom_rect;
    D2D1_RECT_F status_meta_rect;
    D2D1_RECT_F ins_row_rect[INSPECTOR_MAX_ROWS];
    D2D1_RECT_F ins_minus_rect[INSPECTOR_MAX_ROWS];
    D2D1_RECT_F ins_plus_rect[INSPECTOR_MAX_ROWS];
    int ins_row_count;
} UiLayoutState;

static UiLayoutState g_layout = {0};
#define g_modal_rect g_layout.modal_rect
#define g_modal_close_rect g_layout.modal_close_rect
#define g_value_modal_rect g_layout.value_modal_rect
#define g_cfg_row_rect g_layout.cfg_row_rect
#define g_cfg_minus_rect g_layout.cfg_minus_rect
#define g_cfg_plus_rect g_layout.cfg_plus_rect
#define g_top_run_rect g_layout.top_run_rect
#define g_top_step_rect g_layout.top_step_rect
#define g_top_reset_rect g_layout.top_reset_rect
#define g_top_save_rect g_layout.top_save_rect
#define g_top_undo_rect g_layout.top_undo_rect
#define g_top_redo_rect g_layout.top_redo_rect
#define g_top_grid_rect g_layout.top_grid_rect
#define g_top_collision_rect g_layout.top_collision_rect
#define g_top_velocity_rect g_layout.top_velocity_rect
#define g_top_constraint_rect g_layout.top_constraint_rect
#define g_top_spring_rect g_layout.top_spring_rect
#define g_top_chain_rect g_layout.top_chain_rect
#define g_top_rope_rect g_layout.top_rope_rect
#define g_menu_file_rect g_layout.menu_file_rect
#define g_menu_edit_rect g_layout.menu_edit_rect
#define g_menu_view_rect g_layout.menu_view_rect
#define g_menu_gameobject_rect g_layout.menu_gameobject_rect
#define g_menu_component_rect g_layout.menu_component_rect
#define g_menu_physics_rect g_layout.menu_physics_rect
#define g_menu_window_rect g_layout.menu_window_rect
#define g_menu_help_word_rect g_layout.menu_help_word_rect
#define g_menu_app_icon_rect g_layout.menu_app_icon_rect
#define g_menu_bar_drag_rect g_layout.menu_bar_drag_rect
#define g_win_min_rect g_layout.win_min_rect
#define g_win_max_rect g_layout.win_max_rect
#define g_win_close_rect g_layout.win_close_rect
#define g_menu_dropdown_rect g_layout.menu_dropdown_rect
#define g_menu_item_rect g_layout.menu_item_rect
#define g_menu_item_enabled g_layout.menu_item_enabled
#define g_menu_item_count g_layout.menu_item_count
#define g_explorer_scene_rect g_layout.explorer_scene_rect
#define g_tree_scene_header_rect g_layout.tree_scene_header_rect
#define g_tree_bodies_header_rect g_layout.tree_bodies_header_rect
#define g_tree_constraints_header_rect g_layout.tree_constraints_header_rect
#define g_tree_body_circle_header_rect g_layout.tree_body_circle_header_rect
#define g_tree_body_polygon_header_rect g_layout.tree_body_polygon_header_rect
#define g_tree_constraint_distance_header_rect g_layout.tree_constraint_distance_header_rect
#define g_tree_constraint_spring_header_rect g_layout.tree_constraint_spring_header_rect
#define g_hierarchy_search_rect g_layout.hierarchy_search_rect
#define g_hierarchy_search_clear_rect g_layout.hierarchy_search_clear_rect
#define g_hierarchy_viewport_rect g_layout.hierarchy_viewport_rect
#define g_hierarchy_scroll_track_rect g_layout.hierarchy_scroll_track_rect
#define g_hierarchy_scroll_thumb_rect g_layout.hierarchy_scroll_thumb_rect
#define g_project_viewport_rect g_layout.project_viewport_rect
#define g_tree_project_root_rect g_layout.tree_project_root_rect
#define g_project_scroll_track_rect g_layout.project_scroll_track_rect
#define g_project_scroll_thumb_rect g_layout.project_scroll_thumb_rect
#define g_explorer_body_rect g_layout.explorer_body_rect
#define g_explorer_constraint_rect g_layout.explorer_constraint_rect
#define g_explorer_body_ptr g_layout.explorer_body_ptr
#define g_explorer_constraint_index g_layout.explorer_constraint_index
#define g_explorer_body_count g_layout.explorer_body_count
#define g_explorer_constraint_count g_layout.explorer_constraint_count
#define g_bottom_tab_console_rect g_layout.bottom_tab_console_rect
#define g_bottom_tab_perf_rect g_layout.bottom_tab_perf_rect
#define g_bottom_fold_rect g_layout.bottom_fold_rect
#define g_left_fold_rect g_layout.left_fold_rect
#define g_right_fold_rect g_layout.right_fold_rect
#define g_log_filter_all_rect g_layout.log_filter_all_rect
#define g_log_filter_state_rect g_layout.log_filter_state_rect
#define g_log_filter_physics_rect g_layout.log_filter_physics_rect
#define g_log_filter_collision_rect g_layout.log_filter_collision_rect
#define g_log_filter_warn_rect g_layout.log_filter_warn_rect
#define g_log_clear_rect g_layout.log_clear_rect
#define g_log_search_rect g_layout.log_search_rect
#define g_log_search_clear_rect g_layout.log_search_clear_rect
#define g_log_viewport_rect g_layout.log_viewport_rect
#define g_log_scroll_track_rect g_layout.log_scroll_track_rect
#define g_log_scroll_thumb_rect g_layout.log_scroll_thumb_rect
#define g_inspector_viewport_rect g_layout.inspector_viewport_rect
#define g_inspector_scroll_track_rect g_layout.inspector_scroll_track_rect
#define g_inspector_scroll_thumb_rect g_layout.inspector_scroll_thumb_rect
#define g_debug_viewport_rect g_layout.debug_viewport_rect
#define g_debug_scroll_track_rect g_layout.debug_scroll_track_rect
#define g_debug_scroll_thumb_rect g_layout.debug_scroll_thumb_rect
#define g_perf_export_rect g_layout.perf_export_rect
#define g_dbg_row_rect g_layout.dbg_row_rect
#define g_dbg_minus_rect g_layout.dbg_minus_rect
#define g_dbg_plus_rect g_layout.dbg_plus_rect
#define g_dbg_collision_filter_rect g_layout.dbg_collision_filter_rect
#define g_dbg_collision_row_rect g_layout.dbg_collision_row_rect
#define g_dbg_collision_row_event_index g_layout.dbg_collision_row_event_index
#define g_dbg_collision_row_count g_layout.dbg_collision_row_count
#define g_log_scroll_max g_layout.log_scroll_max
#define g_splitter_left_rect g_layout.splitter_left_rect
#define g_splitter_right_rect g_layout.splitter_right_rect
#define g_splitter_bottom_rect g_layout.splitter_bottom_rect
#define g_status_meta_rect g_layout.status_meta_rect
#define g_ins_row_rect g_layout.ins_row_rect
#define g_ins_minus_rect g_layout.ins_minus_rect
#define g_ins_plus_rect g_layout.ins_plus_rect
#define g_ins_row_count g_layout.ins_row_count
enum { CONSOLE_LOG_CAP = 200, CONSOLE_LINE_MAX = 180 };
enum { HISTORY_MAX_SLOTS = 12 };
enum { COLLISION_EVENT_CAP = 120 };
static wchar_t g_console_logs[CONSOLE_LOG_CAP][CONSOLE_LINE_MAX];
static int g_console_log_head;
static int g_console_log_count;
static CollisionEvent g_collision_events[COLLISION_EVENT_CAP];
static int g_collision_event_head;
static int g_collision_event_count;
static int g_collision_event_filter_selected_only;
static BodyClipboard g_clipboard_body;
static wchar_t g_status_project_path[260];
static wchar_t g_status_user[64];
static HICON g_app_icon_large = NULL;
static HICON g_app_icon_small = NULL;
static const float BREAK_FORCE_PRESETS[] = {0.0f, 60.0f, 120.0f, 250.0f, 500.0f, 1000.0f};
static const int BREAK_FORCE_PRESET_COUNT = 6;
static void apply_layout_preset(int preset);
static void apply_scene(int scene_index);
static void spawn_circle_at_cursor(void);
static void spawn_box_at_cursor(void);
static int save_scene_snapshot(const char* path);
static int load_scene_snapshot(const char* path);
static int file_exists_utf8_path(const char* path);
static void history_push_snapshot(void);
static void history_reset_and_capture(void);
static void history_undo(void);
static void history_redo(void);
static int body_index_of(const PhysicsEngine* engine, const RigidBody* b);
static void clear_collision_events(void);
static void push_collision_event(int body_a_index, int body_b_index, Vec2 point, float rel_speed, float penetration);
static void capture_collision_events(void);
static int collision_event_involves_selected(const CollisionEvent* ev);
static void select_body_by_index(int body_index);
static void cleanup_constraint_selection(void);
static const Constraint* selected_constraint_ref(void);
static int selected_constraint_is_active(void);
static int copy_selected_body_to_clipboard(void);
static int paste_body_from_clipboard(void);
static float body_box_size_hint(const RigidBody* b);
static int compute_stage_rect_from_layout(HWND hwnd, D2D1_RECT_F* out_stage_rect);
static Vec2 clamp_spawn_inside_stage(Vec2 p, float bound_radius);
static int finite_positive(float v, float min_v);
static void trace_spawn_step(const char* stage, const char* fmt, ...);

static D2D1_RECT_F rc(float l, float t, float r, float b) {
    D2D1_RECT_F v = {l, t, r, b};
    return v;
}

static D2D1_COLOR_F rgba(float r, float g, float b, float a) {
    D2D1_COLOR_F v = {r, g, b, a};
    return v;
}

static D2D1_POINT_2F pt(float x, float y) {
    D2D1_POINT_2F v = {x, y};
    return v;
}

static void release_unknown(IUnknown** p) {
    if (p != NULL && *p != NULL) {
        (*p)->lpVtbl->Release(*p);
        *p = NULL;
    }
}

static ScreenPt world_to_screen(Vec2 p) {
    ScreenPt v = {WORLD_ORIGIN_X + p.x * WORLD_SCALE, WORLD_ORIGIN_Y + p.y * WORLD_SCALE};
    return v;
}

static Vec2 screen_to_world(ScreenPt p) {
    return vec2((p.x - WORLD_ORIGIN_X) / WORLD_SCALE, (p.y - WORLD_ORIGIN_Y) / WORLD_SCALE);
}

static void set_brush_color(float r, float g, float b, float a) {
    D2D1_COLOR_F c = rgba(r, g, b, a);
    ID2D1SolidColorBrush_SetColor(g_ui.brush, &c);
}

static int point_in_rect(ScreenPt p, D2D1_RECT_F r) {
    return (p.x >= r.left && p.x <= r.right && p.y >= r.top && p.y <= r.bottom);
}

static void push_console_log(const wchar_t* fmt, ...) {
    va_list args;
    wchar_t text[CONSOLE_LINE_MAX];
    int idx;
    if (fmt == NULL) return;
    va_start(args, fmt);
    vswprintf(text, CONSOLE_LINE_MAX, fmt, args);
    va_end(args);
    idx = (g_console_log_head + g_console_log_count) % CONSOLE_LOG_CAP;
    lstrcpynW(g_console_logs[idx], text, CONSOLE_LINE_MAX);
    if (g_console_log_count < CONSOLE_LOG_CAP) {
        g_console_log_count++;
    } else {
        g_console_log_head = (g_console_log_head + 1) % CONSOLE_LOG_CAP;
    }
}

static void trace_spawn_step(const char* stage, const char* fmt, ...) {
    FILE* fp;
    SYSTEMTIME st;
    DWORD tid;
    va_list args;
    char msg[256];
    if (stage == NULL) return;
    msg[0] = '\0';
    if (fmt != NULL) {
        va_start(args, fmt);
        vsnprintf(msg, sizeof(msg), fmt, args);
        va_end(args);
        msg[sizeof(msg) - 1] = '\0';
    }
    fp = fopen("spawn_trace.log", "a");
    if (fp == NULL) return;
    GetLocalTime(&st);
    tid = GetCurrentThreadId();
    fprintf(fp, "%04u-%02u-%02u %02u:%02u:%02u.%03u [T%lu] %s %s\n",
            (unsigned)st.wYear, (unsigned)st.wMonth, (unsigned)st.wDay,
            (unsigned)st.wHour, (unsigned)st.wMinute, (unsigned)st.wSecond, (unsigned)st.wMilliseconds,
            (unsigned long)tid, stage, msg);
    fclose(fp);
}

static int body_index_of(const PhysicsEngine* engine, const RigidBody* b) {
    int i;
    if (engine == NULL || b == NULL) return -1;
    for (i = 0; i < physics_engine_get_body_count(engine); i++) {
        if (physics_engine_get_body(engine, i) == b) return i;
    }
    return -1;
}

static void clear_collision_events(void) {
    g_collision_event_head = 0;
    g_collision_event_count = 0;
}

static void push_collision_event(int body_a_index, int body_b_index, Vec2 point, float rel_speed, float penetration) {
    int idx = (g_collision_event_head + g_collision_event_count) % COLLISION_EVENT_CAP;
    CollisionEvent* ev = &g_collision_events[idx];
    ev->tick_ms = (unsigned int)GetTickCount();
    ev->scene_index = g_state.scene_index;
    ev->body_a_index = body_a_index;
    ev->body_b_index = body_b_index;
    ev->point = point;
    ev->rel_speed = rel_speed;
    ev->penetration = penetration;
    if (g_collision_event_count < COLLISION_EVENT_CAP) {
        g_collision_event_count++;
    } else {
        g_collision_event_head = (g_collision_event_head + 1) % COLLISION_EVENT_CAP;
    }
}

static int collision_event_involves_selected(const CollisionEvent* ev) {
    int selected_index;
    if (ev == NULL || g_state.engine == NULL || g_state.selected == NULL) return 0;
    selected_index = body_index_of(g_state.engine, g_state.selected);
    if (selected_index < 0) return 0;
    return (ev->body_a_index == selected_index || ev->body_b_index == selected_index);
}

static void capture_collision_events(void) {
    int i;
    unsigned int now_ms;
    int sample_count;
    if (g_state.engine == NULL || physics_engine_get_contact_count(g_state.engine) <= 0) return;
    now_ms = (unsigned int)GetTickCount();
    if ((now_ms - g_state.last_collision_capture_ms) < 140) return;
    g_state.last_collision_capture_ms = now_ms;
    sample_count = physics_engine_get_contact_count(g_state.engine);
    if (sample_count > 3) sample_count = 3;
    for (i = 0; i < sample_count; i++) {
        const CollisionManifold* cm = physics_engine_get_contact(g_state.engine, i);
        int ia = body_index_of(g_state.engine, cm->bodyA);
        int ib = body_index_of(g_state.engine, cm->bodyB);
        float rel_speed = 0.0f;
        if (cm->bodyA != NULL && cm->bodyB != NULL) {
            Vec2 rv = vec2_sub(cm->bodyA->velocity, cm->bodyB->velocity);
            rel_speed = vec2_length(rv);
        }
        push_collision_event(ia, ib, cm->info.point, rel_speed, cm->info.penetration);
    }
}

static void select_body_by_index(int body_index) {
    if (g_state.engine == NULL) return;
    if (body_index < 0 || body_index >= physics_engine_get_body_count(g_state.engine)) return;
    if (physics_engine_get_body(g_state.engine, body_index) == NULL) return;
    g_state.selected = physics_engine_get_body(g_state.engine, body_index);
    g_state.selected_constraint_index = -1;
    g_state.dragging = 0;
    g_state.inspector_focused_row = 0;
}

static int copy_selected_body_to_clipboard(void) {
    if (g_state.selected == NULL || g_state.selected->shape == NULL) return 0;
    if (g_state.selected->type != BODY_DYNAMIC) return 0;
    g_clipboard_body.valid = 1;
    g_clipboard_body.is_circle = (g_state.selected->shape->type == SHAPE_CIRCLE) ? 1 : 0;
    g_clipboard_body.mass = g_state.selected->mass;
    g_clipboard_body.restitution = g_state.selected->shape->restitution;
    g_clipboard_body.friction = g_state.selected->shape->friction;
    g_clipboard_body.damping = g_state.selected->damping;
    g_clipboard_body.size = g_clipboard_body.is_circle ? g_state.selected->shape->data.circle.radius : body_box_size_hint(g_state.selected);
    g_clipboard_body.velocity = g_state.selected->velocity;
    g_clipboard_body.angular_velocity = g_state.selected->angular_velocity;
    return 1;
}

static int paste_body_from_clipboard(void) {
    Shape* sh;
    RigidBody* b;
    Vec2 p;
    D2D1_RECT_F visible_stage_rect;
    float spawn_bound;
    trace_spawn_step("paste.begin", "valid=%d mass=%.4f size=%.4f", g_clipboard_body.valid, g_clipboard_body.mass, g_clipboard_body.size);
    if (g_state.engine == NULL || !g_clipboard_body.valid) {
        trace_spawn_step("paste.abort", "engine-or-clipboard-invalid");
        return 0;
    }
    if (!finite_positive(g_clipboard_body.mass, 0.001f) || !finite_positive(g_clipboard_body.size, 0.05f)) {
        trace_spawn_step("paste.abort", "invalid-clipboard-params");
        return 0;
    }
    if (g_clipboard_body.is_circle) sh = shape_create_circle(max_f(0.2f, g_clipboard_body.size));
    else sh = shape_create_box(max_f(0.4f, g_clipboard_body.size), max_f(0.4f, g_clipboard_body.size));
    if (sh == NULL) {
        trace_spawn_step("paste.abort", "shape-create-failed");
        return 0;
    }
    sh->restitution = clamp(g_clipboard_body.restitution, 0.0f, 1.0f);
    sh->friction = clamp(g_clipboard_body.friction, 0.0f, 2.0f);
    if (!compute_stage_rect_from_layout(g_app_hwnd, &visible_stage_rect)) {
        trace_spawn_step("paste.abort", "stage-rect-failed");
        shape_free(sh);
        return 0;
    }
    if (g_state.mouse_screen.x >= visible_stage_rect.left &&
        g_state.mouse_screen.x <= visible_stage_rect.right &&
        g_state.mouse_screen.y >= visible_stage_rect.top &&
        g_state.mouse_screen.y <= visible_stage_rect.bottom) {
        p = screen_to_world(g_state.mouse_screen);
    } else {
        p = screen_to_world((ScreenPt){(visible_stage_rect.left + visible_stage_rect.right) * 0.5f, (visible_stage_rect.top + visible_stage_rect.bottom) * 0.5f});
    }
    p.x += 1.4f;
    p.y -= 1.2f;
    spawn_bound = g_clipboard_body.is_circle ? max_f(0.2f, g_clipboard_body.size) : max_f(0.4f, g_clipboard_body.size) * 0.72f;
    p = clamp_spawn_inside_stage(p, spawn_bound);
    b = body_create(p.x, p.y, max_f(0.1f, g_clipboard_body.mass), sh);
    if (b == NULL) {
        shape_free(sh);
        trace_spawn_step("paste.abort", "body-create-failed");
        return 0;
    }
    b->velocity = g_clipboard_body.velocity;
    b->angular_velocity = g_clipboard_body.angular_velocity;
    b->damping = clamp(g_clipboard_body.damping, 0.0f, 1.0f);
    physics_engine_add_body(g_state.engine, b);
    trace_spawn_step("paste.added", "x=%.3f y=%.3f mass=%.3f", b->position.x, b->position.y, b->mass);
    g_state.selected = b;
    g_state.selected_constraint_index = -1;
    g_state.inspector_focused_row = 0;
    return 1;
}

static const wchar_t* ui_layout_ini_path(void) {
    return L".\\sandbox_ui.ini";
}

static void load_ui_layout(void) {
    UiLayoutPrefs prefs;
    if (!ui_layout_repo_load(ui_layout_ini_path(), &prefs)) return;
    g_state.ui_left_ratio = prefs.left_ratio;
    g_state.ui_right_ratio = prefs.right_ratio;
    g_state.ui_bottom_open_h = prefs.bottom_open_h;
    g_state.bottom_panel_collapsed = prefs.bottom_collapsed;
    g_state.ui_show_left_panel = prefs.show_left;
    g_state.ui_show_right_panel = prefs.show_right;
    g_state.ui_show_bottom_panel = prefs.show_bottom;
    g_state.ui_theme_light = prefs.theme_light;
    g_state.ui_layout_preset = prefs.preset;
}

static void save_ui_layout(void) {
    UiLayoutPrefs prefs;
    prefs.left_ratio = g_state.ui_left_ratio;
    prefs.right_ratio = g_state.ui_right_ratio;
    prefs.bottom_open_h = g_state.ui_bottom_open_h;
    prefs.bottom_collapsed = g_state.bottom_panel_collapsed;
    prefs.show_left = g_state.ui_show_left_panel;
    prefs.show_right = g_state.ui_show_right_panel;
    prefs.show_bottom = g_state.ui_show_bottom_panel;
    prefs.theme_light = g_state.ui_theme_light;
    prefs.preset = g_state.ui_layout_preset;
    ui_layout_repo_save(ui_layout_ini_path(), &prefs);
}

static void perf_push_sample(float fps, float step_ms) {
    int idx = (g_state.perf_hist_head + g_state.perf_hist_count) % 180;
    g_state.fps_hist[idx] = fps;
    g_state.step_hist[idx] = step_ms;
    if (g_state.perf_hist_count < 180) {
        g_state.perf_hist_count++;
    } else {
        g_state.perf_hist_head = (g_state.perf_hist_head + 1) % 180;
    }
}

static int log_match_filter(const wchar_t* s, int mode) {
    int base_ok = 0;
    if (s == NULL) return 0;
    if (mode == 0) base_ok = 1;
    if (mode == 1) base_ok = (wcsstr(s, L"[状态]") != NULL);
    if (mode == 2) base_ok = (wcsstr(s, L"[物理]") != NULL) || (wcsstr(s, L"[碰撞]") != NULL);
    if (mode == 3) base_ok = (wcsstr(s, L"[警告]") != NULL);
    if (mode == 4) return 0;
    if (!base_ok) return 0;
    if (g_state.log_search_len > 0) {
        return (wcsstr(s, g_state.log_search_buf) != NULL);
    }
    return 1;
}

static void export_perf_report_csv(void) {
    FILE* fp = fopen("perf_report.csv", "w");
    int i;
    if (fp == NULL) {
        push_console_log(L"[错误] 无法写入 perf_report.csv");
        return;
    }
    fputs("index,fps,step_ms\n", fp);
    for (i = 0; i < g_state.perf_hist_count; i++) {
        int idx = (g_state.perf_hist_head + i) % 180;
        fprintf(fp, "%d,%.3f,%.3f\n", i, g_state.fps_hist[idx], g_state.step_hist[idx]);
    }
    fclose(fp);
    push_console_log(L"[性能] 已导出 perf_report.csv (%d 条)", g_state.perf_hist_count);
}

static int menu_cb_save_snapshot(const char* path, void* user) {
    (void)user;
    return save_scene_snapshot(path);
}

static int menu_cb_load_snapshot(const char* path, void* user) {
    (void)user;
    return load_scene_snapshot(path);
}

static void menu_cb_history_reset(void* user) {
    (void)user;
    history_reset_and_capture();
}

static void menu_cb_history_undo(void* user) {
    (void)user;
    history_undo();
}

static void menu_cb_history_redo(void* user) {
    (void)user;
    history_redo();
}

static void menu_cb_history_push(void* user) {
    (void)user;
    history_push_snapshot();
}

static int menu_cb_copy_selected(void* user) {
    (void)user;
    return copy_selected_body_to_clipboard();
}

static int menu_cb_paste_selected(void* user) {
    (void)user;
    return paste_body_from_clipboard();
}

static void menu_cb_spawn_center_circle(void* user) {
    ScreenPt old_mouse;
    (void)user;
    old_mouse = g_state.mouse_screen;
    trace_spawn_step("menu.edit.create.begin", "item=%d", 4);
    g_state.mouse_screen.x = (g_state.stage_left + g_state.stage_right) * 0.5f;
    g_state.mouse_screen.y = (g_state.stage_top + g_state.stage_bottom) * 0.5f;
    history_push_snapshot();
    spawn_circle_at_cursor();
    trace_spawn_step("menu.edit.create.end", "item=%d", 4);
    g_state.mouse_screen = old_mouse;
}

static void menu_cb_spawn_center_box(void* user) {
    ScreenPt old_mouse;
    (void)user;
    old_mouse = g_state.mouse_screen;
    trace_spawn_step("menu.edit.create.begin", "item=%d", 5);
    g_state.mouse_screen.x = (g_state.stage_left + g_state.stage_right) * 0.5f;
    g_state.mouse_screen.y = (g_state.stage_top + g_state.stage_bottom) * 0.5f;
    history_push_snapshot();
    spawn_box_at_cursor();
    trace_spawn_step("menu.edit.create.end", "item=%d", 5);
    g_state.mouse_screen = old_mouse;
}

static void menu_cb_clear_logs(void* user) {
    (void)user;
    g_console_log_count = 0;
    g_console_log_head = 0;
    g_state.log_scroll_offset = 0;
    push_console_log(L"[日志] 已清空");
}

static void menu_cb_log_text(const wchar_t* text, void* user) {
    (void)user;
    if (text == NULL) return;
    push_console_log(L"%s", text);
}

static void menu_cb_step_once(void* user) {
    (void)user;
    if (g_state.engine != NULL) {
        physics_engine_step(g_state.engine);
        cleanup_constraint_selection();
        capture_collision_events();
    }
}

static void menu_cb_reset_scene(void* user) {
    (void)user;
    apply_scene(g_state.scene_index);
    history_reset_and_capture();
}

static void menu_cb_apply_next_layout(void* user) {
    (void)user;
    apply_layout_preset(g_state.ui_layout_preset + 1);
}

static void menu_cb_save_layout(void* user) {
    (void)user;
    save_ui_layout();
}

static void execute_menu_action(HWND hwnd, int menu_id, int item_idx) {
    (void)hwnd;
    MenuFileEditOps fe_ops;
    MenuViewPhysicsWindowOps vpw_ops;
    MenuHelpOps help_ops;
    fe_ops.save_snapshot = menu_cb_save_snapshot;
    fe_ops.load_snapshot = menu_cb_load_snapshot;
    fe_ops.history_reset = menu_cb_history_reset;
    fe_ops.history_undo = menu_cb_history_undo;
    fe_ops.history_redo = menu_cb_history_redo;
    fe_ops.history_push = menu_cb_history_push;
    fe_ops.copy_selected = menu_cb_copy_selected;
    fe_ops.paste_selected = menu_cb_paste_selected;
    fe_ops.spawn_center_circle = menu_cb_spawn_center_circle;
    fe_ops.spawn_center_box = menu_cb_spawn_center_box;
    fe_ops.clear_logs = menu_cb_clear_logs;
    fe_ops.log_text = menu_cb_log_text;
    fe_ops.draw_constraints = &g_state.draw_constraints;
    fe_ops.user = NULL;
    if (menu_file_edit_execute(menu_id, item_idx, &fe_ops)) {
        return;
    }
    vpw_ops.draw_centers = &g_state.draw_centers;
    vpw_ops.draw_contacts = &g_state.draw_contacts;
    vpw_ops.draw_velocity = &g_state.draw_velocity;
    vpw_ops.running = &g_state.running;
    vpw_ops.show_left = &g_state.ui_show_left_panel;
    vpw_ops.show_right = &g_state.ui_show_right_panel;
    vpw_ops.show_bottom = &g_state.ui_show_bottom_panel;
    vpw_ops.bottom_active_tab = &g_state.bottom_active_tab;
    vpw_ops.theme_light = &g_state.ui_theme_light;
    vpw_ops.ui_layout_preset = &g_state.ui_layout_preset;
    vpw_ops.step_once = menu_cb_step_once;
    vpw_ops.reset_scene = menu_cb_reset_scene;
    vpw_ops.apply_next_layout = menu_cb_apply_next_layout;
    vpw_ops.save_layout = menu_cb_save_layout;
    vpw_ops.user = NULL;
    if (menu_view_physics_window_execute(menu_id, item_idx, &vpw_ops)) {
        return;
    }
    if (menu_id == 4) return;
    help_ops.show_help_modal = &g_state.show_help_modal;
    help_ops.show_config_modal = &g_state.show_config_modal;
    help_ops.help_modal_page = &g_state.help_modal_page;
    if (menu_help_execute(menu_id, item_idx, &help_ops)) {
        return;
    }
}

static const wchar_t* menu_shortcut_text(int menu_id, int item_idx) {
    return menu_model_shortcut_text(menu_id, item_idx);
}

static MenuUiState build_menu_ui_state(void) {
    MenuUiState s;
    s.history_cursor = g_state.history_cursor;
    s.history_top = g_state.history_top;
    s.has_selected_dynamic = (g_state.selected != NULL && g_state.selected->type == BODY_DYNAMIC);
    s.clipboard_valid = g_clipboard_body.valid;
    s.show_bottom_panel = g_state.ui_show_bottom_panel;
    s.has_scene_snapshot = file_exists_utf8_path("scene_snapshot.txt");
    s.has_autosave_snapshot = file_exists_utf8_path("autosave_snapshot.txt");
    s.draw_constraints = g_state.draw_constraints;
    s.draw_centers = g_state.draw_centers;
    s.draw_contacts = g_state.draw_contacts;
    s.draw_velocity = g_state.draw_velocity;
    s.running = g_state.running;
    s.show_left_panel = g_state.ui_show_left_panel;
    s.show_right_panel = g_state.ui_show_right_panel;
    s.theme_light = g_state.ui_theme_light;
    return s;
}

static int menu_item_enabled_state(int menu_id, int item_idx) {
    MenuUiState s = build_menu_ui_state();
    return menu_model_item_enabled_state(menu_id, item_idx, &s);
}

static int menu_item_count_for_menu(int menu_id) {
    return menu_model_item_count_for_menu(menu_id);
}

static int menu_next_visible_id(int current_id, int dir) {
    return menu_model_next_visible_id(current_id, dir);
}

static int menu_find_enabled_from(int menu_id, int start, int dir) {
    MenuUiState s = build_menu_ui_state();
    return menu_model_find_enabled_from(menu_id, start, dir, &s);
}

static void apply_layout_preset(int preset) {
    if (preset < 0) preset = 0;
    preset = preset % 3;
    g_state.ui_layout_preset = preset;
    if (preset == 0) {
        g_state.ui_left_ratio = 0.18f;
        g_state.ui_right_ratio = 0.25f;
        g_state.ui_bottom_open_h = 132.0f;
        g_state.ui_show_left_panel = 1;
        g_state.ui_show_right_panel = 1;
        g_state.ui_show_bottom_panel = 1;
        g_state.bottom_panel_collapsed = 1;
        g_state.bottom_active_tab = 0;
        push_console_log(L"[布局] 编辑布局");
    } else if (preset == 1) {
        g_state.ui_left_ratio = 0.16f;
        g_state.ui_right_ratio = 0.30f;
        g_state.ui_bottom_open_h = 170.0f;
        g_state.ui_show_left_panel = 1;
        g_state.ui_show_right_panel = 1;
        g_state.ui_show_bottom_panel = 1;
        g_state.bottom_panel_collapsed = 0;
        g_state.bottom_active_tab = 0;
        push_console_log(L"[布局] 调试布局");
    } else {
        g_state.ui_left_ratio = 0.14f;
        g_state.ui_right_ratio = 0.22f;
        g_state.ui_bottom_open_h = 220.0f;
        g_state.ui_show_left_panel = 1;
        g_state.ui_show_right_panel = 1;
        g_state.ui_show_bottom_panel = 1;
        g_state.bottom_panel_collapsed = 0;
        g_state.bottom_active_tab = 1;
        push_console_log(L"[布局] 性能布局");
    }
}

static int point_in_body(RigidBody* body, Vec2 point) {
    if (body == NULL || body->shape == NULL) return 0;

    if (body->shape->type == SHAPE_CIRCLE) {
        float radius = body->shape->data.circle.radius;
        Vec2 delta = vec2_sub(point, body->position);
        return vec2_length_sq(delta) <= radius * radius;
    }
    if (body->shape->type == SHAPE_POLYGON) {
        PolygonShape* poly = &body->shape->data.polygon;
        int inside = 0;
        int i;
        int j = poly->vertex_count - 1;
        for (i = 0; i < poly->vertex_count; j = i++) {
            Vec2 vi = body_get_world_point(body, poly->vertices[i]);
            Vec2 vj = body_get_world_point(body, poly->vertices[j]);
            int crosses = ((vi.y > point.y) != (vj.y > point.y));
            if (crosses) {
                float x_at_y = (vj.x - vi.x) * (point.y - vi.y) / (vj.y - vi.y) + vi.x;
                if (point.x < x_at_y) inside = !inside;
            }
        }
        return inside;
    }
    return 0;
}

static RigidBody* pick_dynamic_body(PhysicsEngine* engine, Vec2 p) {
    int i;
    if (engine == NULL) return NULL;
    for (i = physics_engine_get_body_count(engine) - 1; i >= 0; i--) {
        RigidBody* b = physics_engine_get_body(engine, i);
        if (b == NULL || b->type != BODY_DYNAMIC) continue;
        if (point_in_body(b, p)) return b;
    }
    return NULL;
}

static float point_segment_distance_sq(Vec2 p, Vec2 a, Vec2 b, float* out_t, Vec2* out_proj) {
    Vec2 ab = vec2_sub(b, a);
    float ab_len_sq = vec2_length_sq(ab);
    float t = 0.0f;
    Vec2 proj = a;
    if (ab_len_sq > 1e-6f) {
        t = clamp(vec2_dot(vec2_sub(p, a), ab) / ab_len_sq, 0.0f, 1.0f);
        proj = vec2_add(a, vec2_scale(ab, t));
    }
    if (out_t != NULL) *out_t = t;
    if (out_proj != NULL) *out_proj = proj;
    return vec2_length_sq(vec2_sub(p, proj));
}

static const Constraint* selected_constraint_ref(void) {
    if (g_state.engine == NULL || g_state.selected_constraint_index < 0) {
        return NULL;
    }
    return physics_engine_get_constraint(g_state.engine, g_state.selected_constraint_index);
}

static int selected_constraint_is_active(void) {
    const Constraint* c = selected_constraint_ref();
    return (c != NULL && c->active);
}

static void constraint_world_anchors(const Constraint* c, Vec2* pa, Vec2* pb) {
    if (c == NULL || c->body_a == NULL || c->body_b == NULL) {
        if (pa != NULL) *pa = vec2(0.0f, 0.0f);
        if (pb != NULL) *pb = vec2(0.0f, 0.0f);
        return;
    }
    if (pa != NULL) {
        *pa = body_get_world_point(c->body_a, c->local_anchor_a);
    }
    if (pb != NULL) {
        *pb = body_get_world_point(c->body_b, c->local_anchor_b);
    }
}

static int pick_constraint_at_point(PhysicsEngine* engine, Vec2 p) {
    float best_dist_sq = 1.6f * 1.6f;
    int best_index = -1;
    int i;
    if (engine == NULL) return -1;
    for (i = 0; i < physics_engine_get_constraint_count(engine); i++) {
        const Constraint* c = physics_engine_get_constraint(engine, i);
        Vec2 a;
        Vec2 b;
        Vec2 proj;
        float t;
        float d_sq;
        if (!c->active || c->body_a == NULL || c->body_b == NULL) continue;
        constraint_world_anchors(c, &a, &b);
        d_sq = point_segment_distance_sq(p, a, b, &t, &proj);
        if (d_sq < best_dist_sq) {
            best_dist_sq = d_sq;
            best_index = i;
        }
    }
    return best_index;
}

static void cleanup_constraint_selection(void) {
    if (g_state.engine == NULL) {
        g_state.selected_constraint_index = -1;
        g_state.constraint_seed_body = NULL;
        g_state.constraint_create_mode = 0;
        return;
    }
    if (!selected_constraint_is_active()) {
        g_state.selected_constraint_index = -1;
    }
    if (g_state.constraint_seed_body != NULL) {
        int found = 0;
        for (int i = 0; i < physics_engine_get_body_count(g_state.engine); i++) {
            if (physics_engine_get_body(g_state.engine, i) == g_state.constraint_seed_body) {
                found = 1;
                break;
            }
        }
        if (!found) g_state.constraint_seed_body = NULL;
    }
}

static void cycle_selected_constraint_break_force(void) {
    int i;
    const Constraint* c = selected_constraint_ref();
    if (c == NULL) return;
    for (i = 0; i < BREAK_FORCE_PRESET_COUNT; i++) {
        if (fabsf(c->break_force - BREAK_FORCE_PRESETS[i]) < 1e-3f) {
            physics_engine_constraint_set_break_force(g_state.engine, g_state.selected_constraint_index,
                                                     BREAK_FORCE_PRESETS[(i + 1) % BREAK_FORCE_PRESET_COUNT]);
            return;
        }
    }
    physics_engine_constraint_set_break_force(g_state.engine, g_state.selected_constraint_index, BREAK_FORCE_PRESETS[1]);
}

static void update_selected_constraint_param(float ds, float dd) {
    const Constraint* c = selected_constraint_ref();
    if (c == NULL) return;
    physics_engine_constraint_set_stiffness(g_state.engine, g_state.selected_constraint_index, c->stiffness + ds);
    physics_engine_constraint_set_damping(g_state.engine, g_state.selected_constraint_index, c->damping + dd);
}

static int spring_preset_from_params(float stiffness, float damping) {
    if (fabsf(stiffness - 4.5f) <= 0.15f && fabsf(damping - 0.6f) <= 0.08f) return 0;
    if (fabsf(stiffness - 7.0f) <= 0.15f && fabsf(damping - 0.9f) <= 0.08f) return 1;
    if (fabsf(stiffness - 10.0f) <= 0.20f && fabsf(damping - 1.2f) <= 0.10f) return 2;
    return -1;
}

static int spring_preset_for_constraint(const Constraint* c) {
    if (c == NULL || c->type != CONSTRAINT_SPRING) return -1;
    return spring_preset_from_params(c->stiffness, c->damping);
}

static void apply_spring_preset_to_constraint_index(int constraint_index, int preset_mode) {
    if (g_state.engine == NULL || constraint_index < 0) {
        return;
    }
    if (preset_mode <= 0) {
        physics_engine_constraint_set_stiffness(g_state.engine, constraint_index, 4.5f);
        physics_engine_constraint_set_damping(g_state.engine, constraint_index, 0.6f);
        return;
    }
    if (preset_mode == 1) {
        physics_engine_constraint_set_stiffness(g_state.engine, constraint_index, 7.0f);
        physics_engine_constraint_set_damping(g_state.engine, constraint_index, 0.9f);
        return;
    }
    physics_engine_constraint_set_stiffness(g_state.engine, constraint_index, 10.0f);
    physics_engine_constraint_set_damping(g_state.engine, constraint_index, 1.2f);
}

static float body_visual_radius(const RigidBody* b) {
    int i;
    float r = 0.0f;
    if (b == NULL || b->shape == NULL) return 0.0f;
    if (b->shape->type == SHAPE_CIRCLE) {
        return max_f(0.0f, b->shape->data.circle.radius);
    }
    for (i = 0; i < b->shape->data.polygon.vertex_count; i++) {
        float d = vec2_length(b->shape->data.polygon.vertices[i]);
        if (d > r) r = d;
    }
    return r;
}

static int create_segmented_link_constraint(RigidBody* start, RigidBody* end, int rope_mode, Constraint** out_last) {
    enum { MAX_LINK_BODIES = 24 };
    RigidBody* links[MAX_LINK_BODIES];
    int link_count;
    int link_target;
    int link_max_by_overlap;
    int i;
    Vec2 delta;
    float dist;
    float start_r;
    float end_r;
    Vec2 dir;
    Vec2 anchor_a;
    Vec2 anchor_b;
    Vec2 center_a;
    Vec2 center_b;
    Vec2 link_line_a;
    Vec2 link_line_b;
    float span;
    float seg_len;
    float radius;
    float endpoint_pad;
    float preferred_seg_len;
    float min_seg_len;
    float mass;
    float rope_stiffness = 2.8f;
    float rope_damping = 0.35f;
    Constraint* last = NULL;
    SceneConfig* cfg;

    if (out_last != NULL) *out_last = NULL;
    if (g_state.engine == NULL || start == NULL || end == NULL) return 0;

    delta = vec2_sub(end->position, start->position);
    dist = vec2_length(delta);
    if (dist < 1.2f) return 0;
    dir = vec2_scale(delta, 1.0f / dist);

    cfg = &g_state.scenes[g_state.scene_index];
    radius = rope_mode ? 0.42f : 0.58f;
    endpoint_pad = radius * (rope_mode ? 1.25f : 1.40f) + 0.20f;
    preferred_seg_len = rope_mode ? 2.6f : 2.3f;
    min_seg_len = radius * 2.3f;

    start_r = body_visual_radius(start);
    end_r = body_visual_radius(end);
    center_a = start->position;
    center_b = end->position;
    anchor_a = center_a;
    anchor_b = center_b;
    link_line_a = vec2_add(center_a, vec2_scale(dir, start_r + endpoint_pad));
    link_line_b = vec2_sub(center_b, vec2_scale(dir, end_r + endpoint_pad));
    span = vec2_length(vec2_sub(link_line_b, link_line_a));
    if (span < min_seg_len * 1.2f) return 0;

    link_target = (int)(span / preferred_seg_len) - 1;
    if (link_target < 1) link_target = 1;
    link_max_by_overlap = (int)(span / min_seg_len) - 1;
    if (link_max_by_overlap < 1) link_max_by_overlap = 1;
    link_count = min_f((float)link_target, (float)link_max_by_overlap);
    link_count = (int)link_count;
    if (link_count < 1) link_count = 1;
    if (link_count > MAX_LINK_BODIES) link_count = MAX_LINK_BODIES;
    seg_len = span / (float)(link_count + 1);
    mass = max_f(0.06f, cfg->ball_mass * (rope_mode ? 0.10f : 0.15f));
    memset(links, 0, sizeof(links));

    for (i = 0; i < link_count; i++) {
        float t = (float)(i + 1) / (float)(link_count + 1);
        Vec2 p = vec2_add(link_line_a, vec2_scale(vec2_sub(link_line_b, link_line_a), t));
        Shape* sh = shape_create_circle(radius);
        RigidBody* b;
        if (sh == NULL) break;
        sh->restitution = rope_mode ? 0.10f : 0.04f;
        sh->friction = rope_mode ? 0.22f : 0.28f;
        b = body_create(p.x, p.y, mass, sh);
        if (b == NULL) {
            shape_free(sh);
            break;
        }
        b->damping = rope_mode ? 0.995f : 0.985f;
        physics_engine_add_body(g_state.engine, b);
        links[i] = b;
    }
    if (links[link_count - 1] == NULL) {
        for (i = 0; i < link_count; i++) {
            if (links[i] != NULL) physics_engine_remove_body(g_state.engine, links[i]);
        }
        return 0;
    }

    {
        RigidBody* prev = start;
        Vec2 prev_anchor = anchor_a;
        for (i = 0; i < link_count + 1; i++) {
            RigidBody* curr = (i < link_count) ? links[i] : end;
            Vec2 curr_anchor = (i < link_count) ? curr->position : anchor_b;
            float rest_len = seg_len;
            int collide_connected = 1;
            Constraint* c;
            if (i == 0) rest_len += (start_r + endpoint_pad);
            if (i == link_count) rest_len += (end_r + endpoint_pad);
            if (rope_mode) {
                c = physics_engine_add_spring_constraint(
                    g_state.engine, prev, curr, prev_anchor, curr_anchor, rest_len, rope_stiffness, rope_damping, collide_connected);
            } else {
                c = physics_engine_add_distance_constraint(
                    g_state.engine, prev, curr, prev_anchor, curr_anchor, 0.96f, collide_connected);
                if (c != NULL) c->rest_length = rest_len;
            }
            if (c == NULL) {
                int j;
                for (j = 0; j < link_count; j++) {
                    if (links[j] != NULL) physics_engine_remove_body(g_state.engine, links[j]);
                }
                return 0;
            }
            c->break_force = 0.0f;
            last = c;
            prev = curr;
            prev_anchor = curr_anchor;
        }
    }

    physics_engine_detect_collisions(g_state.engine);
    physics_engine_resolve_collisions(g_state.engine);

    if (out_last != NULL) *out_last = last;
    return link_count;
}

static void remove_selected_constraint(void) {
    if (!selected_constraint_is_active()) return;
    history_push_snapshot();
    physics_engine_constraint_set_active(g_state.engine, g_state.selected_constraint_index, 0);
    g_state.selected_constraint_index = -1;
    push_console_log(L"[Physics] 约束已删除");
}

static void try_create_constraint_from_selection(int mode) {
    Constraint* c = NULL;
    int created_link_count = 0;
    if (g_state.engine == NULL || g_state.selected == NULL || g_state.selected->type != BODY_DYNAMIC) {
        g_state.constraint_seed_body = NULL;
        g_state.constraint_create_mode = 0;
        return;
    }

    if (g_state.constraint_seed_body == NULL || g_state.constraint_create_mode != mode) {
        g_state.constraint_seed_body = g_state.selected;
        g_state.constraint_create_mode = mode;
        push_console_log(L"[Physics] 约束创建起点已选择");
        return;
    }

    if (g_state.constraint_seed_body == g_state.selected) {
        g_state.constraint_seed_body = NULL;
        g_state.constraint_create_mode = 0;
        push_console_log(L"[Physics] 约束创建已取消");
        return;
    }

    if (mode == 1) {
        c = physics_engine_add_distance_constraint(
            g_state.engine, g_state.constraint_seed_body, g_state.selected,
            g_state.constraint_seed_body->position, g_state.selected->position, 0.92f, 0);
    } else if (mode == 2) {
        float rest = vec2_length(vec2_sub(g_state.selected->position, g_state.constraint_seed_body->position));
        float spring_stiffness = 7.0f;
        float spring_damping = 0.9f;
        if (g_state.spring_preset_mode == 0) {
            spring_stiffness = 4.5f;
            spring_damping = 0.6f;
        } else if (g_state.spring_preset_mode == 2) {
            spring_stiffness = 10.0f;
            spring_damping = 1.2f;
        }
        c = physics_engine_add_spring_constraint(
            g_state.engine, g_state.constraint_seed_body, g_state.selected,
            g_state.constraint_seed_body->position, g_state.selected->position, rest, spring_stiffness, spring_damping, 0);
    } else if (mode == 3) {
        created_link_count = create_segmented_link_constraint(g_state.constraint_seed_body, g_state.selected, 0, &c);
    } else if (mode == 4) {
        created_link_count = create_segmented_link_constraint(g_state.constraint_seed_body, g_state.selected, 1, &c);
    }
    if (c != NULL) {
        history_push_snapshot();
        physics_engine_constraint_set_break_force(
            g_state.engine, physics_engine_find_constraint_index(g_state.engine, c), 0.0f);
        g_state.selected_constraint_index = physics_engine_find_constraint_index(g_state.engine, c);
        g_state.selected = NULL;
        g_state.dragging = 0;
        g_state.inspector_focused_row = 0;
        if (mode == 3) push_console_log(L"[物理] 创建链条成功 节点=%d", created_link_count);
        else if (mode == 4) push_console_log(L"[物理] 创建橡皮绳成功 节点=%d", created_link_count);
        else push_console_log(L"[物理] 创建约束成功 类型=%s", (mode == 1) ? L"距离" : L"弹簧");
    } else if (mode == 3 || mode == 4) {
        push_console_log(L"[提示] 创建%s失败：两物体距离过近或资源不足", (mode == 3) ? L"链条" : L"橡皮绳");
    }
    g_state.constraint_seed_body = NULL;
    g_state.constraint_create_mode = 0;
}

static int rand_range(int min_v, int max_v) {
    return min_v + (rand() % (max_v - min_v + 1));
}

static int finite_positive(float v, float min_v) {
    return isfinite(v) && (v >= min_v);
}

static float body_bound_radius(const RigidBody* body) {
    float r = 1.0f;
    int i;
    if (body == NULL || body->shape == NULL) return r;
    if (body->shape->type == SHAPE_CIRCLE) {
        return body->shape->data.circle.radius;
    }
    if (body->shape->type == SHAPE_POLYGON) {
        const PolygonShape* poly = &body->shape->data.polygon;
        r = 0.0f;
        for (i = 0; i < poly->vertex_count; i++) {
            float d = vec2_length(poly->vertices[i]);
            if (d > r) r = d;
        }
        if (r < 1.0f) r = 1.0f;
    }
    return r;
}

static int count_dynamic_bodies(const PhysicsEngine* engine) {
    int i;
    int count = 0;
    if (engine == NULL) return 0;
    for (i = 0; i < physics_engine_get_body_count(engine); i++) {
        const RigidBody* b = physics_engine_get_body(engine, i);
        if (b != NULL && b->type == BODY_DYNAMIC) count++;
    }
    return count;
}

static int stage_world_bounds(float* min_x, float* max_x, float* min_y, float* max_y) {
    ScreenPt smin;
    ScreenPt smax;
    Vec2 wmin;
    Vec2 wmax;
    if (min_x == NULL || max_x == NULL || min_y == NULL || max_y == NULL) return 0;
    if ((g_state.stage_right - g_state.stage_left) < 16.0f || (g_state.stage_bottom - g_state.stage_top) < 16.0f) return 0;
    smin.x = g_state.stage_left;
    smin.y = g_state.stage_top;
    smax.x = g_state.stage_right;
    smax.y = g_state.stage_bottom;
    wmin = screen_to_world(smin);
    wmax = screen_to_world(smax);
    *min_x = min_f(wmin.x, wmax.x);
    *max_x = max_f(wmin.x, wmax.x);
    *min_y = min_f(wmin.y, wmax.y);
    *max_y = max_f(wmin.y, wmax.y);
    return 1;
}

static void clamp_panel_widths(float* left_w, float* right_w);

static int compute_stage_rect_for_size(float w, float h, int full_center_mode, D2D1_RECT_F* out_stage_rect) {
    float menu_h = 30.0f;
    float toolbar_h = 44.0f;
    float top_h = menu_h + toolbar_h + 12.0f;
    float status_h = 32.0f;
    D2D1_RECT_F top_rect;
    D2D1_RECT_F status_rect;
    D2D1_RECT_F center_rect;
    if (out_stage_rect == NULL) return 0;
    if (w <= 1.0f || h <= 1.0f) return 0;
    top_rect = rc(8.0f, 8.0f, w - 8.0f, 8.0f + top_h);
    status_rect = rc(8.0f, h - status_h - 8.0f, w - 8.0f, h - 8.0f);
    if (full_center_mode) {
        center_rect = rc(8.0f, top_rect.bottom + 8.0f, w - 8.0f, status_rect.top - 8.0f);
    } else {
        float bottom_h = g_state.ui_show_bottom_panel ? (g_state.bottom_panel_collapsed ? 24.0f : g_state.ui_bottom_open_h) : 0.0f;
        float left_w = w * g_state.ui_left_ratio;
        float right_w = w * g_state.ui_right_ratio;
        float work_bottom;
        float center_left;
        float center_right;
        D2D1_RECT_F bottom_rect;
        clamp_panel_widths(&left_w, &right_w);
        if (g_state.ui_show_bottom_panel && bottom_h < 24.0f) bottom_h = 24.0f;
        bottom_rect = g_state.ui_show_bottom_panel
                          ? rc(8.0f, status_rect.top - bottom_h - 8.0f, w - 8.0f, status_rect.top - 8.0f)
                          : rc(0.0f, 0.0f, 0.0f, 0.0f);
        work_bottom = g_state.ui_show_bottom_panel ? (bottom_rect.top - 8.0f) : (status_rect.top - 8.0f);
        center_left = g_state.ui_show_left_panel ? (8.0f + left_w + 8.0f) : 8.0f;
        center_right = g_state.ui_show_right_panel ? (w - right_w - 16.0f) : (w - 8.0f);
        center_rect = rc(center_left, top_rect.bottom + 8.0f, center_right, work_bottom);
    }
    *out_stage_rect = rc(center_rect.left + 10.0f, center_rect.top + 46.0f, center_rect.right - 10.0f, center_rect.bottom - 8.0f);
    return 1;
}

static int compute_stage_rect_from_layout(HWND hwnd, D2D1_RECT_F* out_stage_rect) {
    RECT wr;
    GetClientRect(hwnd, &wr);
    return compute_stage_rect_for_size((float)(wr.right - wr.left), (float)(wr.bottom - wr.top), 0, out_stage_rect);
}

static void refresh_stage_bounds_from_layout(HWND hwnd) {
    D2D1_RECT_F stage_rect;
    if (!compute_stage_rect_from_layout(hwnd, &stage_rect)) return;
    g_state.stage_left = stage_rect.left;
    g_state.stage_top = stage_rect.top;
    g_state.stage_right = stage_rect.right;
    g_state.stage_bottom = stage_rect.bottom;
}

static int compute_stage_rect_max(HWND hwnd, D2D1_RECT_F* out_stage_rect) {
    RECT wr;
    GetClientRect(hwnd, &wr);
    return compute_stage_rect_for_size((float)(wr.right - wr.left), (float)(wr.bottom - wr.top), 1, out_stage_rect);
}

static Vec2 clamp_spawn_inside_stage(Vec2 p, float bound_radius) {
    float min_x;
    float max_x;
    float min_y;
    float max_y;
    float r = max_f(bound_radius, 0.1f);
    if (!stage_world_bounds(&min_x, &max_x, &min_y, &max_y)) return p;
    if (p.x < min_x + r) p.x = min_x + r;
    if (p.x > max_x - r) p.x = max_x - r;
    if (p.y < min_y + r) p.y = min_y + r;
    if (p.y > max_y - r) p.y = max_y - r;
    return p;
}

static void fit_scene_bodies_into_stage(void) {
    int i;
    int has_body = 0;
    float scene_min_x = 0.0f;
    float scene_max_x = 0.0f;
    float scene_min_y = 0.0f;
    float scene_max_y = 0.0f;
    float stage_min_x;
    float stage_max_x;
    float stage_min_y;
    float stage_max_y;
    float dx;
    float dy;
    float stage_h;
    const float vertical_bias_ratio = 0.04f;
    if (g_state.engine == NULL) return;
    if (!stage_world_bounds(&stage_min_x, &stage_max_x, &stage_min_y, &stage_max_y)) return;

    for (i = 0; i < physics_engine_get_body_count(g_state.engine); i++) {
        RigidBody* b = physics_engine_get_body(g_state.engine, i);
        float r;
        float bx0;
        float bx1;
        float by0;
        float by1;
        if (b == NULL || b->shape == NULL || !b->active) continue;
        r = body_bound_radius(b);
        bx0 = b->position.x - r;
        bx1 = b->position.x + r;
        by0 = b->position.y - r;
        by1 = b->position.y + r;
        if (!has_body) {
            scene_min_x = bx0;
            scene_max_x = bx1;
            scene_min_y = by0;
            scene_max_y = by1;
            has_body = 1;
        } else {
            if (bx0 < scene_min_x) scene_min_x = bx0;
            if (bx1 > scene_max_x) scene_max_x = bx1;
            if (by0 < scene_min_y) scene_min_y = by0;
            if (by1 > scene_max_y) scene_max_y = by1;
        }
    }
    if (!has_body) return;

    dx = ((stage_min_x + stage_max_x) * 0.5f) - ((scene_min_x + scene_max_x) * 0.5f);
    dy = ((stage_min_y + stage_max_y) * 0.5f) - ((scene_min_y + scene_max_y) * 0.5f);
    stage_h = stage_max_y - stage_min_y;
    if (stage_h > 0.0f) {
        dy += stage_h * vertical_bias_ratio;
    }
    for (i = 0; i < physics_engine_get_body_count(g_state.engine); i++) {
        RigidBody* b = physics_engine_get_body(g_state.engine, i);
        if (b == NULL || b->shape == NULL || !b->active) continue;
        b->position.x += dx;
        b->position.y += dy;
    }
    for (i = 0; i < physics_engine_get_body_count(g_state.engine); i++) {
        RigidBody* b = physics_engine_get_body(g_state.engine, i);
        float r;
        if (b == NULL || b->type != BODY_DYNAMIC || b->shape == NULL || !b->active) continue;
        r = body_bound_radius(b);
        b->position = clamp_spawn_inside_stage(b->position, r);
    }
}

static void recycle_out_of_bounds_objects(HWND hwnd) {
    int i;
    if (g_state.engine == NULL) return;

    {
        D2D1_RECT_F stage_rect;
        ScreenPt smin;
        ScreenPt smax;
        float min_x;
        float max_x;
        float min_y;
        float max_y;
        const float margin = 0.4f;
        Vec2 wmin;
        Vec2 wmax;
        if (!compute_stage_rect_max(hwnd, &stage_rect)) return;
        smin.x = stage_rect.left;
        smin.y = stage_rect.top;
        smax.x = stage_rect.right;
        smax.y = stage_rect.bottom;
        wmin = screen_to_world(smin);
        wmax = screen_to_world(smax);
        min_x = min_f(wmin.x, wmax.x);
        max_x = max_f(wmin.x, wmax.x);
        min_y = min_f(wmin.y, wmax.y);
        max_y = max_f(wmin.y, wmax.y);

        for (i = physics_engine_get_body_count(g_state.engine) - 1; i >= 0; i--) {
            RigidBody* b = physics_engine_get_body(g_state.engine, i);
            float r;
            int out;
            if (b == NULL || b->type != BODY_DYNAMIC) continue;
            r = body_bound_radius(b);
            out = (b->position.x < min_x - r - margin) || (b->position.x > max_x + r + margin) ||
                  (b->position.y < min_y - r - margin) || (b->position.y > max_y + r + margin);
            if (!out) continue;
            if (b == g_state.selected) {
                g_state.selected = NULL;
                g_state.dragging = 0;
            }
            physics_engine_remove_body(g_state.engine, b);
            g_state.recycled_count++;
        }
    }
}

static int can_move_body_to(RigidBody* body, Vec2 new_pos, RigidBody* ignore_a, RigidBody* ignore_b) {
    RigidBody* probe;
    int i;
    if (g_state.engine == NULL || body == NULL || body->shape == NULL) return 0;

    probe = body_create(new_pos.x, new_pos.y, max_f(body->mass, 0.001f), body->shape);
    if (probe == NULL) return 0;
    body_set_shape_ownership(probe, 0);
    probe->angle = body->angle;
    probe->type = body->type;

    for (i = 0; i < physics_engine_get_body_count(g_state.engine); i++) {
        RigidBody* b = physics_engine_get_body(g_state.engine, i);
        CollisionInfo info = {0};
        if (b == NULL || b == body || b == ignore_a || b == ignore_b || b->shape == NULL || !b->active) continue;
        /* Allow circle-chain push only when the moving body is also a circle. */
        if (body->shape->type == SHAPE_CIRCLE &&
            b->type == BODY_DYNAMIC &&
            b->shape->type == SHAPE_CIRCLE) {
            continue;
        }
        if (collision_detect(probe, b, &info) && info.penetration > 0.01f) {
            body_free(probe);
            return 0;
        }
    }
    body_free(probe);
    return 1;
}

static int resolve_spawn_overlap_by_push(RigidBody* spawned) {
    int iter;
    RigidBody* touched[96];
    int touched_count = 0;
    int ti;
    if (g_state.engine == NULL || spawned == NULL) return 0;

    touched[touched_count++] = spawned;

    for (iter = 0; iter < 18; iter++) {
        int i;
        int had_overlap = 0;
        physics_engine_detect_collisions(g_state.engine);

        for (i = 0; i < physics_engine_get_contact_count(g_state.engine); i++) {
            const CollisionManifold* cm = physics_engine_get_contact(g_state.engine, i);
            RigidBody* other = NULL;
            Vec2 dir_to_other;
            float sep;

            if (cm->bodyA == spawned) {
                other = cm->bodyB;
                dir_to_other = cm->info.normal;
            } else if (cm->bodyB == spawned) {
                other = cm->bodyA;
                dir_to_other = vec2_negate(cm->info.normal);
            } else {
                continue;
            }

            if (cm->info.penetration <= 0.01f) continue;
            had_overlap = 1;
            sep = cm->info.penetration + 0.20f;

            if (other != NULL &&
                other->type == BODY_DYNAMIC &&
                other->shape != NULL &&
                other->shape->type == SHAPE_CIRCLE) {
                Vec2 target_other = vec2_add(other->position, vec2_scale(dir_to_other, sep));
                if (can_move_body_to(other, target_other, spawned, NULL)) {
                    other->position = target_other;
                    other->velocity = vec2_scale(dir_to_other, 2.5f);
                    other->angular_velocity = 0.0f;
                    for (ti = 0; ti < touched_count; ti++) {
                        if (touched[ti] == other) break;
                    }
                    if (ti == touched_count && touched_count < 96) touched[touched_count++] = other;
                } else {
                    Vec2 target_spawned = vec2_sub(spawned->position, vec2_scale(dir_to_other, sep));
                    if (can_move_body_to(spawned, target_spawned, other, NULL)) {
                        spawned->position = target_spawned;
                        spawned->velocity = vec2_scale(vec2_negate(dir_to_other), 1.5f);
                        spawned->angular_velocity = 0.0f;
                    }
                }
            } else {
                Vec2 target_spawned = vec2_sub(spawned->position, vec2_scale(dir_to_other, sep));
                if (can_move_body_to(spawned, target_spawned, other, NULL)) {
                    spawned->position = target_spawned;
                    spawned->velocity = vec2(0.0f, 0.0f);
                    spawned->angular_velocity = 0.0f;
                }
            }
        }

        if (!had_overlap) break;
    }

    physics_engine_detect_collisions(g_state.engine);
    for (int i = 0; i < physics_engine_get_contact_count(g_state.engine); i++) {
        const CollisionManifold* cm = physics_engine_get_contact(g_state.engine, i);
        int a_touched = 0;
        int b_touched = 0;
        for (ti = 0; ti < touched_count; ti++) {
            if (cm->bodyA == touched[ti]) a_touched = 1;
            if (cm->bodyB == touched[ti]) b_touched = 1;
        }

        if (a_touched || b_touched) {
            /* General overlap guard for spawned/touched cluster. */
            if (cm->info.penetration > 0.03f &&
                (cm->bodyA == spawned || cm->bodyB == spawned)) {
                return 0;
            }

            /* Hard barrier: do not allow circles to remain inside polygon bodies. */
            if (cm->info.penetration > 0.01f) {
                int a_circle = (cm->bodyA && cm->bodyA->shape && cm->bodyA->shape->type == SHAPE_CIRCLE);
                int b_circle = (cm->bodyB && cm->bodyB->shape && cm->bodyB->shape->type == SHAPE_CIRCLE);
                int a_poly = (cm->bodyA && cm->bodyA->shape && cm->bodyA->shape->type == SHAPE_POLYGON);
                int b_poly = (cm->bodyB && cm->bodyB->shape && cm->bodyB->shape->type == SHAPE_POLYGON);
                if ((a_circle && b_poly) || (b_circle && a_poly)) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

static void spawn_circle_at_cursor(void) {
    SceneConfig* cfg;
    Shape* shape;
    RigidBody* body;
    Vec2 p;
    trace_spawn_step("spawn.circle.begin", "mx=%.1f my=%.1f", g_state.mouse_screen.x, g_state.mouse_screen.y);
    if (g_state.engine == NULL) return;
    if (g_state.mouse_screen.x < g_state.stage_left || g_state.mouse_screen.x > g_state.stage_right ||
        g_state.mouse_screen.y < g_state.stage_top || g_state.mouse_screen.y > g_state.stage_bottom) {
        push_console_log(L"[警告] 鼠标未在舞台内，无法创建圆");
        return;
    }
    cfg = &g_state.scenes[g_state.scene_index];
    if (!finite_positive(cfg->spawn_circle_radius, 0.05f) || !finite_positive(cfg->ball_mass, 0.001f)) {
        trace_spawn_step("spawn.circle.abort", "invalid-config r=%.4f m=%.4f", cfg->spawn_circle_radius, cfg->ball_mass);
        push_console_log(L"[错误] 圆创建失败：参数无效");
        return;
    }
    p = screen_to_world(g_state.mouse_screen);
    p = clamp_spawn_inside_stage(p, cfg->spawn_circle_radius);
    shape = shape_create_circle(cfg->spawn_circle_radius);
    if (shape == NULL) {
        trace_spawn_step("spawn.circle.abort", "shape-create-failed");
        push_console_log(L"[错误] 圆创建失败：内存不足");
        return;
    }
    shape->restitution = cfg->ball_restitution;
    shape->friction = 0.25f;
    body = body_create(p.x, p.y, cfg->ball_mass, shape);
    if (body == NULL) {
        shape_free(shape);
        trace_spawn_step("spawn.circle.abort", "body-create-failed");
        push_console_log(L"[错误] 圆创建失败：对象分配失败");
        return;
    }
    physics_engine_add_body(g_state.engine, body);
    trace_spawn_step("spawn.circle.added", "x=%.3f y=%.3f m=%.3f", body->position.x, body->position.y, body->mass);
    if (!resolve_spawn_overlap_by_push(body)) {
        physics_engine_remove_body(g_state.engine, body);
        trace_spawn_step("spawn.circle.abort", "overlap-rejected");
        push_console_log(L"[警告] 圆创建失败：与现有物体重叠");
        return;
    }
    body->velocity = vec2(rand_range(-50, 50) / 10.0f, rand_range(-40, 0) / 10.0f);
    history_push_snapshot();
    trace_spawn_step("spawn.circle.done", "ok");
    push_console_log(L"[创建] 圆 位置(%.1f,%.1f) 质量=%.2f", body->position.x, body->position.y, body->mass);
}

static void spawn_box_at_cursor(void) {
    SceneConfig* cfg;
    Shape* shape;
    RigidBody* body;
    Vec2 p;
    float half;
    trace_spawn_step("spawn.box.begin", "mx=%.1f my=%.1f", g_state.mouse_screen.x, g_state.mouse_screen.y);
    if (g_state.engine == NULL) return;
    if (g_state.mouse_screen.x < g_state.stage_left || g_state.mouse_screen.x > g_state.stage_right ||
        g_state.mouse_screen.y < g_state.stage_top || g_state.mouse_screen.y > g_state.stage_bottom) {
        push_console_log(L"[警告] 鼠标未在舞台内，无法创建方块");
        return;
    }
    cfg = &g_state.scenes[g_state.scene_index];
    if (!finite_positive(cfg->spawn_box_size, 0.10f) || !finite_positive(cfg->box_mass, 0.001f)) {
        trace_spawn_step("spawn.box.abort", "invalid-config size=%.4f m=%.4f", cfg->spawn_box_size, cfg->box_mass);
        push_console_log(L"[错误] 方块创建失败：参数无效");
        return;
    }
    half = cfg->spawn_box_size * 0.72f;
    p = screen_to_world(g_state.mouse_screen);
    p = clamp_spawn_inside_stage(p, half);
    shape = shape_create_box(cfg->spawn_box_size, cfg->spawn_box_size);
    if (shape == NULL) {
        trace_spawn_step("spawn.box.abort", "shape-create-failed");
        push_console_log(L"[错误] 方块创建失败：内存不足");
        return;
    }
    shape->restitution = cfg->box_restitution;
    shape->friction = 0.35f;
    body = body_create(p.x, p.y, cfg->box_mass, shape);
    if (body == NULL) {
        shape_free(shape);
        trace_spawn_step("spawn.box.abort", "body-create-failed");
        push_console_log(L"[错误] 方块创建失败：对象分配失败");
        return;
    }
    physics_engine_add_body(g_state.engine, body);
    trace_spawn_step("spawn.box.added", "x=%.3f y=%.3f m=%.3f", body->position.x, body->position.y, body->mass);
    if (!resolve_spawn_overlap_by_push(body)) {
        physics_engine_remove_body(g_state.engine, body);
        trace_spawn_step("spawn.box.abort", "overlap-rejected");
        push_console_log(L"[警告] 方块创建失败：与现有物体重叠");
        return;
    }
    body->angular_velocity = rand_range(-40, 40) / 10.0f;
    history_push_snapshot();
    trace_spawn_step("spawn.box.done", "ok");
    push_console_log(L"[创建] 方块 位置(%.1f,%.1f) 质量=%.2f", body->position.x, body->position.y, body->mass);
}

static void draw_text(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    ui_draw_text(g_ui.target, g_ui.brush, text, rect, fmt, color);
}

static void draw_text_vcenter(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    ui_draw_text_vcenter(g_ui.target, g_ui.brush, g_ui.dwrite_factory, text, rect, fmt, color);
}

static void draw_text_hvcenter(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    ui_draw_text_hvcenter(g_ui.target, g_ui.brush, text, rect, fmt, color);
}

static void draw_text_right_vcenter(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    ui_draw_text_right_vcenter(g_ui.target, g_ui.brush, g_ui.dwrite_factory, text, rect, fmt, color);
}

static float measure_text_width(const wchar_t* text, IDWriteTextFormat* fmt) {
    return ui_measure_text_width(g_ui.dwrite_factory, text, fmt);
}

static int scene_overlap_resolve(PhysicsEngine* engine, RigidBody* body, void* user) {
    (void)engine;
    (void)user;
    return resolve_spawn_overlap_by_push(body);
}

static void apply_scene(int scene_index) {
    const SceneConfig* cfg;
    if (scene_index < 0 || scene_index >= SCENE_COUNT) {
        scene_index = 0;
    }
    if (g_state.engine != NULL) {
        physics_engine_free(g_state.engine);
        g_state.engine = NULL;
    }
    g_state.engine = physics_engine_create();
    if (g_state.engine == NULL) {
        return;
    }

    cfg = &g_state.scenes[scene_index];
    physics_engine_set_gravity(g_state.engine, vec2(0.0f, cfg->gravity_y));
    physics_engine_set_time_step(g_state.engine, cfg->time_step);
    physics_engine_set_damping(g_state.engine, cfg->damping);
    physics_engine_set_iterations(g_state.engine, cfg->iterations);
    scene_builder_build(g_state.engine, scene_index, cfg, scene_overlap_resolve, NULL);
    g_state.scene_index = scene_index;
    g_state.selected = NULL;
    g_state.selected_constraint_index = -1;
    g_state.constraint_seed_body = NULL;
    g_state.constraint_create_mode = 0;
    g_state.dragging = 0;
    g_state.inspector_focused_row = 0;
    g_state.recycled_count = 0;
    g_state.last_contact_count = 0;
    g_state.last_collision_capture_ms = 0;
    clear_collision_events();
    g_state.scene_needs_stage_fit = 1;
    push_console_log(L"[场景] 切换到: %s", scene_catalog_name(g_state.scene_index));
    if (!g_state.history_replaying) {
        history_reset_and_capture();
    }
}

static float body_box_size_hint(const RigidBody* b) {
    int i;
    float max_x = 1.0f;
    float min_x = -1.0f;
    if (b == NULL || b->shape == NULL || b->shape->type != SHAPE_POLYGON) return 4.0f;
    for (i = 0; i < b->shape->data.polygon.vertex_count; i++) {
        float x = b->shape->data.polygon.vertices[i].x;
        if (x > max_x) max_x = x;
        if (x < min_x) min_x = x;
    }
    return max_f(2.0f, max_x - min_x);
}

static int save_scene_snapshot(const char* path) {
    int ok;
    trace_spawn_step("snapshot.save.begin", "path=%s engine=%p", (path != NULL) ? path : "(null)", (void*)g_state.engine);
    ok = snapshot_repo_save(g_state.engine, path);
    if (ok) trace_spawn_step("snapshot.save.end", "path=%s ok=1", path);
    return ok;
}

static int load_scene_snapshot(const char* path) {
    if (!snapshot_repo_load(g_state.engine, path)) return 0;
    g_state.selected = NULL;
    g_state.selected_constraint_index = -1;
    g_state.dragging = 0;
    g_state.inspector_focused_row = 0;
    g_state.last_collision_capture_ms = 0;
    clear_collision_events();
    return 1;
}

static int file_exists_utf8_path(const char* path) {
    DWORD attr;
    wchar_t wpath[260];
    if (path == NULL) return 0;
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, 260);
    attr = GetFileAttributesW(wpath);
    if (attr == INVALID_FILE_ATTRIBUTES) return 0;
    return ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

static HistoryState* history_state_view(void) {
    /* Layout-compatible view over contiguous history fields in g_state. */
    return (HistoryState*)&g_state.history_cursor;
}

static int history_save_snapshot_cb(const char* path, void* user) {
    (void)user;
    return save_scene_snapshot(path);
}

static int history_load_snapshot_cb(const char* path, void* user) {
    (void)user;
    return load_scene_snapshot(path);
}

static void history_log_text_cb(const wchar_t* text, void* user) {
    (void)user;
    if (text == NULL) return;
    push_console_log(L"%s", text);
}

static HistoryServiceOps history_service_ops(void) {
    HistoryServiceOps ops;
    ops.max_slots = HISTORY_MAX_SLOTS;
    ops.save_snapshot = history_save_snapshot_cb;
    ops.load_snapshot = history_load_snapshot_cb;
    ops.log_text = history_log_text_cb;
    ops.user = NULL;
    return ops;
}

static void history_push_snapshot(void) {
    HistoryServiceOps ops = history_service_ops();
    trace_spawn_step("history.push.begin", "replay=%d engine=%p cursor=%d top=%d",
                     g_state.history_replaying, (void*)g_state.engine, g_state.history_cursor, g_state.history_top);
    if (g_state.history_replaying) return;
    if (g_state.engine == NULL) return;
    history_service_push_snapshot(history_state_view(), &ops);
    trace_spawn_step("history.push.end", "cursor=%d top=%d", g_state.history_cursor, g_state.history_top);
}

static void history_reset_and_capture(void) {
    HistoryServiceOps ops = history_service_ops();
    history_service_reset_and_capture(history_state_view(), &ops);
}

static void history_undo(void) {
    HistoryServiceOps ops = history_service_ops();
    history_service_undo(history_state_view(), &ops);
}

static void history_redo(void) {
    HistoryServiceOps ops = history_service_ops();
    history_service_redo(history_state_view(), &ops);
}

static void sync_scene_runtime_params(void) {
    SceneConfig* cfg;
    if (g_state.engine == NULL) return;
    cfg = &g_state.scenes[g_state.scene_index];
    runtime_param_sync_engine(g_state.engine, cfg);
}

static int can_adjust_param(const SceneConfig* cfg, int param_index, int sign) {
    return runtime_param_can_adjust(cfg, param_index, sign);
}

static void adjust_focused_param(int sign) {
    SceneConfig* cfg = &g_state.scenes[g_state.scene_index];
    if (runtime_param_adjust(cfg, g_state.focused_param, sign)) {
        sync_scene_runtime_params();
    }
}

static HRESULT create_app_icon_bitmap(void) {
    HRESULT hr = S_OK;
    IWICBitmapDecoder* decoder = NULL;
    IWICBitmapFrameDecode* frame = NULL;
    IWICFormatConverter* converter = NULL;
    const wchar_t* candidates[] = {
        L"assets/icons/physics_sandbox_32.png",
        L"../assets/icons/physics_sandbox_32.png",
        L"..\\assets\\icons\\physics_sandbox_32.png"
    };
    int i;

    if (g_ui.app_icon_bitmap != NULL) return S_OK;
    if (g_ui.wic_factory == NULL || g_ui.target == NULL) return E_FAIL;

    hr = E_FAIL;
    for (i = 0; i < (int)(sizeof(candidates) / sizeof(candidates[0])); i++) {
        hr = IWICImagingFactory_CreateDecoderFromFilename(g_ui.wic_factory, candidates[i], NULL, GENERIC_READ,
                                                          WICDecodeMetadataCacheOnLoad, &decoder);
        if (SUCCEEDED(hr)) break;
    }
    if (FAILED(hr)) return hr;

    hr = IWICBitmapDecoder_GetFrame(decoder, 0, &frame);
    if (SUCCEEDED(hr)) {
        hr = IWICImagingFactory_CreateFormatConverter(g_ui.wic_factory, &converter);
    }
    if (SUCCEEDED(hr)) {
        hr = IWICFormatConverter_Initialize(converter, (IWICBitmapSource*)frame, &GUID_WICPixelFormat32bppPBGRA,
                                            WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);
    }
    if (SUCCEEDED(hr)) {
        hr = ID2D1HwndRenderTarget_CreateBitmapFromWicBitmap(g_ui.target, (IWICBitmapSource*)converter, NULL, &g_ui.app_icon_bitmap);
    }

    release_unknown((IUnknown**)&converter);
    release_unknown((IUnknown**)&frame);
    release_unknown((IUnknown**)&decoder);
    return hr;
}

static HRESULT create_device_resources(HWND hwnd) {
    RECT wr;
    D2D1_RENDER_TARGET_PROPERTIES rtp;
    D2D1_HWND_RENDER_TARGET_PROPERTIES htp;
    D2D1_COLOR_F bc;
    UINT32 width;
    UINT32 height;
    HRESULT hr = S_OK;

    GetClientRect(hwnd, &wr);
    width = (UINT32)(wr.right - wr.left);
    height = (UINT32)(wr.bottom - wr.top);

    if (g_ui.target == NULL) {
        rtp.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
        rtp.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
        rtp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_UNKNOWN;
        rtp.dpiX = 0.0f;
        rtp.dpiY = 0.0f;
        rtp.usage = D2D1_RENDER_TARGET_USAGE_NONE;
        rtp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

        htp.hwnd = hwnd;
        htp.pixelSize.width = width;
        htp.pixelSize.height = height;
        htp.presentOptions = D2D1_PRESENT_OPTIONS_NONE;

        hr = ID2D1Factory_CreateHwndRenderTarget(g_ui.d2d_factory, &rtp, &htp, &g_ui.target);
        if (SUCCEEDED(hr)) {
            // Keep D2D coordinates in raw pixels to match mouse/input/layout coordinates.
            ID2D1HwndRenderTarget_SetDpi(g_ui.target, 96.0f, 96.0f);
        }
    }
    if (SUCCEEDED(hr) && g_ui.brush == NULL) {
        bc = rgba(0.1f, 0.1f, 0.1f, 1.0f);
        hr = ID2D1HwndRenderTarget_CreateSolidColorBrush(g_ui.target, &bc, NULL, &g_ui.brush);
    }
    if (SUCCEEDED(hr) && g_ui.app_icon_bitmap == NULL) {
        hr = create_app_icon_bitmap();
    }
    return hr;
}

static void discard_device_resources(void) {
    release_unknown((IUnknown**)&g_ui.app_icon_bitmap);
    release_unknown((IUnknown**)&g_ui.brush);
    release_unknown((IUnknown**)&g_ui.target);
}

static HRESULT create_text_formats(void) {
    HRESULT hr;
    hr = IDWriteFactory_CreateTextFormat(g_ui.dwrite_factory, L"Roboto", NULL, DWRITE_FONT_WEIGHT_SEMI_BOLD,
                                         DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 28.0f, L"en-us",
                                         &g_ui.fmt_title);
    if (FAILED(hr)) return hr;

    hr = IDWriteFactory_CreateTextFormat(g_ui.dwrite_factory, L"Roboto", NULL, DWRITE_FONT_WEIGHT_SEMI_BOLD,
                                         DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 22.0f, L"en-us",
                                         &g_ui.fmt_value);
    if (FAILED(hr)) return hr;

    hr = IDWriteFactory_CreateTextFormat(g_ui.dwrite_factory, L"Source Han Sans SC", NULL, DWRITE_FONT_WEIGHT_NORMAL,
                                         DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 21.0f, L"zh-cn",
                                         &g_ui.fmt_ui);
    if (FAILED(hr)) return hr;

    hr = IDWriteFactory_CreateTextFormat(g_ui.dwrite_factory, L"Roboto", NULL, DWRITE_FONT_WEIGHT_NORMAL,
                                         DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 18.0f, L"en-us",
                                         &g_ui.fmt_mono);
    if (FAILED(hr)) return hr;

    hr = IDWriteFactory_CreateTextFormat(g_ui.dwrite_factory, L"Source Han Sans SC", NULL, DWRITE_FONT_WEIGHT_NORMAL,
                                         DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 16.5f, L"zh-cn",
                                         &g_ui.fmt_info);
    if (FAILED(hr)) return hr;

    hr = IDWriteFactory_CreateTextFormat(g_ui.dwrite_factory, L"Source Han Sans SC", NULL, DWRITE_FONT_WEIGHT_NORMAL,
                                         DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20.0f, L"zh-cn",
                                         &g_ui.fmt_button);
    if (FAILED(hr)) return hr;

    hr = IDWriteFactory_CreateTextFormat(g_ui.dwrite_factory, L"Roboto", NULL, DWRITE_FONT_WEIGHT_SEMI_BOLD,
                                         DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 22.0f, L"en-us",
                                         &g_ui.fmt_value_center);
    if (FAILED(hr)) return hr;

    hr = IDWriteFactory_CreateTextFormat(g_ui.dwrite_factory, L"Roboto", NULL, DWRITE_FONT_WEIGHT_NORMAL,
                                         DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 18.0f, L"en-us",
                                         &g_ui.fmt_mono_center);
    if (FAILED(hr)) return hr;

    hr = IDWriteFactory_CreateTextFormat(g_ui.dwrite_factory, L"Roboto", NULL, DWRITE_FONT_WEIGHT_NORMAL,
                                         DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20.0f, L"en-us",
                                         &g_ui.fmt_icon);
    if (FAILED(hr)) return hr;

    IDWriteTextFormat_SetTextAlignment(g_ui.fmt_title, DWRITE_TEXT_ALIGNMENT_CENTER);
    IDWriteTextFormat_SetParagraphAlignment(g_ui.fmt_title, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    IDWriteTextFormat_SetTextAlignment(g_ui.fmt_button, DWRITE_TEXT_ALIGNMENT_CENTER);
    IDWriteTextFormat_SetParagraphAlignment(g_ui.fmt_button, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    IDWriteTextFormat_SetTextAlignment(g_ui.fmt_icon, DWRITE_TEXT_ALIGNMENT_CENTER);
    IDWriteTextFormat_SetParagraphAlignment(g_ui.fmt_icon, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    IDWriteTextFormat_SetTextAlignment(g_ui.fmt_value_center, DWRITE_TEXT_ALIGNMENT_CENTER);
    IDWriteTextFormat_SetParagraphAlignment(g_ui.fmt_value_center, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    IDWriteTextFormat_SetTextAlignment(g_ui.fmt_mono_center, DWRITE_TEXT_ALIGNMENT_CENTER);
    IDWriteTextFormat_SetParagraphAlignment(g_ui.fmt_mono_center, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
    return S_OK;
}

static HRESULT init_ui(void) {
    HRESULT hr;
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &IID_ID2D1Factory, NULL, (void**)&g_ui.d2d_factory);
    if (FAILED(hr)) return hr;

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, &IID_IDWriteFactory, (IUnknown**)&g_ui.dwrite_factory);
    if (FAILED(hr)) return hr;

    hr = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory,
                          (void**)&g_ui.wic_factory);
    if (FAILED(hr)) return hr;

    return create_text_formats();
}

static void shutdown_ui(void) {
    discard_device_resources();
    release_unknown((IUnknown**)&g_ui.fmt_title);
    release_unknown((IUnknown**)&g_ui.fmt_value);
    release_unknown((IUnknown**)&g_ui.fmt_ui);
    release_unknown((IUnknown**)&g_ui.fmt_mono);
    release_unknown((IUnknown**)&g_ui.fmt_info);
    release_unknown((IUnknown**)&g_ui.fmt_button);
    release_unknown((IUnknown**)&g_ui.fmt_icon);
    release_unknown((IUnknown**)&g_ui.fmt_value_center);
    release_unknown((IUnknown**)&g_ui.fmt_mono_center);
    release_unknown((IUnknown**)&g_ui.wic_factory);
    release_unknown((IUnknown**)&g_ui.dwrite_factory);
    release_unknown((IUnknown**)&g_ui.d2d_factory);
}

static void draw_body_2d(RigidBody* body) {
    if (body == NULL || body->shape == NULL) return;

    if (body->shape->type == SHAPE_CIRCLE) {
        float r = body->shape->data.circle.radius * WORLD_SCALE;
        ScreenPt c = world_to_screen(body->position);
        D2D1_ELLIPSE e;
        e.point = pt(c.x, c.y);
        e.radiusX = r;
        e.radiusY = r;
        if (body == g_state.selected) {
            set_brush_color(0.98f, 0.68f, 0.30f, 1.0f);
        } else if (body->type == BODY_STATIC) {
            set_brush_color(0.70f, 0.72f, 0.76f, 1.0f);
        } else {
            set_brush_color(0.27f, 0.62f, 0.95f, 1.0f);
        }
        ID2D1HwndRenderTarget_FillEllipse(g_ui.target, &e, (ID2D1Brush*)g_ui.brush);
        if (g_state.draw_centers) {
            D2D1_ELLIPSE ce = {pt(c.x, c.y), 2.0f, 2.0f};
            set_brush_color(0.15f, 0.47f, 0.86f, 1.0f);
            ID2D1HwndRenderTarget_FillEllipse(g_ui.target, &ce, (ID2D1Brush*)g_ui.brush);
        }
        return;
    }

    if (body->shape->type == SHAPE_POLYGON) {
        PolygonShape* p = &body->shape->data.polygon;
        int i;
        ID2D1PathGeometry* geo = NULL;
        ID2D1GeometrySink* sink = NULL;
        D2D1_POINT_2F pts[64];
        int n = p->vertex_count;
        if (n > 64) n = 64;

        if (body == g_state.selected) {
            set_brush_color(0.98f, 0.68f, 0.30f, 1.0f);
        } else if (body->type == BODY_STATIC) {
            set_brush_color(0.70f, 0.72f, 0.76f, 1.0f);
        } else {
            set_brush_color(0.31f, 0.73f, 0.46f, 1.0f);
        }

        for (i = 0; i < n; i++) {
            Vec2 wv = body_get_world_point(body, p->vertices[i]);
            ScreenPt sv = world_to_screen(wv);
            pts[i] = pt(sv.x, sv.y);
        }

        if (SUCCEEDED(ID2D1Factory_CreatePathGeometry(g_ui.d2d_factory, &geo)) &&
            SUCCEEDED(ID2D1PathGeometry_Open(geo, &sink))) {
            ID2D1GeometrySink_BeginFigure(sink, pts[0], D2D1_FIGURE_BEGIN_FILLED);
            if (n > 1) {
                ID2D1GeometrySink_AddLines(sink, &pts[1], (UINT32)(n - 1));
            }
            ID2D1GeometrySink_EndFigure(sink, D2D1_FIGURE_END_CLOSED);
            ID2D1GeometrySink_Close(sink);
            ID2D1HwndRenderTarget_FillGeometry(g_ui.target, (ID2D1Geometry*)geo, (ID2D1Brush*)g_ui.brush, NULL);
        }
        release_unknown((IUnknown**)&sink);
        release_unknown((IUnknown**)&geo);
        if (g_state.draw_centers) {
            ScreenPt c = world_to_screen(body->position);
            D2D1_ELLIPSE ce = {pt(c.x, c.y), 2.0f, 2.0f};
            set_brush_color(0.15f, 0.47f, 0.86f, 1.0f);
            ID2D1HwndRenderTarget_FillEllipse(g_ui.target, &ce, (ID2D1Brush*)g_ui.brush);
        }
    }
}

static void draw_velocity(RigidBody* body) {
    if (!g_state.draw_velocity || body == NULL || body->type != BODY_DYNAMIC) return;
    ScreenPt from = world_to_screen(body->position);
    Vec2 tip_w = vec2_add(body->position, vec2_scale(body->velocity, 0.2f));
    ScreenPt to = world_to_screen(tip_w);
    set_brush_color(0.08f, 0.55f, 0.27f, 1.0f);
    ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(from.x, from.y), pt(to.x, to.y), (ID2D1Brush*)g_ui.brush, 2.0f,
                                   NULL);
}

static void draw_contacts(void) {
    int i;
    if (!g_state.draw_contacts || g_state.engine == NULL) return;
    set_brush_color(0.77f, 0.1f, 0.13f, 1.0f);
    for (i = 0; i < physics_engine_get_contact_count(g_state.engine); i++) {
        const CollisionManifold* m = physics_engine_get_contact(g_state.engine, i);
        ScreenPt p = world_to_screen(m->info.point);
        Vec2 end_w = vec2_add(m->info.point, vec2_scale(m->info.normal, 2.0f));
        ScreenPt n = world_to_screen(end_w);
        D2D1_ELLIPSE e = {pt(p.x, p.y), 3.0f, 3.0f};
        ID2D1HwndRenderTarget_FillEllipse(g_ui.target, &e, (ID2D1Brush*)g_ui.brush);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(p.x, p.y), pt(n.x, n.y), (ID2D1Brush*)g_ui.brush, 1.5f, NULL);
    }
}

static void draw_constraints_debug(void) {
    int i;
    if (!g_state.draw_constraints || g_state.engine == NULL) return;

    for (i = 0; i < physics_engine_get_constraint_count(g_state.engine); i++) {
        const Constraint* c = physics_engine_get_constraint(g_state.engine, i);
        Vec2 aw;
        Vec2 bw;
        ScreenPt a;
        ScreenPt b;
        D2D1_ELLIPSE ea;
        D2D1_ELLIPSE eb;
        float stress = 0.0f;
        float line_w = (i == g_state.selected_constraint_index) ? 3.2f : 2.0f;
        if (!c->active || c->body_a == NULL || c->body_b == NULL) continue;

        constraint_world_anchors(c, &aw, &bw);
        a = world_to_screen(aw);
        b = world_to_screen(bw);

        if (c->break_force > 1e-4f) {
            stress = clamp(c->last_force / c->break_force, 0.0f, 2.0f);
        }

        if (c->type == CONSTRAINT_DISTANCE) {
            set_brush_color(0.10f + 0.45f * stress, 0.52f - 0.22f * stress, 0.92f - 0.55f * stress, 1.0f);
        } else {
            set_brush_color(0.96f - 0.36f * stress, 0.57f - 0.22f * stress, 0.19f + 0.12f * stress, 1.0f);
        }
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(a.x, a.y), pt(b.x, b.y), (ID2D1Brush*)g_ui.brush, line_w, NULL);

        ea = (D2D1_ELLIPSE){pt(a.x, a.y), (i == g_state.selected_constraint_index) ? 4.2f : 3.2f, (i == g_state.selected_constraint_index) ? 4.2f : 3.2f};
        eb = (D2D1_ELLIPSE){pt(b.x, b.y), (i == g_state.selected_constraint_index) ? 4.2f : 3.2f, (i == g_state.selected_constraint_index) ? 4.2f : 3.2f};
        ID2D1HwndRenderTarget_FillEllipse(g_ui.target, &ea, (ID2D1Brush*)g_ui.brush);
        ID2D1HwndRenderTarget_FillEllipse(g_ui.target, &eb, (ID2D1Brush*)g_ui.brush);
    }

    if (g_state.constraint_seed_body != NULL) {
        ScreenPt s = world_to_screen(g_state.constraint_seed_body->position);
        D2D1_ELLIPSE mark = {pt(s.x, s.y), 7.0f, 7.0f};
        set_brush_color(0.94f, 0.78f, 0.20f, 1.0f);
        ID2D1HwndRenderTarget_DrawEllipse(g_ui.target, &mark, (ID2D1Brush*)g_ui.brush, 2.0f, NULL);
    }
}

static void drag_selected_body_stepwise(void) {
    Vec2 from;
    Vec2 to;
    Vec2 delta;
    Vec2 step;
    Vec2 prev;
    float dist;
    int steps;
    int i;
    const float max_penetration = 0.18f;
    ScreenPt m = g_state.mouse_screen;

    if (!g_state.dragging || g_state.selected == NULL || g_state.engine == NULL) return;

    if (m.x > g_state.stage_right) m.x = g_state.stage_right;
    if (m.x < g_state.stage_left) m.x = g_state.stage_left;
    if (m.y > g_state.stage_bottom) m.y = g_state.stage_bottom;
    if (m.y < g_state.stage_top) m.y = g_state.stage_top;
    to = screen_to_world(m);
    from = g_state.selected->position;
    delta = vec2_sub(to, from);
    dist = vec2_length(delta);
    if (dist < 1e-4f) return;

    steps = (int)ceilf(dist / 0.55f);
    if (steps < 1) steps = 1;
    if (steps > 40) steps = 40;
    step = vec2_scale(delta, 1.0f / (float)steps);

    for (i = 0; i < steps; i++) {
        int c;
        int blocked = 0;
        prev = g_state.selected->position;
        g_state.selected->position = vec2_add(g_state.selected->position, step);
        g_state.selected->velocity = vec2(0.0f, 0.0f);
        g_state.selected->angular_velocity = 0.0f;
        g_state.selected->force = vec2(0.0f, 0.0f);
        g_state.selected->torque = 0.0f;
        physics_engine_detect_collisions(g_state.engine);
        physics_engine_resolve_collisions(g_state.engine);
        physics_engine_detect_collisions(g_state.engine);

        for (c = 0; c < physics_engine_get_contact_count(g_state.engine); c++) {
            const CollisionManifold* cm = physics_engine_get_contact(g_state.engine, c);
            RigidBody* other = NULL;
            Vec2 push_dir = vec2(0.0f, 0.0f);
            float sep;
            Vec2 target;
            if (cm == NULL) continue;
            if (cm->bodyA != g_state.selected && cm->bodyB != g_state.selected) continue;
            if (cm->info.penetration <= max_penetration) continue;

            if (cm->bodyA == g_state.selected) {
                other = cm->bodyB;
                push_dir = cm->info.normal;
            } else {
                other = cm->bodyA;
                push_dir = vec2_negate(cm->info.normal);
            }

            if (other != NULL &&
                other->type == BODY_DYNAMIC &&
                other->shape != NULL &&
                other->shape->type == SHAPE_CIRCLE) {
                sep = cm->info.penetration + 0.24f;
                target = vec2_add(other->position, vec2_scale(push_dir, sep));
                if (can_move_body_to(other, target, g_state.selected, NULL)) {
                    other->position = target;
                    other->velocity = vec2_scale(push_dir, 2.8f);
                    other->angular_velocity = 0.0f;
                    continue;
                }
            }

            blocked = 1;
            break;
        }
        if (blocked) {
            g_state.selected->position = prev;
            g_state.selected->velocity = vec2(0.0f, 0.0f);
            g_state.selected->angular_velocity = 0.0f;
            physics_engine_detect_collisions(g_state.engine);
            physics_engine_resolve_collisions(g_state.engine);
            break;
        }

        physics_engine_detect_collisions(g_state.engine);
        physics_engine_resolve_collisions(g_state.engine);
    }
}

static void draw_card_round(D2D1_RECT_F r, float radius, D2D1_COLOR_F fill, D2D1_COLOR_F border) {
    ui_draw_card_round(g_ui.target, g_ui.brush, r, radius, fill, border);
}

static void draw_panel_header_band(D2D1_RECT_F panel_rect, float top_h, float inset) {
    ui_draw_panel_header_band(g_ui.target, g_ui.brush, panel_rect, top_h, inset);
}

static void draw_outer_shadow_rr(D2D1_ROUNDED_RECT rr) {
    ui_draw_outer_shadow_rr(g_ui.target, g_ui.brush, rr);
}

static void draw_action_button(D2D1_RECT_F r, const wchar_t* label, int active, int hovered) {
    ui_draw_action_button(g_ui.target, g_ui.brush, g_ui.fmt_button, r, label, active, hovered);
}

static void draw_toolbar_icon_button(D2D1_RECT_F r, ToolbarIconId icon, int active, int hovered) {
    ui_draw_toolbar_icon_button(g_ui.target, g_ui.brush, r, icon, active, hovered);
}

static void draw_text_tab_button(D2D1_RECT_F rect, const wchar_t* label, int active, int hovered) {
    D2D1_RECT_F indicator;
    D2D1_COLOR_F text_color;
    float text_w;
    float tab_w;
    float indicator_w;
    float cx;
    if (active) text_color = rgba(0.94f, 0.97f, 1.0f, 1.0f);
    else if (hovered) text_color = rgba(0.82f, 0.89f, 0.97f, 1.0f);
    else text_color = rgba(0.70f, 0.78f, 0.90f, 1.0f);
    draw_text_hvcenter(label, rect, g_ui.fmt_info, text_color);
    if (active) {
        text_w = measure_text_width(label, g_ui.fmt_info);
        tab_w = rect.right - rect.left;
        indicator_w = text_w + 16.0f;
        if (indicator_w < 24.0f) indicator_w = 24.0f;
        if (indicator_w > tab_w - 8.0f) indicator_w = tab_w - 8.0f;
        cx = (rect.left + rect.right) * 0.5f;
        indicator = rc(cx - indicator_w * 0.5f, rect.bottom - 2.0f, cx + indicator_w * 0.5f, rect.bottom);
        draw_card_round(indicator, 1.0f, rgba(0.56f, 0.73f, 0.95f, 1.0f), rgba(0.56f, 0.73f, 0.95f, 1.0f));
    }
}

static const wchar_t* body_kind_name(const RigidBody* b) {
    if (b == NULL) return L"--";
    if (b->type == BODY_STATIC) return L"静态";
    if (b->type == BODY_KINEMATIC) return L"运动学";
    return L"动态";
}

static const wchar_t* body_shape_name(const RigidBody* b) {
    if (b == NULL || b->shape == NULL) return L"--";
    return (b->shape->type == SHAPE_CIRCLE) ? L"圆" : L"多边形";
}

static int inspector_row_count(void) {
    const Constraint* c = selected_constraint_ref();
    if (c != NULL && c->active) {
        if (c->type == CONSTRAINT_SPRING) return 6;
        return 5;
    }
    if (g_state.selected != NULL) return 8;
    return 0;
}

static void begin_value_input_for_inspector_row(int row) {
    const Constraint* c = selected_constraint_ref();
    g_state.show_value_input = 1;
    g_state.value_input_target = 1;
    g_state.value_input_row = row;
    g_state.value_input_len = 0;
    g_state.value_input_caret = 0;
    g_state.value_input_buf[0] = L'\0';
    if (c != NULL && c->active) {
        if (row == 0) swprintf(g_state.value_input_buf, 64, L"%.4f", c->rest_length);
        if (row == 1) swprintf(g_state.value_input_buf, 64, L"%.4f", c->stiffness);
        if (row == 2) swprintf(g_state.value_input_buf, 64, L"%.4f", c->damping);
        if (row == 3) swprintf(g_state.value_input_buf, 64, L"%.2f", c->break_force);
        if (row == 4) swprintf(g_state.value_input_buf, 64, L"%d", c->collide_connected ? 1 : 0);
        if (row == 5 && c->type == CONSTRAINT_SPRING) {
            int preset = spring_preset_for_constraint(c);
            if (preset < 0) preset = 1;
            swprintf(g_state.value_input_buf, 64, L"%d", preset);
        }
    } else if (g_state.selected != NULL) {
        RigidBody* b = g_state.selected;
        if (row == 0) swprintf(g_state.value_input_buf, 64, L"%.4f", b->mass);
        if (row == 1) swprintf(g_state.value_input_buf, 64, L"%.4f", b->position.x);
        if (row == 2) swprintf(g_state.value_input_buf, 64, L"%.4f", b->position.y);
        if (row == 3) swprintf(g_state.value_input_buf, 64, L"%.4f", b->velocity.x);
        if (row == 4) swprintf(g_state.value_input_buf, 64, L"%.4f", b->velocity.y);
        if (row == 5) swprintf(g_state.value_input_buf, 64, L"%.4f", b->angular_velocity);
        if (row == 6) swprintf(g_state.value_input_buf, 64, L"%.4f", b->damping);
        if (row == 7) swprintf(g_state.value_input_buf, 64, L"%.4f", b->shape ? b->shape->restitution : 0.0f);
    }
    g_state.value_input_len = (int)lstrlenW(g_state.value_input_buf);
    g_state.value_input_caret = g_state.value_input_len;
}

static void begin_value_input_for_hierarchy_filter(void) {
    lstrcpynW(g_state.value_input_buf, g_state.hierarchy_filter_buf, 64);
    g_state.value_input_target = 3;
    g_state.value_input_row = 0;
    g_state.show_value_input = 1;
    g_state.value_input_len = (int)lstrlenW(g_state.value_input_buf);
    g_state.value_input_caret = g_state.value_input_len;
}

static void begin_value_input_for_debug_param(int row) {
    SceneConfig* cfg = &g_state.scenes[g_state.scene_index];
    g_state.value_input_target = 4;
    g_state.value_input_row = row;
    g_state.show_value_input = 1;
    g_state.value_input_len = 0;
    g_state.value_input_caret = 0;
    g_state.value_input_buf[0] = L'\0';
    if (row == 0) swprintf(g_state.value_input_buf, 64, L"%.2f", cfg->gravity_y);
    if (row == 1) swprintf(g_state.value_input_buf, 64, L"%.4f", cfg->time_step);
    if (row == 2) swprintf(g_state.value_input_buf, 64, L"%d", cfg->iterations);
    g_state.value_input_len = (int)lstrlenW(g_state.value_input_buf);
    g_state.value_input_caret = g_state.value_input_len;
}

static void apply_value_input(void) {
    const Constraint* csel = selected_constraint_ref();
    double v = 0.0;
    int has_digit = 0;
    int i;
    for (i = 0; g_state.value_input_buf[i] != L'\0'; i++) {
        if (g_state.value_input_buf[i] >= L'0' && g_state.value_input_buf[i] <= L'9') {
            has_digit = 1;
            break;
        }
    }
    if (!has_digit) {
        push_console_log(L"[警告] 输入无效");
        return;
    }
    v = _wtof(g_state.value_input_buf);
    if (g_state.value_input_target == 2) {
        lstrcpynW(g_state.log_search_buf, g_state.value_input_buf, 48);
        g_state.log_search_len = (int)lstrlenW(g_state.log_search_buf);
        g_state.log_scroll_offset = 0;
        push_console_log(L"[日志] 搜索关键字: %s", g_state.log_search_len > 0 ? g_state.log_search_buf : L"(空)");
        return;
    }
    if (g_state.value_input_target == 3) {
        lstrcpynW(g_state.hierarchy_filter_buf, g_state.value_input_buf, 32);
        g_state.hierarchy_filter_len = (int)lstrlenW(g_state.hierarchy_filter_buf);
        push_console_log(L"[层级] 过滤关键字: %s", g_state.hierarchy_filter_len > 0 ? g_state.hierarchy_filter_buf : L"(空)");
        return;
    }
    if (g_state.value_input_target == 4) {
        SceneConfig* cfg = &g_state.scenes[g_state.scene_index];
        int row = g_state.value_input_row;
        history_push_snapshot();
        if (row == 0) {
            cfg->gravity_y = clamp((float)v, 0.0f, 30.0f);
        } else if (row == 1) {
            cfg->time_step = clamp((float)v, 0.001f, 0.05f);
        } else if (row == 2) {
            int iv = (int)v;
            if (iv < 1) iv = 1;
            if (iv > 64) iv = 64;
            cfg->iterations = iv;
        }
        sync_scene_runtime_params();
        push_console_log(L"[调试] 已输入模拟参数");
        return;
    }
    if (g_state.value_input_target != 1) return;
    history_push_snapshot();
    if (csel != NULL && csel->active) {
        int row = g_state.value_input_row;
        if (row == 0) physics_engine_constraint_set_rest_length(g_state.engine, g_state.selected_constraint_index, (float)v);
        if (row == 1) {
            float min_stiffness = (csel->type == CONSTRAINT_DISTANCE) ? 0.08f : 0.20f;
            physics_engine_constraint_set_stiffness(g_state.engine, g_state.selected_constraint_index, max_f(min_stiffness, (float)v));
        }
        if (row == 2) physics_engine_constraint_set_damping(g_state.engine, g_state.selected_constraint_index, (float)v);
        if (row == 3) physics_engine_constraint_set_break_force(g_state.engine, g_state.selected_constraint_index, (float)v);
        if (row == 4) physics_engine_constraint_set_collide_connected(g_state.engine, g_state.selected_constraint_index, (((int)v) != 0) ? 1 : 0);
        if (row == 5 && csel->type == CONSTRAINT_SPRING) {
            int preset = (int)v;
            if (preset < 0) preset = 0;
            if (preset > 2) preset = 2;
            apply_spring_preset_to_constraint_index(g_state.selected_constraint_index, preset);
        }
        push_console_log(L"[检查器] 已输入约束参数");
    } else if (g_state.selected != NULL) {
        RigidBody* b = g_state.selected;
        int row = g_state.value_input_row;
        if (row == 0) {
            b->mass = max_f(0.1f, (float)v);
            b->inv_mass = (b->mass > 0.0f) ? (1.0f / b->mass) : 0.0f;
            if (b->shape != NULL) {
                b->inertia = shape_get_moment_of_inertia(b->shape, b->mass);
                b->inv_inertia = (b->inertia > 0.0f) ? (1.0f / b->inertia) : 0.0f;
            }
        }
        if (row == 1) b->position.x = (float)v;
        if (row == 2) b->position.y = (float)v;
        if (row == 3) b->velocity.x = (float)v;
        if (row == 4) b->velocity.y = (float)v;
        if (row == 5) b->angular_velocity = (float)v;
        if (row == 6) b->damping = clamp((float)v, 0.0f, 1.0f);
        if (row == 7 && b->shape != NULL) b->shape->restitution = clamp((float)v, 0.0f, 1.0f);
        push_console_log(L"[检查器] 已输入物体参数");
    }
}

static const wchar_t* inspector_row_hint_text(void) {
    static wchar_t hint[128];
    int row = g_state.inspector_focused_row;
    hint[0] = L'\0';
    if (selected_constraint_is_active()) {
        const Constraint* c = selected_constraint_ref();
        if (row == 0) lstrcpyW(hint, L"目标长度 >= 0 (单位: m)");
        if (row == 1) {
            if (c != NULL && c->type == CONSTRAINT_DISTANCE) lstrcpyW(hint, L"距离刚度范围: 0.08 ~ 1.0");
            else lstrcpyW(hint, L"弹簧刚度 >= 0.20 (单位: N/m)");
        }
        if (row == 2) lstrcpyW(hint, L"阻尼 >= 0 (单位: Ns/m)");
        if (row == 3) lstrcpyW(hint, L"断裂阈值 >= 0 (单位: N)");
        if (row == 4) lstrcpyW(hint, L"连体碰撞: 0=禁止 1=允许");
        if (row == 5 && c != NULL && c->type == CONSTRAINT_SPRING) lstrcpyW(hint, L"弹性预设: 0=软 1=中 2=高");
    } else if (g_state.selected != NULL) {
        if (row == 0) lstrcpyW(hint, L"质量 > 0 (单位: kg)");
        if (row == 1 || row == 2) lstrcpyW(hint, L"位置 (单位: m)");
        if (row == 3 || row == 4) lstrcpyW(hint, L"速度 (单位: m/s)");
        if (row == 5) lstrcpyW(hint, L"角速度 (单位: rad/s)");
        if (row == 6) lstrcpyW(hint, L"阻尼范围: 0.0 ~ 1.0");
        if (row == 7) lstrcpyW(hint, L"弹性范围: 0.0 ~ 1.0");
    }
    if (hint[0] == L'\0') lstrcpyW(hint, L"");
    return hint;
}

static void clear_right_panel_ui_state(void) {
    int i;
    g_ins_row_count = 0;
    g_dbg_collision_row_count = 0;
    g_dbg_collision_filter_rect = rc(0, 0, 0, 0);
    g_inspector_viewport_rect = rc(0, 0, 0, 0);
    g_inspector_scroll_track_rect = rc(0, 0, 0, 0);
    g_inspector_scroll_thumb_rect = rc(0, 0, 0, 0);
    g_debug_viewport_rect = rc(0, 0, 0, 0);
    g_debug_scroll_track_rect = rc(0, 0, 0, 0);
    g_debug_scroll_thumb_rect = rc(0, 0, 0, 0);
    g_state.inspector_scroll_max = 0;
    g_state.inspector_scroll_offset = 0;
    g_state.debug_scroll_max = 0;
    g_state.debug_scroll_offset = 0;
    g_state.debug_focused_row = 0;
    g_state.keyboard_focus_area = 1;
    for (i = 0; i < 3; i++) {
        g_dbg_row_rect[i] = rc(0, 0, 0, 0);
        g_dbg_minus_rect[i] = rc(0, 0, 0, 0);
        g_dbg_plus_rect[i] = rc(0, 0, 0, 0);
    }
}

static D2D1_RECT_F render_right_inspector_section(D2D1_RECT_F right_rect) {
    wchar_t line[128];
    float px = right_rect.left + 10.0f;
    float pw = (right_rect.right - right_rect.left) - 20.0f;
    float section_gap = 10.0f;
    float panel_h = (right_rect.bottom - right_rect.top) - 20.0f;
    float inspector_h;
    float debug_h;
    D2D1_RECT_F inspector;
    D2D1_RECT_F debug_rect;
    float step;
    float lx0;
    float lx1;
    float vx0;
    float vx1;
    float inspector_content_h;
    float inspector_view_h;
    float inspector_offset_y;
    float inspector_content_right;

    if (panel_h < 120.0f) panel_h = 120.0f;
    inspector_h = (panel_h - section_gap) * 0.56f;
    debug_h = panel_h - section_gap - inspector_h;
    if (inspector_h < 140.0f) {
        inspector_h = 140.0f;
        debug_h = panel_h - section_gap - inspector_h;
    }
    if (debug_h < 120.0f) {
        debug_h = 120.0f;
        inspector_h = panel_h - section_gap - debug_h;
        if (inspector_h < 120.0f) inspector_h = 120.0f;
    }

    inspector = rc(px, right_rect.top + 10.0f, px + pw, right_rect.top + 10.0f + inspector_h);
    draw_card_round(inspector, 10.0f, rgba(0.16f, 0.18f, 0.22f, 1.0f), rgba(0.29f, 0.33f, 0.40f, 1.0f));
    draw_panel_header_band(inspector, 38.0f, 12.0f);
    draw_text_vcenter(L"属性", rc(inspector.left + 16.0f, inspector.top + 4.0f, inspector.right - 12.0f, inspector.top + 40.0f),
                      g_ui.fmt_ui, rgba(0.88f, 0.92f, 0.97f, 1.0f));
    step = 32.0f;
    lx0 = inspector.left + 12.0f;
    lx1 = inspector.left + 110.0f;
    vx0 = inspector.left + 116.0f;
    vx1 = inspector.right - 12.0f;
    g_inspector_viewport_rect = rc(inspector.left + 8.0f, inspector.top + 44.0f, inspector.right - 14.0f, inspector.bottom - 8.0f);
    inspector_content_right = g_inspector_viewport_rect.right - 2.0f;
    inspector_view_h = g_inspector_viewport_rect.bottom - g_inspector_viewport_rect.top;
    g_ins_row_count = inspector_row_count();
    if (g_state.inspector_focused_row >= g_ins_row_count) g_state.inspector_focused_row = g_ins_row_count - 1;
    if (g_state.inspector_focused_row < 0) g_state.inspector_focused_row = 0;
    inspector_content_h = (g_ins_row_count == 0) ? 28.0f : (g_ins_row_count * step + 44.0f);
    g_state.inspector_scroll_max = 0;
    if (inspector_content_h > inspector_view_h) {
        g_state.inspector_scroll_max = (int)(inspector_content_h - inspector_view_h + 0.5f);
    }
    if (g_state.inspector_scroll_offset < 0) g_state.inspector_scroll_offset = 0;
    if (g_state.inspector_scroll_offset > g_state.inspector_scroll_max) g_state.inspector_scroll_offset = g_state.inspector_scroll_max;
    inspector_offset_y = g_inspector_viewport_rect.top + 2.0f - (float)g_state.inspector_scroll_offset;
    ID2D1HwndRenderTarget_PushAxisAlignedClip(g_ui.target, &g_inspector_viewport_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    if (g_ins_row_count == 0) {
        draw_text(L"未选中对象（点击舞台或左侧列表）", rc(lx0, inspector_offset_y + 2.0f, vx1, inspector_offset_y + 26.0f),
                  g_ui.fmt_info, rgba(0.76f, 0.82f, 0.92f, 1.0f));
    } else {
        int ri;
        for (ri = 0; ri < g_ins_row_count && ri < INSPECTOR_MAX_ROWS; ri++) {
            const wchar_t* label = L"--";
            D2D1_RECT_F rr = rc(inspector.left + 10.0f, inspector_offset_y + ri * step, inspector_content_right, inspector_offset_y + ri * step + 26.0f);
            g_ins_row_rect[ri] = rr;
            g_ins_minus_rect[ri] = rc(0, 0, 0, 0);
            g_ins_plus_rect[ri] = rc(0, 0, 0, 0);
            draw_card_round(rr, 5.0f,
                            (ri == g_state.inspector_focused_row) ? rgba(0.24f, 0.30f, 0.40f, 1.0f) : rgba(0.19f, 0.22f, 0.28f, 1.0f),
                            rgba(0.31f, 0.37f, 0.46f, 1.0f));
            {
                const Constraint* c = selected_constraint_ref();
                if (c != NULL && c->active) {
                if (ri == 0) { label = L"目标长度"; swprintf(line, 128, L"%.2f", c->rest_length); }
                if (ri == 1) { label = L"刚度"; swprintf(line, 128, L"%.2f", c->stiffness); }
                if (ri == 2) { label = L"阻尼"; swprintf(line, 128, L"%.2f", c->damping); }
                if (ri == 3) { label = L"断裂阈值"; swprintf(line, 128, L"%.0f", c->break_force); }
                if (ri == 4) { label = L"连体碰撞"; swprintf(line, 128, L"%s", c->collide_connected ? L"允许" : L"禁止"); }
                if (ri == 5 && c->type == CONSTRAINT_SPRING) {
                    int preset = spring_preset_for_constraint(c);
                    label = L"弹性预设";
                    swprintf(line, 128, L"%s", (preset == 0) ? L"软" : ((preset == 1) ? L"中" : ((preset == 2) ? L"高" : L"自定义")));
                }
                } else if (g_state.selected != NULL) {
                RigidBody* b = g_state.selected;
                if (ri == 0) { label = L"质量"; swprintf(line, 128, L"%.2f", b->mass); }
                if (ri == 1) { label = L"位置X"; swprintf(line, 128, L"%.2f", b->position.x); }
                if (ri == 2) { label = L"位置Y"; swprintf(line, 128, L"%.2f", b->position.y); }
                if (ri == 3) { label = L"速度X"; swprintf(line, 128, L"%.2f", b->velocity.x); }
                if (ri == 4) { label = L"速度Y"; swprintf(line, 128, L"%.2f", b->velocity.y); }
                if (ri == 5) { label = L"角速度"; swprintf(line, 128, L"%.2f", b->angular_velocity); }
                if (ri == 6) { label = L"阻尼"; swprintf(line, 128, L"%.2f", b->damping); }
                if (ri == 7) { label = L"弹性"; swprintf(line, 128, L"%.2f", (b->shape ? b->shape->restitution : 0.0f)); }
                }
            }
            draw_text(label, rc(rr.left + 6.0f, rr.top + 1.0f, lx1, rr.bottom - 1.0f), g_ui.fmt_info, rgba(0.70f, 0.77f, 0.86f, 1.0f));
            draw_text(line, rc(vx0, rr.top + 1.0f, rr.right - 8.0f, rr.bottom - 1.0f), g_ui.fmt_info, rgba(0.89f, 0.93f, 0.97f, 1.0f));
        }
        {
            const wchar_t* ih = inspector_row_hint_text();
            draw_text(L"提示: Enter输入  双击快速输入  ↑↓切换",
                      rc(lx0, inspector_offset_y + g_ins_row_count * step + 8.0f, vx1, inspector_offset_y + g_ins_row_count * step + 28.0f),
                      g_ui.fmt_info, rgba(0.62f, 0.70f, 0.80f, 1.0f));
            draw_text(ih, rc(lx0, inspector_offset_y + g_ins_row_count * step + 34.0f, vx1, inspector_offset_y + g_ins_row_count * step + 56.0f),
                      g_ui.fmt_info, rgba(0.69f, 0.76f, 0.86f, 1.0f));
        }
    }
    ID2D1HwndRenderTarget_PopAxisAlignedClip(g_ui.target);
    if (g_state.inspector_scroll_max > 0) {
        float track_h;
        float thumb_h;
        float travel;
        float ratio;
        float thumb_top;
        g_inspector_scroll_track_rect = rc(inspector.right - 10.0f, g_inspector_viewport_rect.top, inspector.right - 6.0f, g_inspector_viewport_rect.bottom);
        track_h = g_inspector_scroll_track_rect.bottom - g_inspector_scroll_track_rect.top;
        thumb_h = (inspector_view_h / inspector_content_h) * track_h;
        if (thumb_h < 20.0f) thumb_h = 20.0f;
        if (thumb_h > track_h) thumb_h = track_h;
        travel = track_h - thumb_h;
        ratio = (g_state.inspector_scroll_max > 0) ? ((float)g_state.inspector_scroll_offset / (float)g_state.inspector_scroll_max) : 0.0f;
        thumb_top = g_inspector_scroll_track_rect.top + travel * ratio;
        g_inspector_scroll_thumb_rect = rc(g_inspector_scroll_track_rect.left, thumb_top, g_inspector_scroll_track_rect.right, thumb_top + thumb_h);
        draw_card_round(g_inspector_scroll_track_rect, 2.0f, rgba(0.13f, 0.15f, 0.19f, 1.0f), rgba(0.24f, 0.28f, 0.35f, 1.0f));
        draw_card_round(g_inspector_scroll_thumb_rect, 2.0f,
                        point_in_rect(g_state.mouse_screen, g_inspector_scroll_thumb_rect) ? rgba(0.40f, 0.52f, 0.69f, 1.0f)
                                                                                            : rgba(0.33f, 0.43f, 0.57f, 1.0f),
                        rgba(0.52f, 0.65f, 0.82f, 1.0f));
    } else {
        g_inspector_scroll_track_rect = rc(0, 0, 0, 0);
        g_inspector_scroll_thumb_rect = rc(0, 0, 0, 0);
    }

    debug_rect = rc(px, inspector.bottom + section_gap, px + pw, inspector.bottom + section_gap + debug_h);
    return debug_rect;
}

static void render_right_debug_section(D2D1_RECT_F debug_rect) {
    wchar_t line[128];
    float debug_content_h;
    float debug_view_h;
    float debug_offset_y;
    float debug_content_right;
    draw_card_round(debug_rect, 10.0f, rgba(0.16f, 0.18f, 0.22f, 1.0f), rgba(0.29f, 0.33f, 0.40f, 1.0f));
    draw_panel_header_band(debug_rect, 34.0f, 10.0f);
    draw_text_vcenter(L"物理调试", rc(debug_rect.left + 16.0f, debug_rect.top + 4.0f, debug_rect.right - 12.0f, debug_rect.top + 36.0f),
                      g_ui.fmt_ui, rgba(0.78f, 0.86f, 0.96f, 1.0f));
    g_debug_viewport_rect = rc(debug_rect.left + 8.0f, debug_rect.top + 40.0f, debug_rect.right - 14.0f, debug_rect.bottom - 8.0f);
    debug_content_right = g_debug_viewport_rect.right - 2.0f;
    debug_view_h = g_debug_viewport_rect.bottom - g_debug_viewport_rect.top;
    debug_content_h = 148.0f;
    g_state.debug_scroll_max = 0;
    if (debug_content_h > debug_view_h) {
        g_state.debug_scroll_max = (int)(debug_content_h - debug_view_h + 0.5f);
    }
    if (g_state.debug_scroll_offset < 0) g_state.debug_scroll_offset = 0;
    if (g_state.debug_scroll_offset > g_state.debug_scroll_max) g_state.debug_scroll_offset = g_state.debug_scroll_max;
    debug_offset_y = g_debug_viewport_rect.top - (float)g_state.debug_scroll_offset;
    ID2D1HwndRenderTarget_PushAxisAlignedClip(g_ui.target, &g_debug_viewport_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    draw_text(L"模拟参数", rc(debug_rect.left + 12.0f, debug_offset_y, debug_rect.left + 120.0f, debug_offset_y + 22.0f),
              g_ui.fmt_info, rgba(0.70f, 0.78f, 0.90f, 1.0f));
    {
        int di;
        const wchar_t* labels[3] = {L"重力Y", L"时间步长", L"迭代"};
        if (g_state.debug_focused_row < 0) g_state.debug_focused_row = 0;
        if (g_state.debug_focused_row > 2) g_state.debug_focused_row = 2;
        for (di = 0; di < 3; di++) {
            D2D1_RECT_F rr = rc(debug_rect.left + 10.0f, debug_offset_y + 32.0f + di * 32.0f, debug_content_right, debug_offset_y + 58.0f + di * 32.0f);
            g_dbg_row_rect[di] = rr;
            g_dbg_minus_rect[di] = rc(0, 0, 0, 0);
            g_dbg_plus_rect[di] = rc(0, 0, 0, 0);
            draw_card_round(rr, 5.0f,
                            (di == g_state.debug_focused_row) ? rgba(0.24f, 0.30f, 0.40f, 1.0f) : rgba(0.20f, 0.25f, 0.33f, 1.0f),
                            (di == g_state.debug_focused_row) ? rgba(0.41f, 0.50f, 0.64f, 1.0f) : rgba(0.33f, 0.40f, 0.52f, 1.0f));
            if (di == 0) swprintf(line, 128, L"%.2f", g_state.engine ? physics_engine_get_gravity(g_state.engine).y : 0.0f);
            if (di == 1) swprintf(line, 128, L"%.4f", g_state.engine ? physics_engine_get_time_step(g_state.engine) : 0.0f);
            if (di == 2) swprintf(line, 128, L"%d", g_state.engine ? physics_engine_get_iterations(g_state.engine) : 0);
            draw_text(labels[di], rc(rr.left + 6.0f, rr.top + 1.0f, rr.left + 110.0f, rr.bottom - 1.0f), g_ui.fmt_info, rgba(0.74f, 0.81f, 0.91f, 1.0f));
            draw_text(line, rc(rr.left + 116.0f, rr.top + 1.0f, rr.right - 8.0f, rr.bottom - 1.0f), g_ui.fmt_info, rgba(0.92f, 0.95f, 0.99f, 1.0f));
        }
    }
    ID2D1HwndRenderTarget_PopAxisAlignedClip(g_ui.target);
    if (g_state.debug_scroll_max > 0) {
        float track_h;
        float thumb_h;
        float travel;
        float ratio;
        float thumb_top;
        g_debug_scroll_track_rect = rc(debug_rect.right - 10.0f, g_debug_viewport_rect.top, debug_rect.right - 6.0f, g_debug_viewport_rect.bottom);
        track_h = g_debug_scroll_track_rect.bottom - g_debug_scroll_track_rect.top;
        thumb_h = (debug_view_h / debug_content_h) * track_h;
        if (thumb_h < 20.0f) thumb_h = 20.0f;
        if (thumb_h > track_h) thumb_h = track_h;
        travel = track_h - thumb_h;
        ratio = (g_state.debug_scroll_max > 0) ? ((float)g_state.debug_scroll_offset / (float)g_state.debug_scroll_max) : 0.0f;
        thumb_top = g_debug_scroll_track_rect.top + travel * ratio;
        g_debug_scroll_thumb_rect = rc(g_debug_scroll_track_rect.left, thumb_top, g_debug_scroll_track_rect.right, thumb_top + thumb_h);
        draw_card_round(g_debug_scroll_track_rect, 2.0f, rgba(0.13f, 0.15f, 0.19f, 1.0f), rgba(0.24f, 0.28f, 0.35f, 1.0f));
        draw_card_round(g_debug_scroll_thumb_rect, 2.0f,
                        point_in_rect(g_state.mouse_screen, g_debug_scroll_thumb_rect) ? rgba(0.40f, 0.52f, 0.69f, 1.0f)
                                                                                        : rgba(0.33f, 0.43f, 0.57f, 1.0f),
                        rgba(0.52f, 0.65f, 0.82f, 1.0f));
    } else {
        g_debug_scroll_track_rect = rc(0, 0, 0, 0);
        g_debug_scroll_thumb_rect = rc(0, 0, 0, 0);
    }
    g_dbg_collision_row_count = 0;
    g_dbg_collision_filter_rect = rc(0, 0, 0, 0);
}

static void render_side_fold_buttons(
    D2D1_RECT_F top_rect,
    float work_bottom,
    D2D1_RECT_F left_rect,
    D2D1_RECT_F center_rect,
    D2D1_RECT_F right_rect
) {
    float fold_top = ((top_rect.bottom + 8.0f) + work_bottom) * 0.5f - 13.0f;
    D2D1_COLOR_F fold_bg = rgba(0.19f, 0.22f, 0.29f, 1.0f);
    D2D1_COLOR_F fold_bd = rgba(0.33f, 0.39f, 0.49f, 1.0f);
    D2D1_COLOR_F fold_fg = rgba(0.86f, 0.91f, 0.97f, 1.0f);
    if (g_state.ui_show_left_panel) {
        g_left_fold_rect = rc(left_rect.right - 10.0f, fold_top, left_rect.right + 10.0f, fold_top + 26.0f);
        draw_card_round(g_left_fold_rect, 5.0f,
                        point_in_rect(g_state.mouse_screen, g_left_fold_rect) ? rgba(0.24f, 0.30f, 0.40f, 1.0f) : fold_bg,
                        fold_bd);
        ui_draw_icon_chevron_lr(g_ui.target, g_ui.brush, g_left_fold_rect, 1, fold_fg, 1.5f);
    } else {
        g_left_fold_rect = rc(center_rect.left + 2.0f, fold_top, center_rect.left + 22.0f, fold_top + 26.0f);
        draw_card_round(g_left_fold_rect, 5.0f,
                        point_in_rect(g_state.mouse_screen, g_left_fold_rect) ? rgba(0.24f, 0.30f, 0.40f, 1.0f) : fold_bg,
                        fold_bd);
        ui_draw_icon_chevron_lr(g_ui.target, g_ui.brush, g_left_fold_rect, 0, fold_fg, 1.5f);
    }
    if (g_state.ui_show_right_panel) {
        g_right_fold_rect = rc(right_rect.left - 10.0f, fold_top, right_rect.left + 10.0f, fold_top + 26.0f);
        draw_card_round(g_right_fold_rect, 5.0f,
                        point_in_rect(g_state.mouse_screen, g_right_fold_rect) ? rgba(0.24f, 0.30f, 0.40f, 1.0f) : fold_bg,
                        fold_bd);
        ui_draw_icon_chevron_lr(g_ui.target, g_ui.brush, g_right_fold_rect, 0, fold_fg, 1.5f);
    } else {
        g_right_fold_rect = rc(center_rect.right - 22.0f, fold_top, center_rect.right - 2.0f, fold_top + 26.0f);
        draw_card_round(g_right_fold_rect, 5.0f,
                        point_in_rect(g_state.mouse_screen, g_right_fold_rect) ? rgba(0.24f, 0.30f, 0.40f, 1.0f) : fold_bg,
                        fold_bd);
        ui_draw_icon_chevron_lr(g_ui.target, g_ui.brush, g_right_fold_rect, 1, fold_fg, 1.5f);
    }
}

static void render_bottom_panel_tabs(D2D1_RECT_F bottom_rect) {
    if (g_state.ui_show_bottom_panel) {
        D2D1_RECT_F active_tab_indicator;
        int tab_console_active;
        int tab_perf_active;
        int show_bottom_tabs = !g_state.bottom_panel_collapsed;
        g_bottom_fold_rect = rc(bottom_rect.right - 28.0f, bottom_rect.top + 6.0f, bottom_rect.right - 8.0f, bottom_rect.top + 24.0f);
        if (!show_bottom_tabs) {
            g_bottom_tab_console_rect = rc(0, 0, 0, 0);
            g_bottom_tab_perf_rect = rc(0, 0, 0, 0);
        } else {
            g_bottom_tab_console_rect = rc(bottom_rect.left + 14.0f, bottom_rect.top + 7.0f, bottom_rect.left + 94.0f, bottom_rect.top + 29.0f);
            g_bottom_tab_perf_rect = rc(bottom_rect.left + 96.0f, bottom_rect.top + 7.0f, bottom_rect.left + 176.0f, bottom_rect.top + 29.0f);
            tab_console_active = (g_state.bottom_active_tab == 0);
            tab_perf_active = (g_state.bottom_active_tab == 1);
            draw_text_hvcenter(L"控制台", g_bottom_tab_console_rect, g_ui.fmt_info,
                               tab_console_active ? rgba(0.94f, 0.97f, 1.0f, 1.0f) : rgba(0.70f, 0.78f, 0.90f, 1.0f));
            draw_text_hvcenter(L"性能", g_bottom_tab_perf_rect, g_ui.fmt_info,
                               tab_perf_active ? rgba(0.94f, 0.97f, 1.0f, 1.0f) : rgba(0.70f, 0.78f, 0.90f, 1.0f));
            active_tab_indicator = tab_console_active
                                       ? rc(g_bottom_tab_console_rect.left + 12.0f, g_bottom_tab_console_rect.bottom - 2.0f,
                                            g_bottom_tab_console_rect.right - 12.0f, g_bottom_tab_console_rect.bottom)
                                       : rc(g_bottom_tab_perf_rect.left + 12.0f, g_bottom_tab_perf_rect.bottom - 2.0f,
                                            g_bottom_tab_perf_rect.right - 12.0f, g_bottom_tab_perf_rect.bottom);
            draw_card_round(active_tab_indicator, 1.0f, rgba(0.56f, 0.73f, 0.95f, 1.0f), rgba(0.56f, 0.73f, 0.95f, 1.0f));
        }
        ui_draw_icon_chevron(g_ui.target, g_ui.brush, g_bottom_fold_rect, g_state.bottom_panel_collapsed ? 1 : 0, rgba(0.90f, 0.94f, 0.99f, 1.0f), 1.5f);
    } else {
        g_bottom_fold_rect = rc(0, 0, 0, 0);
        g_bottom_tab_console_rect = rc(0, 0, 0, 0);
        g_bottom_tab_perf_rect = rc(0, 0, 0, 0);
        g_log_viewport_rect = rc(0, 0, 0, 0);
        g_log_scroll_track_rect = rc(0, 0, 0, 0);
        g_log_scroll_thumb_rect = rc(0, 0, 0, 0);
        g_dbg_collision_filter_rect = rc(0, 0, 0, 0);
        g_dbg_collision_row_count = 0;
        g_log_scroll_max = 0;
    }
}

static void render_bottom_perf_tab(D2D1_RECT_F bottom_rect) {
    D2D1_RECT_F graph = rc(bottom_rect.left + 280.0f, bottom_rect.top + 36.0f, bottom_rect.right - 12.0f, bottom_rect.bottom - 12.0f);
    int gi;
    wchar_t line[128];
    g_log_viewport_rect = rc(0, 0, 0, 0);
    g_log_scroll_track_rect = rc(0, 0, 0, 0);
    g_log_scroll_thumb_rect = rc(0, 0, 0, 0);
    g_dbg_collision_filter_rect = rc(0, 0, 0, 0);
    g_dbg_collision_row_count = 0;
    g_log_scroll_max = 0;
    g_perf_export_rect = rc(bottom_rect.right - 130.0f, bottom_rect.top + 6.0f, bottom_rect.right - 36.0f, bottom_rect.top + 28.0f);
    draw_action_button(g_perf_export_rect, L"导出CSV", 0, point_in_rect(g_state.mouse_screen, g_perf_export_rect));
    draw_card_round(graph, 6.0f, rgba(0.14f, 0.18f, 0.24f, 1.0f), rgba(0.30f, 0.38f, 0.50f, 1.0f));
    swprintf(line, 128, L"FPS: %d", g_state.fps_display);
    draw_text(line, rc(bottom_rect.left + 12.0f, bottom_rect.top + 40.0f, bottom_rect.left + 180.0f, bottom_rect.top + 66.0f),
              g_ui.fmt_info, rgba(0.62f, 0.88f, 0.62f, 1.0f));
    swprintf(line, 128, L"物理耗时: %.2fms", g_state.physics_step_ms);
    draw_text(line, rc(bottom_rect.left + 12.0f, bottom_rect.top + 66.0f, bottom_rect.left + 260.0f, bottom_rect.top + 92.0f),
              g_ui.fmt_info, rgba(0.95f, 0.71f, 0.42f, 1.0f));
    swprintf(line, 128, L"粗检测: %s", g_state.engine && physics_engine_get_broadphase_use_grid(g_state.engine) ? L"网格" : L"暴力");
    draw_text(line, rc(bottom_rect.left + 12.0f, bottom_rect.top + 92.0f, bottom_rect.left + 260.0f, bottom_rect.top + 118.0f),
              g_ui.fmt_info, rgba(0.82f, 0.88f, 0.96f, 1.0f));
    if (g_state.perf_hist_count >= 2) {
        for (gi = 1; gi < g_state.perf_hist_count; gi++) {
            int idx0 = (g_state.perf_hist_head + gi - 1) % 180;
            int idx1 = (g_state.perf_hist_head + gi) % 180;
            float x0 = graph.left + (graph.right - graph.left) * ((float)(gi - 1) / (float)(g_state.perf_hist_count - 1));
            float x1 = graph.left + (graph.right - graph.left) * ((float)gi / (float)(g_state.perf_hist_count - 1));
            float y0_fps = graph.bottom - 4.0f - clamp(g_state.fps_hist[idx0], 0.0f, 120.0f) / 120.0f * (graph.bottom - graph.top - 8.0f);
            float y1_fps = graph.bottom - 4.0f - clamp(g_state.fps_hist[idx1], 0.0f, 120.0f) / 120.0f * (graph.bottom - graph.top - 8.0f);
            float y0_ms = graph.bottom - 4.0f - clamp(g_state.step_hist[idx0], 0.0f, 16.0f) / 16.0f * (graph.bottom - graph.top - 8.0f);
            float y1_ms = graph.bottom - 4.0f - clamp(g_state.step_hist[idx1], 0.0f, 16.0f) / 16.0f * (graph.bottom - graph.top - 8.0f);
            set_brush_color(0.44f, 0.90f, 0.44f, 0.95f);
            ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0, y0_fps), pt(x1, y1_fps), (ID2D1Brush*)g_ui.brush, 1.3f, NULL);
            set_brush_color(0.98f, 0.73f, 0.38f, 0.95f);
            ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0, y0_ms), pt(x1, y1_ms), (ID2D1Brush*)g_ui.brush, 1.3f, NULL);
        }
        draw_text(L"绿:FPS  橙:耗时", rc(graph.left + 6.0f, graph.top + 4.0f, graph.left + 140.0f, graph.top + 20.0f), g_ui.fmt_info,
                  rgba(0.76f, 0.85f, 0.95f, 1.0f));
    }
}

static void render_bottom_console_tab(D2D1_RECT_F bottom_rect) {
    int li;
    int shown = 0;
    int skip = g_state.log_scroll_offset;
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
    g_state.log_search_buf[0] = L'\0';
    g_state.log_search_len = 0;
    x0 = controls_right;
    g_log_search_clear_rect = rc(0, 0, 0, 0);
    g_log_search_rect = rc(0, 0, 0, 0);
    g_log_clear_rect = rc(x0 - 58.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);
    x0 = g_log_clear_rect.left - gap;
    g_log_filter_warn_rect = rc(x0 - 60.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);
    x0 = g_log_filter_warn_rect.left - gap;
    g_log_filter_collision_rect = rc(x0 - 60.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);
    x0 = g_log_filter_collision_rect.left - gap;
    g_log_filter_physics_rect = rc(x0 - 60.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);
    x0 = g_log_filter_physics_rect.left - gap;
    g_log_filter_state_rect = rc(x0 - 60.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);
    x0 = g_log_filter_state_rect.left - gap;
    g_log_filter_all_rect = rc(x0 - 46.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);
    draw_text_tab_button(g_log_filter_all_rect, L"全部", g_state.log_filter_mode == 0, point_in_rect(g_state.mouse_screen, g_log_filter_all_rect));
    draw_text_tab_button(g_log_filter_state_rect, L"状态", g_state.log_filter_mode == 1, point_in_rect(g_state.mouse_screen, g_log_filter_state_rect));
    draw_text_tab_button(g_log_filter_physics_rect, L"物理", g_state.log_filter_mode == 2, point_in_rect(g_state.mouse_screen, g_log_filter_physics_rect));
    draw_text_tab_button(g_log_filter_collision_rect, L"碰撞", g_state.log_filter_mode == 4, point_in_rect(g_state.mouse_screen, g_log_filter_collision_rect));
    draw_text_tab_button(g_log_filter_warn_rect, L"警告", g_state.log_filter_mode == 3, point_in_rect(g_state.mouse_screen, g_log_filter_warn_rect));
    draw_text_tab_button(g_log_clear_rect, L"清空", 0, point_in_rect(g_state.mouse_screen, g_log_clear_rect));
    viewport_top = bottom_rect.top + 38.0f;
    viewport_bottom = bottom_rect.bottom - 10.0f;
    if (g_state.log_filter_mode == 4) {
        g_dbg_collision_filter_rect = rc(bottom_rect.left + 12.0f, bottom_rect.bottom - 32.0f, bottom_rect.left + 136.0f, bottom_rect.bottom - 10.0f);
        draw_action_button(g_dbg_collision_filter_rect, L"仅选中相关", g_collision_event_filter_selected_only,
                           point_in_rect(g_state.mouse_screen, g_dbg_collision_filter_rect));
        viewport_bottom = g_dbg_collision_filter_rect.top - 6.0f;
    } else {
        g_dbg_collision_filter_rect = rc(0, 0, 0, 0);
    }
    viewport_h = viewport_bottom - viewport_top;
    if (viewport_h < line_h * 2.0f) viewport_h = line_h * 2.0f;
    g_log_viewport_rect = rc(bottom_rect.left + 10.0f, viewport_top, bottom_rect.right - 18.0f, viewport_top + viewport_h);
    max_lines = (int)(viewport_h / line_h);
    if (max_lines < 1) max_lines = 1;
    if (g_state.log_filter_mode == 4) {
        int i;
        g_dbg_collision_row_count = 0;
        for (i = g_collision_event_count - 1; i >= 0; i--) {
            int idx = (g_collision_event_head + i) % COLLISION_EVENT_CAP;
            const CollisionEvent* ev = &g_collision_events[idx];
            float tsec = (float)ev->tick_ms / 1000.0f;
            swprintf(line, 128, L"[碰撞] t=%.1fs #%d-#%d v=%.2f p=%.2f", tsec, ev->body_a_index + 1, ev->body_b_index + 1, ev->rel_speed, ev->penetration);
            if (g_collision_event_filter_selected_only && !collision_event_involves_selected(ev)) continue;
            if (g_state.log_search_len > 0 && wcsstr(line, g_state.log_search_buf) == NULL) continue;
            total_lines++;
        }
        g_log_scroll_max = (total_lines > max_lines) ? (total_lines - max_lines) : 0;
        if (g_state.log_scroll_offset < 0) g_state.log_scroll_offset = 0;
        if (g_state.log_scroll_offset > g_log_scroll_max) g_state.log_scroll_offset = g_log_scroll_max;
        skip = g_state.log_scroll_offset;
        ID2D1HwndRenderTarget_PushAxisAlignedClip(g_ui.target, &g_log_viewport_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        for (i = g_collision_event_count - 1; i >= 0 && shown < max_lines; i--) {
            int idx = (g_collision_event_head + i) % COLLISION_EVENT_CAP;
            const CollisionEvent* ev = &g_collision_events[idx];
            float tsec = (float)ev->tick_ms / 1000.0f;
            swprintf(line, 128, L"[碰撞] t=%.1fs #%d-#%d v=%.2f p=%.2f", tsec, ev->body_a_index + 1, ev->body_b_index + 1, ev->rel_speed, ev->penetration);
            if (g_collision_event_filter_selected_only && !collision_event_involves_selected(ev)) continue;
            if (g_state.log_search_len > 0 && wcsstr(line, g_state.log_search_buf) == NULL) continue;
            if (skip > 0) { skip--; continue; }
            if (shown < DEBUG_EVENT_ROWS_MAX) {
                g_dbg_collision_row_rect[shown] = rc(g_log_viewport_rect.left, g_log_viewport_rect.top + shown * line_h,
                                                     g_log_viewport_rect.right, g_log_viewport_rect.top + (shown + 1) * line_h);
                g_dbg_collision_row_event_index[shown] = idx;
                g_dbg_collision_row_count = shown + 1;
            }
            draw_text(line,
                      rc(g_log_viewport_rect.left + 2.0f, g_log_viewport_rect.top + shown * line_h,
                         g_log_viewport_rect.right - 2.0f, g_log_viewport_rect.top + (shown + 1) * line_h),
                      g_ui.fmt_info, shown == 0 ? rgba(0.90f, 0.95f, 1.0f, 1.0f) : rgba(0.76f, 0.85f, 0.95f, 1.0f));
            shown++;
        }
        if (shown == 0) {
            draw_text(g_collision_event_filter_selected_only ? L"无选中对象相关碰撞" : L"暂无碰撞记录",
                      rc(g_log_viewport_rect.left + 2.0f, g_log_viewport_rect.top + 4.0f, g_log_viewport_rect.right - 2.0f, g_log_viewport_rect.top + line_h + 4.0f),
                      g_ui.fmt_info, rgba(0.65f, 0.74f, 0.86f, 1.0f));
        }
        ID2D1HwndRenderTarget_PopAxisAlignedClip(g_ui.target);
    } else {
        g_dbg_collision_row_count = 0;
        for (li = g_console_log_count - 1; li >= 0; li--) {
            int idx = (g_console_log_head + li) % CONSOLE_LOG_CAP;
            if (!log_match_filter(g_console_logs[idx], g_state.log_filter_mode)) continue;
            total_lines++;
        }
        g_log_scroll_max = (total_lines > max_lines) ? (total_lines - max_lines) : 0;
        if (g_state.log_scroll_offset < 0) g_state.log_scroll_offset = 0;
        if (g_state.log_scroll_offset > g_log_scroll_max) g_state.log_scroll_offset = g_log_scroll_max;
        skip = g_state.log_scroll_offset;
        ID2D1HwndRenderTarget_PushAxisAlignedClip(g_ui.target, &g_log_viewport_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        for (li = g_console_log_count - 1; li >= 0 && shown < max_lines; li--) {
            int idx = (g_console_log_head + li) % CONSOLE_LOG_CAP;
            if (!log_match_filter(g_console_logs[idx], g_state.log_filter_mode)) continue;
            if (skip > 0) { skip--; continue; }
            draw_text(g_console_logs[idx],
                      rc(g_log_viewport_rect.left + 2.0f, g_log_viewport_rect.top + shown * line_h,
                         g_log_viewport_rect.right - 2.0f, g_log_viewport_rect.top + (shown + 1) * line_h),
                      g_ui.fmt_info, shown == 0 ? rgba(0.90f, 0.95f, 1.0f, 1.0f) : rgba(0.76f, 0.85f, 0.95f, 1.0f));
            shown++;
        }
        ID2D1HwndRenderTarget_PopAxisAlignedClip(g_ui.target);
    }
    if (g_log_scroll_max > 0) {
        float track_h;
        float thumb_h;
        float travel;
        float ratio;
        float thumb_top;
        g_log_scroll_track_rect = rc(g_log_viewport_rect.right + 2.0f, g_log_viewport_rect.top, g_log_viewport_rect.right + 6.0f, g_log_viewport_rect.bottom);
        track_h = g_log_scroll_track_rect.bottom - g_log_scroll_track_rect.top;
        thumb_h = ((float)max_lines / (float)total_lines) * track_h;
        if (thumb_h < 20.0f) thumb_h = 20.0f;
        if (thumb_h > track_h) thumb_h = track_h;
        travel = track_h - thumb_h;
        ratio = (g_log_scroll_max > 0) ? ((float)g_state.log_scroll_offset / (float)g_log_scroll_max) : 0.0f;
        thumb_top = g_log_scroll_track_rect.top + travel * ratio;
        g_log_scroll_thumb_rect = rc(g_log_scroll_track_rect.left, thumb_top, g_log_scroll_track_rect.right, thumb_top + thumb_h);
        draw_card_round(g_log_scroll_track_rect, 2.0f, rgba(0.13f, 0.15f, 0.19f, 1.0f), rgba(0.24f, 0.28f, 0.35f, 1.0f));
        draw_card_round(g_log_scroll_thumb_rect, 2.0f,
                        point_in_rect(g_state.mouse_screen, g_log_scroll_thumb_rect) ? rgba(0.40f, 0.52f, 0.69f, 1.0f)
                                                                                      : rgba(0.33f, 0.43f, 0.57f, 1.0f),
                        rgba(0.52f, 0.65f, 0.82f, 1.0f));
    } else {
        g_log_scroll_track_rect = rc(0, 0, 0, 0);
        g_log_scroll_thumb_rect = rc(0, 0, 0, 0);
    }
    if (g_state.log_scroll_offset > 0) {
        draw_text(L"滚动查看更多日志", rc(bottom_rect.right - 170.0f, bottom_rect.top + 8.0f, bottom_rect.right - 34.0f, bottom_rect.top + 26.0f),
                  g_ui.fmt_info, rgba(0.78f, 0.86f, 0.96f, 1.0f));
    }
}

static void render_bottom_panel_content(D2D1_RECT_F bottom_rect) {
    if (g_state.ui_show_bottom_panel && !g_state.bottom_panel_collapsed) {
        if (g_state.bottom_active_tab == 0) {
            render_bottom_console_tab(bottom_rect);
        } else {
            render_bottom_perf_tab(bottom_rect);
        }
    }
}

static void clear_left_panel_ui_state(void) {
    g_explorer_body_count = 0;
    g_explorer_constraint_count = 0;
    g_explorer_scene_rect = rc(0, 0, 0, 0);
    g_tree_scene_header_rect = rc(0, 0, 0, 0);
    g_tree_bodies_header_rect = rc(0, 0, 0, 0);
    g_tree_constraints_header_rect = rc(0, 0, 0, 0);
    g_hierarchy_search_rect = rc(0, 0, 0, 0);
    g_hierarchy_search_clear_rect = rc(0, 0, 0, 0);
    g_hierarchy_viewport_rect = rc(0, 0, 0, 0);
    g_hierarchy_scroll_track_rect = rc(0, 0, 0, 0);
    g_hierarchy_scroll_thumb_rect = rc(0, 0, 0, 0);
    g_project_viewport_rect = rc(0, 0, 0, 0);
    g_tree_project_root_rect = rc(0, 0, 0, 0);
    g_project_scroll_track_rect = rc(0, 0, 0, 0);
    g_project_scroll_thumb_rect = rc(0, 0, 0, 0);
    g_state.hierarchy_scroll_max = 0;
    g_state.hierarchy_scroll_offset = 0;
    g_state.project_scroll_max = 0;
    g_state.project_scroll_offset = 0;
    g_state.project_tree_last_scan_ms = 0;
    project_tree_clear();
}

typedef struct {
    int body_visible_count;
    int constraint_visible_count;
    int body_circle_visible_count;
    int body_polygon_visible_count;
    int constraint_distance_visible_count;
    int constraint_spring_visible_count;
    int body_group_rows;
    int constraint_group_rows;
} HierarchyStats;

static void init_left_panel_headers_and_search(D2D1_RECT_F hierarchy_rect, D2D1_RECT_F project_rect, D2D1_RECT_F hierarchy_viewport) {
    g_explorer_body_count = 0;
    g_explorer_constraint_count = 0;
    g_hierarchy_viewport_rect = hierarchy_viewport;
    draw_card_round(hierarchy_rect, 8.0f, rgba(0.16f, 0.18f, 0.22f, 1.0f), rgba(0.29f, 0.33f, 0.40f, 1.0f));
    draw_card_round(project_rect, 8.0f, rgba(0.16f, 0.18f, 0.22f, 1.0f), rgba(0.29f, 0.33f, 0.40f, 1.0f));
    draw_panel_header_band(hierarchy_rect, 34.0f, 10.0f);
    draw_panel_header_band(project_rect, 34.0f, 10.0f);
    draw_text_vcenter(L"层级", rc(hierarchy_rect.left + 16.0f, hierarchy_rect.top + 4.0f, hierarchy_rect.right - 12.0f, hierarchy_rect.top + 36.0f),
                      g_ui.fmt_ui, rgba(0.76f, 0.82f, 0.90f, 1.0f));
    draw_text_vcenter(L"项目", rc(project_rect.left + 16.0f, project_rect.top + 4.0f, project_rect.right - 12.0f, project_rect.top + 36.0f),
                      g_ui.fmt_ui, rgba(0.76f, 0.82f, 0.90f, 1.0f));
    g_tree_scene_header_rect = rc(0, 0, 0, 0);
    g_tree_bodies_header_rect = rc(0, 0, 0, 0);
    g_tree_constraints_header_rect = rc(0, 0, 0, 0);
    g_tree_body_circle_header_rect = rc(0, 0, 0, 0);
    g_tree_body_polygon_header_rect = rc(0, 0, 0, 0);
    g_tree_constraint_distance_header_rect = rc(0, 0, 0, 0);
    g_tree_constraint_spring_header_rect = rc(0, 0, 0, 0);
}

static float compute_hierarchy_content_height(float row_h, const HierarchyStats* hs) {
    float content_h = 0.0f;
    content_h += row_h;
    if (g_state.tree_scene_expanded) content_h += row_h + 4.0f;
    content_h += row_h;
    if (g_state.tree_bodies_expanded) {
        content_h += hs->body_group_rows * row_h;
        if (g_state.tree_body_circle_expanded) content_h += hs->body_circle_visible_count * (row_h + 4.0f);
        if (g_state.tree_body_polygon_expanded) content_h += hs->body_polygon_visible_count * (row_h + 4.0f);
    }
    content_h += row_h;
    if (g_state.tree_constraints_expanded) {
        content_h += hs->constraint_group_rows * row_h;
        if (g_state.tree_constraint_distance_expanded) content_h += hs->constraint_distance_visible_count * (row_h + 4.0f);
        if (g_state.tree_constraint_spring_expanded) content_h += hs->constraint_spring_visible_count * (row_h + 4.0f);
    }
    content_h += 6.0f;
    return content_h;
}

static void render_project_tree_panel(D2D1_RECT_F project_rect, float row_h) {
    int pi;
    float py;
    float project_content_h;
    float project_view_h;
    wchar_t line[128];
    D2D1_RECT_F project_viewport = rc(project_rect.left + 10.0f, project_rect.top + 44.0f, project_rect.right - 18.0f, project_rect.bottom - 34.0f);
    unsigned int now_ms = (unsigned int)GetTickCount();
    if (project_tree_count() <= 0 || (now_ms - g_state.project_tree_last_scan_ms) > 1500) {
        project_tree_build(L".", 3);
        g_state.project_tree_last_scan_ms = now_ms;
    }
    project_content_h = row_h + (g_state.tree_project_expanded ? (project_tree_count() * 24.0f) : 0.0f) + 6.0f;
    g_project_viewport_rect = project_viewport;
    project_view_h = project_viewport.bottom - project_viewport.top;
    g_state.project_scroll_max = 0;
    if (project_content_h > project_view_h) {
        g_state.project_scroll_max = (int)(project_content_h - project_view_h + 0.5f);
    }
    if (g_state.project_scroll_offset < 0) g_state.project_scroll_offset = 0;
    if (g_state.project_scroll_offset > g_state.project_scroll_max) g_state.project_scroll_offset = g_state.project_scroll_max;
    py = project_viewport.top - (float)g_state.project_scroll_offset;
    ID2D1HwndRenderTarget_PushAxisAlignedClip(g_ui.target, &project_viewport, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    g_tree_project_root_rect = rc(project_rect.left + 10.0f, py, project_rect.right - 18.0f, py + row_h);
    ui_draw_tree_disclosure_icon(g_ui.target, g_ui.brush, rc(g_tree_project_root_rect.left + 2.0f, g_tree_project_root_rect.top + 3.0f,
                                 g_tree_project_root_rect.left + 14.0f, g_tree_project_root_rect.bottom - 3.0f),
                              g_state.tree_project_expanded, rgba(0.82f, 0.87f, 0.94f, 1.0f), 1.4f);
    draw_text(L"项目根目录", rc(g_tree_project_root_rect.left + 16.0f, g_tree_project_root_rect.top,
                               g_tree_project_root_rect.right, g_tree_project_root_rect.bottom),
              g_ui.fmt_mono, rgba(0.82f, 0.87f, 0.94f, 1.0f));
    if (g_state.tree_project_expanded) {
        for (pi = 0; pi < project_tree_count(); pi++) {
            D2D1_RECT_F pr = rc(project_rect.left + 24.0f, py + row_h + pi * 24.0f, project_rect.right - 18.0f, py + row_h + pi * 24.0f + 20.0f);
            draw_text(project_tree_line_at(pi), pr, g_ui.fmt_info, rgba(0.86f, 0.90f, 0.96f, 1.0f));
        }
    }
    ID2D1HwndRenderTarget_PopAxisAlignedClip(g_ui.target);
    if (g_state.project_scroll_max > 0) {
        float track_h;
        float thumb_h;
        float travel;
        float ratio;
        float thumb_top;
        g_project_scroll_track_rect = rc(project_rect.right - 12.0f, project_viewport.top, project_rect.right - 8.0f, project_viewport.bottom);
        track_h = g_project_scroll_track_rect.bottom - g_project_scroll_track_rect.top;
        thumb_h = (project_view_h / project_content_h) * track_h;
        if (thumb_h < 20.0f) thumb_h = 20.0f;
        if (thumb_h > track_h) thumb_h = track_h;
        travel = track_h - thumb_h;
        ratio = (g_state.project_scroll_max > 0) ? ((float)g_state.project_scroll_offset / (float)g_state.project_scroll_max) : 0.0f;
        thumb_top = g_project_scroll_track_rect.top + travel * ratio;
        g_project_scroll_thumb_rect = rc(g_project_scroll_track_rect.left, thumb_top, g_project_scroll_track_rect.right, thumb_top + thumb_h);
        draw_card_round(g_project_scroll_track_rect, 2.0f, rgba(0.13f, 0.15f, 0.19f, 1.0f), rgba(0.24f, 0.28f, 0.35f, 1.0f));
        draw_card_round(g_project_scroll_thumb_rect, 2.0f,
                        point_in_rect(g_state.mouse_screen, g_project_scroll_thumb_rect) ? rgba(0.40f, 0.52f, 0.69f, 1.0f)
                                                                                          : rgba(0.33f, 0.43f, 0.57f, 1.0f),
                        rgba(0.52f, 0.65f, 0.82f, 1.0f));
    } else {
        g_project_scroll_track_rect = rc(0, 0, 0, 0);
        g_project_scroll_thumb_rect = rc(0, 0, 0, 0);
    }
    swprintf(line, 128, L"目录: %d  文件: %d", project_tree_dir_count(), project_tree_file_count());
    draw_text(line, rc(project_rect.left + 12.0f, project_rect.bottom - 28.0f, project_rect.right - 12.0f, project_rect.bottom - 8.0f),
              g_ui.fmt_info, rgba(0.66f, 0.73f, 0.83f, 1.0f));
}

static HierarchyStats compute_hierarchy_stats(void) {
    HierarchyStats s;
    int i;
    wchar_t line[128];
    s.body_visible_count = 0;
    s.constraint_visible_count = 0;
    s.body_circle_visible_count = 0;
    s.body_polygon_visible_count = 0;
    s.constraint_distance_visible_count = 0;
    s.constraint_spring_visible_count = 0;
    s.body_group_rows = 0;
    s.constraint_group_rows = 0;
    if (g_state.engine != NULL) {
        for (i = 0; i < physics_engine_get_body_count(g_state.engine); i++) {
            RigidBody* b = physics_engine_get_body(g_state.engine, i);
            if (b == NULL || b->type != BODY_DYNAMIC) continue;
            swprintf(line, 128, L"#%d %s/%s", s.body_visible_count + 1, body_kind_name(b), body_shape_name(b));
            if (g_state.hierarchy_filter_len > 0 && wcsstr(line, g_state.hierarchy_filter_buf) == NULL) continue;
            s.body_visible_count++;
            if (b->shape != NULL && b->shape->type == SHAPE_CIRCLE) s.body_circle_visible_count++;
            else s.body_polygon_visible_count++;
        }
        for (i = 0; i < physics_engine_get_constraint_count(g_state.engine); i++) {
            const Constraint* c = physics_engine_get_constraint(g_state.engine, i);
            const wchar_t* ctype;
            if (c == NULL || !c->active) continue;
            ctype = (c->type == CONSTRAINT_DISTANCE) ? L"距离" : L"弹簧";
            swprintf(line, 128, L"#%d %s", s.constraint_visible_count + 1, ctype);
            if (g_state.hierarchy_filter_len > 0 && wcsstr(line, g_state.hierarchy_filter_buf) == NULL) continue;
            s.constraint_visible_count++;
            if (c->type == CONSTRAINT_DISTANCE) s.constraint_distance_visible_count++;
            else s.constraint_spring_visible_count++;
        }
    }
    s.body_group_rows = (s.body_circle_visible_count > 0 ? 1 : 0) + (s.body_polygon_visible_count > 0 ? 1 : 0);
    s.constraint_group_rows = (s.constraint_distance_visible_count > 0 ? 1 : 0) + (s.constraint_spring_visible_count > 0 ? 1 : 0);
    return s;
}

static void render_menu_bar_and_window_controls(D2D1_RECT_F menu_rect, int is_zoomed) {
    static const wchar_t* menu_items[] = {L"文件", L"编辑", L"查看", L"调试", L"窗口", L"帮助"};
    int mi;
    float menu_item_gap = 4.0f;
    float menu_item_w = 56.0f;
    float icon_sz = 22.0f;
    float icon_left_margin = 10.0f;
    float icon_menu_gap = 21.0f;
    float icon_top = menu_rect.top + ((menu_rect.bottom - menu_rect.top) - icon_sz) * 0.5f;
    float mx;
    float btn_w = 34.0f;
    float btn_h = (menu_rect.bottom - menu_rect.top) - 8.0f;
    float btn_y = menu_rect.top + 4.0f;
    float btn_right = menu_rect.right - 10.0f;
    g_menu_app_icon_rect = rc(menu_rect.left + icon_left_margin, icon_top,
                              menu_rect.left + icon_left_margin + icon_sz, icon_top + icon_sz);
    mx = g_menu_app_icon_rect.right + icon_menu_gap;
    g_menu_file_rect = rc(0, 0, 0, 0);
    g_menu_edit_rect = rc(0, 0, 0, 0);
    g_menu_view_rect = rc(0, 0, 0, 0);
    g_menu_gameobject_rect = rc(0, 0, 0, 0);
    g_menu_component_rect = rc(0, 0, 0, 0);
    g_menu_physics_rect = rc(0, 0, 0, 0);
    g_menu_window_rect = rc(0, 0, 0, 0);
    g_menu_help_word_rect = rc(0, 0, 0, 0);
    g_win_close_rect = rc(btn_right - btn_w, btn_y, btn_right, btn_y + btn_h);
    g_win_max_rect = rc(g_win_close_rect.left - btn_w - 4.0f, btn_y, g_win_close_rect.left - 4.0f, btn_y + btn_h);
    g_win_min_rect = rc(g_win_max_rect.left - btn_w - 4.0f, btn_y, g_win_max_rect.left - 4.0f, btn_y + btn_h);
    g_menu_bar_drag_rect = rc(g_menu_help_word_rect.right + 8.0f, menu_rect.top + 2.0f, g_win_min_rect.left - 8.0f, menu_rect.bottom - 2.0f);
    for (mi = 0; mi < 6; mi++) {
        float mw = menu_item_w;
        D2D1_RECT_F mr = rc(mx, menu_rect.top + 4.0f, mx + mw, menu_rect.bottom - 4.0f);
        D2D1_COLOR_F mc = g_state.ui_theme_light ? rgba(0.20f, 0.27f, 0.38f, 1.0f) : rgba(0.82f, 0.86f, 0.92f, 1.0f);
        if (mi == 0) g_menu_file_rect = mr;
        if (mi == 1) g_menu_edit_rect = mr;
        if (mi == 2) g_menu_view_rect = mr;
        if (mi == 3) g_menu_physics_rect = mr;
        if (mi == 4) g_menu_window_rect = mr;
        if (mi == 5) {
            g_menu_help_word_rect = mr;
            if (point_in_rect(g_state.mouse_screen, mr)) {
                mc = rgba(0.66f, 0.74f, 0.86f, 1.0f);
            }
        }
        if (mi != 5 && point_in_rect(g_state.mouse_screen, mr)) {
            mc = rgba(0.66f, 0.74f, 0.86f, 1.0f);
        }
        draw_text(menu_items[mi], mr, g_ui.fmt_info, mc);
        mx += mw + menu_item_gap;
    }
    if (g_ui.app_icon_bitmap != NULL) {
        ID2D1HwndRenderTarget_DrawBitmap(g_ui.target, g_ui.app_icon_bitmap, &g_menu_app_icon_rect, 1.0f,
                                         D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, NULL);
    }
    g_menu_bar_drag_rect = rc(mx + 8.0f, menu_rect.top + 2.0f, g_win_min_rect.left - 8.0f, menu_rect.bottom - 2.0f);
    if (g_menu_bar_drag_rect.right < g_menu_bar_drag_rect.left) {
        g_menu_bar_drag_rect = rc(0, 0, 0, 0);
    }
    if (point_in_rect(g_state.mouse_screen, g_win_min_rect)) {
        draw_card_round(g_win_min_rect, 0.0f, rgba(0.22f, 0.27f, 0.36f, 1.0f), rgba(0.22f, 0.27f, 0.36f, 1.0f));
    }
    if (point_in_rect(g_state.mouse_screen, g_win_max_rect)) {
        draw_card_round(g_win_max_rect, 0.0f, rgba(0.22f, 0.27f, 0.36f, 1.0f), rgba(0.22f, 0.27f, 0.36f, 1.0f));
    }
    if (point_in_rect(g_state.mouse_screen, g_win_close_rect)) {
        draw_card_round(g_win_close_rect, 0.0f, rgba(0.79f, 0.23f, 0.28f, 1.0f), rgba(0.79f, 0.23f, 0.28f, 1.0f));
    }
    set_brush_color(0.90f, 0.94f, 0.99f, 1.0f);
    ID2D1HwndRenderTarget_DrawLine(g_ui.target,
                                   pt((g_win_min_rect.left + g_win_min_rect.right) * 0.5f - 5.0f, g_win_min_rect.bottom - 10.0f),
                                   pt((g_win_min_rect.left + g_win_min_rect.right) * 0.5f + 5.0f, g_win_min_rect.bottom - 10.0f),
                                   (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
    {
        D2D1_RECT_F mr = rc((g_win_max_rect.left + g_win_max_rect.right) * 0.5f - 4.8f, g_win_max_rect.top + 8.0f,
                            (g_win_max_rect.left + g_win_max_rect.right) * 0.5f + 4.8f, g_win_max_rect.top + 17.0f);
        ID2D1HwndRenderTarget_DrawRectangle(g_ui.target, &mr, (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        if (is_zoomed) {
            D2D1_RECT_F mr2 = rc(mr.left + 2.0f, mr.top - 2.0f, mr.right + 2.0f, mr.bottom - 2.0f);
            ID2D1HwndRenderTarget_DrawRectangle(g_ui.target, &mr2, (ID2D1Brush*)g_ui.brush, 1.0f, NULL);
        }
    }
    ID2D1HwndRenderTarget_DrawLine(g_ui.target,
                                   pt((g_win_close_rect.left + g_win_close_rect.right) * 0.5f - 4.5f, g_win_close_rect.top + 8.0f),
                                   pt((g_win_close_rect.left + g_win_close_rect.right) * 0.5f + 4.5f, g_win_close_rect.bottom - 8.0f),
                                   (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
    ID2D1HwndRenderTarget_DrawLine(g_ui.target,
                                   pt((g_win_close_rect.left + g_win_close_rect.right) * 0.5f + 4.5f, g_win_close_rect.top + 8.0f),
                                   pt((g_win_close_rect.left + g_win_close_rect.right) * 0.5f - 4.5f, g_win_close_rect.bottom - 8.0f),
                                   (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
}

static D2D1_RECT_F menu_anchor_rect(int menu_id) {
    if (menu_id == 1) return g_menu_file_rect;
    if (menu_id == 2) return g_menu_edit_rect;
    if (menu_id == 3) return g_menu_view_rect;
    if (menu_id == 4) return g_menu_component_rect;
    if (menu_id == 5) return g_menu_physics_rect;
    if (menu_id == 6) return g_menu_window_rect;
    if (menu_id == 7) return g_menu_help_word_rect;
    return g_menu_file_rect;
}

static void layout_top_toolbar_buttons(D2D1_RECT_F toolbar_rect) {
    float btn_w = 50.0f;
    float btn_y0 = toolbar_rect.top + 6.0f;
    float btn_y1 = toolbar_rect.bottom - 6.0f;
    float x = toolbar_rect.left + 12.0f + measure_text_width(L"中", g_ui.fmt_ui);
    g_top_run_rect = rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    g_top_step_rect = rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    g_top_reset_rect = rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 8.0f;
    g_top_save_rect = rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    g_top_undo_rect = rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    g_top_redo_rect = rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    g_top_grid_rect = rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    g_top_collision_rect = rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    g_top_velocity_rect = rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    g_top_constraint_rect = rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    g_top_spring_rect = rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    g_top_chain_rect = rc(x, btn_y0, x + btn_w, btn_y1); x += btn_w + 4.0f;
    g_top_rope_rect = rc(x, btn_y0, x + btn_w, btn_y1);
}

static void render_top_toolbar_buttons(void) {
    draw_toolbar_icon_button(g_top_run_rect, g_state.running ? TB_ICON_PAUSE : TB_ICON_RUN, g_state.running, point_in_rect(g_state.mouse_screen, g_top_run_rect));
    draw_toolbar_icon_button(g_top_step_rect, TB_ICON_STEP, 0, point_in_rect(g_state.mouse_screen, g_top_step_rect));
    draw_toolbar_icon_button(g_top_reset_rect, TB_ICON_RESET, 0, point_in_rect(g_state.mouse_screen, g_top_reset_rect));
    draw_toolbar_icon_button(g_top_save_rect, TB_ICON_SAVE, 0, point_in_rect(g_state.mouse_screen, g_top_save_rect));
    draw_toolbar_icon_button(g_top_undo_rect, TB_ICON_UNDO, 0, point_in_rect(g_state.mouse_screen, g_top_undo_rect));
    draw_toolbar_icon_button(g_top_redo_rect, TB_ICON_REDO, 0, point_in_rect(g_state.mouse_screen, g_top_redo_rect));
    draw_toolbar_icon_button(g_top_grid_rect, TB_ICON_GRID, g_state.draw_centers, point_in_rect(g_state.mouse_screen, g_top_grid_rect));
    draw_toolbar_icon_button(g_top_collision_rect, TB_ICON_COLLISION, g_state.draw_contacts, point_in_rect(g_state.mouse_screen, g_top_collision_rect));
    draw_toolbar_icon_button(g_top_velocity_rect, TB_ICON_VELOCITY, g_state.draw_velocity, point_in_rect(g_state.mouse_screen, g_top_velocity_rect));
    draw_toolbar_icon_button(g_top_constraint_rect, TB_ICON_CONSTRAINT, g_state.constraint_create_mode == 1,
                             point_in_rect(g_state.mouse_screen, g_top_constraint_rect));
    draw_toolbar_icon_button(g_top_spring_rect, TB_ICON_SPRING, g_state.constraint_create_mode == 2,
                             point_in_rect(g_state.mouse_screen, g_top_spring_rect));
    draw_toolbar_icon_button(g_top_chain_rect, TB_ICON_CHAIN, g_state.constraint_create_mode == 3,
                             point_in_rect(g_state.mouse_screen, g_top_chain_rect));
    draw_toolbar_icon_button(g_top_rope_rect, TB_ICON_ROPE, g_state.constraint_create_mode == 4,
                             point_in_rect(g_state.mouse_screen, g_top_rope_rect));
}

static void render_hierarchy_scrollbar(D2D1_RECT_F hierarchy_rect, D2D1_RECT_F hierarchy_viewport, float viewport_h, float content_h) {
    if (g_state.hierarchy_scroll_max > 0) {
        float track_h;
        float thumb_h;
        float travel;
        float ratio;
        float thumb_top;
        g_hierarchy_scroll_track_rect = rc(hierarchy_rect.right - 12.0f, hierarchy_viewport.top, hierarchy_rect.right - 8.0f, hierarchy_viewport.bottom);
        track_h = g_hierarchy_scroll_track_rect.bottom - g_hierarchy_scroll_track_rect.top;
        thumb_h = (viewport_h / content_h) * track_h;
        if (thumb_h < 24.0f) thumb_h = 24.0f;
        if (thumb_h > track_h) thumb_h = track_h;
        travel = track_h - thumb_h;
        ratio = (g_state.hierarchy_scroll_max > 0) ? ((float)g_state.hierarchy_scroll_offset / (float)g_state.hierarchy_scroll_max) : 0.0f;
        thumb_top = g_hierarchy_scroll_track_rect.top + travel * ratio;
        g_hierarchy_scroll_thumb_rect = rc(g_hierarchy_scroll_track_rect.left, thumb_top, g_hierarchy_scroll_track_rect.right, thumb_top + thumb_h);
        draw_card_round(g_hierarchy_scroll_track_rect, 2.0f, rgba(0.13f, 0.15f, 0.19f, 1.0f), rgba(0.24f, 0.28f, 0.35f, 1.0f));
        draw_card_round(g_hierarchy_scroll_thumb_rect, 2.0f,
                        g_state.hierarchy_scroll_dragging ? rgba(0.45f, 0.60f, 0.80f, 1.0f)
                                                         : (point_in_rect(g_state.mouse_screen, g_hierarchy_scroll_thumb_rect) ? rgba(0.40f, 0.52f, 0.69f, 1.0f)
                                                                                                                      : rgba(0.33f, 0.43f, 0.57f, 1.0f)),
                        rgba(0.52f, 0.65f, 0.82f, 1.0f));
    } else {
        g_hierarchy_scroll_track_rect = rc(0, 0, 0, 0);
        g_hierarchy_scroll_thumb_rect = rc(0, 0, 0, 0);
    }
}

static float render_hierarchy_scene_section(D2D1_RECT_F hierarchy_rect, float y, float row_h) {
    float content_right = g_hierarchy_viewport_rect.right - 2.0f;
    g_tree_scene_header_rect = rc(hierarchy_rect.left + 10.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
    ui_draw_tree_disclosure_icon(g_ui.target, g_ui.brush, rc(g_tree_scene_header_rect.left + 2.0f, g_tree_scene_header_rect.top + 3.0f,
                                 g_tree_scene_header_rect.left + 14.0f, g_tree_scene_header_rect.bottom - 3.0f),
                              g_state.tree_scene_expanded, rgba(0.82f, 0.87f, 0.94f, 1.0f), 1.4f);
    draw_text(L"场景", rc(g_tree_scene_header_rect.left + 16.0f, g_tree_scene_header_rect.top,
                         g_tree_scene_header_rect.right, g_tree_scene_header_rect.bottom),
              g_ui.fmt_mono, rgba(0.82f, 0.87f, 0.94f, 1.0f));
    y += row_h;
    if (g_state.tree_scene_expanded) {
        g_explorer_scene_rect = rc(hierarchy_rect.left + 24.0f, y, content_right, y + row_h);
        draw_text(scene_catalog_name(g_state.scene_index), g_explorer_scene_rect, g_ui.fmt_mono, rgba(0.88f, 0.92f, 0.97f, 1.0f));
        y += row_h + 4.0f;
    }
    return y;
}

static float render_hierarchy_bodies_section(D2D1_RECT_F hierarchy_rect, float y, float row_h,
                                             int body_visible_count, int body_circle_visible_count, int body_polygon_visible_count,
                                             int* io_bi, int* io_bidx) {
    int i;
    wchar_t line[128];
    int bi = *io_bi;
    int bidx = *io_bidx;
    float content_right = g_hierarchy_viewport_rect.right - 2.0f;
    g_tree_bodies_header_rect = rc(hierarchy_rect.left + 10.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
    swprintf(line, 128, L"物体 (%d)", body_visible_count);
    ui_draw_tree_disclosure_icon(g_ui.target, g_ui.brush, rc(g_tree_bodies_header_rect.left + 2.0f, g_tree_bodies_header_rect.top + 3.0f,
                                 g_tree_bodies_header_rect.left + 14.0f, g_tree_bodies_header_rect.bottom - 3.0f),
                              g_state.tree_bodies_expanded, rgba(0.82f, 0.87f, 0.94f, 1.0f), 1.4f);
    draw_text(line, rc(g_tree_bodies_header_rect.left + 16.0f, g_tree_bodies_header_rect.top,
                       g_tree_bodies_header_rect.right, g_tree_bodies_header_rect.bottom),
              g_ui.fmt_mono, rgba(0.82f, 0.87f, 0.94f, 1.0f));
    y += row_h;
    g_tree_body_circle_header_rect = rc(0, 0, 0, 0);
    g_tree_body_polygon_header_rect = rc(0, 0, 0, 0);
    if (g_state.tree_bodies_expanded && g_state.engine != NULL) {
        if (body_circle_visible_count > 0) {
            int circle_idx = 0;
            g_tree_body_circle_header_rect = rc(hierarchy_rect.left + 24.0f, y, content_right, y + row_h);
            swprintf(line, 128, L"圆 (%d)", body_circle_visible_count);
            ui_draw_tree_disclosure_icon(g_ui.target, g_ui.brush, rc(g_tree_body_circle_header_rect.left + 2.0f, g_tree_body_circle_header_rect.top + 3.0f,
                                         g_tree_body_circle_header_rect.left + 14.0f, g_tree_body_circle_header_rect.bottom - 3.0f),
                                      g_state.tree_body_circle_expanded, rgba(0.74f, 0.82f, 0.92f, 1.0f), 1.3f);
            draw_text(line, rc(g_tree_body_circle_header_rect.left + 16.0f, g_tree_body_circle_header_rect.top,
                               g_tree_body_circle_header_rect.right, g_tree_body_circle_header_rect.bottom),
                      g_ui.fmt_info, rgba(0.74f, 0.82f, 0.92f, 1.0f));
            y += row_h;
            if (g_state.tree_body_circle_expanded) {
                for (i = 0; i < physics_engine_get_body_count(g_state.engine) && bidx < EXPLORER_MAX_ITEMS; i++) {
                    RigidBody* b = physics_engine_get_body(g_state.engine, i);
                    D2D1_RECT_F row;
                    if (b == NULL || b->type != BODY_DYNAMIC || b->shape == NULL || b->shape->type != SHAPE_CIRCLE) continue;
                    swprintf(line, 128, L"#%d %s/%s", bi + 1, body_kind_name(b), body_shape_name(b));
                    if (g_state.hierarchy_filter_len > 0 && wcsstr(line, g_state.hierarchy_filter_buf) == NULL) continue;
                    row = rc(hierarchy_rect.left + 38.0f, y + circle_idx * (row_h + 4.0f), content_right, y + circle_idx * (row_h + 4.0f) + row_h);
                    g_explorer_body_rect[bidx] = row;
                    g_explorer_body_ptr[bidx] = b;
                    draw_text(line, row, g_ui.fmt_info,
                              (b == g_state.selected) ? rgba(0.98f, 0.78f, 0.42f, 1.0f) : rgba(0.87f, 0.91f, 0.96f, 1.0f));
                    bi++;
                    bidx++;
                    circle_idx++;
                }
                y += circle_idx * (row_h + 4.0f);
            }
        }
        if (body_polygon_visible_count > 0) {
            int poly_idx = 0;
            g_tree_body_polygon_header_rect = rc(hierarchy_rect.left + 24.0f, y, content_right, y + row_h);
            swprintf(line, 128, L"多边形 (%d)", body_polygon_visible_count);
            ui_draw_tree_disclosure_icon(g_ui.target, g_ui.brush, rc(g_tree_body_polygon_header_rect.left + 2.0f, g_tree_body_polygon_header_rect.top + 3.0f,
                                         g_tree_body_polygon_header_rect.left + 14.0f, g_tree_body_polygon_header_rect.bottom - 3.0f),
                                      g_state.tree_body_polygon_expanded, rgba(0.74f, 0.82f, 0.92f, 1.0f), 1.3f);
            draw_text(line, rc(g_tree_body_polygon_header_rect.left + 16.0f, g_tree_body_polygon_header_rect.top,
                               g_tree_body_polygon_header_rect.right, g_tree_body_polygon_header_rect.bottom),
                      g_ui.fmt_info, rgba(0.74f, 0.82f, 0.92f, 1.0f));
            y += row_h;
            if (g_state.tree_body_polygon_expanded) {
                for (i = 0; i < physics_engine_get_body_count(g_state.engine) && bidx < EXPLORER_MAX_ITEMS; i++) {
                    RigidBody* b = physics_engine_get_body(g_state.engine, i);
                    D2D1_RECT_F row;
                    if (b == NULL || b->type != BODY_DYNAMIC || b->shape == NULL || b->shape->type != SHAPE_POLYGON) continue;
                    swprintf(line, 128, L"#%d %s/%s", bi + 1, body_kind_name(b), body_shape_name(b));
                    if (g_state.hierarchy_filter_len > 0 && wcsstr(line, g_state.hierarchy_filter_buf) == NULL) continue;
                    row = rc(hierarchy_rect.left + 38.0f, y + poly_idx * (row_h + 4.0f), content_right, y + poly_idx * (row_h + 4.0f) + row_h);
                    g_explorer_body_rect[bidx] = row;
                    g_explorer_body_ptr[bidx] = b;
                    draw_text(line, row, g_ui.fmt_info,
                              (b == g_state.selected) ? rgba(0.98f, 0.78f, 0.42f, 1.0f) : rgba(0.87f, 0.91f, 0.96f, 1.0f));
                    bi++;
                    bidx++;
                    poly_idx++;
                }
                y += poly_idx * (row_h + 4.0f);
            }
        }
    }
    *io_bi = bi;
    *io_bidx = bidx;
    return y;
}

static float render_hierarchy_constraints_section(D2D1_RECT_F hierarchy_rect, float y, float row_h,
                                                  int constraint_visible_count, int constraint_distance_visible_count, int constraint_spring_visible_count,
                                                  int* io_ci, int* io_cidx) {
    int i;
    wchar_t line[128];
    int ci = *io_ci;
    int cidx = *io_cidx;
    float content_right = g_hierarchy_viewport_rect.right - 2.0f;
    g_tree_constraints_header_rect = rc(hierarchy_rect.left + 10.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
    swprintf(line, 128, L"约束 (%d)", constraint_visible_count);
    ui_draw_tree_disclosure_icon(g_ui.target, g_ui.brush, rc(g_tree_constraints_header_rect.left + 2.0f, g_tree_constraints_header_rect.top + 3.0f,
                                 g_tree_constraints_header_rect.left + 14.0f, g_tree_constraints_header_rect.bottom - 3.0f),
                              g_state.tree_constraints_expanded, rgba(0.82f, 0.87f, 0.94f, 1.0f), 1.4f);
    draw_text(line, rc(g_tree_constraints_header_rect.left + 16.0f, g_tree_constraints_header_rect.top,
                       g_tree_constraints_header_rect.right, g_tree_constraints_header_rect.bottom),
              g_ui.fmt_mono, rgba(0.82f, 0.87f, 0.94f, 1.0f));
    y += row_h;
    g_tree_constraint_distance_header_rect = rc(0, 0, 0, 0);
    g_tree_constraint_spring_header_rect = rc(0, 0, 0, 0);
    if (g_state.tree_constraints_expanded && g_state.engine != NULL) {
        if (constraint_distance_visible_count > 0) {
            int dist_idx = 0;
            g_tree_constraint_distance_header_rect = rc(hierarchy_rect.left + 24.0f, y, content_right, y + row_h);
            swprintf(line, 128, L"距离约束 (%d)", constraint_distance_visible_count);
            ui_draw_tree_disclosure_icon(g_ui.target, g_ui.brush, rc(g_tree_constraint_distance_header_rect.left + 2.0f, g_tree_constraint_distance_header_rect.top + 3.0f,
                                         g_tree_constraint_distance_header_rect.left + 14.0f, g_tree_constraint_distance_header_rect.bottom - 3.0f),
                                      g_state.tree_constraint_distance_expanded, rgba(0.74f, 0.82f, 0.92f, 1.0f), 1.3f);
            draw_text(line, rc(g_tree_constraint_distance_header_rect.left + 16.0f, g_tree_constraint_distance_header_rect.top,
                               g_tree_constraint_distance_header_rect.right, g_tree_constraint_distance_header_rect.bottom),
                      g_ui.fmt_info, rgba(0.74f, 0.82f, 0.92f, 1.0f));
            y += row_h;
            if (g_state.tree_constraint_distance_expanded) {
                for (i = 0; i < physics_engine_get_constraint_count(g_state.engine) && cidx < EXPLORER_MAX_ITEMS; i++) {
                    const Constraint* c = physics_engine_get_constraint(g_state.engine, i);
                    D2D1_RECT_F row;
                    if (c == NULL || !c->active || c->type != CONSTRAINT_DISTANCE) continue;
                    swprintf(line, 128, L"#%d 距离", ci + 1);
                    if (g_state.hierarchy_filter_len > 0 && wcsstr(line, g_state.hierarchy_filter_buf) == NULL) continue;
                    row = rc(hierarchy_rect.left + 38.0f, y + dist_idx * (row_h + 4.0f), content_right, y + dist_idx * (row_h + 4.0f) + row_h);
                    g_explorer_constraint_rect[cidx] = row;
                    g_explorer_constraint_index[cidx] = i;
                    draw_text(line, row, g_ui.fmt_info,
                              (i == g_state.selected_constraint_index) ? rgba(0.98f, 0.78f, 0.42f, 1.0f) : rgba(0.87f, 0.91f, 0.96f, 1.0f));
                    ci++;
                    cidx++;
                    dist_idx++;
                }
                y += dist_idx * (row_h + 4.0f);
            }
        }
        if (constraint_spring_visible_count > 0) {
            int spring_idx = 0;
            g_tree_constraint_spring_header_rect = rc(hierarchy_rect.left + 24.0f, y, content_right, y + row_h);
            swprintf(line, 128, L"弹簧约束 (%d)", constraint_spring_visible_count);
            ui_draw_tree_disclosure_icon(g_ui.target, g_ui.brush, rc(g_tree_constraint_spring_header_rect.left + 2.0f, g_tree_constraint_spring_header_rect.top + 3.0f,
                                         g_tree_constraint_spring_header_rect.left + 14.0f, g_tree_constraint_spring_header_rect.bottom - 3.0f),
                                      g_state.tree_constraint_spring_expanded, rgba(0.74f, 0.82f, 0.92f, 1.0f), 1.3f);
            draw_text(line, rc(g_tree_constraint_spring_header_rect.left + 16.0f, g_tree_constraint_spring_header_rect.top,
                               g_tree_constraint_spring_header_rect.right, g_tree_constraint_spring_header_rect.bottom),
                      g_ui.fmt_info, rgba(0.74f, 0.82f, 0.92f, 1.0f));
            y += row_h;
            if (g_state.tree_constraint_spring_expanded) {
                for (i = 0; i < physics_engine_get_constraint_count(g_state.engine) && cidx < EXPLORER_MAX_ITEMS; i++) {
                    const Constraint* c = physics_engine_get_constraint(g_state.engine, i);
                    D2D1_RECT_F row;
                    if (c == NULL || !c->active || c->type != CONSTRAINT_SPRING) continue;
                    swprintf(line, 128, L"#%d 弹簧", ci + 1);
                    if (g_state.hierarchy_filter_len > 0 && wcsstr(line, g_state.hierarchy_filter_buf) == NULL) continue;
                    row = rc(hierarchy_rect.left + 38.0f, y + spring_idx * (row_h + 4.0f), content_right, y + spring_idx * (row_h + 4.0f) + row_h);
                    g_explorer_constraint_rect[cidx] = row;
                    g_explorer_constraint_index[cidx] = i;
                    draw_text(line, row, g_ui.fmt_info,
                              (i == g_state.selected_constraint_index) ? rgba(0.98f, 0.78f, 0.42f, 1.0f) : rgba(0.87f, 0.91f, 0.96f, 1.0f));
                    ci++;
                    cidx++;
                    spring_idx++;
                }
                y += spring_idx * (row_h + 4.0f);
            }
        }
    }
    *io_ci = ci;
    *io_cidx = cidx;
    return y;
}

static void render_left_hierarchy_content(D2D1_RECT_F left_rect) {
    float row_h = 24.0f;
    float y = left_rect.top + 46.0f;
    float split_y = left_rect.top + (left_rect.bottom - left_rect.top) * 0.58f;
    D2D1_RECT_F hierarchy_rect = rc(left_rect.left + 8.0f, left_rect.top + 8.0f, left_rect.right - 8.0f, split_y - 4.0f);
    D2D1_RECT_F project_rect = rc(left_rect.left + 8.0f, split_y + 4.0f, left_rect.right - 8.0f, left_rect.bottom - 8.0f);
    D2D1_RECT_F hierarchy_viewport = rc(hierarchy_rect.left + 10.0f, hierarchy_rect.top + 40.0f, hierarchy_rect.right - 18.0f, hierarchy_rect.bottom - 10.0f);
    float viewport_h = hierarchy_viewport.bottom - hierarchy_viewport.top;
    float content_h;
    int bi = 0;
    int ci = 0;
    int bidx = 0;
    int cidx = 0;
    int body_visible_count = 0;
    int constraint_visible_count = 0;
    int body_circle_visible_count = 0;
    int body_polygon_visible_count = 0;
    int constraint_distance_visible_count = 0;
    int constraint_spring_visible_count = 0;
    init_left_panel_headers_and_search(hierarchy_rect, project_rect, hierarchy_viewport);
    {
        HierarchyStats hs = compute_hierarchy_stats();
        body_visible_count = hs.body_visible_count;
        constraint_visible_count = hs.constraint_visible_count;
        body_circle_visible_count = hs.body_circle_visible_count;
        body_polygon_visible_count = hs.body_polygon_visible_count;
        constraint_distance_visible_count = hs.constraint_distance_visible_count;
        constraint_spring_visible_count = hs.constraint_spring_visible_count;
        content_h = compute_hierarchy_content_height(row_h, &hs);
    }
    g_state.hierarchy_scroll_max = 0;
    if (content_h > viewport_h) {
        g_state.hierarchy_scroll_max = (int)(content_h - viewport_h + 0.5f);
    }
    if (g_state.hierarchy_scroll_offset < 0) g_state.hierarchy_scroll_offset = 0;
    if (g_state.hierarchy_scroll_offset > g_state.hierarchy_scroll_max) g_state.hierarchy_scroll_offset = g_state.hierarchy_scroll_max;

    y = hierarchy_rect.top + 40.0f - (float)g_state.hierarchy_scroll_offset;
    ID2D1HwndRenderTarget_PushAxisAlignedClip(g_ui.target, &hierarchy_viewport, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    g_explorer_scene_rect = rc(0, 0, 0, 0);
    g_explorer_body_count = 0;
    g_explorer_constraint_count = 0;

    y = render_hierarchy_scene_section(hierarchy_rect, y, row_h);
    y = render_hierarchy_bodies_section(hierarchy_rect, y, row_h, body_visible_count, body_circle_visible_count, body_polygon_visible_count, &bi, &bidx);
    g_explorer_body_count = bidx;
    y = render_hierarchy_constraints_section(hierarchy_rect, y, row_h, constraint_visible_count, constraint_distance_visible_count, constraint_spring_visible_count, &ci, &cidx);
    g_explorer_constraint_count = cidx;
    ID2D1HwndRenderTarget_PopAxisAlignedClip(g_ui.target);

    render_hierarchy_scrollbar(hierarchy_rect, hierarchy_viewport, viewport_h, content_h);
    render_project_tree_panel(project_rect, row_h);
}

static void render_top_bar_background(D2D1_RECT_F menu_rect, D2D1_RECT_F toolbar_rect, float width) {
    D2D1_RECT_F menu_sep = rc(0.0f, menu_rect.bottom + 1.0f, width, menu_rect.bottom + 2.5f);
    D2D1_ROUNDED_RECT toolbar_rr;
    toolbar_rr.rect = toolbar_rect;
    toolbar_rr.radiusX = 7.0f;
    toolbar_rr.radiusY = 7.0f;
    draw_outer_shadow_rr(toolbar_rr);
    draw_card_round(menu_sep, 0.0f,
                    g_state.ui_theme_light ? rgba(0.74f, 0.80f, 0.90f, 1.0f) : rgba(0.25f, 0.29f, 0.35f, 1.0f),
                    g_state.ui_theme_light ? rgba(0.74f, 0.80f, 0.90f, 1.0f) : rgba(0.25f, 0.29f, 0.35f, 1.0f));
    draw_card_round(toolbar_rect, 7.0f,
                    g_state.ui_theme_light ? rgba(0.92f, 0.94f, 0.98f, 1.0f) : rgba(0.16f, 0.17f, 0.20f, 1.0f),
                    g_state.ui_theme_light ? rgba(0.75f, 0.80f, 0.90f, 1.0f) : rgba(0.30f, 0.32f, 0.37f, 1.0f));
}

static void render_top_bar_content(D2D1_RECT_F top_rect, float width, float menu_h, float toolbar_h, HWND hwnd) {
    D2D1_RECT_F menu_rect = rc(top_rect.left + 4.0f, top_rect.top + 3.0f, top_rect.right - 4.0f, top_rect.top + 3.0f + menu_h);
    D2D1_RECT_F toolbar_rect = rc(top_rect.left + 4.0f, menu_rect.bottom + 8.0f, top_rect.right - 4.0f, menu_rect.bottom + 8.0f + toolbar_h);
    int is_zoomed = IsZoomed(hwnd) ? 1 : 0;
    render_top_bar_background(menu_rect, toolbar_rect, width);
    render_menu_bar_and_window_controls(menu_rect, is_zoomed);
    layout_top_toolbar_buttons(toolbar_rect);
    render_top_toolbar_buttons();
}

static void render_shell_panels(D2D1_RECT_F left_rect, D2D1_RECT_F center_rect, D2D1_RECT_F right_rect,
                                D2D1_RECT_F bottom_rect, D2D1_RECT_F status_rect,
                                D2D1_ROUNDED_RECT left_rr, D2D1_ROUNDED_RECT center_rr, D2D1_ROUNDED_RECT right_rr,
                                D2D1_ROUNDED_RECT bottom_rr, D2D1_ROUNDED_RECT status_rr) {
    if (g_state.ui_show_left_panel) draw_outer_shadow_rr(left_rr);
    draw_outer_shadow_rr(center_rr);
    if (g_state.ui_show_right_panel) draw_outer_shadow_rr(right_rr);
    if (g_state.ui_show_bottom_panel) draw_outer_shadow_rr(bottom_rr);
    draw_outer_shadow_rr(status_rr);

    if (g_state.ui_show_left_panel) {
        draw_card_round(left_rect, 9.0f,
                        g_state.ui_theme_light ? rgba(0.96f, 0.97f, 0.995f, 1.0f) : rgba(0.18f, 0.19f, 0.22f, 1.0f),
                        g_state.ui_theme_light ? rgba(0.80f, 0.84f, 0.92f, 1.0f) : rgba(0.30f, 0.32f, 0.37f, 1.0f));
    }
    draw_card_round(center_rect, 9.0f,
                    g_state.ui_theme_light ? rgba(0.96f, 0.97f, 0.995f, 1.0f) : rgba(0.18f, 0.19f, 0.22f, 1.0f),
                    g_state.ui_theme_light ? rgba(0.80f, 0.84f, 0.92f, 1.0f) : rgba(0.30f, 0.32f, 0.37f, 1.0f));
    if (g_state.ui_show_right_panel) {
        draw_card_round(right_rect, 9.0f,
                        g_state.ui_theme_light ? rgba(0.96f, 0.97f, 0.995f, 1.0f) : rgba(0.18f, 0.19f, 0.22f, 1.0f),
                        g_state.ui_theme_light ? rgba(0.80f, 0.84f, 0.92f, 1.0f) : rgba(0.30f, 0.32f, 0.37f, 1.0f));
    }
    if (g_state.ui_show_bottom_panel) {
        draw_card_round(bottom_rect, 9.0f,
                        g_state.ui_theme_light ? rgba(0.96f, 0.97f, 0.995f, 1.0f) : rgba(0.18f, 0.19f, 0.22f, 1.0f),
                        g_state.ui_theme_light ? rgba(0.80f, 0.84f, 0.92f, 1.0f) : rgba(0.30f, 0.32f, 0.37f, 1.0f));
    }
    draw_card_round(status_rect, 8.0f,
                    g_state.ui_theme_light ? rgba(0.94f, 0.95f, 0.985f, 1.0f) : rgba(0.15f, 0.16f, 0.19f, 1.0f),
                    g_state.ui_theme_light ? rgba(0.77f, 0.82f, 0.90f, 1.0f) : rgba(0.27f, 0.29f, 0.34f, 1.0f));
}

static void render_stage_content(D2D1_ROUNDED_RECT stage_rr, D2D1_RECT_F stage_rect) {
    draw_outer_shadow_rr(stage_rr);
    set_brush_color(0.11f, 0.12f, 0.14f, 1.0f);
    ID2D1HwndRenderTarget_FillRoundedRectangle(g_ui.target, &stage_rr, (ID2D1Brush*)g_ui.brush);
    set_brush_color(0.24f, 0.27f, 0.32f, 1.0f);
    ID2D1HwndRenderTarget_DrawRoundedRectangle(g_ui.target, &stage_rr, (ID2D1Brush*)g_ui.brush, 1.0f, NULL);
    if (g_state.engine != NULL) {
        int i;
        HRESULT clip_hr;
        ID2D1RoundedRectangleGeometry* stage_clip_geo = NULL;
        ID2D1Layer* stage_layer = NULL;
        int used_layer_clip = 0;
        D2D1_LAYER_PARAMETERS layer_params;
        D2D1_MATRIX_3X2_F identity;

        clip_hr = ID2D1Factory_CreateRoundedRectangleGeometry(g_ui.d2d_factory, &stage_rr, &stage_clip_geo);
        if (SUCCEEDED(clip_hr)) {
            clip_hr = ID2D1HwndRenderTarget_CreateLayer(g_ui.target, NULL, &stage_layer);
        }
        if (SUCCEEDED(clip_hr) && stage_layer != NULL && stage_clip_geo != NULL) {
            identity._11 = 1.0f;
            identity._12 = 0.0f;
            identity._21 = 0.0f;
            identity._22 = 1.0f;
            identity._31 = 0.0f;
            identity._32 = 0.0f;

            layer_params.contentBounds = stage_rect;
            layer_params.geometricMask = (ID2D1Geometry*)stage_clip_geo;
            layer_params.maskAntialiasMode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
            layer_params.maskTransform = identity;
            layer_params.opacity = 1.0f;
            layer_params.opacityBrush = NULL;
            layer_params.layerOptions = D2D1_LAYER_OPTIONS_NONE;
            ID2D1HwndRenderTarget_PushLayer(g_ui.target, &layer_params, stage_layer);
            used_layer_clip = 1;
        } else {
            ID2D1HwndRenderTarget_PushAxisAlignedClip(g_ui.target, &stage_rect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }

        for (i = 0; i < physics_engine_get_body_count(g_state.engine); i++) {
            draw_body_2d(physics_engine_get_body(g_state.engine, i));
            draw_velocity(physics_engine_get_body(g_state.engine, i));
        }
        draw_constraints_debug();
        draw_contacts();
        if (used_layer_clip) {
            ID2D1HwndRenderTarget_PopLayer(g_ui.target);
        } else {
            ID2D1HwndRenderTarget_PopAxisAlignedClip(g_ui.target);
        }
        release_unknown((IUnknown**)&stage_layer);
        release_unknown((IUnknown**)&stage_clip_geo);
    }
}

static void render_center_header(D2D1_RECT_F center_rect) {
    wchar_t line[128];
    draw_panel_header_band(center_rect, 38.0f, 10.0f);
    draw_text_vcenter(L"场景", rc(center_rect.left + 16.0f, center_rect.top + 4.0f, center_rect.right - 12.0f, center_rect.top + 40.0f),
                      g_ui.fmt_ui, rgba(0.90f, 0.93f, 0.98f, 1.0f));
    swprintf(line, 128, L"%s丨%s", scene_catalog_name(g_state.scene_index), g_state.running ? L"运行中" : L"已暂停");
    draw_text_right_vcenter(line, rc(center_rect.left + 130.0f, center_rect.top + 4.0f, center_rect.right - 36.0f, center_rect.top + 40.0f),
                            g_ui.fmt_info, rgba(0.67f, 0.75f, 0.88f, 1.0f));
}

static void render_right_panel_content(D2D1_RECT_F right_rect) {
    if (g_state.ui_show_right_panel) {
        D2D1_RECT_F debug_rect = render_right_inspector_section(right_rect);
        render_right_debug_section(debug_rect);
    } else {
        clear_right_panel_ui_state();
    }
}

static void render_open_menu_dropdown(void);
static void render_status_bar(D2D1_RECT_F status_rect);
static void render_modals(float w, float h);

static void render_frame_overlays(D2D1_RECT_F bottom_rect, D2D1_RECT_F status_rect, float w, float h) {
    render_bottom_panel_tabs(bottom_rect);
    render_bottom_panel_content(bottom_rect);
    render_open_menu_dropdown();
    render_status_bar(status_rect);
    render_modals(w, h);
}

static void clamp_panel_widths(float* left_w, float* right_w) {
    if (*left_w < 148.0f) *left_w = 148.0f;
    if (*left_w > 360.0f) *left_w = 360.0f;
    if (*right_w < 220.0f) *right_w = 220.0f;
    if (*right_w > 460.0f) *right_w = 460.0f;
}

typedef struct {
    D2D1_RECT_F top_rect;
    D2D1_RECT_F status_rect;
    D2D1_RECT_F bottom_rect;
    D2D1_RECT_F left_rect;
    D2D1_RECT_F right_rect;
    D2D1_RECT_F center_rect;
    D2D1_RECT_F stage_rect;
    float work_bottom;
} RenderLayout;

static RenderLayout compute_render_layout(float w, float h, float top_h, float status_h, float bottom_h, float left_w, float right_w) {
    RenderLayout layout;
    float center_left;
    float center_right;
    layout.top_rect = rc(8.0f, 8.0f, w - 8.0f, 8.0f + top_h);
    layout.status_rect = rc(8.0f, h - status_h - 8.0f, w - 8.0f, h - 8.0f);
    layout.bottom_rect = g_state.ui_show_bottom_panel
                             ? rc(8.0f, layout.status_rect.top - bottom_h - 8.0f, w - 8.0f, layout.status_rect.top - 8.0f)
                             : rc(0.0f, 0.0f, 0.0f, 0.0f);
    layout.work_bottom = g_state.ui_show_bottom_panel ? (layout.bottom_rect.top - 8.0f) : (layout.status_rect.top - 8.0f);
    center_left = g_state.ui_show_left_panel ? (8.0f + left_w + 8.0f) : 8.0f;
    center_right = g_state.ui_show_right_panel ? (w - right_w - 16.0f) : (w - 8.0f);
    layout.left_rect = g_state.ui_show_left_panel ? rc(8.0f, layout.top_rect.bottom + 8.0f, 8.0f + left_w, layout.work_bottom) : rc(0.0f, 0.0f, 0.0f, 0.0f);
    layout.right_rect = g_state.ui_show_right_panel ? rc(w - right_w - 8.0f, layout.top_rect.bottom + 8.0f, w - 8.0f, layout.work_bottom) : rc(0.0f, 0.0f, 0.0f, 0.0f);
    layout.center_rect = rc(center_left, layout.top_rect.bottom + 8.0f, center_right, layout.work_bottom);
    layout.stage_rect = rc(layout.center_rect.left + 10.0f, layout.center_rect.top + 46.0f, layout.center_rect.right - 10.0f, layout.center_rect.bottom - 8.0f);
    return layout;
}

static void build_shell_round_rects(D2D1_RECT_F left_rect, D2D1_RECT_F center_rect, D2D1_RECT_F right_rect,
                                    D2D1_RECT_F bottom_rect, D2D1_RECT_F status_rect, D2D1_RECT_F stage_rect,
                                    D2D1_ROUNDED_RECT* left_rr, D2D1_ROUNDED_RECT* center_rr, D2D1_ROUNDED_RECT* right_rr,
                                    D2D1_ROUNDED_RECT* bottom_rr, D2D1_ROUNDED_RECT* status_rr, D2D1_ROUNDED_RECT* stage_rr) {
    left_rr->rect = left_rect;
    left_rr->radiusX = 9.0f;
    left_rr->radiusY = 9.0f;
    center_rr->rect = center_rect;
    center_rr->radiusX = 9.0f;
    center_rr->radiusY = 9.0f;
    right_rr->rect = right_rect;
    right_rr->radiusX = 9.0f;
    right_rr->radiusY = 9.0f;
    bottom_rr->rect = bottom_rect;
    bottom_rr->radiusX = 9.0f;
    bottom_rr->radiusY = 9.0f;
    status_rr->rect = status_rect;
    status_rr->radiusX = 8.0f;
    status_rr->radiusY = 8.0f;
    stage_rr->rect = stage_rect;
    stage_rr->radiusX = 8.0f;
    stage_rr->radiusY = 8.0f;
}

static void reset_splitter_rects(void) {
    g_splitter_left_rect = rc(0, 0, 0, 0);
    g_splitter_right_rect = rc(0, 0, 0, 0);
    g_splitter_bottom_rect = rc(0, 0, 0, 0);
}

static void update_stage_bounds_and_fit(D2D1_RECT_F stage_rect) {
    g_state.stage_left = stage_rect.left;
    g_state.stage_top = stage_rect.top;
    g_state.stage_right = stage_rect.right;
    g_state.stage_bottom = stage_rect.bottom;
    if (g_state.scene_needs_stage_fit) {
        fit_scene_bodies_into_stage();
        g_state.scene_needs_stage_fit = 0;
    }
}

static void begin_render_pass(D2D1_COLOR_F clear_color) {
    ID2D1HwndRenderTarget_BeginDraw(g_ui.target);
    ID2D1HwndRenderTarget_Clear(g_ui.target, &clear_color);
}

static void finalize_render_pass(void) {
    HRESULT hr = ID2D1HwndRenderTarget_EndDraw(g_ui.target, NULL, NULL);
    if (hr == D2DERR_RECREATE_TARGET) {
        discard_device_resources();
    }
}

static void draw_menu_check_mark(D2D1_RECT_F row_rect, int enabled) {
    D2D1_RECT_F icon_rect;
    float icon_w = 12.0f;
    float icon_h = 12.0f;
    float cy = (row_rect.top + row_rect.bottom) * 0.5f;
    icon_rect = rc(row_rect.left + 8.0f, cy - icon_h * 0.5f, row_rect.left + 8.0f + icon_w, cy + icon_h * 0.5f);
    set_brush_color(enabled ? 0.86f : 0.56f, enabled ? 0.92f : 0.63f, enabled ? 0.99f : 0.72f, 1.0f);
    ID2D1HwndRenderTarget_DrawLine(g_ui.target,
                                   pt(icon_rect.left + 2.3f, icon_rect.top + 6.4f),
                                   pt(icon_rect.left + 5.0f, icon_rect.top + 9.0f),
                                   (ID2D1Brush*)g_ui.brush, 1.8f, NULL);
    ID2D1HwndRenderTarget_DrawLine(g_ui.target,
                                   pt(icon_rect.left + 5.0f, icon_rect.top + 9.0f),
                                   pt(icon_rect.right - 2.0f, icon_rect.top + 3.1f),
                                   (ID2D1Brush*)g_ui.brush, 1.8f, NULL);
}

static void render_workspace_content(D2D1_RECT_F top_rect, float work_bottom,
                                     D2D1_RECT_F left_rect, D2D1_RECT_F right_rect, D2D1_RECT_F center_rect,
                                     D2D1_ROUNDED_RECT stage_rr, D2D1_RECT_F stage_rect,
                                     D2D1_RECT_F bottom_rect, D2D1_RECT_F status_rect, float w, float h) {
    g_left_fold_rect = rc(0, 0, 0, 0);
    g_right_fold_rect = rc(0, 0, 0, 0);
    if (g_state.ui_show_left_panel) {
        render_left_hierarchy_content(left_rect);
    } else {
        clear_left_panel_ui_state();
    }
    render_center_header(center_rect);
    render_stage_content(stage_rr, stage_rect);
    render_right_panel_content(right_rect);
    render_side_fold_buttons(top_rect, work_bottom, left_rect, center_rect, right_rect);
    render_frame_overlays(bottom_rect, status_rect, w, h);
}

static void render_open_menu_dropdown(void) {
    if (g_state.open_menu_id > 0) {
        const wchar_t* rows[8] = {0};
        int row_n = 0;
        int ri;
        MenuUiState menu_state = build_menu_ui_state();
        D2D1_RECT_F anchor = g_menu_file_rect;
        float max_item_w = 0.0f;
        float max_short_w = 0.0f;
        float shortcut_slot = 0.0f;
        float dropdown_w = 0.0f;
        row_n = menu_model_item_count_for_menu(g_state.open_menu_id);
        for (ri = 0; ri < row_n && ri < 8; ri++) {
            rows[ri] = menu_model_item_text(g_state.open_menu_id, ri);
        }
        anchor = menu_anchor_rect(g_state.open_menu_id);
        for (ri = 0; ri < row_n; ri++) {
            wchar_t measure_row[96];
            const wchar_t* sk = menu_shortcut_text(g_state.open_menu_id, ri);
            float iw;
            float sw;
            swprintf(measure_row, 96, L"%s", rows[ri]);
            iw = measure_text_width(measure_row, g_ui.fmt_info);
            sw = measure_text_width(sk, g_ui.fmt_info);
            iw += 20.0f;
            if (iw > max_item_w) max_item_w = iw;
            if (sw > max_short_w) max_short_w = sw;
        }
        if (max_short_w > 0.0f) shortcut_slot = max_short_w + 16.0f;
        dropdown_w = max_item_w + 24.0f + (shortcut_slot > 0.0f ? (shortcut_slot + 4.0f) : 0.0f);
        if (dropdown_w < 128.0f) dropdown_w = 128.0f;
        g_menu_item_count = row_n;
        g_menu_dropdown_rect = rc(anchor.left, anchor.bottom + 2.0f, anchor.left + dropdown_w, anchor.bottom + 2.0f + row_n * 28.0f + 6.0f);
        draw_card_round(g_menu_dropdown_rect, 7.0f, rgba(0.15f, 0.16f, 0.20f, 1.0f), rgba(0.28f, 0.31f, 0.37f, 1.0f));
        for (ri = 0; ri < row_n; ri++) {
            D2D1_RECT_F rr = rc(g_menu_dropdown_rect.left + 4.0f, g_menu_dropdown_rect.top + 3.0f + ri * 28.0f,
                                g_menu_dropdown_rect.right - 4.0f, g_menu_dropdown_rect.top + 3.0f + ri * 28.0f + 24.0f);
            int enabled = 1;
            int checked = menu_model_item_checked_state(g_state.open_menu_id, ri, &menu_state);
            g_menu_item_rect[ri] = rr;
            enabled = menu_model_item_enabled_state(g_state.open_menu_id, ri, &menu_state);
            g_menu_item_enabled[ri] = enabled;
            draw_card_round(rr, 5.0f,
                            (enabled && (point_in_rect(g_state.mouse_screen, rr) || ri == g_state.open_menu_focus_index))
                                ? rgba(0.25f, 0.33f, 0.45f, 1.0f)
                                : rgba(0.18f, 0.20f, 0.25f, 1.0f),
                            rgba(0.30f, 0.33f, 0.40f, 1.0f));
            if (checked) {
                draw_menu_check_mark(rr, enabled);
            }
            draw_text(rows[ri], rc(rr.left + 24.0f, rr.top, rr.right - (shortcut_slot > 0.0f ? (shortcut_slot + 10.0f) : 6.0f), rr.bottom), g_ui.fmt_info,
                      enabled ? rgba(0.87f, 0.91f, 0.97f, 1.0f) : rgba(0.53f, 0.59f, 0.68f, 1.0f));
            {
                const wchar_t* sk = menu_shortcut_text(g_state.open_menu_id, ri);
                if (sk[0] != L'\0') {
                    draw_text(sk, rc(rr.right - (shortcut_slot + 6.0f), rr.top, rr.right - 6.0f, rr.bottom), g_ui.fmt_info,
                              enabled ? rgba(0.63f, 0.72f, 0.84f, 1.0f) : rgba(0.45f, 0.51f, 0.60f, 1.0f));
                }
            }
        }
    } else {
        g_menu_item_count = 0;
        g_menu_dropdown_rect = rc(0, 0, 0, 0);
    }
}

static void render_status_bar(D2D1_RECT_F status_rect) {
    wchar_t line[128];
    wchar_t line_right[128];
    const AppRuntimeSnapshot* snapshot = app_runtime_get_last_snapshot(&g_app_runtime);
    int constraint_count = (g_state.engine != NULL) ? physics_engine_get_constraint_count(g_state.engine) : 0;
    int contact_count = (g_state.engine != NULL) ? physics_engine_get_contact_count(g_state.engine) : 0;
    if (snapshot != NULL && snapshot->valid) {
        constraint_count = snapshot->constraint_count;
        contact_count = snapshot->contact_count;
    }
    swprintf(line, 128, L"对象:%d  约束:%d  接触:%d  回收:%d",
             count_dynamic_bodies(g_state.engine),
             constraint_count,
             contact_count,
             g_state.recycled_count);
    {
        const wchar_t* tip = L"";
        if (point_in_rect(g_state.mouse_screen, g_top_run_rect)) tip = L"运行/暂停模拟";
        else if (point_in_rect(g_state.mouse_screen, g_top_step_rect)) tip = L"单步执行一帧";
        else if (point_in_rect(g_state.mouse_screen, g_top_reset_rect)) tip = L"重置当前场景";
        else if (point_in_rect(g_state.mouse_screen, g_top_save_rect)) tip = L"保存快照";
        else if (point_in_rect(g_state.mouse_screen, g_top_undo_rect)) tip = L"撤销";
        else if (point_in_rect(g_state.mouse_screen, g_top_redo_rect)) tip = L"重做";
        else if (point_in_rect(g_state.mouse_screen, g_top_grid_rect)) tip = L"网格/中心显示";
        else if (point_in_rect(g_state.mouse_screen, g_top_collision_rect)) tip = L"显示/隐藏接触点";
        else if (point_in_rect(g_state.mouse_screen, g_top_velocity_rect)) tip = L"显示/隐藏速度向量";
        else if (point_in_rect(g_state.mouse_screen, g_top_constraint_rect)) tip = L"约束工具：先选中一个物体，再选第二个物体建立距离约束";
        else if (point_in_rect(g_state.mouse_screen, g_top_spring_rect)) tip = L"弹性约束：先选中一个物体，再选第二个物体建立弹簧约束";
        else if (point_in_rect(g_state.mouse_screen, g_top_chain_rect)) tip = L"链条：先选中一个物体，再选第二个物体生成链节+距离约束";
        else if (point_in_rect(g_state.mouse_screen, g_top_rope_rect)) tip = L"橡皮绳：先选中一个物体，再选第二个物体生成弹性节段";
        else if (point_in_rect(g_state.mouse_screen, g_splitter_left_rect) || point_in_rect(g_state.mouse_screen, g_splitter_right_rect)) tip = L"拖拽调整左右面板宽度";
        else if (point_in_rect(g_state.mouse_screen, g_splitter_bottom_rect)) tip = L"拖拽调整底部面板高度";
        else if (point_in_rect(g_state.mouse_screen, g_menu_window_rect)) tip = L"切换布局预设（编辑/调试/性能）";
        else if (point_in_rect(g_state.mouse_screen, g_status_meta_rect)) tip = g_status_project_path;
        if (tip[0] != L'\0') {
            draw_text(tip, rc(status_rect.left + 330.0f, status_rect.top + 6.0f, status_rect.right - 630.0f, status_rect.bottom - 4.0f),
                      g_ui.fmt_info, rgba(0.88f, 0.93f, 0.99f, 1.0f));
        }
    }
    draw_text(line, rc(status_rect.left + 12.0f, status_rect.top + 6.0f, status_rect.right - 200.0f, status_rect.bottom - 4.0f),
              g_ui.fmt_info, rgba(0.77f, 0.84f, 0.93f, 1.0f));
    swprintf(line, 128, L"用户:%s | v0.3", g_status_user[0] ? g_status_user : L"unknown");
    g_status_meta_rect = rc(status_rect.right - 620.0f, status_rect.top + 6.0f, status_rect.right - 392.0f, status_rect.bottom - 4.0f);
    draw_text(line, g_status_meta_rect, g_ui.fmt_info, rgba(0.55f, 0.65f, 0.79f, 1.0f));
    swprintf(line_right, 128, L"约束调试:%s", g_state.draw_constraints ? L"开" : L"关");
    draw_text(line_right, rc(status_rect.right - 360.0f, status_rect.top + 6.0f, status_rect.right - 220.0f, status_rect.bottom - 4.0f),
              g_ui.fmt_info, rgba(0.66f, 0.74f, 0.85f, 1.0f));
}

static void render_help_modal_content(D2D1_RECT_F modal) {
    if (g_state.help_modal_page == 1) {
        draw_text(L"物理调试指南", rc(modal.left + 18.0f, modal.top + 14.0f, modal.right - 60.0f, modal.top + 46.0f), g_ui.fmt_title,
                  rgba(0.88f, 0.92f, 0.97f, 1.0f));
        draw_text(L"1) 打开约束调试（菜单: 组件 -> 约束调试开关）", rc(modal.left + 22.0f, modal.top + 72.0f, modal.right - 24.0f, modal.top + 102.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"2) 用 N 单步并观察右侧碰撞日志（v/p变化）", rc(modal.left + 22.0f, modal.top + 102.0f, modal.right - 24.0f, modal.top + 132.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"3) 点碰撞日志条目可快速选中相关对象", rc(modal.left + 22.0f, modal.top + 132.0f, modal.right - 24.0f, modal.top + 162.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"4) 在检查器里用 +/- 或 Enter 修改质量/速度/约束", rc(modal.left + 22.0f, modal.top + 162.0f, modal.right - 24.0f, modal.top + 192.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"5) 打开底部性能页，观察 FPS 与物理耗时曲线", rc(modal.left + 22.0f, modal.top + 192.0f, modal.right - 24.0f, modal.top + 222.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
    } else if (g_state.help_modal_page == 2) {
        draw_text(L"版本信息", rc(modal.left + 18.0f, modal.top + 14.0f, modal.right - 60.0f, modal.top + 46.0f), g_ui.fmt_title,
                  rgba(0.88f, 0.92f, 0.97f, 1.0f));
        draw_text(L"Physics Engine Sandbox Editor", rc(modal.left + 22.0f, modal.top + 74.0f, modal.right - 24.0f, modal.top + 104.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"版本: v0.3", rc(modal.left + 22.0f, modal.top + 104.0f, modal.right - 24.0f, modal.top + 134.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"核心: 2D 刚体 / 约束 / 碰撞 / 编辑器化 UI", rc(modal.left + 22.0f, modal.top + 134.0f, modal.right - 24.0f, modal.top + 164.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"构建: DirectWrite + 自研物理内核", rc(modal.left + 22.0f, modal.top + 164.0f, modal.right - 24.0f, modal.top + 194.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
    } else {
        draw_text(L"使用说明", rc(modal.left + 18.0f, modal.top + 14.0f, modal.right - 60.0f, modal.top + 46.0f), g_ui.fmt_title,
                  rgba(0.88f, 0.92f, 0.97f, 1.0f));
        draw_text(L"1) 场景切换: F1~F9直选，[ / ]循环切换", rc(modal.left + 22.0f, modal.top + 72.0f, modal.right - 24.0f, modal.top + 102.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"2) 运行控制: Space运行/暂停 N单步 R重置 F11切布局", rc(modal.left + 22.0f, modal.top + 102.0f, modal.right - 24.0f, modal.top + 132.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"3) 文件/编辑: Ctrl+S保存 Ctrl+O加载 Ctrl+C复制 Ctrl+V粘贴", rc(modal.left + 22.0f, modal.top + 132.0f, modal.right - 24.0f, modal.top + 162.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"4) 创建与删除: 1生成圆 2生成方块 Delete删除选中", rc(modal.left + 22.0f, modal.top + 162.0f, modal.right - 24.0f, modal.top + 192.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"5) 显示开关: C接触点 V速度向量 X中心点", rc(modal.left + 22.0f, modal.top + 192.0f, modal.right - 24.0f, modal.top + 222.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"6) 鼠标左键: 选中并拖拽动态物体", rc(modal.left + 22.0f, modal.top + 222.0f, modal.right - 24.0f, modal.top + 252.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"7) 约束: J距离, K弹簧, L链条, P橡皮绳（均为两次选择）", rc(modal.left + 22.0f, modal.top + 252.0f, modal.right - 24.0f, modal.top + 282.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"8) 约束编辑: T/G刚度 Y/H阻尼 U是否碰撞 B断裂阈值", rc(modal.left + 22.0f, modal.top + 282.0f, modal.right - 24.0f, modal.top + 312.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"9) M 开关约束调试绘制, Esc 关闭当前弹窗", rc(modal.left + 22.0f, modal.top + 312.0f, modal.right - 24.0f, modal.top + 342.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
        draw_text(L"10) 物理调试: 点击碰撞日志条目可快速选中对象", rc(modal.left + 22.0f, modal.top + 342.0f, modal.right - 24.0f, modal.top + 372.0f),
                  g_ui.fmt_mono, rgba(0.74f, 0.81f, 0.90f, 1.0f));
    }
}

static void render_config_modal_content(D2D1_RECT_F modal) {
    SceneConfig* cfg = &g_state.scenes[g_state.scene_index];
    D2D1_RECT_F row;
    D2D1_RECT_F minus_btn;
    D2D1_RECT_F plus_btn;
    wchar_t line[128];
    static const wchar_t* row_name[8] = {L"重力 Y", L"时间步长", L"阻尼", L"迭代次数", L"球弹性", L"箱弹性", L"球质量", L"箱质量"};
    int r;
    draw_text(L"配置面板", rc(modal.left + 18.0f, modal.top + 14.0f, modal.right - 60.0f, modal.top + 46.0f), g_ui.fmt_title,
              rgba(0.88f, 0.92f, 0.97f, 1.0f));
    draw_text(L"上下选择，左右调参，也可点 - / +", rc(modal.left + 22.0f, modal.top + 60.0f, modal.right - 24.0f, modal.top + 88.0f),
              g_ui.fmt_mono, rgba(0.66f, 0.74f, 0.84f, 1.0f));

    for (r = 0; r < 8; r++) {
        row = rc(modal.left + 22.0f, modal.top + 98.0f + r * 50.0f, modal.right - 22.0f, modal.top + 136.0f + r * 50.0f);
        g_cfg_row_rect[r] = row;
        {
            D2D1_ROUNDED_RECT row_rr;
            row_rr.rect = row;
            row_rr.radiusX = 7.0f;
            row_rr.radiusY = 7.0f;
            draw_outer_shadow_rr(row_rr);
        }
        draw_card_round(row, 7.0f,
                        (r == g_state.focused_param) ? rgba(0.24f, 0.30f, 0.40f, 1.0f) : rgba(0.17f, 0.20f, 0.26f, 1.0f),
                        (r == g_state.focused_param) ? rgba(0.43f, 0.55f, 0.71f, 1.0f) : rgba(0.29f, 0.34f, 0.42f, 1.0f));
        if (r == 0) swprintf(line, 128, L"%.2f", cfg->gravity_y);
        if (r == 1) swprintf(line, 128, L"%.4f", cfg->time_step);
        if (r == 2) swprintf(line, 128, L"%.3f", cfg->damping);
        if (r == 3) swprintf(line, 128, L"%d", cfg->iterations);
        if (r == 4) swprintf(line, 128, L"%.2f", cfg->ball_restitution);
        if (r == 5) swprintf(line, 128, L"%.2f", cfg->box_restitution);
        if (r == 6) swprintf(line, 128, L"%.1f", cfg->ball_mass);
        if (r == 7) swprintf(line, 128, L"%.1f", cfg->box_mass);
        draw_text(row_name[r], rc(row.left + 12.0f, row.top + 9.0f, row.left + 180.0f, row.bottom - 8.0f), g_ui.fmt_mono,
                  rgba(0.73f, 0.80f, 0.89f, 1.0f));
        draw_text(line, rc(row.left + 190.0f, row.top + 9.0f, row.right - 94.0f, row.bottom - 8.0f), g_ui.fmt_mono,
                  rgba(0.90f, 0.94f, 0.99f, 1.0f));
        minus_btn = rc(row.right - 84.0f, row.top + 7.0f, row.right - 48.0f, row.bottom - 7.0f);
        plus_btn = rc(row.right - 42.0f, row.top + 7.0f, row.right - 6.0f, row.bottom - 7.0f);
        g_cfg_minus_rect[r] = minus_btn;
        g_cfg_plus_rect[r] = plus_btn;
        draw_text(L"-", minus_btn, g_ui.fmt_icon,
                  can_adjust_param(cfg, r, -1) ? rgba(0.78f, 0.84f, 0.93f, 1.0f) : rgba(0.49f, 0.55f, 0.64f, 1.0f));
        draw_text(L"+", plus_btn, g_ui.fmt_icon,
                  can_adjust_param(cfg, r, 1) ? rgba(0.78f, 0.84f, 0.93f, 1.0f) : rgba(0.49f, 0.55f, 0.64f, 1.0f));
    }
}

static void render_modals(float w, float h) {
    if (g_state.show_value_input || g_state.show_config_modal || g_state.show_help_modal) {
        D2D1_RECT_F modal = rc(w * 0.13f, h * 0.12f, w * 0.87f, h * 0.88f);
        D2D1_RECT_F overlay = rc(0.0f, 0.0f, w, h);
        set_brush_color(0.0f, 0.0f, 0.0f, 0.48f);
        ID2D1HwndRenderTarget_FillRectangle(g_ui.target, &overlay, (ID2D1Brush*)g_ui.brush);
        if (g_state.show_value_input) {
            D2D1_RECT_F vm = rc(w * 0.34f, h * 0.40f, w * 0.66f, h * 0.58f);
            g_value_modal_rect = vm;
            draw_card_round(vm, 10.0f, rgba(0.14f, 0.16f, 0.20f, 1.0f), rgba(0.30f, 0.34f, 0.41f, 1.0f));
            draw_text(g_state.value_input_target == 2 ? L"日志搜索关键字" : L"输入数值",
                      rc(vm.left + 14.0f, vm.top + 10.0f, vm.right - 14.0f, vm.top + 36.0f),
                      g_ui.fmt_ui, rgba(0.86f, 0.91f, 0.97f, 1.0f));
            draw_card_round(rc(vm.left + 14.0f, vm.top + 44.0f, vm.right - 14.0f, vm.top + 78.0f), 6.0f,
                            rgba(0.10f, 0.12f, 0.16f, 1.0f), rgba(0.34f, 0.40f, 0.50f, 1.0f));
            draw_text(g_state.value_input_buf, rc(vm.left + 22.0f, vm.top + 50.0f, vm.right - 22.0f, vm.top + 72.0f), g_ui.fmt_mono,
                      rgba(0.90f, 0.94f, 0.99f, 1.0f));
            {
                wchar_t prefix[64];
                int i;
                int caret = g_state.value_input_caret;
                if (caret < 0) caret = 0;
                if (caret > g_state.value_input_len) caret = g_state.value_input_len;
                for (i = 0; i < caret && i < 63; i++) {
                    prefix[i] = g_state.value_input_buf[i];
                }
                prefix[i] = L'\0';
                float cx = vm.left + 22.0f + measure_text_width(prefix, g_ui.fmt_mono);
                if (cx > vm.right - 24.0f) cx = vm.right - 24.0f;
                set_brush_color(0.45f, 0.62f, 0.84f, 1.0f);
                ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx, vm.top + 50.0f), pt(cx, vm.top + 72.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
            }
            draw_text(L"Enter 确认  Esc 取消  Backspace 删除", rc(vm.left + 14.0f, vm.top + 88.0f, vm.right - 14.0f, vm.top + 112.0f), g_ui.fmt_info,
                      rgba(0.62f, 0.70f, 0.82f, 1.0f));
        } else {
            g_modal_rect = modal;
            g_modal_close_rect = rc(modal.right - 40.0f, modal.top + 12.0f, modal.right - 12.0f, modal.top + 40.0f);
            draw_card_round(modal, 10.0f, rgba(0.14f, 0.16f, 0.20f, 1.0f), rgba(0.30f, 0.34f, 0.41f, 1.0f));
            draw_text(L"x", g_modal_close_rect, g_ui.fmt_icon, rgba(0.84f, 0.89f, 0.95f, 1.0f));
        }

        if (!g_state.show_value_input && g_state.show_help_modal) render_help_modal_content(modal);
        if (!g_state.show_value_input && g_state.show_config_modal) render_config_modal_content(modal);
    }
}

static void render(HWND hwnd) {
    RECT wr;
    float w;
    float h;
    float menu_h = 30.0f;
    float toolbar_h = 44.0f;
    float top_h = menu_h + toolbar_h + 12.0f;
    float status_h = 32.0f;
    float bottom_h = g_state.ui_show_bottom_panel ? (g_state.bottom_panel_collapsed ? 24.0f : g_state.ui_bottom_open_h) : 0.0f;
    float left_w;
    float right_w;
    HRESULT hr = create_device_resources(hwnd);
    if (FAILED(hr)) return;

    GetClientRect(hwnd, &wr);
    w = (float)(wr.right - wr.left);
    h = (float)(wr.bottom - wr.top);
    left_w = w * g_state.ui_left_ratio;
    right_w = w * g_state.ui_right_ratio;
    clamp_panel_widths(&left_w, &right_w);
    if (g_state.ui_show_bottom_panel && bottom_h < 24.0f) bottom_h = 24.0f;

    D2D1_COLOR_F clear_color = g_state.ui_theme_light ? rgba(0.92f, 0.94f, 0.98f, 1.0f) : rgba(0.10f, 0.11f, 0.13f, 1.0f);
    RenderLayout layout = compute_render_layout(w, h, top_h, status_h, bottom_h, left_w, right_w);
    {
        float work_bottom = layout.work_bottom;
        D2D1_RECT_F top_rect = layout.top_rect;
        D2D1_RECT_F status_rect = layout.status_rect;
        D2D1_RECT_F bottom_rect = layout.bottom_rect;
        D2D1_RECT_F left_rect = layout.left_rect;
        D2D1_RECT_F right_rect = layout.right_rect;
        D2D1_RECT_F center_rect = layout.center_rect;
        D2D1_RECT_F stage_rect = layout.stage_rect;
        reset_splitter_rects();

    D2D1_ROUNDED_RECT left_rr;
    D2D1_ROUNDED_RECT center_rr;
    D2D1_ROUNDED_RECT right_rr;
    D2D1_ROUNDED_RECT bottom_rr;
    D2D1_ROUNDED_RECT status_rr;
    D2D1_ROUNDED_RECT stage_rr;

    update_stage_bounds_and_fit(stage_rect);
    begin_render_pass(clear_color);

    build_shell_round_rects(left_rect, center_rect, right_rect, bottom_rect, status_rect, stage_rect,
                            &left_rr, &center_rr, &right_rr, &bottom_rr, &status_rr, &stage_rr);

    render_shell_panels(left_rect, center_rect, right_rect, bottom_rect, status_rect, left_rr, center_rr, right_rr, bottom_rr, status_rr);

    render_top_bar_content(top_rect, w, menu_h, toolbar_h, hwnd);
    render_workspace_content(top_rect, work_bottom, left_rect, right_rect, center_rect, stage_rr, stage_rect, bottom_rect, status_rect, w, h);
    }

    finalize_render_pass();
}

static void app_cmd_toggle_run(void* user) {
    (void)user;
    g_state.running = !g_state.running;
}

static void app_cmd_reset_scene(void* user) {
    (void)user;
    apply_scene(g_state.scene_index);
}

static void app_cmd_spawn_circle(void* user) {
    (void)user;
    trace_spawn_step("key.1.begin", "");
    spawn_circle_at_cursor();
    trace_spawn_step("key.1.end", "");
}

static void app_cmd_spawn_box(void* user) {
    (void)user;
    trace_spawn_step("key.2.begin", "");
    spawn_box_at_cursor();
    trace_spawn_step("key.2.end", "");
}

static void process_app_events(void) {
    AppEvent ev;
    while (app_runtime_pop_event(&g_app_runtime, &ev)) {
        if (ev.type == APP_EVENT_COMMAND_EXECUTED) {
            if (ev.command_type == APP_CMD_TOGGLE_RUN) {
                push_console_log(L"[状态] 模拟:%s", g_state.running ? L"运行" : L"暂停");
            } else if (ev.command_type == APP_CMD_RESET_SCENE) {
                push_console_log(L"[状态] 已重置当前场景");
            } else if (ev.command_type == APP_CMD_SPAWN_CIRCLE) {
                push_console_log(L"[创建] 已生成圆形对象");
            } else if (ev.command_type == APP_CMD_SPAWN_BOX) {
                push_console_log(L"[创建] 已生成方块对象");
            }
        } else if (ev.type == APP_EVENT_RUNTIME_TICK) {
            g_state.physics_step_ms = ev.runtime_snapshot.step_ms;
            if (g_state.running && ev.runtime_snapshot.contact_count != g_state.last_contact_count) {
                unsigned int now_ms = (unsigned int)GetTickCount();
                if ((now_ms - g_state.last_contact_log_ms) >= 500) {
                    push_console_log(L"[碰撞] 接触数量: %d -> %d", g_state.last_contact_count, ev.runtime_snapshot.contact_count);
                    g_state.last_contact_log_ms = now_ms;
                }
                g_state.last_contact_count = ev.runtime_snapshot.contact_count;
            }
        }
    }
}

static void tick(HWND hwnd) {
    refresh_stage_bounds_from_layout(hwnd);
    cleanup_constraint_selection();
    drag_selected_body_stepwise();
    if (g_state.running && g_state.engine != NULL) {
        LARGE_INTEGER q0;
        LARGE_INTEGER q1;
        LARGE_INTEGER fq;
        double ms = 0.0;
        QueryPerformanceCounter(&q0);
        physics_engine_step(g_state.engine);
        QueryPerformanceCounter(&q1);
        QueryPerformanceFrequency(&fq);
        if (fq.QuadPart > 0) {
            ms = (double)(q1.QuadPart - q0.QuadPart) * 1000.0 / (double)fq.QuadPart;
            g_state.physics_step_ms = (float)ms;
        }
        recycle_out_of_bounds_objects(hwnd);
        cleanup_constraint_selection();
        capture_collision_events();
    }
    {
        unsigned int now_ms = (unsigned int)GetTickCount();
        g_state.fps_accum_frames++;
        if (g_state.fps_last_tick_ms == 0) g_state.fps_last_tick_ms = now_ms;
        if ((now_ms - g_state.fps_last_tick_ms) >= 500) {
            unsigned int dt_ms = now_ms - g_state.fps_last_tick_ms;
            if (dt_ms > 0) {
                g_state.fps_display = (int)((1000.0 * g_state.fps_accum_frames) / (double)dt_ms + 0.5);
            }
            g_state.fps_accum_frames = 0;
            g_state.fps_last_tick_ms = now_ms;
        }
        if (g_state.engine != NULL && (now_ms - g_state.last_autosave_ms) >= 30000) {
            if (save_scene_snapshot("autosave_snapshot.txt")) {
                g_state.last_autosave_ms = now_ms;
            }
        }
    }
    app_runtime_report_tick(&g_app_runtime, g_state.engine, g_state.running, g_state.physics_step_ms);
    perf_push_sample((float)g_state.fps_display, g_state.physics_step_ms);
    process_app_events();
    InvalidateRect(hwnd, NULL, FALSE);
}

static int handle_value_input_keydown(HWND hwnd, WPARAM wparam) {
    if (!g_state.show_value_input) return 0;
    if (wparam == VK_RETURN) {
        apply_value_input();
        g_state.show_value_input = 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == VK_ESCAPE) {
        g_state.show_value_input = 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == VK_BACK) {
        if (g_state.value_input_caret > 0 && g_state.value_input_len > 0) {
            int i;
            for (i = g_state.value_input_caret - 1; i < g_state.value_input_len; i++) {
                g_state.value_input_buf[i] = g_state.value_input_buf[i + 1];
            }
            g_state.value_input_len--;
            g_state.value_input_caret--;
            g_state.value_input_buf[g_state.value_input_len] = L'\0';
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 1;
    }
    if (wparam == VK_DELETE) {
        if (g_state.value_input_caret < g_state.value_input_len) {
            int i;
            for (i = g_state.value_input_caret; i < g_state.value_input_len; i++) {
                g_state.value_input_buf[i] = g_state.value_input_buf[i + 1];
            }
            g_state.value_input_len--;
            g_state.value_input_buf[g_state.value_input_len] = L'\0';
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 1;
    }
    if (wparam == VK_LEFT) {
        if (g_state.value_input_caret > 0) g_state.value_input_caret--;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == VK_RIGHT) {
        if (g_state.value_input_caret < g_state.value_input_len) g_state.value_input_caret++;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == VK_HOME) {
        g_state.value_input_caret = 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == VK_END) {
        g_state.value_input_caret = g_state.value_input_len;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    return 1;
}

static int handle_ctrl_shortcuts_keydown(HWND hwnd, WPARAM wparam) {
    int ctrl_down = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    if (!ctrl_down) return 0;

    if (wparam == 'S') {
        if (save_scene_snapshot("scene_snapshot.txt")) push_console_log(L"[快捷键] Ctrl+S 已保存 scene_snapshot.txt");
        else push_console_log(L"[错误] 保存 scene_snapshot.txt 失败");
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == 'O') {
        if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
            if (load_scene_snapshot("autosave_snapshot.txt")) {
                history_reset_and_capture();
                push_console_log(L"[快捷键] Ctrl+Shift+O 已恢复自动保存");
            } else push_console_log(L"[错误] 恢复 autosave_snapshot.txt 失败");
        } else {
            if (load_scene_snapshot("scene_snapshot.txt")) {
                history_reset_and_capture();
                push_console_log(L"[快捷键] Ctrl+O 已加载 scene_snapshot.txt");
            } else push_console_log(L"[错误] 加载 scene_snapshot.txt 失败");
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == 'Z') {
        history_undo();
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == 'Y') {
        history_redo();
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == 'C') {
        if (copy_selected_body_to_clipboard()) push_console_log(L"[快捷键] Ctrl+C 已复制对象");
        else push_console_log(L"[提示] 当前无可复制对象");
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == 'V') {
        trace_spawn_step("key.ctrlv.begin", "");
        history_push_snapshot();
        if (paste_body_from_clipboard()) push_console_log(L"[快捷键] Ctrl+V 已粘贴对象");
        else push_console_log(L"[提示] 剪贴板为空");
        trace_spawn_step("key.ctrlv.end", "");
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == 'F') {
        return 0;
    }
    if (wparam == 'L') {
        begin_value_input_for_hierarchy_filter();
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    return 0;
}

static int handle_open_menu_keydown(HWND hwnd, WPARAM wparam) {
    if (g_state.open_menu_id <= 0) return 0;
    {
        int n = menu_item_count_for_menu(g_state.open_menu_id);
        if (wparam == VK_ESCAPE) {
            g_state.open_menu_id = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
        if (wparam == VK_UP && n > 0) {
            g_state.open_menu_focus_index = menu_find_enabled_from(g_state.open_menu_id, g_state.open_menu_focus_index - 1, -1);
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
        if (wparam == VK_DOWN && n > 0) {
            g_state.open_menu_focus_index = menu_find_enabled_from(g_state.open_menu_id, g_state.open_menu_focus_index + 1, 1);
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
        if (wparam == VK_LEFT || wparam == VK_RIGHT) {
            int next = menu_next_visible_id(g_state.open_menu_id, (wparam == VK_RIGHT) ? 1 : -1);
            g_state.open_menu_id = next;
            g_state.open_menu_focus_index = menu_find_enabled_from(next, 0, 1);
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
        if (wparam == VK_RETURN && n > 0) {
            int fi = g_state.open_menu_focus_index;
            if (fi >= 0 && fi < n && menu_item_enabled_state(g_state.open_menu_id, fi)) {
                execute_menu_action(hwnd, g_state.open_menu_id, fi);
            } else {
                push_console_log(L"[提示] 该菜单项当前不可用");
            }
            g_state.open_menu_id = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
    }
    return 0;
}

static int handle_modal_keydown(HWND hwnd, WPARAM wparam) {
    if (wparam == VK_ESCAPE) {
        g_state.open_menu_id = 0;
        g_state.show_config_modal = 0;
        g_state.show_help_modal = 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (g_state.show_config_modal) {
        if (wparam == VK_UP) g_state.focused_param = (g_state.focused_param + 7) % 8;
        if (wparam == VK_DOWN) g_state.focused_param = (g_state.focused_param + 1) % 8;
        if (wparam == VK_LEFT) adjust_focused_param(-1);
        if (wparam == VK_RIGHT) adjust_focused_param(1);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (g_state.show_help_modal) {
        return 1;
    }
    return 0;
}

static int handle_modal_lbuttondown(HWND hwnd) {
    int r;
    if (!(g_state.show_value_input || g_state.show_config_modal || g_state.show_help_modal)) return 0;
    if (g_state.show_value_input) {
        if (!point_in_rect(g_state.mouse_screen, g_value_modal_rect)) {
            g_state.show_value_input = 0;
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_modal_close_rect)) {
        g_state.show_config_modal = 0;
        g_state.show_help_modal = 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (!point_in_rect(g_state.mouse_screen, g_modal_rect)) {
        g_state.show_config_modal = 0;
        g_state.show_help_modal = 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (g_state.show_config_modal) {
        SceneConfig* cfg = &g_state.scenes[g_state.scene_index];
        for (r = 0; r < 8; r++) {
            if (point_in_rect(g_state.mouse_screen, g_cfg_row_rect[r])) {
                g_state.focused_param = r;
            }
            if (point_in_rect(g_state.mouse_screen, g_cfg_minus_rect[r])) {
                g_state.focused_param = r;
                if (can_adjust_param(cfg, r, -1)) adjust_focused_param(-1);
            }
            if (point_in_rect(g_state.mouse_screen, g_cfg_plus_rect[r])) {
                g_state.focused_param = r;
                if (can_adjust_param(cfg, r, 1)) adjust_focused_param(1);
            }
        }
        InvalidateRect(hwnd, NULL, FALSE);
    }
    return 1;
}

static int handle_menu_lbuttondown(HWND hwnd) {
    int idx;
    if (g_state.open_menu_id > 0) {
        for (idx = 0; idx < g_menu_item_count; idx++) {
            if (point_in_rect(g_state.mouse_screen, g_menu_item_rect[idx])) {
                if (g_menu_item_enabled[idx]) {
                    execute_menu_action(hwnd, g_state.open_menu_id, idx);
                } else {
                    push_console_log(L"[提示] 该菜单项当前不可用");
                }
                g_state.open_menu_id = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 1;
            }
        }
    }
    if (point_in_rect(g_state.mouse_screen, g_menu_file_rect)) {
        g_state.open_menu_id = (g_state.open_menu_id == 1) ? 0 : 1;
        g_state.open_menu_focus_index = (g_state.open_menu_id > 0) ? -1 : 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_menu_edit_rect)) {
        g_state.open_menu_id = (g_state.open_menu_id == 2) ? 0 : 2;
        g_state.open_menu_focus_index = (g_state.open_menu_id > 0) ? -1 : 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_menu_view_rect)) {
        g_state.open_menu_id = (g_state.open_menu_id == 3) ? 0 : 3;
        g_state.open_menu_focus_index = (g_state.open_menu_id > 0) ? -1 : 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_menu_physics_rect)) {
        g_state.open_menu_id = (g_state.open_menu_id == 5) ? 0 : 5;
        g_state.open_menu_focus_index = (g_state.open_menu_id > 0) ? -1 : 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_menu_window_rect)) {
        g_state.open_menu_id = (g_state.open_menu_id == 6) ? 0 : 6;
        g_state.open_menu_focus_index = (g_state.open_menu_id > 0) ? -1 : 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_menu_help_word_rect)) {
        g_state.open_menu_id = (g_state.open_menu_id == 7) ? 0 : 7;
        g_state.open_menu_focus_index = (g_state.open_menu_id > 0) ? -1 : 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (g_state.open_menu_id > 0) {
        g_state.open_menu_id = 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_menu_bar_drag_rect)) {
        ReleaseCapture();
        SendMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
        return 1;
    }
    return 0;
}

static int handle_layout_lbuttondown(HWND hwnd) {
    if (point_in_rect(g_state.mouse_screen, g_left_fold_rect)) {
        g_state.ui_show_left_panel = !g_state.ui_show_left_panel;
        save_ui_layout();
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_right_fold_rect)) {
        g_state.ui_show_right_panel = !g_state.ui_show_right_panel;
        save_ui_layout();
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (g_state.hierarchy_scroll_max > 0 && point_in_rect(g_state.mouse_screen, g_hierarchy_scroll_thumb_rect)) {
        g_state.hierarchy_scroll_dragging = 1;
        g_state.hierarchy_scroll_drag_start_y = g_state.mouse_screen.y;
        g_state.hierarchy_scroll_offset_start = g_state.hierarchy_scroll_offset;
        SetCapture(hwnd);
        return 1;
    }
    if (g_state.hierarchy_scroll_max > 0 && point_in_rect(g_state.mouse_screen, g_hierarchy_scroll_track_rect)) {
        float page = g_hierarchy_viewport_rect.bottom - g_hierarchy_viewport_rect.top;
        int step = (int)(page * 0.85f);
        if (step < 24) step = 24;
        if (g_state.mouse_screen.y < g_hierarchy_scroll_thumb_rect.top) g_state.hierarchy_scroll_offset -= step;
        else if (g_state.mouse_screen.y > g_hierarchy_scroll_thumb_rect.bottom) g_state.hierarchy_scroll_offset += step;
        if (g_state.hierarchy_scroll_offset < 0) g_state.hierarchy_scroll_offset = 0;
        if (g_state.hierarchy_scroll_offset > g_state.hierarchy_scroll_max) g_state.hierarchy_scroll_offset = g_state.hierarchy_scroll_max;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (g_state.ui_show_bottom_panel && !g_state.bottom_panel_collapsed && g_state.bottom_active_tab == 0 &&
        g_log_scroll_max > 0 && point_in_rect(g_state.mouse_screen, g_log_scroll_track_rect)) {
        float page = g_log_viewport_rect.bottom - g_log_viewport_rect.top;
        int step = (int)((page / 24.0f) * 0.85f);
        if (step < 1) step = 1;
        if (g_state.mouse_screen.y < g_log_scroll_thumb_rect.top) g_state.log_scroll_offset -= step;
        else if (g_state.mouse_screen.y > g_log_scroll_thumb_rect.bottom) g_state.log_scroll_offset += step;
        if (g_state.log_scroll_offset < 0) g_state.log_scroll_offset = 0;
        if (g_state.log_scroll_offset > g_log_scroll_max) g_state.log_scroll_offset = g_log_scroll_max;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_splitter_left_rect)) {
        g_state.ui_drag_splitter = 1;
        SetCapture(hwnd);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_splitter_right_rect)) {
        g_state.ui_drag_splitter = 2;
        SetCapture(hwnd);
        return 1;
    }
    if (g_state.ui_show_bottom_panel && !g_state.bottom_panel_collapsed && point_in_rect(g_state.mouse_screen, g_splitter_bottom_rect)) {
        g_state.ui_drag_splitter = 3;
        SetCapture(hwnd);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_win_close_rect)) {
        PostMessageW(hwnd, WM_CLOSE, 0, 0);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_win_min_rect)) {
        ShowWindow(hwnd, SW_MINIMIZE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_win_max_rect)) {
        ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
        return 1;
    }
    return 0;
}

static int handle_top_toolbar_lbuttondown(HWND hwnd) {
    if (point_in_rect(g_state.mouse_screen, g_top_run_rect)) {
        AppCommand cmd;
        if (input_try_map_toolbar_command(TOOLBAR_ACTION_TOGGLE_RUN, &cmd)) {
            app_runtime_dispatch(&g_app_runtime, cmd);
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_step_rect)) {
        if (g_state.engine != NULL) {
            LARGE_INTEGER q0;
            LARGE_INTEGER q1;
            LARGE_INTEGER fq;
            double ms = 0.0;
            QueryPerformanceCounter(&q0);
            physics_engine_step(g_state.engine);
            QueryPerformanceCounter(&q1);
            QueryPerformanceFrequency(&fq);
            if (fq.QuadPart > 0) {
                ms = (double)(q1.QuadPart - q0.QuadPart) * 1000.0 / (double)fq.QuadPart;
                g_state.physics_step_ms = (float)ms;
            }
            cleanup_constraint_selection();
            capture_collision_events();
        }
        push_console_log(L"[状态] 单步执行 1 帧");
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_reset_rect)) {
        AppCommand cmd;
        if (input_try_map_toolbar_command(TOOLBAR_ACTION_RESET_SCENE, &cmd)) {
            app_runtime_dispatch(&g_app_runtime, cmd);
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_save_rect)) {
        if (save_scene_snapshot("scene_snapshot.txt")) push_console_log(L"[命令] 已保存 scene_snapshot.txt");
        else push_console_log(L"[错误] 保存 scene_snapshot.txt 失败");
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_undo_rect)) {
        history_undo();
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_redo_rect)) {
        history_redo();
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_grid_rect)) {
        g_state.draw_centers = !g_state.draw_centers;
        push_console_log(L"[视图] 网格/中心:%s", g_state.draw_centers ? L"开" : L"关");
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_collision_rect)) {
        g_state.draw_contacts = !g_state.draw_contacts;
        push_console_log(L"[视图] 碰撞点:%s", g_state.draw_contacts ? L"开" : L"关");
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_velocity_rect)) {
        g_state.draw_velocity = !g_state.draw_velocity;
        push_console_log(L"[视图] 速度向量:%s", g_state.draw_velocity ? L"开" : L"关");
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_constraint_rect)) {
        if (g_state.selected != NULL && g_state.selected->type == BODY_DYNAMIC) {
            try_create_constraint_from_selection(1);
        } else {
            push_console_log(L"[提示] 请先选中一个动态物体，再点击约束按钮");
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_spring_rect)) {
        if (g_state.selected != NULL && g_state.selected->type == BODY_DYNAMIC) {
            try_create_constraint_from_selection(2);
        } else {
            push_console_log(L"[提示] 请先选中一个动态物体，再点击弹性约束按钮");
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_chain_rect)) {
        if (g_state.selected != NULL && g_state.selected->type == BODY_DYNAMIC) {
            try_create_constraint_from_selection(3);
        } else {
            push_console_log(L"[提示] 请先选中一个动态物体，再点击链条按钮");
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_rope_rect)) {
        if (g_state.selected != NULL && g_state.selected->type == BODY_DYNAMIC) {
            try_create_constraint_from_selection(4);
        } else {
            push_console_log(L"[提示] 请先选中一个动态物体，再点击橡皮绳按钮");
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    return 0;
}

static int handle_bottom_console_lbuttondown(HWND hwnd) {
    int idx;
    if (point_in_rect(g_state.mouse_screen, g_bottom_fold_rect)) {
        g_state.bottom_panel_collapsed = !g_state.bottom_panel_collapsed;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_bottom_tab_console_rect)) {
        g_state.bottom_active_tab = 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_bottom_tab_perf_rect)) {
        g_state.bottom_active_tab = 1;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (g_state.ui_show_bottom_panel && !g_state.bottom_panel_collapsed && g_state.bottom_active_tab == 0) {
        if (point_in_rect(g_state.mouse_screen, g_log_filter_all_rect)) {
            g_state.log_filter_mode = 0;
            g_state.log_scroll_offset = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
        if (point_in_rect(g_state.mouse_screen, g_log_filter_state_rect)) {
            g_state.log_filter_mode = 1;
            g_state.log_scroll_offset = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
        if (point_in_rect(g_state.mouse_screen, g_log_filter_physics_rect)) {
            g_state.log_filter_mode = 2;
            g_state.log_scroll_offset = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
        if (point_in_rect(g_state.mouse_screen, g_log_filter_collision_rect)) {
            g_state.log_filter_mode = 4;
            g_state.log_scroll_offset = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
        if (point_in_rect(g_state.mouse_screen, g_log_filter_warn_rect)) {
            g_state.log_filter_mode = 3;
            g_state.log_scroll_offset = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
        if (point_in_rect(g_state.mouse_screen, g_log_clear_rect)) {
            g_console_log_count = 0;
            g_console_log_head = 0;
            g_state.log_scroll_offset = 0;
            push_console_log(L"[日志] 已清空");
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
    }
    if (!g_state.bottom_panel_collapsed && g_state.bottom_active_tab == 1) {
        if (point_in_rect(g_state.mouse_screen, g_perf_export_rect)) {
            export_perf_report_csv();
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
    }
    if (point_in_rect(g_state.mouse_screen, g_dbg_collision_filter_rect)) {
        g_collision_event_filter_selected_only = !g_collision_event_filter_selected_only;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    for (idx = 0; idx < g_dbg_collision_row_count; idx++) {
        if (point_in_rect(g_state.mouse_screen, g_dbg_collision_row_rect[idx])) {
            int ev_idx = g_dbg_collision_row_event_index[idx];
            if (ev_idx >= 0 && ev_idx < COLLISION_EVENT_CAP) {
                const CollisionEvent* ev = &g_collision_events[ev_idx];
                int target = (ev->body_a_index >= 0) ? ev->body_a_index : ev->body_b_index;
                if (target >= 0) {
                    select_body_by_index(target);
                    push_console_log(L"[选中] 碰撞日志对象 #%d", target + 1);
                }
                InvalidateRect(hwnd, NULL, FALSE);
                return 1;
            }
        }
    }
    return 0;
}

static int handle_inspector_debug_lbuttondown(HWND hwnd) {
    int idx;
    for (idx = 0; point_in_rect(g_state.mouse_screen, g_inspector_viewport_rect) && idx < g_ins_row_count && idx < INSPECTOR_MAX_ROWS; idx++) {
        if (point_in_rect(g_state.mouse_screen, g_ins_row_rect[idx])) {
            g_state.inspector_focused_row = idx;
            g_state.keyboard_focus_area = 1;
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
    }
    for (idx = 0; point_in_rect(g_state.mouse_screen, g_debug_viewport_rect) && idx < 3; idx++) {
        if (point_in_rect(g_state.mouse_screen, g_dbg_row_rect[idx])) {
            g_state.debug_focused_row = idx;
            g_state.keyboard_focus_area = 2;
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
    }
    return 0;
}

static int handle_tree_and_search_lbuttondown(HWND hwnd) {
    int idx;
    if (point_in_rect(g_state.mouse_screen, g_hierarchy_search_rect)) {
        begin_value_input_for_hierarchy_filter();
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_hierarchy_search_clear_rect)) {
        g_state.hierarchy_filter_buf[0] = L'\0';
        g_state.hierarchy_filter_len = 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_tree_scene_header_rect)) {
        g_state.tree_scene_expanded = !g_state.tree_scene_expanded;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_tree_bodies_header_rect)) {
        g_state.tree_bodies_expanded = !g_state.tree_bodies_expanded;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_tree_body_circle_header_rect)) {
        g_state.tree_body_circle_expanded = !g_state.tree_body_circle_expanded;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_tree_body_polygon_header_rect)) {
        g_state.tree_body_polygon_expanded = !g_state.tree_body_polygon_expanded;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_tree_constraints_header_rect)) {
        g_state.tree_constraints_expanded = !g_state.tree_constraints_expanded;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_tree_constraint_distance_header_rect)) {
        g_state.tree_constraint_distance_expanded = !g_state.tree_constraint_distance_expanded;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_tree_constraint_spring_header_rect)) {
        g_state.tree_constraint_spring_expanded = !g_state.tree_constraint_spring_expanded;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_tree_project_root_rect)) {
        g_state.tree_project_expanded = !g_state.tree_project_expanded;
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_hierarchy_viewport_rect) && point_in_rect(g_state.mouse_screen, g_explorer_scene_rect)) {
        apply_scene((g_state.scene_index + 1) % SCENE_COUNT);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    for (idx = 0; idx < g_explorer_body_count && point_in_rect(g_state.mouse_screen, g_hierarchy_viewport_rect); idx++) {
        if (point_in_rect(g_state.mouse_screen, g_explorer_body_rect[idx])) {
            g_state.selected = g_explorer_body_ptr[idx];
            g_state.selected_constraint_index = -1;
            g_state.dragging = 0;
            g_state.inspector_focused_row = 0;
            g_state.keyboard_focus_area = 1;
            push_console_log(L"[选中] 物体已选中");
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
    }
    for (idx = 0; idx < g_explorer_constraint_count && point_in_rect(g_state.mouse_screen, g_hierarchy_viewport_rect); idx++) {
        if (point_in_rect(g_state.mouse_screen, g_explorer_constraint_rect[idx])) {
            g_state.selected_constraint_index = g_explorer_constraint_index[idx];
            g_state.selected = NULL;
            g_state.dragging = 0;
            g_state.inspector_focused_row = 0;
            g_state.keyboard_focus_area = 1;
            push_console_log(L"[选中] 约束已选中");
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
    }
    return 0;
}

static int handle_stage_pick_lbuttondown(HWND hwnd) {
    if (g_state.mouse_screen.x >= g_state.stage_left &&
        g_state.mouse_screen.x <= g_state.stage_right &&
        g_state.mouse_screen.y >= g_state.stage_top &&
        g_state.mouse_screen.y <= g_state.stage_bottom &&
        g_state.engine != NULL) {
        Vec2 p;
        SetCapture(hwnd);
        p = screen_to_world(g_state.mouse_screen);
        g_state.selected = pick_dynamic_body(g_state.engine, p);
        g_state.dragging = (g_state.selected != NULL);
        if (!g_state.dragging) {
            g_state.selected_constraint_index = pick_constraint_at_point(g_state.engine, p);
            g_state.inspector_focused_row = 0;
            g_state.keyboard_focus_area = 1;
            if (g_state.selected_constraint_index >= 0) push_console_log(L"[选中] 舞台约束选中");
        } else {
            g_state.selected_constraint_index = -1;
            g_state.inspector_focused_row = 0;
            g_state.keyboard_focus_area = 1;
            push_console_log(L"[选中] 舞台物体选中");
        }
        return 1;
    }
    return 0;
}

static int handle_keydown_log_paging(WPARAM wparam) {
    int changed = 0;
    if (g_state.ui_show_bottom_panel && !g_state.bottom_panel_collapsed && g_state.bottom_active_tab == 0) {
        int delta = 0;
        if (input_try_map_log_paging((unsigned int)wparam, &delta)) {
            g_state.log_scroll_offset += delta;
            if (g_state.log_scroll_offset > g_log_scroll_max) g_state.log_scroll_offset = g_log_scroll_max;
            if (g_state.log_scroll_offset < 0) g_state.log_scroll_offset = 0;
            changed = 1;
        }
    }
    return changed;
}

static int handle_keydown_runtime_controls(WPARAM wparam) {
    int changed = 0;
    if (wparam == VK_SPACE || wparam == 'R') {
        AppCommand cmd;
        if (input_try_map_keydown_command((unsigned int)wparam, &cmd)) {
            app_runtime_dispatch(&g_app_runtime, cmd);
            changed = 1;
        }
    }
    if (wparam == 'N' && g_state.engine) {
        LARGE_INTEGER q0;
        LARGE_INTEGER q1;
        LARGE_INTEGER fq;
        double ms = 0.0;
        QueryPerformanceCounter(&q0);
        physics_engine_step(g_state.engine);
        QueryPerformanceCounter(&q1);
        QueryPerformanceFrequency(&fq);
        if (fq.QuadPart > 0) {
            ms = (double)(q1.QuadPart - q0.QuadPart) * 1000.0 / (double)fq.QuadPart;
            g_state.physics_step_ms = (float)ms;
        }
        cleanup_constraint_selection();
        capture_collision_events();
        push_console_log(L"[状态] 单步执行 1 帧");
        changed = 1;
    }
    {
        RuntimeToggleAction action = RUNTIME_TOGGLE_NONE;
        if (input_try_map_runtime_toggle((unsigned int)wparam, &action)) {
            switch (action) {
                case RUNTIME_TOGGLE_DRAW_CONTACTS:
                    g_state.draw_contacts = !g_state.draw_contacts;
                    changed = 1;
                    break;
                case RUNTIME_TOGGLE_DRAW_VELOCITY:
                    g_state.draw_velocity = !g_state.draw_velocity;
                    changed = 1;
                    break;
                case RUNTIME_TOGGLE_DRAW_CENTERS:
                    g_state.draw_centers = !g_state.draw_centers;
                    changed = 1;
                    break;
                case RUNTIME_TOGGLE_DRAW_CONSTRAINTS:
                    g_state.draw_constraints = !g_state.draw_constraints;
                    changed = 1;
                    break;
                case RUNTIME_TOGGLE_BOTTOM_PANEL:
                    g_state.bottom_panel_collapsed = !g_state.bottom_panel_collapsed;
                    changed = 1;
                    break;
                case RUNTIME_TOGGLE_NEXT_LAYOUT:
                    apply_layout_preset(g_state.ui_layout_preset + 1);
                    save_ui_layout();
                    changed = 1;
                    break;
                default:
                    break;
            }
        }
    }
    return changed;
}

static int handle_keydown_inspector_controls(WPARAM wparam) {
    int changed = 0;
    if (g_state.keyboard_focus_area == 2) {
        if (wparam == VK_UP) { g_state.debug_focused_row = (g_state.debug_focused_row + 2) % 3; changed = 1; }
        if (wparam == VK_DOWN) { g_state.debug_focused_row = (g_state.debug_focused_row + 1) % 3; changed = 1; }
        if (wparam == VK_RETURN) { begin_value_input_for_debug_param(g_state.debug_focused_row); changed = 1; }
        return changed;
    }
    if (inspector_row_count() > 0) {
        int rcnt = inspector_row_count();
        if (wparam == VK_UP) { g_state.inspector_focused_row = (g_state.inspector_focused_row + rcnt - 1) % rcnt; changed = 1; }
        if (wparam == VK_DOWN) { g_state.inspector_focused_row = (g_state.inspector_focused_row + 1) % rcnt; changed = 1; }
        if (wparam == VK_RETURN) { begin_value_input_for_inspector_row(g_state.inspector_focused_row); changed = 1; }
    }
    return changed;
}

static int handle_keydown_spawn_constraint_delete(WPARAM wparam) {
    int changed = 0;
    if (wparam == '1' || wparam == '2') {
        AppCommand cmd;
        if (input_try_map_keydown_command((unsigned int)wparam, &cmd)) {
            app_runtime_dispatch(&g_app_runtime, cmd);
            changed = 1;
        }
    }
    {
        int constraint_kind = 0;
        if (input_try_map_constraint_kind((unsigned int)wparam, &constraint_kind)) {
            try_create_constraint_from_selection(constraint_kind);
            changed = 1;
        }
    }
    {
        float len_delta = 0.0f;
        float stiffness_delta = 0.0f;
        if (input_try_map_constraint_param_delta((unsigned int)wparam, &len_delta, &stiffness_delta)) {
            update_selected_constraint_param(len_delta, stiffness_delta);
            changed = 1;
        }
    }
    if (wparam == 'U' && selected_constraint_ref() != NULL) {
        physics_engine_constraint_set_collide_connected(
            g_state.engine,
            g_state.selected_constraint_index,
            !physics_engine_constraint_get_collide_connected(g_state.engine, g_state.selected_constraint_index));
        changed = 1;
    }
    if (wparam == 'B') { cycle_selected_constraint_break_force(); changed = 1; }
    if (wparam == VK_DELETE && g_state.engine != NULL) {
        if (selected_constraint_ref() != NULL) {
            remove_selected_constraint();
            changed = 1;
        } else if (g_state.selected != NULL) {
            history_push_snapshot();
            physics_engine_remove_body(g_state.engine, g_state.selected);
            push_console_log(L"[删除] 已删除选中物体");
            g_state.selected = NULL;
            g_state.dragging = 0;
            cleanup_constraint_selection();
            changed = 1;
        }
    }
    return changed;
}

static int handle_keydown_scene_switch(WPARAM wparam) {
    int changed = 0;
    int scene_index = 0;
    int scene_step = 0;
    if (input_try_map_scene_index((unsigned int)wparam, &scene_index)) {
        apply_scene(scene_index);
        changed = 1;
    }
    if (input_try_map_scene_step((unsigned int)wparam, &scene_step)) {
        apply_scene((g_state.scene_index + scene_step + SCENE_COUNT) % SCENE_COUNT);
        changed = 1;
    }
    return changed;
}

static int handle_mousemove_scroll_drag(HWND hwnd) {
    if (g_state.hierarchy_scroll_dragging && g_state.hierarchy_scroll_max > 0) {
        float track_h = g_hierarchy_scroll_track_rect.bottom - g_hierarchy_scroll_track_rect.top;
        float thumb_h = g_hierarchy_scroll_thumb_rect.bottom - g_hierarchy_scroll_thumb_rect.top;
        float travel = track_h - thumb_h;
        if (travel > 1.0f) {
            float dy = g_state.mouse_screen.y - g_state.hierarchy_scroll_drag_start_y;
            int off = g_state.hierarchy_scroll_offset_start + (int)(dy * (float)g_state.hierarchy_scroll_max / travel);
            if (off < 0) off = 0;
            if (off > g_state.hierarchy_scroll_max) off = g_state.hierarchy_scroll_max;
            g_state.hierarchy_scroll_offset = off;
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    return 0;
}

static int handle_mousemove_splitter_drag(HWND hwnd) {
    if (g_state.ui_drag_splitter != 0) {
        RECT cr;
        float cw;
        float ch;
        GetClientRect(hwnd, &cr);
        cw = (float)(cr.right - cr.left);
        ch = (float)(cr.bottom - cr.top);
        if (g_state.ui_drag_splitter == 1) {
            g_state.ui_left_ratio = clamp((g_state.mouse_screen.x - 8.0f) / cw, 0.10f, 0.28f);
        } else if (g_state.ui_drag_splitter == 2) {
            g_state.ui_right_ratio = clamp((cw - g_state.mouse_screen.x - 8.0f) / cw, 0.12f, 0.34f);
        } else if (g_state.ui_drag_splitter == 3) {
            float status_top = ch - 32.0f - 8.0f;
            g_state.ui_bottom_open_h = clamp(status_top - g_state.mouse_screen.y - 8.0f, 96.0f, 260.0f);
            g_state.bottom_panel_collapsed = 0;
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    return 0;
}

static void scroll_by_wheel_step(int* offset, int max, int delta, int step) {
    if (delta < 0) *offset += step;
    if (delta > 0) *offset -= step;
    if (*offset < 0) *offset = 0;
    if (*offset > max) *offset = max;
}

static int handle_mousewheel_hierarchy(HWND hwnd, int delta) {
    if ((point_in_rect(g_state.mouse_screen, g_hierarchy_viewport_rect) || point_in_rect(g_state.mouse_screen, g_hierarchy_scroll_track_rect))
        && g_state.hierarchy_scroll_max > 0) {
        scroll_by_wheel_step(&g_state.hierarchy_scroll_offset, g_state.hierarchy_scroll_max, delta, 24);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    return 0;
}

static int handle_mousewheel_project(HWND hwnd, int delta) {
    if ((point_in_rect(g_state.mouse_screen, g_project_viewport_rect) || point_in_rect(g_state.mouse_screen, g_project_scroll_track_rect))
        && g_state.project_scroll_max > 0) {
        scroll_by_wheel_step(&g_state.project_scroll_offset, g_state.project_scroll_max, delta, 24);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    return 0;
}

static int handle_mousewheel_inspector(HWND hwnd, int delta) {
    if ((point_in_rect(g_state.mouse_screen, g_inspector_viewport_rect) || point_in_rect(g_state.mouse_screen, g_inspector_scroll_track_rect))
        && g_state.inspector_scroll_max > 0) {
        scroll_by_wheel_step(&g_state.inspector_scroll_offset, g_state.inspector_scroll_max, delta, 24);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    return 0;
}

static int handle_mousewheel_debug(HWND hwnd, int delta) {
    if ((point_in_rect(g_state.mouse_screen, g_debug_viewport_rect) || point_in_rect(g_state.mouse_screen, g_debug_scroll_track_rect))
        && g_state.debug_scroll_max > 0) {
        scroll_by_wheel_step(&g_state.debug_scroll_offset, g_state.debug_scroll_max, delta, 24);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    return 0;
}

static int handle_mousewheel_log(HWND hwnd, int delta) {
    if (g_state.ui_show_bottom_panel && !g_state.bottom_panel_collapsed && g_state.bottom_active_tab == 0) {
        if (point_in_rect(g_state.mouse_screen, g_log_viewport_rect) || point_in_rect(g_state.mouse_screen, g_log_scroll_track_rect)) {
            scroll_by_wheel_step(&g_state.log_scroll_offset, g_log_scroll_max, delta, 1);
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
    }
    return 0;
}

static int handle_mousewheel_dispatch(HWND hwnd, int delta) {
    if (handle_mousewheel_hierarchy(hwnd, delta)) return 1;
    if (handle_mousewheel_project(hwnd, delta)) return 1;
    if (handle_mousewheel_inspector(hwnd, delta)) return 1;
    if (handle_mousewheel_debug(hwnd, delta)) return 1;
    if (handle_mousewheel_log(hwnd, delta)) return 1;
    return 0;
}

static int handle_syskeydown(HWND hwnd, WPARAM wparam) {
    int mid = 0;
    if (wparam == 'F') mid = 1;
    if (wparam == 'E') mid = 2;
    if (wparam == 'G') mid = 3;
    if (wparam == 'C') mid = 4;
    if (wparam == 'P') mid = 5;
    if (wparam == 'W') mid = 6;
    if (wparam == 'H') mid = 7;
    if (wparam == '1') {
        g_state.ui_show_left_panel = !g_state.ui_show_left_panel;
        save_ui_layout();
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == '2') {
        g_state.ui_show_right_panel = !g_state.ui_show_right_panel;
        save_ui_layout();
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == '3') {
        g_state.ui_show_bottom_panel = !g_state.ui_show_bottom_panel;
        save_ui_layout();
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == 'T') {
        g_state.ui_theme_light = !g_state.ui_theme_light;
        save_ui_layout();
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (mid > 0) {
        g_state.open_menu_id = mid;
        g_state.open_menu_focus_index = menu_find_enabled_from(mid, 0, 1);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    return 0;
}

static int handle_lbuttondblclk(HWND hwnd) {
    int idx;
    if (g_state.show_value_input || g_state.show_config_modal || g_state.show_help_modal) return 1;
    if (point_in_rect(g_state.mouse_screen, g_top_run_rect)) {
        AppCommand cmd;
        if (input_try_map_keydown_command((unsigned int)VK_SPACE, &cmd)) {
            app_runtime_dispatch(&g_app_runtime, cmd);
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_menu_bar_drag_rect)) {
        ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
        return 1;
    }
    for (idx = 0; point_in_rect(g_state.mouse_screen, g_inspector_viewport_rect) && idx < g_ins_row_count && idx < INSPECTOR_MAX_ROWS; idx++) {
        if (point_in_rect(g_state.mouse_screen, g_ins_row_rect[idx])) {
            g_state.inspector_focused_row = idx;
            g_state.keyboard_focus_area = 1;
            begin_value_input_for_inspector_row(idx);
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
    }
    for (idx = 0; point_in_rect(g_state.mouse_screen, g_debug_viewport_rect) && idx < 3; idx++) {
        if (point_in_rect(g_state.mouse_screen, g_dbg_row_rect[idx])) {
            g_state.debug_focused_row = idx;
            g_state.keyboard_focus_area = 2;
            begin_value_input_for_debug_param(idx);
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
    }
    return 0;
}

static int handle_keydown(HWND hwnd, WPARAM wparam) {
    if (handle_ctrl_shortcuts_keydown(hwnd, wparam)) return 1;
    if (handle_value_input_keydown(hwnd, wparam)) return 1;
    if (handle_open_menu_keydown(hwnd, wparam)) return 1;
    if (handle_modal_keydown(hwnd, wparam)) return 1;
    handle_keydown_log_paging(wparam);
    handle_keydown_runtime_controls(wparam);
    handle_keydown_inspector_controls(wparam);
    handle_keydown_spawn_constraint_delete(wparam);
    handle_keydown_scene_switch(wparam);
    return 1;
}

static int handle_lbuttondown(HWND hwnd) {
    if (handle_modal_lbuttondown(hwnd)) return 1;
    if (handle_layout_lbuttondown(hwnd)) return 1;
    if (handle_menu_lbuttondown(hwnd)) return 1;
    if (handle_top_toolbar_lbuttondown(hwnd)) return 1;
    if (handle_tree_and_search_lbuttondown(hwnd)) return 1;
    if (handle_bottom_console_lbuttondown(hwnd)) return 1;
    if (handle_inspector_debug_lbuttondown(hwnd)) return 1;
    if (handle_stage_pick_lbuttondown(hwnd)) return 1;
    return 1;
}

static int handle_char(HWND hwnd, WPARAM wparam) {
    if (g_state.show_value_input) {
        wchar_t ch = (wchar_t)wparam;
        int allow = 0;
        if (g_state.value_input_target == 2 || g_state.value_input_target == 3) {
            allow = (ch >= 32 && ch != 127);
        } else {
            allow = ((ch >= L'0' && ch <= L'9') || ch == L'.' || ch == L'-' || ch == L'+');
        }
        if (allow && g_state.value_input_len < 63) {
            int i;
            for (i = g_state.value_input_len; i > g_state.value_input_caret; i--) {
                g_state.value_input_buf[i] = g_state.value_input_buf[i - 1];
            }
            g_state.value_input_buf[g_state.value_input_caret] = ch;
            g_state.value_input_len++;
            g_state.value_input_caret++;
            g_state.value_input_buf[g_state.value_input_len] = L'\0';
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 1;
    }
    return 1;
}

static int handle_mousemove(HWND hwnd, LPARAM lparam) {
    g_state.mouse_screen.x = (float)GET_X_LPARAM(lparam);
    g_state.mouse_screen.y = (float)GET_Y_LPARAM(lparam);
    if (handle_mousemove_scroll_drag(hwnd)) return 1;
    if (handle_mousemove_splitter_drag(hwnd)) return 1;
    return 1;
}

static int handle_mousewheel(HWND hwnd, WPARAM wparam, LPARAM lparam) {
    int delta = GET_WHEEL_DELTA_WPARAM(wparam);
    POINT sp;
    sp.x = GET_X_LPARAM(lparam);
    sp.y = GET_Y_LPARAM(lparam);
    ScreenToClient(hwnd, &sp);
    g_state.mouse_screen.x = (float)sp.x;
    g_state.mouse_screen.y = (float)sp.y;
    if (handle_mousewheel_dispatch(hwnd, delta)) return 1;
    return 1;
}

static int handle_setcursor(HWND hwnd, LPARAM lparam) {
    POINT cp;
    if (LOWORD(lparam) == HTCLIENT) {
        GetCursorPos(&cp);
        ScreenToClient(hwnd, &cp);
        g_state.mouse_screen.x = (float)cp.x;
        g_state.mouse_screen.y = (float)cp.y;
        if (point_in_rect(g_state.mouse_screen, g_splitter_left_rect) || point_in_rect(g_state.mouse_screen, g_splitter_right_rect)) {
            SetCursor(LoadCursorW(NULL, (LPCWSTR)IDC_SIZEWE));
            return 1;
        }
        if (point_in_rect(g_state.mouse_screen, g_splitter_bottom_rect)) {
            SetCursor(LoadCursorW(NULL, (LPCWSTR)IDC_SIZENS));
            return 1;
        }
    }
    return 0;
}

static int handle_lbuttonup(void) {
    ReleaseCapture();
    if (g_state.ui_drag_splitter != 0) save_ui_layout();
    g_state.ui_drag_splitter = 0;
    g_state.hierarchy_scroll_dragging = 0;
    g_state.dragging = 0;
    return 1;
}

static int handle_timer(HWND hwnd) {
    tick(hwnd);
    return 1;
}

static LRESULT handle_syskeydown_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (handle_syskeydown(hwnd, wparam)) return 0;
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

static int handle_lbuttondblclk_message(HWND hwnd, LPARAM lparam) {
    g_state.mouse_screen.x = (float)GET_X_LPARAM(lparam);
    g_state.mouse_screen.y = (float)GET_Y_LPARAM(lparam);
    if (handle_lbuttondblclk(hwnd)) return 1;
    return 1;
}

static LRESULT handle_nchittest(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    LRESULT hit = DefWindowProcW(hwnd, msg, wparam, lparam);
    if (hit == HTCLIENT) {
        POINT sp;
        RECT cr;
        int border = IsZoomed(hwnd) ? 0 : 6;
        ScreenPt mp;
        sp.x = GET_X_LPARAM(lparam);
        sp.y = GET_Y_LPARAM(lparam);
        ScreenToClient(hwnd, &sp);
        mp.x = (float)sp.x;
        mp.y = (float)sp.y;
        GetClientRect(hwnd, &cr);
        if (border > 0) {
            int on_left = (sp.x >= 0 && sp.x < border);
            int on_right = (sp.x < cr.right && sp.x >= cr.right - border);
            int on_top = (sp.y >= 0 && sp.y < border);
            int on_bottom = (sp.y < cr.bottom && sp.y >= cr.bottom - border);
            if (on_top && on_left) return HTTOPLEFT;
            if (on_top && on_right) return HTTOPRIGHT;
            if (on_bottom && on_left) return HTBOTTOMLEFT;
            if (on_bottom && on_right) return HTBOTTOMRIGHT;
            if (on_top) return HTTOP;
            if (on_bottom) return HTBOTTOM;
            if (on_left) return HTLEFT;
            if (on_right) return HTRIGHT;
        }
        if (point_in_rect(mp, g_win_min_rect) || point_in_rect(mp, g_win_max_rect) || point_in_rect(mp, g_win_close_rect)) {
            return HTCLIENT;
        }
        if (point_in_rect(mp, g_menu_bar_drag_rect)) {
            return HTCAPTION;
        }
    }
    return hit;
}

static int handle_paint(HWND hwnd) {
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    render(hwnd);
    EndPaint(hwnd, &ps);
    return 1;
}

static int handle_size(LPARAM lparam) {
    if (g_ui.target != NULL) {
        D2D1_SIZE_U size = {(UINT32)LOWORD(lparam), (UINT32)HIWORD(lparam)};
        ID2D1HwndRenderTarget_Resize(g_ui.target, &size);
    }
    return 1;
}

static int handle_destroy(void) {
    if (g_app_icon_small != NULL) {
        DestroyIcon(g_app_icon_small);
        g_app_icon_small = NULL;
    }
    if (g_app_icon_large != NULL) {
        DestroyIcon(g_app_icon_large);
        g_app_icon_large = NULL;
    }
    PostQuitMessage(0);
    return 1;
}

static void init_status_context(void) {
    g_status_project_path[0] = L'\0';
    g_status_user[0] = L'\0';
    GetCurrentDirectoryW(260, g_status_project_path);
    {
        DWORD uname_len = 63;
        if (!GetUserNameW(g_status_user, &uname_len)) {
            g_status_user[0] = L'\0';
        }
    }
}

static int init_platform_ui(void) {
    SetProcessDPIAware();
    if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) return 0;
    if (FAILED(init_ui())) {
        CoUninitialize();
        return 0;
    }
    return 1;
}

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static void init_app_runtime_callbacks(void);
static void init_state_defaults(void);

static int register_main_window_class(HINSTANCE inst, WNDCLASSEXW* out_wc) {
    if (out_wc == NULL) return 0;
    ZeroMemory(out_wc, sizeof(*out_wc));
    out_wc->cbSize = sizeof(*out_wc);
    out_wc->style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    out_wc->lpfnWndProc = wnd_proc;
    out_wc->hInstance = inst;
    g_app_icon_large = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(IDI_APP_MAIN), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    g_app_icon_small = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(IDI_APP_MAIN), IMAGE_ICON,
                                         GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    out_wc->hIcon = g_app_icon_large ? g_app_icon_large : LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
    out_wc->hIconSm = g_app_icon_small ? g_app_icon_small : out_wc->hIcon;
    out_wc->hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    out_wc->hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    out_wc->lpszClassName = L"PhysicsSandboxDWrite";
    return RegisterClassExW(out_wc) ? 1 : 0;
}

static void init_app_bootstrap_state(void) {
    init_state_defaults();
    init_app_runtime_callbacks();
    clear_collision_events();
    g_collision_event_filter_selected_only = 0;
    g_clipboard_body.valid = 0;
    load_ui_layout();
    push_console_log(L"[启动] 物理沙盒已启动");
    if (file_exists_utf8_path("autosave_snapshot.txt")) {
        push_console_log(L"[启动] 检测到 autosave_snapshot.txt，可在 文件 菜单恢复");
    }
    apply_scene(0);
}

static HWND create_main_window(HINSTANCE inst, const wchar_t* class_name) {
    HWND hwnd = CreateWindowExW(0, class_name, L"物理引擎沙盒（DirectWrite）",
                                (WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU),
                                CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, inst, NULL);
    if (hwnd == NULL) return NULL;
    g_app_hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);
    SetTimer(hwnd, 1, 16, NULL);
    return hwnd;
}

static int run_message_loop(void) {
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

static int shutdown_and_get_exit_code(int exit_code) {
    if (g_state.engine) physics_engine_free(g_state.engine);
    save_ui_layout();
    shutdown_ui();
    CoUninitialize();
    return exit_code;
}

static void init_app_runtime_callbacks(void) {
    AppCommandCallbacks callbacks;
    callbacks.toggle_run = app_cmd_toggle_run;
    callbacks.reset_scene = app_cmd_reset_scene;
    callbacks.spawn_circle = app_cmd_spawn_circle;
    callbacks.spawn_box = app_cmd_spawn_box;
    callbacks.user = NULL;
    app_runtime_init(&g_app_runtime, callbacks);
}

static void init_state_defaults(void) {
    scene_catalog_copy_defaults(g_state.scenes, SCENE_COUNT);
    srand((unsigned int)time(NULL));
    g_state.running = 0;
    g_state.draw_contacts = 0;
    g_state.draw_velocity = 0;
    g_state.draw_centers = 0;
    g_state.draw_constraints = 1;
    g_state.bottom_panel_collapsed = 1;
    g_state.bottom_active_tab = 0;
    g_state.ui_show_left_panel = 1;
    g_state.ui_show_right_panel = 1;
    g_state.ui_show_bottom_panel = 1;
    g_state.ui_theme_light = 0;
    g_state.ui_left_ratio = 0.18f;
    g_state.ui_right_ratio = 0.25f;
    g_state.ui_bottom_open_h = 132.0f;
    g_state.ui_drag_splitter = 0;
    g_state.ui_layout_preset = 0;
    g_state.log_filter_mode = 0;
    g_state.log_scroll_offset = 0;
    g_state.hierarchy_scroll_offset = 0;
    g_state.hierarchy_scroll_max = 0;
    g_state.hierarchy_scroll_dragging = 0;
    g_state.hierarchy_scroll_drag_start_y = 0.0f;
    g_state.hierarchy_scroll_offset_start = 0;
    g_state.tree_scene_expanded = 1;
    g_state.tree_bodies_expanded = 1;
    g_state.tree_constraints_expanded = 1;
    g_state.tree_project_expanded = 1;
    g_state.tree_body_circle_expanded = 1;
    g_state.tree_body_polygon_expanded = 1;
    g_state.tree_constraint_distance_expanded = 1;
    g_state.tree_constraint_spring_expanded = 1;
    g_state.open_menu_id = 0;
    g_state.open_menu_focus_index = -1;
    g_state.show_config_modal = 0;
    g_state.show_help_modal = 0;
    g_state.help_modal_page = 0;
    g_state.show_value_input = 0;
    g_state.value_input_target = 0;
    g_state.value_input_row = 0;
    g_state.value_input_len = 0;
    g_state.value_input_caret = 0;
    g_state.log_search_buf[0] = L'\0';
    g_state.log_search_len = 0;
    g_state.hierarchy_filter_buf[0] = L'\0';
    g_state.hierarchy_filter_len = 0;
    g_state.fps_display = 0;
    g_state.physics_step_ms = 0.0f;
    g_state.fps_last_tick_ms = 0;
    g_state.fps_accum_frames = 0;
    g_state.last_contact_count = 0;
    g_state.last_contact_log_ms = 0;
    g_state.last_collision_capture_ms = 0;
    g_state.last_autosave_ms = (unsigned int)GetTickCount();
    g_state.history_cursor = 0;
    g_state.history_top = 0;
    g_state.history_count = 0;
    g_state.history_replaying = 0;
    g_state.constraint_break_force_default = 250.0f;
    g_state.spring_preset_mode = 1;
    g_state.debug_focused_row = 0;
    g_state.keyboard_focus_area = 1;
}

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_NCCALCSIZE:
            return 0;
        case WM_NCHITTEST:
            return handle_nchittest(hwnd, msg, wparam, lparam);
        case WM_PAINT:
            handle_paint(hwnd);
            return 0;
        case WM_SIZE:
            handle_size(lparam);
            return 0;
        case WM_TIMER:
            handle_timer(hwnd);
            return 0;
        case WM_KEYDOWN:
            handle_keydown(hwnd, wparam);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        case WM_CHAR:
            handle_char(hwnd, wparam);
            return 0;
        case WM_SYSKEYDOWN:
            return handle_syskeydown_message(hwnd, msg, wparam, lparam);
        case WM_MOUSEMOVE:
            handle_mousemove(hwnd, lparam);
            return 0;
        case WM_MOUSEWHEEL:
            handle_mousewheel(hwnd, wparam, lparam);
            return 0;
        case WM_SETCURSOR:
            if (handle_setcursor(hwnd, lparam)) return TRUE;
            break;
        case WM_LBUTTONDOWN: {
            g_state.mouse_screen.x = (float)GET_X_LPARAM(lparam);
            g_state.mouse_screen.y = (float)GET_Y_LPARAM(lparam);
            handle_lbuttondown(hwnd);
            return 0;
        }
        case WM_LBUTTONUP:
            handle_lbuttonup();
            return 0;
        case WM_LBUTTONDBLCLK:
            handle_lbuttondblclk_message(hwnd, lparam);
            return 0;
        case WM_DESTROY:
            handle_destroy();
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show) {
    WNDCLASSEXW wc;
    HWND hwnd;
    int exit_code;
    (void)prev;
    (void)cmd;
    (void)show;

    init_status_context();
    if (!init_platform_ui()) return 1;

    init_app_bootstrap_state();

    if (!register_main_window_class(inst, &wc)) return shutdown_and_get_exit_code(1);

    hwnd = create_main_window(inst, wc.lpszClassName);
    if (hwnd == NULL) return shutdown_and_get_exit_code(1);

    exit_code = run_message_loop();
    return shutdown_and_get_exit_code(exit_code);
}
