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

#define IDI_APP_MAIN 1001

typedef struct {
    float gravity_y;
    float time_step;
    float damping;
    int iterations;
    float ball_mass;
    float box_mass;
    float spawn_circle_radius;
    float spawn_box_size;
    float ball_restitution;
    float box_restitution;
} SceneConfig;

enum { SCENE_COUNT = 9 };

static const wchar_t* SCENE_NAMES[SCENE_COUNT] = {
    L"默认", L"高速", L"质量比", L"高堆叠", L"斜坡摩擦", L"弹性矩阵",
    L"拖拽压测", L"边界", L"性能"
};

static const SceneConfig SCENE_DEFAULTS[SCENE_COUNT] = {
    {9.8f, 1.0f / 60.0f, 0.992f, 8, 1.0f, 2.0f, 2.2f, 5.0f, 0.80f, 0.45f},
    {9.8f, 1.0f / 120.0f, 0.997f, 16, 0.6f, 2.5f, 1.0f, 4.0f, 0.75f, 0.35f},
    {9.8f, 1.0f / 60.0f, 0.995f, 14, 0.2f, 20.0f, 1.8f, 6.0f, 0.55f, 0.18f},
    {9.8f, 1.0f / 60.0f, 0.998f, 22, 1.0f, 4.0f, 2.0f, 5.5f, 0.25f, 0.05f},
    {9.8f, 1.0f / 90.0f, 0.996f, 14, 1.0f, 2.0f, 1.5f, 4.5f, 0.35f, 0.20f},
    {9.8f, 1.0f / 120.0f, 0.999f, 20, 1.0f, 2.0f, 1.6f, 4.5f, 0.90f, 0.10f},
    {9.8f, 1.0f / 60.0f, 0.992f, 14, 1.0f, 2.0f, 2.2f, 5.0f, 0.70f, 0.30f},
    {9.8f, 1.0f / 60.0f, 0.996f, 14, 1.2f, 2.0f, 2.0f, 5.0f, 0.65f, 0.30f},
    {9.8f, 1.0f / 90.0f, 0.990f, 10, 0.8f, 1.6f, 1.2f, 3.6f, 0.30f, 0.08f}
};

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
static const float WORLD_SCALE = 12.0f;
static const float WORLD_ORIGIN_X = 20.0f;
static const float WORLD_ORIGIN_Y = 20.0f;
static D2D1_RECT_F g_modal_rect;
static D2D1_RECT_F g_modal_close_rect;
static D2D1_RECT_F g_value_modal_rect;
static D2D1_RECT_F g_cfg_row_rect[8];
static D2D1_RECT_F g_cfg_minus_rect[8];
static D2D1_RECT_F g_cfg_plus_rect[8];
static D2D1_RECT_F g_top_run_rect;
static D2D1_RECT_F g_top_step_rect;
static D2D1_RECT_F g_top_reset_rect;
static D2D1_RECT_F g_top_save_rect;
static D2D1_RECT_F g_top_undo_rect;
static D2D1_RECT_F g_top_redo_rect;
static D2D1_RECT_F g_top_grid_rect;
static D2D1_RECT_F g_top_collision_rect;
static D2D1_RECT_F g_top_velocity_rect;
static D2D1_RECT_F g_top_constraint_rect;
static D2D1_RECT_F g_top_spring_rect;
static D2D1_RECT_F g_top_chain_rect;
static D2D1_RECT_F g_top_rope_rect;
static D2D1_RECT_F g_menu_file_rect;
static D2D1_RECT_F g_menu_edit_rect;
static D2D1_RECT_F g_menu_view_rect;
static D2D1_RECT_F g_menu_gameobject_rect;
static D2D1_RECT_F g_menu_component_rect;
static D2D1_RECT_F g_menu_physics_rect;
static D2D1_RECT_F g_menu_window_rect;
static D2D1_RECT_F g_menu_help_word_rect;
static D2D1_RECT_F g_menu_app_icon_rect;
static D2D1_RECT_F g_menu_bar_drag_rect;
static D2D1_RECT_F g_win_min_rect;
static D2D1_RECT_F g_win_max_rect;
static D2D1_RECT_F g_win_close_rect;
static D2D1_RECT_F g_menu_dropdown_rect;
static D2D1_RECT_F g_menu_item_rect[8];
static int g_menu_item_enabled[8];
static int g_menu_item_count;
static D2D1_RECT_F g_explorer_scene_rect;
static D2D1_RECT_F g_tree_scene_header_rect;
static D2D1_RECT_F g_tree_bodies_header_rect;
static D2D1_RECT_F g_tree_constraints_header_rect;
static D2D1_RECT_F g_tree_body_circle_header_rect;
static D2D1_RECT_F g_tree_body_polygon_header_rect;
static D2D1_RECT_F g_tree_constraint_distance_header_rect;
static D2D1_RECT_F g_tree_constraint_spring_header_rect;
static D2D1_RECT_F g_hierarchy_search_rect;
static D2D1_RECT_F g_hierarchy_search_clear_rect;
static D2D1_RECT_F g_hierarchy_viewport_rect;
static D2D1_RECT_F g_hierarchy_scroll_track_rect;
static D2D1_RECT_F g_hierarchy_scroll_thumb_rect;
static D2D1_RECT_F g_project_viewport_rect;
static D2D1_RECT_F g_tree_project_root_rect;
static D2D1_RECT_F g_project_scroll_track_rect;
static D2D1_RECT_F g_project_scroll_thumb_rect;
enum { PROJECT_TREE_MAX_ITEMS = 512, PROJECT_TREE_LINE_MAX = 180 };
static wchar_t g_project_tree_items[PROJECT_TREE_MAX_ITEMS][PROJECT_TREE_LINE_MAX];
static int g_project_tree_count;
static int g_project_tree_dir_count;
static int g_project_tree_file_count;
enum { EXPLORER_MAX_ITEMS = 512 };
static D2D1_RECT_F g_explorer_body_rect[EXPLORER_MAX_ITEMS];
static D2D1_RECT_F g_explorer_constraint_rect[EXPLORER_MAX_ITEMS];
static RigidBody* g_explorer_body_ptr[EXPLORER_MAX_ITEMS];
static int g_explorer_constraint_index[EXPLORER_MAX_ITEMS];
static int g_explorer_body_count;
static int g_explorer_constraint_count;
static D2D1_RECT_F g_bottom_tab_console_rect;
static D2D1_RECT_F g_bottom_tab_perf_rect;
static D2D1_RECT_F g_bottom_fold_rect;
static D2D1_RECT_F g_left_fold_rect;
static D2D1_RECT_F g_right_fold_rect;
static D2D1_RECT_F g_log_filter_all_rect;
static D2D1_RECT_F g_log_filter_state_rect;
static D2D1_RECT_F g_log_filter_physics_rect;
static D2D1_RECT_F g_log_filter_collision_rect;
static D2D1_RECT_F g_log_filter_warn_rect;
static D2D1_RECT_F g_log_clear_rect;
static D2D1_RECT_F g_log_search_rect;
static D2D1_RECT_F g_log_search_clear_rect;
static D2D1_RECT_F g_log_viewport_rect;
static D2D1_RECT_F g_log_scroll_track_rect;
static D2D1_RECT_F g_log_scroll_thumb_rect;
static D2D1_RECT_F g_inspector_viewport_rect;
static D2D1_RECT_F g_inspector_scroll_track_rect;
static D2D1_RECT_F g_inspector_scroll_thumb_rect;
static D2D1_RECT_F g_debug_viewport_rect;
static D2D1_RECT_F g_debug_scroll_track_rect;
static D2D1_RECT_F g_debug_scroll_thumb_rect;
static D2D1_RECT_F g_perf_export_rect;
static D2D1_RECT_F g_dbg_row_rect[3];
static D2D1_RECT_F g_dbg_minus_rect[3];
static D2D1_RECT_F g_dbg_plus_rect[3];
static D2D1_RECT_F g_dbg_collision_filter_rect;
enum { DEBUG_EVENT_ROWS_MAX = 24 };
static D2D1_RECT_F g_dbg_collision_row_rect[DEBUG_EVENT_ROWS_MAX];
static int g_dbg_collision_row_event_index[DEBUG_EVENT_ROWS_MAX];
static int g_dbg_collision_row_count;
static int g_log_scroll_max;
static D2D1_RECT_F g_splitter_left_rect;
static D2D1_RECT_F g_splitter_right_rect;
static D2D1_RECT_F g_splitter_bottom_rect;
static D2D1_RECT_F g_status_meta_rect;
enum { INSPECTOR_MAX_ROWS = 12 };
static D2D1_RECT_F g_ins_row_rect[INSPECTOR_MAX_ROWS];
static D2D1_RECT_F g_ins_minus_rect[INSPECTOR_MAX_ROWS];
static D2D1_RECT_F g_ins_plus_rect[INSPECTOR_MAX_ROWS];
static int g_ins_row_count;
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
static int compute_stage_rect_max_from_target(D2D1_RECT_F* out_stage_rect);
static Vec2 clamp_spawn_inside_stage_max(Vec2 p, float bound_radius);
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
    D2D1_RECT_F max_stage_rect;
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
    if (!compute_stage_rect_max_from_target(&max_stage_rect)) {
        trace_spawn_step("paste.abort", "stage-rect-failed");
        shape_free(sh);
        return 0;
    }
    if (g_state.mouse_screen.x >= max_stage_rect.left &&
        g_state.mouse_screen.x <= max_stage_rect.right &&
        g_state.mouse_screen.y >= max_stage_rect.top &&
        g_state.mouse_screen.y <= max_stage_rect.bottom) {
        p = screen_to_world(g_state.mouse_screen);
    } else {
        p = screen_to_world((ScreenPt){(max_stage_rect.left + max_stage_rect.right) * 0.5f, (max_stage_rect.top + max_stage_rect.bottom) * 0.5f});
    }
    p.x += 1.4f;
    p.y -= 1.2f;
    spawn_bound = g_clipboard_body.is_circle ? max_f(0.2f, g_clipboard_body.size) : max_f(0.4f, g_clipboard_body.size) * 0.72f;
    p = clamp_spawn_inside_stage_max(p, spawn_bound);
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
    wchar_t buf[64];
    GetPrivateProfileStringW(L"layout", L"left_ratio", L"0.18", buf, 64, ui_layout_ini_path());
    g_state.ui_left_ratio = (float)_wtof(buf);
    GetPrivateProfileStringW(L"layout", L"right_ratio", L"0.25", buf, 64, ui_layout_ini_path());
    g_state.ui_right_ratio = (float)_wtof(buf);
    GetPrivateProfileStringW(L"layout", L"bottom_open_h", L"132", buf, 64, ui_layout_ini_path());
    g_state.ui_bottom_open_h = (float)_wtof(buf);
    GetPrivateProfileStringW(L"layout", L"bottom_collapsed", L"1", buf, 64, ui_layout_ini_path());
    g_state.bottom_panel_collapsed = _wtoi(buf) ? 1 : 0;
    GetPrivateProfileStringW(L"layout", L"show_left", L"1", buf, 64, ui_layout_ini_path());
    g_state.ui_show_left_panel = _wtoi(buf) ? 1 : 0;
    GetPrivateProfileStringW(L"layout", L"show_right", L"1", buf, 64, ui_layout_ini_path());
    g_state.ui_show_right_panel = _wtoi(buf) ? 1 : 0;
    GetPrivateProfileStringW(L"layout", L"show_bottom", L"1", buf, 64, ui_layout_ini_path());
    g_state.ui_show_bottom_panel = _wtoi(buf) ? 1 : 0;
    GetPrivateProfileStringW(L"layout", L"theme_light", L"0", buf, 64, ui_layout_ini_path());
    g_state.ui_theme_light = _wtoi(buf) ? 1 : 0;
    GetPrivateProfileStringW(L"layout", L"preset", L"0", buf, 64, ui_layout_ini_path());
    g_state.ui_layout_preset = _wtoi(buf) % 3;
    g_state.ui_left_ratio = clamp(g_state.ui_left_ratio, 0.10f, 0.28f);
    g_state.ui_right_ratio = clamp(g_state.ui_right_ratio, 0.12f, 0.34f);
    g_state.ui_bottom_open_h = clamp(g_state.ui_bottom_open_h, 96.0f, 260.0f);
}

static void save_ui_layout(void) {
    wchar_t buf[64];
    swprintf(buf, 64, L"%.4f", g_state.ui_left_ratio);
    WritePrivateProfileStringW(L"layout", L"left_ratio", buf, ui_layout_ini_path());
    swprintf(buf, 64, L"%.4f", g_state.ui_right_ratio);
    WritePrivateProfileStringW(L"layout", L"right_ratio", buf, ui_layout_ini_path());
    swprintf(buf, 64, L"%.1f", g_state.ui_bottom_open_h);
    WritePrivateProfileStringW(L"layout", L"bottom_open_h", buf, ui_layout_ini_path());
    swprintf(buf, 64, L"%d", g_state.bottom_panel_collapsed ? 1 : 0);
    WritePrivateProfileStringW(L"layout", L"bottom_collapsed", buf, ui_layout_ini_path());
    swprintf(buf, 64, L"%d", g_state.ui_show_left_panel ? 1 : 0);
    WritePrivateProfileStringW(L"layout", L"show_left", buf, ui_layout_ini_path());
    swprintf(buf, 64, L"%d", g_state.ui_show_right_panel ? 1 : 0);
    WritePrivateProfileStringW(L"layout", L"show_right", buf, ui_layout_ini_path());
    swprintf(buf, 64, L"%d", g_state.ui_show_bottom_panel ? 1 : 0);
    WritePrivateProfileStringW(L"layout", L"show_bottom", buf, ui_layout_ini_path());
    swprintf(buf, 64, L"%d", g_state.ui_theme_light ? 1 : 0);
    WritePrivateProfileStringW(L"layout", L"theme_light", buf, ui_layout_ini_path());
    swprintf(buf, 64, L"%d", g_state.ui_layout_preset);
    WritePrivateProfileStringW(L"layout", L"preset", buf, ui_layout_ini_path());
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

static void execute_menu_action(HWND hwnd, int menu_id, int item_idx) {
    (void)hwnd;
    if (menu_id == 1) {
        if (item_idx == 0) {
            if (save_scene_snapshot("scene_snapshot.txt")) push_console_log(L"[文件] 已保存 scene_snapshot.txt");
            else push_console_log(L"[错误] 保存 scene_snapshot.txt 失败");
        } else if (item_idx == 1) {
            if (load_scene_snapshot("scene_snapshot.txt")) {
                history_reset_and_capture();
                push_console_log(L"[文件] 已加载 scene_snapshot.txt");
            } else push_console_log(L"[错误] 加载 scene_snapshot.txt 失败");
        } else if (item_idx == 2) {
            if (load_scene_snapshot("autosave_snapshot.txt")) {
                history_reset_and_capture();
                push_console_log(L"[文件] 已恢复 autosave_snapshot.txt");
            } else push_console_log(L"[错误] 恢复 autosave_snapshot.txt 失败");
        } else if (item_idx == 3) {
            g_state.draw_constraints = !g_state.draw_constraints;
        }
        return;
    }
    if (menu_id == 2) {
        if (item_idx == 0) {
            history_undo();
        } else if (item_idx == 1) {
            history_redo();
        } else if (item_idx == 2) {
            if (copy_selected_body_to_clipboard()) push_console_log(L"[编辑] 已复制选中对象");
            else push_console_log(L"[提示] 当前无可复制对象");
        } else if (item_idx == 3) {
            history_push_snapshot();
            if (paste_body_from_clipboard()) {
                push_console_log(L"[编辑] 已粘贴对象");
            } else push_console_log(L"[提示] 剪贴板为空");
        } else if (item_idx == 4) {
            ScreenPt old_mouse = g_state.mouse_screen;
            trace_spawn_step("menu.edit.create.begin", "item=%d", item_idx);
            g_state.mouse_screen.x = (g_state.stage_left + g_state.stage_right) * 0.5f;
            g_state.mouse_screen.y = (g_state.stage_top + g_state.stage_bottom) * 0.5f;
            history_push_snapshot();
            spawn_circle_at_cursor();
            trace_spawn_step("menu.edit.create.end", "item=%d", item_idx);
            g_state.mouse_screen = old_mouse;
        } else if (item_idx == 5) {
            ScreenPt old_mouse = g_state.mouse_screen;
            trace_spawn_step("menu.edit.create.begin", "item=%d", item_idx);
            g_state.mouse_screen.x = (g_state.stage_left + g_state.stage_right) * 0.5f;
            g_state.mouse_screen.y = (g_state.stage_top + g_state.stage_bottom) * 0.5f;
            history_push_snapshot();
            spawn_box_at_cursor();
            trace_spawn_step("menu.edit.create.end", "item=%d", item_idx);
            g_state.mouse_screen = old_mouse;
        } else if (item_idx == 6) {
            g_console_log_count = 0;
            g_console_log_head = 0;
            g_state.log_scroll_offset = 0;
            push_console_log(L"[日志] 已清空");
        }
        return;
    }
    if (menu_id == 3) {
        if (item_idx == 0) g_state.draw_centers = !g_state.draw_centers;
        else if (item_idx == 1) g_state.draw_contacts = !g_state.draw_contacts;
        else if (item_idx == 2) g_state.draw_velocity = !g_state.draw_velocity;
        return;
    }
    if (menu_id == 4) return;
    if (menu_id == 5) {
        if (item_idx == 0) g_state.running = !g_state.running;
        else if (item_idx == 1 && g_state.engine != NULL) {
            physics_engine_step(g_state.engine);
            cleanup_constraint_selection();
            capture_collision_events();
        } else if (item_idx == 2) {
            apply_scene(g_state.scene_index);
            history_reset_and_capture();
        }
        return;
    }
    if (menu_id == 6) {
        if (item_idx == 0) apply_layout_preset(g_state.ui_layout_preset + 1);
        else if (item_idx == 1) g_state.ui_show_left_panel = !g_state.ui_show_left_panel;
        else if (item_idx == 2) g_state.ui_show_right_panel = !g_state.ui_show_right_panel;
        else if (item_idx == 3) g_state.ui_show_bottom_panel = !g_state.ui_show_bottom_panel;
        else if (item_idx == 4 && g_state.ui_show_bottom_panel) g_state.bottom_active_tab = (g_state.bottom_active_tab + 1) % 2;
        else if (item_idx == 5) g_state.ui_theme_light = !g_state.ui_theme_light;
        save_ui_layout();
        return;
    }
    if (menu_id == 7) {
        g_state.show_help_modal = 1;
        g_state.show_config_modal = 0;
        g_state.help_modal_page = item_idx;
        return;
    }
}

static const wchar_t* menu_shortcut_text(int menu_id, int item_idx) {
    if (menu_id == 1 && item_idx == 0) return L"Ctrl+S";
    if (menu_id == 1 && item_idx == 1) return L"Ctrl+O";
    if (menu_id == 1 && item_idx == 2) return L"Ctrl+Shift+O";
    if (menu_id == 2 && item_idx == 0) return L"Ctrl+Z";
    if (menu_id == 2 && item_idx == 1) return L"Ctrl+Y";
    if (menu_id == 2 && item_idx == 2) return L"Ctrl+C";
    if (menu_id == 2 && item_idx == 3) return L"Ctrl+V";
    if (menu_id == 2 && item_idx == 4) return L"1";
    if (menu_id == 2 && item_idx == 5) return L"2";
    if (menu_id == 3 && item_idx == 0) return L"X";
    if (menu_id == 3 && item_idx == 1) return L"C";
    if (menu_id == 3 && item_idx == 2) return L"V";
    if (menu_id == 5 && item_idx == 0) return L"Space";
    if (menu_id == 5 && item_idx == 1) return L"N";
    if (menu_id == 5 && item_idx == 2) return L"R";
    if (menu_id == 6 && item_idx == 0) return L"F11";
    if (menu_id == 6 && item_idx == 1) return L"Alt+1";
    if (menu_id == 6 && item_idx == 2) return L"Alt+2";
    if (menu_id == 6 && item_idx == 3) return L"Alt+3";
    if (menu_id == 6 && item_idx == 5) return L"Alt+T";
    return L"";
}

static int menu_item_enabled_state(int menu_id, int item_idx) {
    if (menu_id == 1 && item_idx == 1) {
        return file_exists_utf8_path("scene_snapshot.txt");
    }
    if (menu_id == 1 && item_idx == 2) {
        return file_exists_utf8_path("autosave_snapshot.txt");
    }
    if (menu_id == 2 && item_idx == 0) return g_state.history_cursor > 0;
    if (menu_id == 2 && item_idx == 1) return g_state.history_cursor < g_state.history_top;
    if (menu_id == 2 && item_idx == 2) return (g_state.selected != NULL && g_state.selected->type == BODY_DYNAMIC);
    if (menu_id == 2 && item_idx == 3) return g_clipboard_body.valid;
    if (menu_id == 6 && item_idx == 4) return g_state.ui_show_bottom_panel;
    return 1;
}

static int menu_item_count_for_menu(int menu_id) {
    if (menu_id == 1) return 4;
    if (menu_id == 2) return 7;
    if (menu_id == 3) return 3;
    if (menu_id == 4) return 0;
    if (menu_id == 5) return 3;
    if (menu_id == 6) return 6;
    if (menu_id == 7) return 3;
    return 0;
}

static int menu_next_visible_id(int current_id, int dir) {
    static const int ids[] = {1, 2, 3, 5, 6, 7};
    int i;
    int idx = 0;
    int n = (int)(sizeof(ids) / sizeof(ids[0]));
    if (dir == 0) return current_id;
    for (i = 0; i < n; i++) {
        if (ids[i] == current_id) {
            idx = i;
            break;
        }
    }
    idx = (idx + (dir > 0 ? 1 : -1) + n) % n;
    return ids[idx];
}

static int menu_find_enabled_from(int menu_id, int start, int dir) {
    int n = menu_item_count_for_menu(menu_id);
    int i;
    if (n <= 0) return 0;
    for (i = 0; i < n; i++) {
        int idx = (start + dir * i + n * 4) % n;
        if (menu_item_enabled_state(menu_id, idx)) return idx;
    }
    return 0;
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

static int compute_stage_rect_from_layout(HWND hwnd, D2D1_RECT_F* out_stage_rect) {
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
    float work_bottom;
    float center_left;
    float center_right;
    D2D1_RECT_F top_rect;
    D2D1_RECT_F status_rect;
    D2D1_RECT_F bottom_rect;
    D2D1_RECT_F center_rect;
    if (out_stage_rect == NULL) return 0;
    GetClientRect(hwnd, &wr);
    w = (float)(wr.right - wr.left);
    h = (float)(wr.bottom - wr.top);
    left_w = w * g_state.ui_left_ratio;
    right_w = w * g_state.ui_right_ratio;
    if (left_w < 148.0f) left_w = 148.0f;
    if (left_w > 360.0f) left_w = 360.0f;
    if (right_w < 220.0f) right_w = 220.0f;
    if (right_w > 460.0f) right_w = 460.0f;
    if (g_state.ui_show_bottom_panel && bottom_h < 24.0f) bottom_h = 24.0f;
    top_rect = rc(8.0f, 8.0f, w - 8.0f, 8.0f + top_h);
    status_rect = rc(8.0f, h - status_h - 8.0f, w - 8.0f, h - 8.0f);
    bottom_rect = g_state.ui_show_bottom_panel
                      ? rc(8.0f, status_rect.top - bottom_h - 8.0f, w - 8.0f, status_rect.top - 8.0f)
                      : rc(0.0f, 0.0f, 0.0f, 0.0f);
    work_bottom = g_state.ui_show_bottom_panel ? (bottom_rect.top - 8.0f) : (status_rect.top - 8.0f);
    center_left = g_state.ui_show_left_panel ? (8.0f + left_w + 8.0f) : 8.0f;
    center_right = g_state.ui_show_right_panel ? (w - right_w - 16.0f) : (w - 8.0f);
    center_rect = rc(center_left, top_rect.bottom + 8.0f, center_right, work_bottom);
    *out_stage_rect = rc(center_rect.left + 10.0f, center_rect.top + 46.0f, center_rect.right - 10.0f, center_rect.bottom - 8.0f);
    return 1;
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
    float w;
    float h;
    float menu_h = 30.0f;
    float toolbar_h = 44.0f;
    float top_h = menu_h + toolbar_h + 12.0f;
    float status_h = 32.0f;
    D2D1_RECT_F top_rect;
    D2D1_RECT_F status_rect;
    D2D1_RECT_F center_rect;
    if (out_stage_rect == NULL) return 0;
    GetClientRect(hwnd, &wr);
    w = (float)(wr.right - wr.left);
    h = (float)(wr.bottom - wr.top);
    top_rect = rc(8.0f, 8.0f, w - 8.0f, 8.0f + top_h);
    status_rect = rc(8.0f, h - status_h - 8.0f, w - 8.0f, h - 8.0f);
    center_rect = rc(8.0f, top_rect.bottom + 8.0f, w - 8.0f, status_rect.top - 8.0f);
    *out_stage_rect = rc(center_rect.left + 10.0f, center_rect.top + 46.0f, center_rect.right - 10.0f, center_rect.bottom - 8.0f);
    return 1;
}

static int compute_stage_rect_max_from_target(D2D1_RECT_F* out_stage_rect) {
    RECT wr;
    float w;
    float h;
    float menu_h = 30.0f;
    float toolbar_h = 44.0f;
    float top_h = menu_h + toolbar_h + 12.0f;
    float status_h = 32.0f;
    D2D1_RECT_F top_rect;
    D2D1_RECT_F status_rect;
    D2D1_RECT_F center_rect;
    if (out_stage_rect == NULL || g_app_hwnd == NULL) return 0;
    GetClientRect(g_app_hwnd, &wr);
    w = (float)(wr.right - wr.left);
    h = (float)(wr.bottom - wr.top);
    if (w <= 1.0f || h <= 1.0f) return 0;
    top_rect = rc(8.0f, 8.0f, w - 8.0f, 8.0f + top_h);
    status_rect = rc(8.0f, h - status_h - 8.0f, w - 8.0f, h - 8.0f);
    center_rect = rc(8.0f, top_rect.bottom + 8.0f, w - 8.0f, status_rect.top - 8.0f);
    *out_stage_rect = rc(center_rect.left + 10.0f, center_rect.top + 46.0f, center_rect.right - 10.0f, center_rect.bottom - 8.0f);
    return 1;
}

static int stage_world_bounds_max(float* min_x, float* max_x, float* min_y, float* max_y) {
    D2D1_RECT_F stage_rect;
    ScreenPt smin;
    ScreenPt smax;
    Vec2 wmin;
    Vec2 wmax;
    if (min_x == NULL || max_x == NULL || min_y == NULL || max_y == NULL) return 0;
    if (!compute_stage_rect_max_from_target(&stage_rect)) return 0;
    smin.x = stage_rect.left;
    smin.y = stage_rect.top;
    smax.x = stage_rect.right;
    smax.y = stage_rect.bottom;
    wmin = screen_to_world(smin);
    wmax = screen_to_world(smax);
    *min_x = min_f(wmin.x, wmax.x);
    *max_x = max_f(wmin.x, wmax.x);
    *min_y = min_f(wmin.y, wmax.y);
    *max_y = max_f(wmin.y, wmax.y);
    return 1;
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

static Vec2 clamp_spawn_inside_stage_max(Vec2 p, float bound_radius) {
    float min_x;
    float max_x;
    float min_y;
    float max_y;
    float r = max_f(bound_radius, 0.1f);
    if (!stage_world_bounds_max(&min_x, &max_x, &min_y, &max_y)) return p;
    if (p.x < min_x + r) p.x = min_x + r;
    if (p.x > max_x - r) p.x = max_x - r;
    if (p.y < min_y + r) p.y = min_y + r;
    if (p.y > max_y - r) p.y = max_y - r;
    return p;
}

static void fit_dynamic_bodies_into_stage(void) {
    int i;
    float min_x;
    float max_x;
    float min_y;
    float max_y;
    if (g_state.engine == NULL) return;
    if (!stage_world_bounds(&min_x, &max_x, &min_y, &max_y)) return;

    for (i = 0; i < physics_engine_get_body_count(g_state.engine); i++) {
        RigidBody* b = physics_engine_get_body(g_state.engine, i);
        float r;
        Vec2 p;
        if (b == NULL || b->type != BODY_DYNAMIC) continue;
        r = body_bound_radius(b);
        p = clamp_spawn_inside_stage(b->position, r);
        b->position = p;
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
    D2D1_RECT_F max_stage_rect;
    trace_spawn_step("spawn.circle.begin", "mx=%.1f my=%.1f", g_state.mouse_screen.x, g_state.mouse_screen.y);
    if (g_state.engine == NULL) return;
    if (!compute_stage_rect_max_from_target(&max_stage_rect)) return;
    if (g_state.mouse_screen.x < max_stage_rect.left || g_state.mouse_screen.x > max_stage_rect.right ||
        g_state.mouse_screen.y < max_stage_rect.top || g_state.mouse_screen.y > max_stage_rect.bottom) {
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
    p = clamp_spawn_inside_stage_max(p, cfg->spawn_circle_radius);
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
    D2D1_RECT_F max_stage_rect;
    float half;
    trace_spawn_step("spawn.box.begin", "mx=%.1f my=%.1f", g_state.mouse_screen.x, g_state.mouse_screen.y);
    if (g_state.engine == NULL) return;
    if (!compute_stage_rect_max_from_target(&max_stage_rect)) return;
    if (g_state.mouse_screen.x < max_stage_rect.left || g_state.mouse_screen.x > max_stage_rect.right ||
        g_state.mouse_screen.y < max_stage_rect.top || g_state.mouse_screen.y > max_stage_rect.bottom) {
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
    p = clamp_spawn_inside_stage_max(p, half);
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
    DWRITE_PARAGRAPH_ALIGNMENT old_align = IDWriteTextFormat_GetParagraphAlignment(fmt);
    IDWriteTextFormat_SetParagraphAlignment(fmt, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    set_brush_color(color.r, color.g, color.b, color.a);
    ID2D1HwndRenderTarget_DrawText(g_ui.target, text, (UINT32)lstrlenW(text), fmt, &rect, (ID2D1Brush*)g_ui.brush,
                                   D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT, DWRITE_MEASURING_MODE_NATURAL);
    IDWriteTextFormat_SetParagraphAlignment(fmt, old_align);
}

static void draw_text_vcenter(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    IDWriteTextLayout* layout = NULL;
    DWRITE_TEXT_METRICS m;
    float slot_h = rect.bottom - rect.top;
    float draw_h;
    float y;
    HRESULT hr;
    if (slot_h <= 0.0f) {
        draw_text(text, rect, fmt, color);
        return;
    }
    hr = IDWriteFactory_CreateTextLayout(
        g_ui.dwrite_factory,
        text,
        (UINT32)lstrlenW(text),
        fmt,
        rect.right - rect.left,
        512.0f,
        &layout
    );
    if (FAILED(hr) || layout == NULL) {
        draw_text(text, rect, fmt, color);
        release_unknown((IUnknown**)&layout);
        return;
    }
    if (FAILED(IDWriteTextLayout_GetMetrics(layout, &m))) {
        draw_text(text, rect, fmt, color);
        release_unknown((IUnknown**)&layout);
        return;
    }
    draw_h = m.height;
    if (draw_h < 1.0f) draw_h = slot_h;
    if (draw_h > slot_h) draw_h = slot_h;
    y = rect.top + (slot_h - draw_h) * 0.5f;
    set_brush_color(color.r, color.g, color.b, color.a);
    ID2D1HwndRenderTarget_DrawTextLayout(g_ui.target, pt(rect.left, y), layout, (ID2D1Brush*)g_ui.brush,
                                         D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
    release_unknown((IUnknown**)&layout);
}

static void draw_text_hvcenter(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    DWRITE_TEXT_ALIGNMENT old_text_align = IDWriteTextFormat_GetTextAlignment(fmt);
    DWRITE_PARAGRAPH_ALIGNMENT old_para_align = IDWriteTextFormat_GetParagraphAlignment(fmt);
    IDWriteTextFormat_SetTextAlignment(fmt, DWRITE_TEXT_ALIGNMENT_CENTER);
    IDWriteTextFormat_SetParagraphAlignment(fmt, DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    draw_text(text, rect, fmt, color);
    IDWriteTextFormat_SetParagraphAlignment(fmt, old_para_align);
    IDWriteTextFormat_SetTextAlignment(fmt, old_text_align);
}

static void draw_text_right_vcenter(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    DWRITE_TEXT_ALIGNMENT old_text_align = IDWriteTextFormat_GetTextAlignment(fmt);
    IDWriteTextFormat_SetTextAlignment(fmt, DWRITE_TEXT_ALIGNMENT_TRAILING);
    draw_text_vcenter(text, rect, fmt, color);
    IDWriteTextFormat_SetTextAlignment(fmt, old_text_align);
}

static float measure_text_width(const wchar_t* text, IDWriteTextFormat* fmt) {
    IDWriteTextLayout* layout = NULL;
    DWRITE_TEXT_METRICS m;
    HRESULT hr;
    float w = 0.0f;
    if (text == NULL || text[0] == L'\0' || g_ui.dwrite_factory == NULL || fmt == NULL) return 0.0f;
    hr = IDWriteFactory_CreateTextLayout(g_ui.dwrite_factory, text, (UINT32)lstrlenW(text), fmt, 1200.0f, 64.0f, &layout);
    if (SUCCEEDED(hr) && layout != NULL) {
        if (SUCCEEDED(IDWriteTextLayout_GetMetrics(layout, &m))) {
            w = m.widthIncludingTrailingWhitespace;
        }
    }
    release_unknown((IUnknown**)&layout);
    return w;
}

static void add_static_world(PhysicsEngine* engine) {
    Shape* ground_shape = shape_create_box(120.0f, 2.0f);
    ground_shape->friction = 0.85f;
    ground_shape->restitution = 0.0f;
    RigidBody* ground = body_create(50.0f, 45.0f, 1000.0f, ground_shape);
    body_set_type(ground, BODY_STATIC);
    physics_engine_add_body(engine, ground);

    Shape* left_wall_shape = shape_create_box(2.0f, 60.0f);
    left_wall_shape->friction = 0.75f;
    RigidBody* left_wall = body_create(0.0f, 25.0f, 1000.0f, left_wall_shape);
    body_set_type(left_wall, BODY_STATIC);
    physics_engine_add_body(engine, left_wall);
}

static void scene_default_bounce(PhysicsEngine* engine, const SceneConfig* cfg) {
    int i;
    for (i = 0; i < 3; i++) {
        Shape* ball_shape = shape_create_circle(2.0f);
        ball_shape->restitution = cfg->ball_restitution;
        RigidBody* ball = body_create(40.0f + i * 8.0f, 10.0f, cfg->ball_mass, ball_shape);
        ball->velocity = vec2((i == 1) ? -8.0f : 8.0f, 0.0f);
        physics_engine_add_body(engine, ball);
    }
    for (i = 0; i < 2; i++) {
        Shape* box_shape = shape_create_box(8.0f, 8.0f);
        box_shape->restitution = cfg->box_restitution;
        RigidBody* box = body_create(30.0f + i * 35.0f, 18.0f, cfg->box_mass, box_shape);
        box->angular_velocity = (i == 0) ? 2.0f : -1.5f;
        physics_engine_add_body(engine, box);
    }
}

static void add_dynamic_body_safe(PhysicsEngine* engine, RigidBody* body) {
    if (engine == NULL || body == NULL) return;
    physics_engine_add_body(engine, body);
    if (engine == g_state.engine) {
        if (!resolve_spawn_overlap_by_push(body)) {
            physics_engine_remove_body(engine, body);
        }
    }
}

static void scene_high_speed_tunnel(PhysicsEngine* engine, const SceneConfig* cfg) {
    int i;
    Shape* thin_shape = shape_create_box(1.2f, 24.0f);
    RigidBody* thin_wall = body_create(58.0f, 28.0f, 1000.0f, thin_shape);
    body_set_type(thin_wall, BODY_STATIC);
    physics_engine_add_body(engine, thin_wall);

    for (i = 0; i < 4; i++) {
        Shape* target_box_shape = shape_create_box(4.2f, 4.2f);
        target_box_shape->restitution = cfg->box_restitution;
        physics_engine_add_body(engine, body_create(74.0f + i * 5.0f, 38.0f - i * 4.4f, cfg->box_mass, target_box_shape));
    }

    for (i = 0; i < 2; i++) {
        Shape* fast_ball_shape = shape_create_circle(1.0f);
        RigidBody* fast_ball;
        fast_ball_shape->friction = 0.05f;
        fast_ball_shape->restitution = cfg->ball_restitution;
        fast_ball = body_create(10.0f, 30.0f + i * 4.0f, cfg->ball_mass, fast_ball_shape);
        fast_ball->velocity = vec2(105.0f + i * 22.0f, -4.0f + i * 2.0f);
        physics_engine_add_body(engine, fast_ball);
    }
}

static void scene_mass_ratio(PhysicsEngine* engine, const SceneConfig* cfg) {
    int i;
    Shape* heavy_box_shape = shape_create_box(10.0f, 10.0f);
    RigidBody* heavy_box;
    heavy_box_shape->restitution = cfg->box_restitution;
    heavy_box_shape->friction = 0.45f;
    heavy_box = body_create(72.0f, 36.0f, cfg->box_mass, heavy_box_shape);
    physics_engine_add_body(engine, heavy_box);

    for (i = 0; i < 10; i++) {
        Shape* light_ball_shape = shape_create_circle(1.5f);
        RigidBody* light_ball;
        light_ball_shape->restitution = cfg->ball_restitution;
        light_ball_shape->friction = 0.15f;
        light_ball = body_create(12.0f + i * 3.5f, 18.2f + (i % 2) * 1.8f, cfg->ball_mass, light_ball_shape);
        light_ball->velocity = vec2(26.0f + (i % 3) * 2.5f, 0.0f);
        physics_engine_add_body(engine, light_ball);
    }
}

static void scene_high_stack(PhysicsEngine* engine, const SceneConfig* cfg) {
    int c;
    int i;
    for (c = 0; c < 3; c++) {
        for (i = 0; i < 12; i++) {
            Shape* box_shape = shape_create_box(4.6f, 4.6f);
            box_shape->friction = 0.38f;
            box_shape->restitution = cfg->box_restitution;
            physics_engine_add_body(engine, body_create(38.0f + c * 12.0f, 43.0f - i * 4.8f, cfg->box_mass, box_shape));
        }
    }

    {
        Shape* striker_shape = shape_create_circle(3.2f);
        RigidBody* striker;
        striker_shape->restitution = cfg->ball_restitution;
        striker_shape->friction = 0.2f;
        striker = body_create(14.0f, 39.0f, cfg->ball_mass * 5.0f, striker_shape);
        striker->velocity = vec2(36.0f, 0.0f);
        physics_engine_add_body(engine, striker);
    }
}

static void scene_friction_slope(PhysicsEngine* engine, const SceneConfig* cfg) {
    int i;
    Shape* ramp_shape = shape_create_box(56.0f, 2.5f);
    RigidBody* ramp = body_create(36.0f, 28.0f, 1000.0f, ramp_shape);
    body_set_type(ramp, BODY_STATIC);
    ramp->angle = -0.42f;
    physics_engine_add_body(engine, ramp);

    for (i = 0; i < 5; i++) {
        Shape* ball_shape = shape_create_circle(1.7f);
        ball_shape->restitution = cfg->ball_restitution;
        ball_shape->friction = 0.03f + i * 0.12f;
        add_dynamic_body_safe(engine, body_create(12.0f + i * 5.0f, 8.0f + i * 1.1f, cfg->ball_mass, ball_shape));
    }
    for (i = 0; i < 4; i++) {
        Shape* box_shape = shape_create_box(4.2f, 4.2f);
        box_shape->restitution = cfg->box_restitution;
        box_shape->friction = 0.10f + i * 0.18f;
        add_dynamic_body_safe(engine, body_create(18.0f + i * 8.0f, 11.0f + i * 1.2f, cfg->box_mass, box_shape));
    }
}

static void scene_restitution_matrix(PhysicsEngine* engine, const SceneConfig* cfg) {
    int i;
    const float rvals[3] = {0.20f, 0.55f, 0.90f};
    for (i = 0; i < 3; i++) {
        Shape* left_shape = shape_create_circle(1.8f);
        Shape* right_shape = shape_create_circle(1.8f);
        RigidBody* left_ball;
        RigidBody* right_ball;
        left_shape->restitution = rvals[i];
        right_shape->restitution = rvals[i];
        left_shape->friction = 0.1f;
        right_shape->friction = 0.1f;
        left_ball = body_create(24.0f, 16.0f + i * 8.0f, cfg->ball_mass, left_shape);
        right_ball = body_create(76.0f, 16.0f + i * 8.0f, cfg->ball_mass, right_shape);
        left_ball->velocity = vec2(14.0f + i * 2.0f, 0.0f);
        right_ball->velocity = vec2(-14.0f - i * 2.0f, 0.0f);
        physics_engine_add_body(engine, left_ball);
        physics_engine_add_body(engine, right_ball);
    }
}

static void scene_drag_stress(PhysicsEngine* engine, const SceneConfig* cfg) {
    int x;
    int y;
    for (y = 0; y < 5; y++) {
        for (x = 0; x < 9; x++) {
            Shape* ball_shape = shape_create_circle(1.9f);
            ball_shape->restitution = cfg->ball_restitution;
            ball_shape->friction = 0.2f;
            physics_engine_add_body(engine, body_create(20.0f + x * 5.0f, 20.0f + y * 3.6f, cfg->ball_mass, ball_shape));
        }
    }
    for (x = 0; x < 8; x++) {
        Shape* box_shape = shape_create_box(4.8f, 4.8f);
        box_shape->restitution = cfg->box_restitution;
        box_shape->friction = 0.35f;
        physics_engine_add_body(engine, body_create(18.0f + x * 9.0f, 11.0f + (x % 2) * 2.5f, cfg->box_mass, box_shape));
    }
}

static void scene_boundary_squeeze(PhysicsEngine* engine, const SceneConfig* cfg) {
    int i;
    Shape* ground_shape;
    RigidBody* ground;
    ground_shape = shape_create_box(120.0f, 2.0f);
    ground_shape->friction = 0.85f;
    ground_shape->restitution = 0.0f;
    ground = body_create(50.0f, 45.0f, 1000.0f, ground_shape);
    body_set_type(ground, BODY_STATIC);
    physics_engine_add_body(engine, ground);

    /* Right-side recycle probe: only launch to +X for clear verification. */
    for (i = 0; i < 14; i++) {
        Shape* shot_shape = shape_create_circle(1.6f);
        RigidBody* shot_ball;
        shot_shape->restitution = cfg->ball_restitution;
        shot_shape->friction = 0.05f;
        shot_ball = body_create(8.0f + (i % 3) * 3.0f, 8.0f + (float)i * 2.4f, cfg->ball_mass, shot_shape);
        shot_ball->velocity = vec2(30.0f + (i % 4) * 4.0f, 0.0f);
        physics_engine_add_body(engine, shot_ball);
    }
}

static void scene_performance_limit(PhysicsEngine* engine, const SceneConfig* cfg) {
    int x;
    int y;
    for (y = 0; y < 10; y++) {
        for (x = 0; x < 16; x++) {
            Shape* ball_shape = shape_create_circle(1.2f);
            ball_shape->restitution = cfg->ball_restitution;
            ball_shape->friction = 0.12f;
            physics_engine_add_body(engine, body_create(8.0f + x * 5.1f, 6.0f + y * 3.8f, cfg->ball_mass, ball_shape));
        }
    }
    for (y = 0; y < 5; y++) {
        for (x = 0; x < 10; x++) {
            Shape* box_shape = shape_create_box(3.4f, 3.4f);
            box_shape->restitution = cfg->box_restitution;
            box_shape->friction = 0.25f;
            physics_engine_add_body(engine, body_create(11.0f + x * 7.8f, 7.5f + y * 6.0f, cfg->box_mass, box_shape));
        }
    }
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
    /* Boundary recycle test scene: do not add static world walls/floor,
       so bodies can exit in all directions for recycle timing checks. */
    if (scene_index != 7) {
        add_static_world(g_state.engine);
    }

    switch (scene_index) {
        case 1:
            scene_high_speed_tunnel(g_state.engine, cfg);
            break;
        case 2:
            scene_mass_ratio(g_state.engine, cfg);
            break;
        case 3:
            scene_high_stack(g_state.engine, cfg);
            break;
        case 4:
            scene_friction_slope(g_state.engine, cfg);
            break;
        case 5:
            scene_restitution_matrix(g_state.engine, cfg);
            break;
        case 6:
            scene_drag_stress(g_state.engine, cfg);
            break;
        case 7:
            scene_boundary_squeeze(g_state.engine, cfg);
            break;
        case 8:
            scene_performance_limit(g_state.engine, cfg);
            break;
        default:
            scene_default_bounce(g_state.engine, cfg);
            break;
    }
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
    push_console_log(L"[场景] 切换到: %s", SCENE_NAMES[g_state.scene_index]);
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
    FILE* fp;
    int i;
    int dyn_count = 0;
    int con_count = 0;
    trace_spawn_step("snapshot.save.begin", "path=%s engine=%p", (path != NULL) ? path : "(null)", (void*)g_state.engine);
    if (g_state.engine == NULL || path == NULL) return 0;
    fp = fopen(path, "w");
    if (fp == NULL) return 0;
    for (i = 0; i < physics_engine_get_body_count(g_state.engine); i++) {
        RigidBody* b = physics_engine_get_body(g_state.engine, i);
        if (b != NULL && b->type == BODY_DYNAMIC) dyn_count++;
    }
    fprintf(fp, "BODIES %d\n", dyn_count);
    for (i = 0; i < physics_engine_get_body_count(g_state.engine); i++) {
        RigidBody* b = physics_engine_get_body(g_state.engine, i);
        if (b == NULL || b->type != BODY_DYNAMIC || b->shape == NULL) continue;
        if (b->shape->type == SHAPE_CIRCLE) {
            fprintf(fp, "b C %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f\n",
                    b->mass, b->position.x, b->position.y, b->velocity.x, b->velocity.y, b->angular_velocity,
                    b->shape->restitution, b->shape->friction, b->shape->data.circle.radius, b->damping);
        } else {
            fprintf(fp, "b B %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f %.6f\n",
                    b->mass, b->position.x, b->position.y, b->velocity.x, b->velocity.y, b->angular_velocity,
                    b->shape->restitution, b->shape->friction, body_box_size_hint(b), b->damping);
        }
    }
    for (i = 0; i < physics_engine_get_constraint_count(g_state.engine); i++) {
        const Constraint* c = physics_engine_get_constraint(g_state.engine, i);
        if (c->active && c->body_a != NULL && c->body_b != NULL && c->body_a->type == BODY_DYNAMIC && c->body_b->type == BODY_DYNAMIC) con_count++;
    }
    fprintf(fp, "CONSTRAINTS %d\n", con_count);
    {
        int j;
        for (i = 0; i < physics_engine_get_constraint_count(g_state.engine); i++) {
            const Constraint* c = physics_engine_get_constraint(g_state.engine, i);
            int ai = -1;
            int bi = -1;
            int k = 0;
            if (!c->active || c->body_a == NULL || c->body_b == NULL || c->body_a->type != BODY_DYNAMIC || c->body_b->type != BODY_DYNAMIC) continue;
            for (j = 0; j < physics_engine_get_body_count(g_state.engine); j++) {
                RigidBody* b = physics_engine_get_body(g_state.engine, j);
                if (b == NULL || b->type != BODY_DYNAMIC) continue;
                if (b == c->body_a) ai = k;
                if (b == c->body_b) bi = k;
                k++;
            }
            if (ai < 0 || bi < 0) continue;
            fprintf(fp, "c %d %d %d %.6f %.6f %.6f %.6f %d\n",
                    (int)c->type, ai, bi, c->rest_length, c->stiffness, c->damping, c->break_force, c->collide_connected);
        }
    }
    fclose(fp);
    trace_spawn_step("snapshot.save.end", "path=%s dyn=%d con=%d", path, dyn_count, con_count);
    return 1;
}

static int load_scene_snapshot(const char* path) {
    FILE* fp;
    int body_n = 0;
    int con_n = 0;
    int i;
    RigidBody* saved[1024];
    int saved_n = 0;
    if (g_state.engine == NULL || path == NULL) return 0;
    fp = fopen(path, "r");
    if (fp == NULL) return 0;
    for (i = physics_engine_get_body_count(g_state.engine) - 1; i >= 0; i--) {
        RigidBody* b = physics_engine_get_body(g_state.engine, i);
        if (b != NULL && b->type == BODY_DYNAMIC) physics_engine_remove_body(g_state.engine, b);
    }
    physics_engine_clear_constraints(g_state.engine);
    if (fscanf(fp, "BODIES %d\n", &body_n) != 1) {
        fclose(fp);
        return 0;
    }
    for (i = 0; i < body_n && i < 1024; i++) {
        char kind = 0;
        float mass, px, py, vx, vy, ang, rest, fric, size, damp = 1.0f;
        Shape* sh;
        RigidBody* b;
        {
            char linebuf[256];
            int n = 0;
            if (fgets(linebuf, sizeof(linebuf), fp) == NULL) break;
            n = sscanf(linebuf, "b %c %f %f %f %f %f %f %f %f %f %f",
                       &kind, &mass, &px, &py, &vx, &vy, &ang, &rest, &fric, &size, &damp);
            if (n != 10 && n != 11) break;
            if (n == 10) damp = 1.0f;
        }
        if (kind == 'C') sh = shape_create_circle(size);
        else sh = shape_create_box(size, size);
        sh->restitution = rest;
        sh->friction = fric;
        b = body_create(px, py, mass, sh);
        b->velocity = vec2(vx, vy);
        b->angular_velocity = ang;
        b->damping = clamp(damp, 0.0f, 1.0f);
        physics_engine_add_body(g_state.engine, b);
        saved[saved_n++] = b;
    }
    if (fscanf(fp, "CONSTRAINTS %d\n", &con_n) == 1) {
        for (i = 0; i < con_n; i++) {
            int type, ai, bi, coll;
            float rl, st, dp, br;
            Constraint* c = NULL;
            if (fscanf(fp, "c %d %d %d %f %f %f %f %d\n", &type, &ai, &bi, &rl, &st, &dp, &br, &coll) != 8) break;
            if (ai < 0 || ai >= saved_n || bi < 0 || bi >= saved_n) continue;
            if (type == CONSTRAINT_DISTANCE) {
                c = physics_engine_add_distance_constraint(g_state.engine, saved[ai], saved[bi], saved[ai]->position, saved[bi]->position, st, coll);
                if (c != NULL) c->rest_length = rl;
            } else {
                c = physics_engine_add_spring_constraint(g_state.engine, saved[ai], saved[bi], saved[ai]->position, saved[bi]->position, rl, st, dp, coll);
            }
            if (c != NULL) c->break_force = br;
        }
    }
    fclose(fp);
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

static void history_slot_path(int idx, char* out_path, int cap) {
    if (out_path == NULL || cap <= 0) return;
    snprintf(out_path, (size_t)cap, "history_%02d.txt", idx);
}

static void history_shift_left(void) {
    int i;
    char src[64];
    char dst[64];
    for (i = 1; i < HISTORY_MAX_SLOTS; i++) {
        history_slot_path(i, src, 64);
        history_slot_path(i - 1, dst, 64);
        if (file_exists_utf8_path(src)) {
            CopyFileA(src, dst, FALSE);
        }
    }
    history_slot_path(HISTORY_MAX_SLOTS - 1, src, 64);
    DeleteFileA(src);
    if (g_state.history_cursor > 0) g_state.history_cursor--;
    if (g_state.history_top > 0) g_state.history_top--;
}

static void history_truncate_after_cursor(void) {
    int i;
    char p[64];
    for (i = g_state.history_cursor + 1; i <= g_state.history_top; i++) {
        history_slot_path(i, p, 64);
        DeleteFileA(p);
    }
    g_state.history_top = g_state.history_cursor;
    g_state.history_count = g_state.history_top + 1;
}

static void history_push_snapshot(void) {
    char p[64];
    trace_spawn_step("history.push.begin", "replay=%d engine=%p cursor=%d top=%d",
                     g_state.history_replaying, (void*)g_state.engine, g_state.history_cursor, g_state.history_top);
    if (g_state.history_replaying) return;
    if (g_state.engine == NULL) return;
    history_truncate_after_cursor();
    if (g_state.history_top >= HISTORY_MAX_SLOTS - 1) {
        history_shift_left();
    }
    g_state.history_cursor++;
    g_state.history_top = g_state.history_cursor;
    g_state.history_count = g_state.history_top + 1;
    history_slot_path(g_state.history_cursor, p, 64);
    save_scene_snapshot(p);
    trace_spawn_step("history.push.end", "slot=%s cursor=%d top=%d", p, g_state.history_cursor, g_state.history_top);
}

static void history_reset_and_capture(void) {
    int i;
    char p[64];
    for (i = 0; i < HISTORY_MAX_SLOTS; i++) {
        history_slot_path(i, p, 64);
        DeleteFileA(p);
    }
    g_state.history_cursor = 0;
    g_state.history_top = 0;
    g_state.history_count = 1;
    history_slot_path(0, p, 64);
    save_scene_snapshot(p);
}

static void history_undo(void) {
    char p[64];
    if (g_state.history_cursor <= 0) {
        push_console_log(L"[历史] 没有可撤销操作");
        return;
    }
    g_state.history_cursor--;
    history_slot_path(g_state.history_cursor, p, 64);
    g_state.history_replaying = 1;
    if (load_scene_snapshot(p)) {
        push_console_log(L"[历史] 已撤销");
    } else {
        push_console_log(L"[错误] 撤销失败");
    }
    g_state.history_replaying = 0;
}

static void history_redo(void) {
    char p[64];
    if (g_state.history_cursor >= g_state.history_top) {
        push_console_log(L"[历史] 没有可重做操作");
        return;
    }
    g_state.history_cursor++;
    history_slot_path(g_state.history_cursor, p, 64);
    g_state.history_replaying = 1;
    if (load_scene_snapshot(p)) {
        push_console_log(L"[历史] 已重做");
    } else {
        push_console_log(L"[错误] 重做失败");
    }
    g_state.history_replaying = 0;
}

static void sync_scene_runtime_params(void) {
    SceneConfig* cfg;
    int i;
    if (g_state.engine == NULL) return;
    cfg = &g_state.scenes[g_state.scene_index];
    physics_engine_set_gravity(g_state.engine, vec2(0.0f, cfg->gravity_y));
    physics_engine_set_time_step(g_state.engine, cfg->time_step);
    physics_engine_set_damping(g_state.engine, cfg->damping);
    physics_engine_set_iterations(g_state.engine, cfg->iterations);
    for (i = 0; i < physics_engine_get_body_count(g_state.engine); i++) {
        RigidBody* b = physics_engine_get_body(g_state.engine, i);
        if (b == NULL || b->shape == NULL || b->type != BODY_DYNAMIC) continue;
        if (b->shape->type == SHAPE_CIRCLE) {
            b->mass = cfg->ball_mass;
            b->inv_mass = (b->mass > 0.0f) ? (1.0f / b->mass) : 0.0f;
            b->inertia = shape_get_moment_of_inertia(b->shape, b->mass);
            b->inv_inertia = (b->inertia > 0.0f) ? (1.0f / b->inertia) : 0.0f;
            b->shape->restitution = cfg->ball_restitution;
        } else if (b->shape->type == SHAPE_POLYGON) {
            b->mass = cfg->box_mass;
            b->inv_mass = (b->mass > 0.0f) ? (1.0f / b->mass) : 0.0f;
            b->inertia = shape_get_moment_of_inertia(b->shape, b->mass);
            b->inv_inertia = (b->inertia > 0.0f) ? (1.0f / b->inertia) : 0.0f;
            b->shape->restitution = cfg->box_restitution;
        }
    }
}

static void debug_adjust_runtime_param(int idx, int sign) {
    SceneConfig* cfg = &g_state.scenes[g_state.scene_index];
    if (sign == 0) return;
    history_push_snapshot();
    if (idx == 0) cfg->gravity_y = clamp(cfg->gravity_y + 0.2f * sign, 0.0f, 30.0f);
    if (idx == 1) cfg->time_step = clamp(cfg->time_step + 0.001f * sign, 0.001f, 0.05f);
    if (idx == 2) {
        cfg->iterations += sign;
        if (cfg->iterations < 1) cfg->iterations = 1;
        if (cfg->iterations > 64) cfg->iterations = 64;
    }
    sync_scene_runtime_params();
    push_console_log(L"[调试] 参数已调整");
}

static float get_param_min(int param_index) {
    switch (param_index) {
        case 0: return -20.0f;
        case 1: return 0.002f;
        case 2: return 0.90f;
        case 3: return 1.0f;
        case 4: return 0.0f;
        case 5: return 0.0f;
        case 6: return 0.1f;
        default: return 0.1f;
    }
}

static float get_param_max(int param_index) {
    switch (param_index) {
        case 0: return 40.0f;
        case 1: return 0.05f;
        case 2: return 1.0f;
        case 3: return 30.0f;
        case 4: return 1.0f;
        case 5: return 1.0f;
        case 6: return 50.0f;
        default: return 50.0f;
    }
}

static float get_param_value(const SceneConfig* cfg, int param_index) {
    if (cfg == NULL) return 0.0f;
    switch (param_index) {
        case 0: return cfg->gravity_y;
        case 1: return cfg->time_step;
        case 2: return cfg->damping;
        case 3: return (float)cfg->iterations;
        case 4: return cfg->ball_restitution;
        case 5: return cfg->box_restitution;
        case 6: return cfg->ball_mass;
        default: return cfg->box_mass;
    }
}

static int can_adjust_param(const SceneConfig* cfg, int param_index, int sign) {
    float v;
    float min_v;
    float max_v;
    const float eps = 1e-6f;
    if (cfg == NULL) return 0;
    if (sign != -1 && sign != 1) return 0;
    v = get_param_value(cfg, param_index);
    min_v = get_param_min(param_index);
    max_v = get_param_max(param_index);
    if (sign < 0) return v > min_v + eps;
    return v < max_v - eps;
}

static int project_tree_append_line(const wchar_t* line) {
    if (line == NULL) return 0;
    if (g_project_tree_count >= PROJECT_TREE_MAX_ITEMS) return 0;
    lstrcpynW(g_project_tree_items[g_project_tree_count], line, PROJECT_TREE_LINE_MAX);
    g_project_tree_count++;
    return 1;
}

static void project_tree_make_indent(int depth, wchar_t* out, int cap) {
    int i;
    int n = 0;
    if (out == NULL || cap <= 0) return;
    for (i = 0; i < depth * 2 && n < cap - 1; i++) {
        out[n++] = L' ';
    }
    out[n] = L'\0';
}

static int project_tree_should_descend(const wchar_t* name, int depth, int max_depth) {
    if (name == NULL) return 0;
    if (depth >= max_depth) return 0;
    if (_wcsicmp(name, L".git") == 0) return 0;
    if (_wcsicmp(name, L"backups") == 0) return 0;
    if (_wcsicmp(name, L"bin") == 0) return 0;
    if (_wcsicmp(name, L"obj") == 0) return 0;
    if (_wcsicmp(name, L"dist") == 0) return 0;
    return 1;
}

static void project_tree_add_entry_line(int depth, int is_dir, const wchar_t* name) {
    wchar_t indent[64];
    wchar_t line[PROJECT_TREE_LINE_MAX];
    if (name == NULL || name[0] == L'\0') return;
    project_tree_make_indent(depth, indent, 64);
    if (is_dir) {
        swprintf(line, PROJECT_TREE_LINE_MAX, L"%ls? %ls/", indent, name);
        g_project_tree_dir_count++;
    } else {
        swprintf(line, PROJECT_TREE_LINE_MAX, L"%ls%ls", indent, name);
        g_project_tree_file_count++;
    }
    project_tree_append_line(line);
}

static void project_tree_build_recursive(const wchar_t* base_path, int depth, int max_depth) {
    int pass;
    if (base_path == NULL) return;
    for (pass = 0; pass < 2 && g_project_tree_count < PROJECT_TREE_MAX_ITEMS; pass++) {
        wchar_t pattern[520];
        WIN32_FIND_DATAW fd;
        HANDLE hFind;
        swprintf(pattern, 520, L"%ls\\*", base_path);
        hFind = FindFirstFileW(pattern, &fd);
        if (hFind == INVALID_HANDLE_VALUE) continue;
        do {
            int is_dir;
            wchar_t child_path[520];
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;
            is_dir = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
            if (pass == 0 && !is_dir) continue;
            if (pass == 1 && is_dir) continue;
            project_tree_add_entry_line(depth, is_dir, fd.cFileName);
            if (g_project_tree_count >= PROJECT_TREE_MAX_ITEMS) break;
            if (is_dir && project_tree_should_descend(fd.cFileName, depth, max_depth)) {
                swprintf(child_path, 520, L"%ls\\%ls", base_path, fd.cFileName);
                project_tree_build_recursive(child_path, depth + 1, max_depth);
            }
        } while (FindNextFileW(hFind, &fd) && g_project_tree_count < PROJECT_TREE_MAX_ITEMS);
        FindClose(hFind);
    }
}

static void build_project_tree_lines(void) {
    g_project_tree_count = 0;
    g_project_tree_dir_count = 0;
    g_project_tree_file_count = 0;
    project_tree_build_recursive(L".", 0, 3);
    if (g_project_tree_count <= 0) {
        project_tree_append_line(L"(空)");
    }
}

static void adjust_focused_param(int sign) {
    SceneConfig* cfg = &g_state.scenes[g_state.scene_index];
    if (!can_adjust_param(cfg, g_state.focused_param, sign)) return;
    if (g_state.focused_param == 0) {
        cfg->gravity_y += sign * 0.5f;
        if (cfg->gravity_y < -20.0f) cfg->gravity_y = -20.0f;
        if (cfg->gravity_y > 40.0f) cfg->gravity_y = 40.0f;
    } else if (g_state.focused_param == 1) {
        cfg->time_step += sign * 0.001f;
        if (cfg->time_step < 0.002f) cfg->time_step = 0.002f;
        if (cfg->time_step > 0.05f) cfg->time_step = 0.05f;
    } else if (g_state.focused_param == 2) {
        cfg->damping += sign * 0.001f;
        if (cfg->damping < 0.90f) cfg->damping = 0.90f;
        if (cfg->damping > 1.0f) cfg->damping = 1.0f;
    } else if (g_state.focused_param == 3) {
        cfg->iterations += sign;
        if (cfg->iterations < 1) cfg->iterations = 1;
        if (cfg->iterations > 30) cfg->iterations = 30;
    } else if (g_state.focused_param == 4) {
        cfg->ball_restitution += sign * 0.01f;
        if (cfg->ball_restitution < 0.0f) cfg->ball_restitution = 0.0f;
        if (cfg->ball_restitution > 1.0f) cfg->ball_restitution = 1.0f;
    } else if (g_state.focused_param == 5) {
        cfg->box_restitution += sign * 0.01f;
        if (cfg->box_restitution < 0.0f) cfg->box_restitution = 0.0f;
        if (cfg->box_restitution > 1.0f) cfg->box_restitution = 1.0f;
    } else if (g_state.focused_param == 6) {
        cfg->ball_mass += sign * 0.1f;
        if (cfg->ball_mass < 0.1f) cfg->ball_mass = 0.1f;
        if (cfg->ball_mass > 50.0f) cfg->ball_mass = 50.0f;
    } else {
        cfg->box_mass += sign * 0.1f;
        if (cfg->box_mass < 0.1f) cfg->box_mass = 0.1f;
        if (cfg->box_mass > 50.0f) cfg->box_mass = 50.0f;
    }
    sync_scene_runtime_params();
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
    D2D1_ROUNDED_RECT rr;
    rr.rect = r;
    rr.radiusX = radius;
    rr.radiusY = radius;
    set_brush_color(fill.r, fill.g, fill.b, fill.a);
    ID2D1HwndRenderTarget_FillRoundedRectangle(g_ui.target, &rr, (ID2D1Brush*)g_ui.brush);
    set_brush_color(border.r, border.g, border.b, border.a);
    ID2D1HwndRenderTarget_DrawRoundedRectangle(g_ui.target, &rr, (ID2D1Brush*)g_ui.brush, 1.0f, NULL);
}

static void draw_panel_header_band(D2D1_RECT_F panel_rect, float top_h, float inset) {
    D2D1_RECT_F hb = rc(panel_rect.left + inset, panel_rect.top + 4.0f, panel_rect.right - inset, panel_rect.top + top_h + 2.0f);
    draw_card_round(hb, 7.0f, rgba(0.14f, 0.15f, 0.18f, 1.0f), rgba(0.24f, 0.26f, 0.31f, 1.0f));
}

static void draw_outer_shadow_rr(D2D1_ROUNDED_RECT rr) {
    int i;
    for (i = 1; i <= 5; i++) {
        float d = (float)i;
        float a = 0.11f - d * 0.018f;
        D2D1_ROUNDED_RECT srr;
        if (a < 0.01f) a = 0.01f;
        srr.rect = rc(rr.rect.left - d, rr.rect.top - d, rr.rect.right + d, rr.rect.bottom + d);
        srr.radiusX = rr.radiusX + d;
        srr.radiusY = rr.radiusY + d;
        set_brush_color(0.05f, 0.07f, 0.10f, a);
        ID2D1HwndRenderTarget_DrawRoundedRectangle(g_ui.target, &srr, (ID2D1Brush*)g_ui.brush, 1.0f, NULL);
    }
}

static void draw_action_button(D2D1_RECT_F r, const wchar_t* label, int active, int hovered) {
    D2D1_ROUNDED_RECT rr;
    D2D1_ROUNDED_RECT sr;
    D2D1_RECT_F shadow = rc(r.left, r.top + 1.5f, r.right, r.bottom + 1.5f);
    D2D1_COLOR_F bg;
    D2D1_COLOR_F bd;

    rr.rect = r;
    rr.radiusX = 10.0f;
    rr.radiusY = 10.0f;
    sr.rect = shadow;
    sr.radiusX = 10.0f;
    sr.radiusY = 10.0f;

    if (active) {
        bg = hovered ? rgba(0.25f, 0.41f, 0.63f, 1.0f) : rgba(0.22f, 0.36f, 0.56f, 1.0f);
        bd = rgba(0.39f, 0.56f, 0.76f, 1.0f);
    } else {
        bg = hovered ? rgba(0.20f, 0.23f, 0.28f, 1.0f) : rgba(0.17f, 0.19f, 0.24f, 1.0f);
        bd = hovered ? rgba(0.35f, 0.39f, 0.47f, 1.0f) : rgba(0.30f, 0.34f, 0.42f, 1.0f);
    }

    set_brush_color(0.05f, 0.07f, 0.10f, 0.22f);
    ID2D1HwndRenderTarget_FillRoundedRectangle(g_ui.target, &sr, (ID2D1Brush*)g_ui.brush);
    set_brush_color(bg.r, bg.g, bg.b, bg.a);
    ID2D1HwndRenderTarget_FillRoundedRectangle(g_ui.target, &rr, (ID2D1Brush*)g_ui.brush);
    set_brush_color(bd.r, bd.g, bd.b, bd.a);
    ID2D1HwndRenderTarget_DrawRoundedRectangle(g_ui.target, &rr, (ID2D1Brush*)g_ui.brush, 1.0f, NULL);
    draw_text(label, r, g_ui.fmt_button, active ? rgba(0.97f, 0.98f, 1.0f, 1.0f) : rgba(0.86f, 0.90f, 0.96f, 1.0f));
}

typedef enum {
    TB_ICON_SAVE = 0,
    TB_ICON_UNDO,
    TB_ICON_REDO,
    TB_ICON_GRID,
    TB_ICON_COLLISION,
    TB_ICON_VELOCITY,
    TB_ICON_CONSTRAINT,
    TB_ICON_SPRING,
    TB_ICON_CHAIN,
    TB_ICON_ROPE,
    TB_ICON_RUN,
    TB_ICON_PAUSE,
    TB_ICON_STEP,
    TB_ICON_RESET
} ToolbarIconId;

static void draw_toolbar_icon_button(D2D1_RECT_F r, ToolbarIconId icon, int active, int hovered) {
    D2D1_ROUNDED_RECT rr;
    D2D1_COLOR_F bg;
    D2D1_COLOR_F bd;
    D2D1_COLOR_F fg = active ? rgba(0.97f, 0.98f, 1.0f, 1.0f) : rgba(0.86f, 0.90f, 0.96f, 1.0f);
    float cx = (r.left + r.right) * 0.5f;
    float cy = (r.top + r.bottom) * 0.5f;
    float x0 = cx - 7.0f;
    float x1 = cx + 7.0f;
    float y0 = cy - 7.0f;
    float y1 = cy + 7.0f;

    rr.rect = r;
    rr.radiusX = 10.0f;
    rr.radiusY = 10.0f;
    if (active) {
        bg = hovered ? rgba(0.25f, 0.41f, 0.63f, 1.0f) : rgba(0.22f, 0.36f, 0.56f, 1.0f);
        bd = rgba(0.39f, 0.56f, 0.76f, 1.0f);
    } else {
        bg = hovered ? rgba(0.20f, 0.23f, 0.28f, 1.0f) : rgba(0.17f, 0.19f, 0.24f, 1.0f);
        bd = hovered ? rgba(0.35f, 0.39f, 0.47f, 1.0f) : rgba(0.30f, 0.34f, 0.42f, 1.0f);
    }
    set_brush_color(bg.r, bg.g, bg.b, bg.a);
    ID2D1HwndRenderTarget_FillRoundedRectangle(g_ui.target, &rr, (ID2D1Brush*)g_ui.brush);
    set_brush_color(bd.r, bd.g, bd.b, bd.a);
    ID2D1HwndRenderTarget_DrawRoundedRectangle(g_ui.target, &rr, (ID2D1Brush*)g_ui.brush, 1.0f, NULL);
    set_brush_color(fg.r, fg.g, fg.b, fg.a);

    if (icon == TB_ICON_SAVE) {
        D2D1_RECT_F fr = rc(x0, y0, x1, y1);
        D2D1_RECT_F sl = rc(x0 + 2.0f, y0 + 2.0f, x1 - 2.0f, cy - 1.0f);
        ID2D1HwndRenderTarget_DrawRectangle(g_ui.target, &fr, (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawRectangle(g_ui.target, &sl, (ID2D1Brush*)g_ui.brush, 1.0f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 2.0f, cy + 2.0f), pt(x1 - 2.0f, cy + 2.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
    } else if (icon == TB_ICON_UNDO || icon == TB_ICON_REDO) {
        int dir = (icon == TB_ICON_UNDO) ? -1 : 1;
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx + dir * 5.0f, cy - 4.0f), pt(cx - dir * 4.0f, cy - 4.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - dir * 4.0f, cy - 4.0f), pt(cx - dir * 4.0f, cy + 3.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - dir * 4.0f, cy + 3.0f), pt(cx + dir * 5.0f, cy + 3.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - dir * 7.0f, cy - 4.0f), pt(cx - dir * 4.0f, cy - 7.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - dir * 7.0f, cy - 4.0f), pt(cx - dir * 4.0f, cy - 1.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
    } else if (icon == TB_ICON_GRID) {
        int gi;
        for (gi = 0; gi < 3; gi++) {
            float gx = x0 + 1.0f + gi * 6.0f;
            float gy = y0 + 1.0f + gi * 6.0f;
            ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(gx, y0 + 1.0f), pt(gx, y1 - 1.0f), (ID2D1Brush*)g_ui.brush, 1.0f, NULL);
            ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 1.0f, gy), pt(x1 - 1.0f, gy), (ID2D1Brush*)g_ui.brush, 1.0f, NULL);
        }
    } else if (icon == TB_ICON_COLLISION) {
        D2D1_ELLIPSE e1 = {pt(cx - 3.0f, cy), 4.0f, 4.0f};
        D2D1_ELLIPSE e2 = {pt(cx + 3.0f, cy), 4.0f, 4.0f};
        ID2D1HwndRenderTarget_DrawEllipse(g_ui.target, &e1, (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawEllipse(g_ui.target, &e2, (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
    } else if (icon == TB_ICON_VELOCITY) {
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0, cy), pt(x1 - 3.0f, cy), (ID2D1Brush*)g_ui.brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x1 - 3.0f, cy), pt(x1 - 7.0f, cy - 3.0f), (ID2D1Brush*)g_ui.brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x1 - 3.0f, cy), pt(x1 - 7.0f, cy + 3.0f), (ID2D1Brush*)g_ui.brush, 1.4f, NULL);
    } else if (icon == TB_ICON_CONSTRAINT) {
        D2D1_ELLIPSE e1 = {pt(cx - 3.6f, cy), 3.0f, 2.3f};
        D2D1_ELLIPSE e2 = {pt(cx + 3.6f, cy), 3.0f, 2.3f};
        ID2D1HwndRenderTarget_DrawEllipse(g_ui.target, &e1, (ID2D1Brush*)g_ui.brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawEllipse(g_ui.target, &e2, (ID2D1Brush*)g_ui.brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - 0.8f, cy), pt(cx + 0.8f, cy), (ID2D1Brush*)g_ui.brush, 1.4f, NULL);
    } else if (icon == TB_ICON_SPRING) {
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 0.5f, cy), pt(x0 + 3.0f, cy), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 3.0f, cy), pt(x0 + 5.0f, cy - 3.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 5.0f, cy - 3.0f), pt(x0 + 7.0f, cy + 3.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 7.0f, cy + 3.0f), pt(x0 + 9.0f, cy - 3.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 9.0f, cy - 3.0f), pt(x0 + 11.0f, cy + 3.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 11.0f, cy + 3.0f), pt(x1 - 0.5f, cy), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
    } else if (icon == TB_ICON_CHAIN) {
        D2D1_ELLIPSE e1 = {pt(cx - 5.2f, cy), 2.4f, 1.9f};
        D2D1_ELLIPSE e2 = {pt(cx, cy), 2.4f, 1.9f};
        D2D1_ELLIPSE e3 = {pt(cx + 5.2f, cy), 2.4f, 1.9f};
        ID2D1HwndRenderTarget_DrawEllipse(g_ui.target, &e1, (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawEllipse(g_ui.target, &e2, (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawEllipse(g_ui.target, &e3, (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - 2.8f, cy), pt(cx - 2.2f, cy), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx + 2.2f, cy), pt(cx + 2.8f, cy), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
    } else if (icon == TB_ICON_ROPE) {
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 0.5f, cy + 1.5f), pt(x0 + 2.8f, cy + 1.5f), (ID2D1Brush*)g_ui.brush, 1.1f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 2.8f, cy + 1.5f), pt(x0 + 4.8f, cy - 2.0f), (ID2D1Brush*)g_ui.brush, 1.1f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 4.8f, cy - 2.0f), pt(x0 + 6.8f, cy + 2.0f), (ID2D1Brush*)g_ui.brush, 1.1f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 6.8f, cy + 2.0f), pt(x0 + 8.8f, cy - 2.0f), (ID2D1Brush*)g_ui.brush, 1.1f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 8.8f, cy - 2.0f), pt(x0 + 10.8f, cy + 2.0f), (ID2D1Brush*)g_ui.brush, 1.1f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(x0 + 10.8f, cy + 2.0f), pt(x1 - 0.5f, cy + 1.5f), (ID2D1Brush*)g_ui.brush, 1.1f, NULL);
    } else if (icon == TB_ICON_RUN) {
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - 3.0f, cy - 5.0f), pt(cx + 5.0f, cy), (ID2D1Brush*)g_ui.brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx + 5.0f, cy), pt(cx - 3.0f, cy + 5.0f), (ID2D1Brush*)g_ui.brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - 3.0f, cy + 5.0f), pt(cx - 3.0f, cy - 5.0f), (ID2D1Brush*)g_ui.brush, 1.4f, NULL);
    } else if (icon == TB_ICON_PAUSE) {
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - 3.5f, cy - 5.0f), pt(cx - 3.5f, cy + 5.0f), (ID2D1Brush*)g_ui.brush, 2.0f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx + 3.5f, cy - 5.0f), pt(cx + 3.5f, cy + 5.0f), (ID2D1Brush*)g_ui.brush, 2.0f, NULL);
    } else if (icon == TB_ICON_STEP) {
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - 6.0f, cy - 6.0f), pt(cx - 6.0f, cy + 6.0f), (ID2D1Brush*)g_ui.brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - 2.0f, cy - 5.0f), pt(cx + 5.0f, cy), (ID2D1Brush*)g_ui.brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx + 5.0f, cy), pt(cx - 2.0f, cy + 5.0f), (ID2D1Brush*)g_ui.brush, 1.4f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - 2.0f, cy + 5.0f), pt(cx - 2.0f, cy - 5.0f), (ID2D1Brush*)g_ui.brush, 1.4f, NULL);
    } else if (icon == TB_ICON_RESET) {
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - 6.0f, cy + 3.0f), pt(cx - 1.0f, cy - 3.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - 1.0f, cy - 3.0f), pt(cx + 5.0f, cy + 1.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx + 5.0f, cy + 1.0f), pt(cx + 2.0f, cy + 1.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx + 5.0f, cy + 1.0f), pt(cx + 5.0f, cy - 2.0f), (ID2D1Brush*)g_ui.brush, 1.2f, NULL);
    }
}

static void draw_icon_minus(D2D1_RECT_F r, D2D1_COLOR_F color, float thickness) {
    float cx = (r.left + r.right) * 0.5f;
    float cy = (r.top + r.bottom) * 0.5f;
    set_brush_color(color.r, color.g, color.b, color.a);
    ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - 4.5f, cy), pt(cx + 4.5f, cy), (ID2D1Brush*)g_ui.brush, thickness, NULL);
}

static void draw_icon_plus(D2D1_RECT_F r, D2D1_COLOR_F color, float thickness) {
    float cx = (r.left + r.right) * 0.5f;
    float cy = (r.top + r.bottom) * 0.5f;
    set_brush_color(color.r, color.g, color.b, color.a);
    ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - 4.5f, cy), pt(cx + 4.5f, cy), (ID2D1Brush*)g_ui.brush, thickness, NULL);
    ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx, cy - 4.5f), pt(cx, cy + 4.5f), (ID2D1Brush*)g_ui.brush, thickness, NULL);
}

static void draw_icon_chevron(D2D1_RECT_F r, int up, D2D1_COLOR_F color, float thickness) {
    float cx = (r.left + r.right) * 0.5f;
    float cy = (r.top + r.bottom) * 0.5f;
    float w = 4.0f;
    float h = 3.0f;
    set_brush_color(color.r, color.g, color.b, color.a);
    if (up) {
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - w, cy + h * 0.5f), pt(cx, cy - h), (ID2D1Brush*)g_ui.brush, thickness, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx, cy - h), pt(cx + w, cy + h * 0.5f), (ID2D1Brush*)g_ui.brush, thickness, NULL);
    } else {
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - w, cy - h * 0.5f), pt(cx, cy + h), (ID2D1Brush*)g_ui.brush, thickness, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx, cy + h), pt(cx + w, cy - h * 0.5f), (ID2D1Brush*)g_ui.brush, thickness, NULL);
    }
}

static void draw_icon_chevron_lr(D2D1_RECT_F r, int left, D2D1_COLOR_F color, float thickness) {
    float cx = (r.left + r.right) * 0.5f;
    float cy = (r.top + r.bottom) * 0.5f;
    float w = 3.5f;
    float h = 4.5f;
    set_brush_color(color.r, color.g, color.b, color.a);
    if (left) {
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx + w * 0.5f, cy - h), pt(cx - w, cy), (ID2D1Brush*)g_ui.brush, thickness, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - w, cy), pt(cx + w * 0.5f, cy + h), (ID2D1Brush*)g_ui.brush, thickness, NULL);
    } else {
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx - w * 0.5f, cy - h), pt(cx + w, cy), (ID2D1Brush*)g_ui.brush, thickness, NULL);
        ID2D1HwndRenderTarget_DrawLine(g_ui.target, pt(cx + w, cy), pt(cx - w * 0.5f, cy + h), (ID2D1Brush*)g_ui.brush, thickness, NULL);
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

static void inspector_adjust_row(int row, int sign) {
    const Constraint* csel = selected_constraint_ref();
    if (sign == 0) return;
    history_push_snapshot();
    if (csel != NULL && csel->active) {
        if (row == 0) physics_engine_constraint_set_rest_length(g_state.engine, g_state.selected_constraint_index, csel->rest_length + 0.20f * sign);
        if (row == 1) {
            float min_stiffness = (csel->type == CONSTRAINT_DISTANCE) ? 0.08f : 0.20f;
            physics_engine_constraint_set_stiffness(g_state.engine, g_state.selected_constraint_index,
                                                    max_f(min_stiffness, csel->stiffness + 0.50f * sign));
        }
        if (row == 2) physics_engine_constraint_set_damping(g_state.engine, g_state.selected_constraint_index, csel->damping + 0.20f * sign);
        if (row == 3) physics_engine_constraint_set_break_force(g_state.engine, g_state.selected_constraint_index, csel->break_force + 20.0f * sign);
        if (row == 4) physics_engine_constraint_set_collide_connected(g_state.engine, g_state.selected_constraint_index, !csel->collide_connected);
        if (row == 5 && csel->type == CONSTRAINT_SPRING) {
            int preset = spring_preset_for_constraint(csel);
            if (preset < 0) preset = 1;
            preset = (preset + (sign > 0 ? 1 : 2)) % 3;
            apply_spring_preset_to_constraint_index(g_state.selected_constraint_index, preset);
        }
        push_console_log(L"[检查器] 约束参数已调整");
        return;
    }
    if (g_state.selected != NULL) {
        RigidBody* b = g_state.selected;
        if (row == 0) {
            b->mass = max_f(0.1f, b->mass + 0.10f * sign);
            b->inv_mass = (b->mass > 0.0f) ? (1.0f / b->mass) : 0.0f;
            if (b->shape != NULL) {
                b->inertia = shape_get_moment_of_inertia(b->shape, b->mass);
                b->inv_inertia = (b->inertia > 0.0f) ? (1.0f / b->inertia) : 0.0f;
            }
        }
        if (row == 1) b->position.x += 0.2f * sign;
        if (row == 2) b->position.y += 0.2f * sign;
        if (row == 3) b->velocity.x += 0.2f * sign;
        if (row == 4) b->velocity.y += 0.2f * sign;
        if (row == 5) b->angular_velocity += 0.1f * sign;
        if (row == 6) b->damping = clamp(b->damping + 0.02f * sign, 0.0f, 1.0f);
        if (row == 7 && b->shape != NULL) b->shape->restitution = clamp(b->shape->restitution + 0.05f * sign, 0.0f, 1.0f);
        push_console_log(L"[检查器] 物体参数已调整");
    }
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

static void begin_value_input_for_log_search(void) {
    lstrcpynW(g_state.value_input_buf, g_state.log_search_buf, 64);
    g_state.value_input_target = 2;
    g_state.value_input_row = 0;
    g_state.show_value_input = 1;
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
    int i;
    wchar_t line[128];
    HRESULT hr = create_device_resources(hwnd);
    if (FAILED(hr)) return;

    GetClientRect(hwnd, &wr);
    w = (float)(wr.right - wr.left);
    h = (float)(wr.bottom - wr.top);
    left_w = w * g_state.ui_left_ratio;
    right_w = w * g_state.ui_right_ratio;
    if (left_w < 148.0f) left_w = 148.0f;
    if (left_w > 360.0f) left_w = 360.0f;
    if (right_w < 220.0f) right_w = 220.0f;
    if (right_w > 460.0f) right_w = 460.0f;
    if (g_state.ui_show_bottom_panel && bottom_h < 24.0f) bottom_h = 24.0f;

    D2D1_COLOR_F clear_color = g_state.ui_theme_light ? rgba(0.92f, 0.94f, 0.98f, 1.0f) : rgba(0.10f, 0.11f, 0.13f, 1.0f);
    D2D1_RECT_F top_rect = rc(8.0f, 8.0f, w - 8.0f, 8.0f + top_h);
    D2D1_RECT_F status_rect = rc(8.0f, h - status_h - 8.0f, w - 8.0f, h - 8.0f);
    D2D1_RECT_F bottom_rect = g_state.ui_show_bottom_panel
                                  ? rc(8.0f, status_rect.top - bottom_h - 8.0f, w - 8.0f, status_rect.top - 8.0f)
                                  : rc(0.0f, 0.0f, 0.0f, 0.0f);
    {
        float work_bottom = g_state.ui_show_bottom_panel ? (bottom_rect.top - 8.0f) : (status_rect.top - 8.0f);
        float center_left = g_state.ui_show_left_panel ? (8.0f + left_w + 8.0f) : 8.0f;
        float center_right = g_state.ui_show_right_panel ? (w - right_w - 16.0f) : (w - 8.0f);
        D2D1_RECT_F left_rect = g_state.ui_show_left_panel ? rc(8.0f, top_rect.bottom + 8.0f, 8.0f + left_w, work_bottom) : rc(0.0f, 0.0f, 0.0f, 0.0f);
        D2D1_RECT_F right_rect = g_state.ui_show_right_panel ? rc(w - right_w - 8.0f, top_rect.bottom + 8.0f, w - 8.0f, work_bottom) : rc(0.0f, 0.0f, 0.0f, 0.0f);
        D2D1_RECT_F center_rect = rc(center_left, top_rect.bottom + 8.0f, center_right, work_bottom);
    D2D1_RECT_F stage_rect = rc(center_rect.left + 10.0f, center_rect.top + 46.0f, center_rect.right - 10.0f, center_rect.bottom - 8.0f);
    g_splitter_left_rect = rc(0, 0, 0, 0);
    g_splitter_right_rect = rc(0, 0, 0, 0);
    g_splitter_bottom_rect = rc(0, 0, 0, 0);

    D2D1_ROUNDED_RECT left_rr;
    D2D1_ROUNDED_RECT center_rr;
    D2D1_ROUNDED_RECT right_rr;
    D2D1_ROUNDED_RECT bottom_rr;
    D2D1_ROUNDED_RECT status_rr;
    D2D1_ROUNDED_RECT stage_rr;

    g_state.stage_left = stage_rect.left;
    g_state.stage_top = stage_rect.top;
    g_state.stage_right = stage_rect.right;
    g_state.stage_bottom = stage_rect.bottom;
    if (g_state.scene_needs_stage_fit) {
        fit_dynamic_bodies_into_stage();
        g_state.scene_needs_stage_fit = 0;
    }
    ID2D1HwndRenderTarget_BeginDraw(g_ui.target);
    ID2D1HwndRenderTarget_Clear(g_ui.target, &clear_color);

    left_rr.rect = left_rect;
    left_rr.radiusX = 9.0f;
    left_rr.radiusY = 9.0f;
    center_rr.rect = center_rect;
    center_rr.radiusX = 9.0f;
    center_rr.radiusY = 9.0f;
    right_rr.rect = right_rect;
    right_rr.radiusX = 9.0f;
    right_rr.radiusY = 9.0f;
    bottom_rr.rect = bottom_rect;
    bottom_rr.radiusX = 9.0f;
    bottom_rr.radiusY = 9.0f;
    status_rr.rect = status_rect;
    status_rr.radiusX = 8.0f;
    status_rr.radiusY = 8.0f;
    stage_rr.rect = stage_rect;
    stage_rr.radiusX = 8.0f;
    stage_rr.radiusY = 8.0f;

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

    {
        D2D1_RECT_F menu_rect = rc(top_rect.left + 4.0f, top_rect.top + 3.0f, top_rect.right - 4.0f, top_rect.top + 3.0f + menu_h);
        D2D1_RECT_F menu_sep = rc(0.0f, menu_rect.bottom + 1.0f, w, menu_rect.bottom + 2.5f);
        D2D1_RECT_F toolbar_rect = rc(top_rect.left + 4.0f, menu_rect.bottom + 8.0f, top_rect.right - 4.0f, menu_rect.bottom + 8.0f + toolbar_h);
        D2D1_ROUNDED_RECT toolbar_rr;
        int is_zoomed = IsZoomed(hwnd) ? 1 : 0;
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
        {
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
        if (g_state.open_menu_id > 0) {
            const wchar_t* rows[8] = {0};
            int row_n = 0;
            int ri;
            D2D1_RECT_F anchor = g_menu_file_rect;
            float max_item_w = 0.0f;
            float max_short_w = 0.0f;
            float shortcut_slot = 0.0f;
            float dropdown_w = 0.0f;
            if (g_state.open_menu_id == 1) { row_n = 4; rows[0] = L"保存快照"; rows[1] = L"加载快照"; rows[2] = L"恢复自动保存"; rows[3] = L"约束调试开关"; anchor = g_menu_file_rect; }
            if (g_state.open_menu_id == 2) {
                row_n = 7;
                rows[0] = L"撤销";
                rows[1] = L"重做";
                rows[2] = L"复制对象";
                rows[3] = L"粘贴对象";
                rows[4] = L"创建圆";
                rows[5] = L"创建方块";
                rows[6] = L"清空日志";
                anchor = g_menu_edit_rect;
            }
            if (g_state.open_menu_id == 3) { row_n = 3; rows[0] = L"中心点显示"; rows[1] = L"碰撞显示"; rows[2] = L"速度向量显示"; anchor = g_menu_view_rect; }
            if (g_state.open_menu_id == 4) { row_n = 0; anchor = g_menu_component_rect; }
            if (g_state.open_menu_id == 5) { row_n = 3; rows[0] = L"运行/暂停"; rows[1] = L"单步"; rows[2] = L"重置场景"; anchor = g_menu_physics_rect; }
            if (g_state.open_menu_id == 6) {
                row_n = 6;
                rows[0] = L"切换布局预设";
                rows[1] = L"显示左侧栏";
                rows[2] = L"显示右侧栏";
                rows[3] = L"显示底部栏";
                rows[4] = L"切换控制台/性能";
                rows[5] = L"切换浅色主题";
                anchor = g_menu_window_rect;
            }
            if (g_state.open_menu_id == 7) {
                row_n = 3;
                rows[0] = L"使用说明";
                rows[1] = L"物理调试指南";
                rows[2] = L"版本信息";
                anchor = g_menu_help_word_rect;
            }
            for (ri = 0; ri < row_n; ri++) {
                wchar_t measure_row[96];
                const wchar_t* sk = menu_shortcut_text(g_state.open_menu_id, ri);
                float iw;
                float sw;
                swprintf(measure_row, 96, L"? %s", rows[ri]);
                iw = measure_text_width(measure_row, g_ui.fmt_info);
                sw = measure_text_width(sk, g_ui.fmt_info);
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
                wchar_t row_text[96];
                int enabled = 1;
                int checked = 0;
                g_menu_item_rect[ri] = rr;
                if (g_state.open_menu_id == 1 && ri == 3) checked = g_state.draw_constraints;
                if (g_state.open_menu_id == 3 && ri == 0) checked = g_state.draw_centers;
                if (g_state.open_menu_id == 3 && ri == 1) checked = g_state.draw_contacts;
                if (g_state.open_menu_id == 3 && ri == 2) checked = g_state.draw_velocity;
                if (g_state.open_menu_id == 5 && ri == 0) checked = g_state.running;
                if (g_state.open_menu_id == 6 && ri == 1) checked = g_state.ui_show_left_panel;
                if (g_state.open_menu_id == 6 && ri == 2) checked = g_state.ui_show_right_panel;
                if (g_state.open_menu_id == 6 && ri == 3) checked = g_state.ui_show_bottom_panel;
                if (g_state.open_menu_id == 6 && ri == 5) checked = g_state.ui_theme_light;
                enabled = menu_item_enabled_state(g_state.open_menu_id, ri);
                g_menu_item_enabled[ri] = enabled;
                swprintf(row_text, 96, L"%s%s", checked ? L"? " : L"  ", rows[ri]);
                draw_card_round(rr, 5.0f,
                                (enabled && (point_in_rect(g_state.mouse_screen, rr) || ri == g_state.open_menu_focus_index))
                                    ? rgba(0.25f, 0.33f, 0.45f, 1.0f)
                                    : rgba(0.18f, 0.20f, 0.25f, 1.0f),
                                rgba(0.30f, 0.33f, 0.40f, 1.0f));
                draw_text(row_text, rc(rr.left + 4.0f, rr.top, rr.right - (shortcut_slot > 0.0f ? (shortcut_slot + 10.0f) : 6.0f), rr.bottom), g_ui.fmt_info,
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
        {
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
    }
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

    g_left_fold_rect = rc(0, 0, 0, 0);
    g_right_fold_rect = rc(0, 0, 0, 0);

    if (g_state.ui_show_left_panel) {
        float row_h = 24.0f;
        float y = left_rect.top + 46.0f;
        float split_y = left_rect.top + (left_rect.bottom - left_rect.top) * 0.58f;
        D2D1_RECT_F hierarchy_rect = rc(left_rect.left + 8.0f, left_rect.top + 8.0f, left_rect.right - 8.0f, split_y - 4.0f);
        D2D1_RECT_F project_rect = rc(left_rect.left + 8.0f, split_y + 4.0f, left_rect.right - 8.0f, left_rect.bottom - 8.0f);
        D2D1_RECT_F hierarchy_viewport = rc(hierarchy_rect.left + 10.0f, hierarchy_rect.top + 68.0f, hierarchy_rect.right - 18.0f, hierarchy_rect.bottom - 10.0f);
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
        int body_group_rows = 0;
        int constraint_group_rows = 0;
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
        g_hierarchy_search_rect = rc(hierarchy_rect.left + 10.0f, hierarchy_rect.top + 40.0f, hierarchy_rect.right - 44.0f, hierarchy_rect.top + 64.0f);
        g_hierarchy_search_clear_rect = rc(hierarchy_rect.right - 40.0f, hierarchy_rect.top + 40.0f, hierarchy_rect.right - 10.0f, hierarchy_rect.top + 64.0f);
        draw_card_round(g_hierarchy_search_rect, 5.0f, rgba(0.18f, 0.20f, 0.25f, 1.0f), rgba(0.27f, 0.31f, 0.38f, 1.0f));
        draw_text(g_state.hierarchy_filter_len > 0 ? g_state.hierarchy_filter_buf : L"过滤对象... (Ctrl+L)", g_hierarchy_search_rect, g_ui.fmt_info,
                  g_state.hierarchy_filter_len > 0 ? rgba(0.88f, 0.92f, 0.97f, 1.0f) : rgba(0.61f, 0.68f, 0.78f, 1.0f));
        draw_action_button(g_hierarchy_search_clear_rect, L"清", 0, point_in_rect(g_state.mouse_screen, g_hierarchy_search_clear_rect));
        g_tree_scene_header_rect = rc(0, 0, 0, 0);
        g_tree_bodies_header_rect = rc(0, 0, 0, 0);
        g_tree_constraints_header_rect = rc(0, 0, 0, 0);
        g_tree_body_circle_header_rect = rc(0, 0, 0, 0);
        g_tree_body_polygon_header_rect = rc(0, 0, 0, 0);
        g_tree_constraint_distance_header_rect = rc(0, 0, 0, 0);
        g_tree_constraint_spring_header_rect = rc(0, 0, 0, 0);
        if (g_state.engine != NULL) {
            for (i = 0; i < physics_engine_get_body_count(g_state.engine); i++) {
                RigidBody* b = physics_engine_get_body(g_state.engine, i);
                if (b == NULL || b->type != BODY_DYNAMIC) continue;
                swprintf(line, 128, L"#%d %s/%s", body_visible_count + 1, body_kind_name(b), body_shape_name(b));
                if (g_state.hierarchy_filter_len > 0 && wcsstr(line, g_state.hierarchy_filter_buf) == NULL) continue;
                body_visible_count++;
                if (b->shape != NULL && b->shape->type == SHAPE_CIRCLE) body_circle_visible_count++;
                else body_polygon_visible_count++;
            }
            for (i = 0; i < physics_engine_get_constraint_count(g_state.engine); i++) {
                const Constraint* c = physics_engine_get_constraint(g_state.engine, i);
                const wchar_t* ctype;
                if (c == NULL || !c->active) continue;
                ctype = (c->type == CONSTRAINT_DISTANCE) ? L"距离" : L"弹簧";
                swprintf(line, 128, L"#%d %s", constraint_visible_count + 1, ctype);
                if (g_state.hierarchy_filter_len > 0 && wcsstr(line, g_state.hierarchy_filter_buf) == NULL) continue;
                constraint_visible_count++;
                if (c->type == CONSTRAINT_DISTANCE) constraint_distance_visible_count++;
                else constraint_spring_visible_count++;
            }
        }
        body_group_rows = (body_circle_visible_count > 0 ? 1 : 0) + (body_polygon_visible_count > 0 ? 1 : 0);
        constraint_group_rows = (constraint_distance_visible_count > 0 ? 1 : 0) + (constraint_spring_visible_count > 0 ? 1 : 0);
        content_h = 0.0f;
        content_h += row_h;
        if (g_state.tree_scene_expanded) content_h += row_h + 4.0f;
        content_h += row_h;
        if (g_state.tree_bodies_expanded) {
            content_h += body_group_rows * row_h;
            if (g_state.tree_body_circle_expanded) content_h += body_circle_visible_count * (row_h + 4.0f);
            if (g_state.tree_body_polygon_expanded) content_h += body_polygon_visible_count * (row_h + 4.0f);
        }
        content_h += row_h;
        if (g_state.tree_constraints_expanded) {
            content_h += constraint_group_rows * row_h;
            if (g_state.tree_constraint_distance_expanded) content_h += constraint_distance_visible_count * (row_h + 4.0f);
            if (g_state.tree_constraint_spring_expanded) content_h += constraint_spring_visible_count * (row_h + 4.0f);
        }
        content_h += 6.0f;
        g_state.hierarchy_scroll_max = 0;
        if (content_h > viewport_h) {
            g_state.hierarchy_scroll_max = (int)(content_h - viewport_h + 0.5f);
        }
        if (g_state.hierarchy_scroll_offset < 0) g_state.hierarchy_scroll_offset = 0;
        if (g_state.hierarchy_scroll_offset > g_state.hierarchy_scroll_max) g_state.hierarchy_scroll_offset = g_state.hierarchy_scroll_max;

        y = hierarchy_rect.top + 68.0f - (float)g_state.hierarchy_scroll_offset;
        ID2D1HwndRenderTarget_PushAxisAlignedClip(g_ui.target, &hierarchy_viewport, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

        g_explorer_scene_rect = rc(0, 0, 0, 0);
        g_explorer_body_count = 0;
        g_explorer_constraint_count = 0;

        g_tree_scene_header_rect = rc(hierarchy_rect.left + 10.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
        draw_text(g_state.tree_scene_expanded ? L"? 场景" : L"? 场景", g_tree_scene_header_rect, g_ui.fmt_mono, rgba(0.82f, 0.87f, 0.94f, 1.0f));
        y += row_h;
        if (g_state.tree_scene_expanded) {
            g_explorer_scene_rect = rc(hierarchy_rect.left + 24.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
            draw_text(SCENE_NAMES[g_state.scene_index], g_explorer_scene_rect, g_ui.fmt_mono, rgba(0.88f, 0.92f, 0.97f, 1.0f));
            y += row_h + 4.0f;
        }

        g_tree_bodies_header_rect = rc(hierarchy_rect.left + 10.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
        swprintf(line, 128, L"%s 物体 (%d)", g_state.tree_bodies_expanded ? L"?" : L"?", body_visible_count);
        draw_text(line, g_tree_bodies_header_rect, g_ui.fmt_mono, rgba(0.82f, 0.87f, 0.94f, 1.0f));
        y += row_h;
        g_tree_body_circle_header_rect = rc(0, 0, 0, 0);
        g_tree_body_polygon_header_rect = rc(0, 0, 0, 0);
        if (g_state.tree_bodies_expanded && g_state.engine != NULL) {
            if (body_circle_visible_count > 0) {
                int circle_idx = 0;
                g_tree_body_circle_header_rect = rc(hierarchy_rect.left + 24.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
                swprintf(line, 128, L"%s 圆 (%d)", g_state.tree_body_circle_expanded ? L"?" : L"?", body_circle_visible_count);
                draw_text(line, g_tree_body_circle_header_rect, g_ui.fmt_info, rgba(0.74f, 0.82f, 0.92f, 1.0f));
                y += row_h;
                if (g_state.tree_body_circle_expanded) {
                    for (i = 0; i < physics_engine_get_body_count(g_state.engine) && bidx < EXPLORER_MAX_ITEMS; i++) {
                        RigidBody* b = physics_engine_get_body(g_state.engine, i);
                        D2D1_RECT_F row;
                        if (b == NULL || b->type != BODY_DYNAMIC || b->shape == NULL || b->shape->type != SHAPE_CIRCLE) continue;
                        swprintf(line, 128, L"#%d %s/%s", bi + 1, body_kind_name(b), body_shape_name(b));
                        if (g_state.hierarchy_filter_len > 0 && wcsstr(line, g_state.hierarchy_filter_buf) == NULL) continue;
                        row = rc(hierarchy_rect.left + 38.0f, y + circle_idx * (row_h + 4.0f), hierarchy_rect.right - 10.0f, y + circle_idx * (row_h + 4.0f) + row_h);
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
                g_tree_body_polygon_header_rect = rc(hierarchy_rect.left + 24.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
                swprintf(line, 128, L"%s 多边形 (%d)", g_state.tree_body_polygon_expanded ? L"?" : L"?", body_polygon_visible_count);
                draw_text(line, g_tree_body_polygon_header_rect, g_ui.fmt_info, rgba(0.74f, 0.82f, 0.92f, 1.0f));
                y += row_h;
                if (g_state.tree_body_polygon_expanded) {
                    for (i = 0; i < physics_engine_get_body_count(g_state.engine) && bidx < EXPLORER_MAX_ITEMS; i++) {
                        RigidBody* b = physics_engine_get_body(g_state.engine, i);
                        D2D1_RECT_F row;
                        if (b == NULL || b->type != BODY_DYNAMIC || b->shape == NULL || b->shape->type != SHAPE_POLYGON) continue;
                        swprintf(line, 128, L"#%d %s/%s", bi + 1, body_kind_name(b), body_shape_name(b));
                        if (g_state.hierarchy_filter_len > 0 && wcsstr(line, g_state.hierarchy_filter_buf) == NULL) continue;
                        row = rc(hierarchy_rect.left + 38.0f, y + poly_idx * (row_h + 4.0f), hierarchy_rect.right - 10.0f, y + poly_idx * (row_h + 4.0f) + row_h);
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
        g_explorer_body_count = bidx;

        g_tree_constraints_header_rect = rc(hierarchy_rect.left + 10.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
        swprintf(line, 128, L"%s 约束 (%d)", g_state.tree_constraints_expanded ? L"?" : L"?", constraint_visible_count);
        draw_text(line, g_tree_constraints_header_rect, g_ui.fmt_mono, rgba(0.82f, 0.87f, 0.94f, 1.0f));
        y += row_h;
        g_tree_constraint_distance_header_rect = rc(0, 0, 0, 0);
        g_tree_constraint_spring_header_rect = rc(0, 0, 0, 0);
        if (g_state.tree_constraints_expanded && g_state.engine != NULL) {
            if (constraint_distance_visible_count > 0) {
                int dist_idx = 0;
                g_tree_constraint_distance_header_rect = rc(hierarchy_rect.left + 24.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
                swprintf(line, 128, L"%s 距离约束 (%d)", g_state.tree_constraint_distance_expanded ? L"?" : L"?", constraint_distance_visible_count);
                draw_text(line, g_tree_constraint_distance_header_rect, g_ui.fmt_info, rgba(0.74f, 0.82f, 0.92f, 1.0f));
                y += row_h;
                if (g_state.tree_constraint_distance_expanded) {
                    for (i = 0; i < physics_engine_get_constraint_count(g_state.engine) && cidx < EXPLORER_MAX_ITEMS; i++) {
                        const Constraint* c = physics_engine_get_constraint(g_state.engine, i);
                        D2D1_RECT_F row;
                        if (c == NULL || !c->active || c->type != CONSTRAINT_DISTANCE) continue;
                        swprintf(line, 128, L"#%d 距离", ci + 1);
                        if (g_state.hierarchy_filter_len > 0 && wcsstr(line, g_state.hierarchy_filter_buf) == NULL) continue;
                        row = rc(hierarchy_rect.left + 38.0f, y + dist_idx * (row_h + 4.0f), hierarchy_rect.right - 10.0f, y + dist_idx * (row_h + 4.0f) + row_h);
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
                g_tree_constraint_spring_header_rect = rc(hierarchy_rect.left + 24.0f, y, hierarchy_rect.right - 10.0f, y + row_h);
                swprintf(line, 128, L"%s 弹簧约束 (%d)", g_state.tree_constraint_spring_expanded ? L"?" : L"?", constraint_spring_visible_count);
                draw_text(line, g_tree_constraint_spring_header_rect, g_ui.fmt_info, rgba(0.74f, 0.82f, 0.92f, 1.0f));
                y += row_h;
                if (g_state.tree_constraint_spring_expanded) {
                    for (i = 0; i < physics_engine_get_constraint_count(g_state.engine) && cidx < EXPLORER_MAX_ITEMS; i++) {
                        const Constraint* c = physics_engine_get_constraint(g_state.engine, i);
                        D2D1_RECT_F row;
                        if (c == NULL || !c->active || c->type != CONSTRAINT_SPRING) continue;
                        swprintf(line, 128, L"#%d 弹簧", ci + 1);
                        if (g_state.hierarchy_filter_len > 0 && wcsstr(line, g_state.hierarchy_filter_buf) == NULL) continue;
                        row = rc(hierarchy_rect.left + 38.0f, y + spring_idx * (row_h + 4.0f), hierarchy_rect.right - 10.0f, y + spring_idx * (row_h + 4.0f) + row_h);
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
        g_explorer_constraint_count = cidx;
        ID2D1HwndRenderTarget_PopAxisAlignedClip(g_ui.target);

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

        {
            int pi;
            float py;
            float project_content_h;
            float project_view_h;
            D2D1_RECT_F project_viewport = rc(project_rect.left + 10.0f, project_rect.top + 44.0f, project_rect.right - 18.0f, project_rect.bottom - 34.0f);
            unsigned int now_ms = (unsigned int)GetTickCount();
            if (g_project_tree_count <= 0 || (now_ms - g_state.project_tree_last_scan_ms) > 1500) {
                build_project_tree_lines();
                g_state.project_tree_last_scan_ms = now_ms;
            }
            project_content_h = row_h + (g_state.tree_project_expanded ? (g_project_tree_count * 24.0f) : 0.0f) + 6.0f;
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
            draw_text(g_state.tree_project_expanded ? L"? 项目根目录" : L"? 项目根目录", g_tree_project_root_rect, g_ui.fmt_mono,
                      rgba(0.82f, 0.87f, 0.94f, 1.0f));
            if (g_state.tree_project_expanded) {
                for (pi = 0; pi < g_project_tree_count; pi++) {
                    D2D1_RECT_F pr = rc(project_rect.left + 24.0f, py + row_h + pi * 24.0f, project_rect.right - 18.0f, py + row_h + pi * 24.0f + 20.0f);
                    draw_text(g_project_tree_items[pi], pr, g_ui.fmt_info, rgba(0.86f, 0.90f, 0.96f, 1.0f));
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
            swprintf(line, 128, L"目录: %d  文件: %d", g_project_tree_dir_count, g_project_tree_file_count);
            draw_text(line, rc(project_rect.left + 12.0f, project_rect.bottom - 28.0f, project_rect.right - 12.0f, project_rect.bottom - 8.0f),
                      g_ui.fmt_info, rgba(0.66f, 0.73f, 0.83f, 1.0f));
        }
    } else {
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
        g_project_tree_count = 0;
        g_project_tree_dir_count = 0;
        g_project_tree_file_count = 0;
    }

    draw_panel_header_band(center_rect, 38.0f, 10.0f);
    draw_text_vcenter(L"场景", rc(center_rect.left + 16.0f, center_rect.top + 4.0f, center_rect.right - 12.0f, center_rect.top + 40.0f),
                      g_ui.fmt_ui, rgba(0.90f, 0.93f, 0.98f, 1.0f));
    swprintf(line, 128, L"%s丨%s", SCENE_NAMES[g_state.scene_index], g_state.running ? L"运行中" : L"已暂停");
    draw_text_right_vcenter(line, rc(center_rect.left + 130.0f, center_rect.top + 4.0f, center_rect.right - 36.0f, center_rect.top + 40.0f),
                            g_ui.fmt_info, rgba(0.67f, 0.75f, 0.88f, 1.0f));

    draw_outer_shadow_rr(stage_rr);
    set_brush_color(0.11f, 0.12f, 0.14f, 1.0f);
    ID2D1HwndRenderTarget_FillRoundedRectangle(g_ui.target, &stage_rr, (ID2D1Brush*)g_ui.brush);
    set_brush_color(0.24f, 0.27f, 0.32f, 1.0f);
    ID2D1HwndRenderTarget_DrawRoundedRectangle(g_ui.target, &stage_rr, (ID2D1Brush*)g_ui.brush, 1.0f, NULL);

    if (g_state.engine != NULL) {
        ID2D1RoundedRectangleGeometry* stage_clip_geo = NULL;
        ID2D1Layer* stage_layer = NULL;
        int used_layer_clip = 0;
        D2D1_LAYER_PARAMETERS layer_params;
        D2D1_MATRIX_3X2_F identity;

        hr = ID2D1Factory_CreateRoundedRectangleGeometry(g_ui.d2d_factory, &stage_rr, &stage_clip_geo);
        if (SUCCEEDED(hr)) {
            hr = ID2D1HwndRenderTarget_CreateLayer(g_ui.target, NULL, &stage_layer);
        }
        if (SUCCEEDED(hr) && stage_layer != NULL && stage_clip_geo != NULL) {
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
    if (g_state.ui_show_right_panel) {
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
        float debug_content_h;
        float debug_view_h;
        float debug_offset_y;

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
        swprintf(line, 128, L"约束调试:%s", g_state.draw_constraints ? L"开" : L"关");
        draw_text_vcenter(line, rc(inspector.right - 150.0f, inspector.top + 4.0f, inspector.right - 12.0f, inspector.top + 40.0f), g_ui.fmt_info,
                          rgba(0.66f, 0.74f, 0.85f, 1.0f));
        step = 24.0f;
        lx0 = inspector.left + 12.0f;
        lx1 = inspector.left + 110.0f;
        vx0 = inspector.left + 116.0f;
        vx1 = inspector.right - 12.0f;
        g_inspector_viewport_rect = rc(inspector.left + 8.0f, inspector.top + 44.0f, inspector.right - 14.0f, inspector.bottom - 8.0f);
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
                D2D1_RECT_F rr = rc(inspector.left + 10.0f, inspector_offset_y + ri * step, inspector.right - 10.0f, inspector_offset_y + ri * step + 22.0f);
                D2D1_RECT_F minus_btn = rc(rr.right - 56.0f, rr.top + 1.0f, rr.right - 30.0f, rr.bottom - 1.0f);
                D2D1_RECT_F plus_btn = rc(rr.right - 28.0f, rr.top + 1.0f, rr.right - 2.0f, rr.bottom - 1.0f);
                g_ins_row_rect[ri] = rr;
                g_ins_minus_rect[ri] = minus_btn;
                g_ins_plus_rect[ri] = plus_btn;
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
                draw_text(line, rc(vx0, rr.top + 1.0f, rr.right - 60.0f, rr.bottom - 1.0f), g_ui.fmt_info, rgba(0.89f, 0.93f, 0.97f, 1.0f));
                draw_icon_minus(minus_btn, rgba(0.84f, 0.89f, 0.95f, 1.0f), 1.4f);
                draw_icon_plus(plus_btn, rgba(0.84f, 0.89f, 0.95f, 1.0f), 1.4f);
            }
            {
                const wchar_t* ih = inspector_row_hint_text();
                draw_text(L"提示: Enter输入  +/-微调  双击快速输入",
                          rc(lx0, inspector_offset_y + g_ins_row_count * step + 2.0f, vx1, inspector_offset_y + g_ins_row_count * step + 20.0f),
                          g_ui.fmt_info, rgba(0.62f, 0.70f, 0.80f, 1.0f));
                draw_text(ih, rc(lx0, inspector_offset_y + g_ins_row_count * step + 20.0f, vx1, inspector_offset_y + g_ins_row_count * step + 38.0f),
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
        draw_card_round(debug_rect, 10.0f, rgba(0.16f, 0.18f, 0.22f, 1.0f), rgba(0.29f, 0.33f, 0.40f, 1.0f));
        draw_panel_header_band(debug_rect, 34.0f, 10.0f);
        draw_text_vcenter(L"物理调试", rc(debug_rect.left + 16.0f, debug_rect.top + 4.0f, debug_rect.right - 12.0f, debug_rect.top + 36.0f),
                          g_ui.fmt_ui, rgba(0.78f, 0.86f, 0.96f, 1.0f));
        g_debug_viewport_rect = rc(debug_rect.left + 8.0f, debug_rect.top + 40.0f, debug_rect.right - 14.0f, debug_rect.bottom - 8.0f);
        debug_view_h = g_debug_viewport_rect.bottom - g_debug_viewport_rect.top;
        debug_content_h = 110.0f;
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
            for (di = 0; di < 3; di++) {
                D2D1_RECT_F rr = rc(debug_rect.left + 10.0f, debug_offset_y + 24.0f + di * 24.0f, debug_rect.right - 10.0f, debug_offset_y + 44.0f + di * 24.0f);
                D2D1_RECT_F minus_btn = rc(rr.right - 56.0f, rr.top + 1.0f, rr.right - 30.0f, rr.bottom - 1.0f);
                D2D1_RECT_F plus_btn = rc(rr.right - 28.0f, rr.top + 1.0f, rr.right - 2.0f, rr.bottom - 1.0f);
                g_dbg_row_rect[di] = rr;
                g_dbg_minus_rect[di] = minus_btn;
                g_dbg_plus_rect[di] = plus_btn;
                draw_card_round(rr, 5.0f, rgba(0.20f, 0.25f, 0.33f, 1.0f), rgba(0.33f, 0.40f, 0.52f, 1.0f));
                if (di == 0) swprintf(line, 128, L"%.2f", g_state.engine ? physics_engine_get_gravity(g_state.engine).y : 0.0f);
                if (di == 1) swprintf(line, 128, L"%.4f", g_state.engine ? physics_engine_get_time_step(g_state.engine) : 0.0f);
                if (di == 2) swprintf(line, 128, L"%d", g_state.engine ? physics_engine_get_iterations(g_state.engine) : 0);
                draw_text(labels[di], rc(rr.left + 6.0f, rr.top + 1.0f, rr.left + 110.0f, rr.bottom - 1.0f), g_ui.fmt_info, rgba(0.74f, 0.81f, 0.91f, 1.0f));
                draw_text(line, rc(rr.left + 116.0f, rr.top + 1.0f, rr.right - 60.0f, rr.bottom - 1.0f), g_ui.fmt_info, rgba(0.92f, 0.95f, 0.99f, 1.0f));
                draw_icon_minus(minus_btn, rgba(0.88f, 0.93f, 0.99f, 1.0f), 1.4f);
                draw_icon_plus(plus_btn, rgba(0.88f, 0.93f, 0.99f, 1.0f), 1.4f);
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
    } else {
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
        for (i = 0; i < 3; i++) {
            g_dbg_row_rect[i] = rc(0, 0, 0, 0);
            g_dbg_minus_rect[i] = rc(0, 0, 0, 0);
            g_dbg_plus_rect[i] = rc(0, 0, 0, 0);
        }
    }

    {
        float fold_top = ((top_rect.bottom + 8.0f) + work_bottom) * 0.5f - 13.0f;
        D2D1_COLOR_F fold_bg = rgba(0.19f, 0.22f, 0.29f, 1.0f);
        D2D1_COLOR_F fold_bd = rgba(0.33f, 0.39f, 0.49f, 1.0f);
        D2D1_COLOR_F fold_fg = rgba(0.86f, 0.91f, 0.97f, 1.0f);
        if (g_state.ui_show_left_panel) {
            g_left_fold_rect = rc(left_rect.right - 10.0f, fold_top, left_rect.right + 10.0f, fold_top + 26.0f);
            draw_card_round(g_left_fold_rect, 5.0f,
                            point_in_rect(g_state.mouse_screen, g_left_fold_rect) ? rgba(0.24f, 0.30f, 0.40f, 1.0f) : fold_bg,
                            fold_bd);
            draw_icon_chevron_lr(g_left_fold_rect, 1, fold_fg, 1.5f);
        } else {
            g_left_fold_rect = rc(center_rect.left + 2.0f, fold_top, center_rect.left + 22.0f, fold_top + 26.0f);
            draw_card_round(g_left_fold_rect, 5.0f,
                            point_in_rect(g_state.mouse_screen, g_left_fold_rect) ? rgba(0.24f, 0.30f, 0.40f, 1.0f) : fold_bg,
                            fold_bd);
            draw_icon_chevron_lr(g_left_fold_rect, 0, fold_fg, 1.5f);
        }
        if (g_state.ui_show_right_panel) {
            g_right_fold_rect = rc(right_rect.left - 10.0f, fold_top, right_rect.left + 10.0f, fold_top + 26.0f);
            draw_card_round(g_right_fold_rect, 5.0f,
                            point_in_rect(g_state.mouse_screen, g_right_fold_rect) ? rgba(0.24f, 0.30f, 0.40f, 1.0f) : fold_bg,
                            fold_bd);
            draw_icon_chevron_lr(g_right_fold_rect, 0, fold_fg, 1.5f);
        } else {
            g_right_fold_rect = rc(center_rect.right - 22.0f, fold_top, center_rect.right - 2.0f, fold_top + 26.0f);
            draw_card_round(g_right_fold_rect, 5.0f,
                            point_in_rect(g_state.mouse_screen, g_right_fold_rect) ? rgba(0.24f, 0.30f, 0.40f, 1.0f) : fold_bg,
                            fold_bd);
            draw_icon_chevron_lr(g_right_fold_rect, 1, fold_fg, 1.5f);
        }
    }

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
        draw_icon_chevron(g_bottom_fold_rect, g_state.bottom_panel_collapsed ? 1 : 0, rgba(0.90f, 0.94f, 0.99f, 1.0f), 1.5f);
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
    if (g_state.ui_show_bottom_panel && !g_state.bottom_panel_collapsed) {
        if (g_state.bottom_active_tab == 0) {
            int li;
            int shown = 0;
            int skip = g_state.log_scroll_offset;
            int total_lines = 0;
            int max_lines;
            float controls_right = bottom_rect.right - 36.0f;
            float x0;
            float gap = 8.0f;
            float line_h = 24.0f;
            float viewport_top;
            float viewport_bottom;
            float viewport_h;
            x0 = controls_right;
            g_log_search_clear_rect = rc(x0 - 60.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);
            x0 = g_log_search_clear_rect.left - gap;
            g_log_search_rect = rc(x0 - 150.0f, bottom_rect.top + 6.0f, x0, bottom_rect.top + 28.0f);
            x0 = g_log_search_rect.left - gap;
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
            draw_card_round(g_log_search_rect, 6.0f, rgba(0.19f, 0.24f, 0.32f, 1.0f), rgba(0.35f, 0.44f, 0.58f, 1.0f));
            draw_text(g_state.log_search_len > 0 ? g_state.log_search_buf : L"搜索... (Ctrl+F)", g_log_search_rect, g_ui.fmt_info,
                      g_state.log_search_len > 0 ? rgba(0.90f, 0.94f, 0.99f, 1.0f) : rgba(0.64f, 0.72f, 0.84f, 1.0f));
            draw_action_button(g_log_filter_all_rect, L"全部", g_state.log_filter_mode == 0, point_in_rect(g_state.mouse_screen, g_log_filter_all_rect));
            draw_action_button(g_log_filter_state_rect, L"状态", g_state.log_filter_mode == 1, point_in_rect(g_state.mouse_screen, g_log_filter_state_rect));
            draw_action_button(g_log_filter_physics_rect, L"物理", g_state.log_filter_mode == 2, point_in_rect(g_state.mouse_screen, g_log_filter_physics_rect));
            draw_action_button(g_log_filter_collision_rect, L"碰撞", g_state.log_filter_mode == 4, point_in_rect(g_state.mouse_screen, g_log_filter_collision_rect));
            draw_action_button(g_log_filter_warn_rect, L"警告", g_state.log_filter_mode == 3, point_in_rect(g_state.mouse_screen, g_log_filter_warn_rect));
            draw_action_button(g_log_clear_rect, L"清空", 0, point_in_rect(g_state.mouse_screen, g_log_clear_rect));
            draw_action_button(g_log_search_clear_rect, L"清词", 0, point_in_rect(g_state.mouse_screen, g_log_search_clear_rect));
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
        } else {
            D2D1_RECT_F graph = rc(bottom_rect.left + 280.0f, bottom_rect.top + 36.0f, bottom_rect.right - 12.0f, bottom_rect.bottom - 12.0f);
            int gi;
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
    }
    if (g_state.open_menu_id > 0) {
        const wchar_t* rows[8] = {0};
        int row_n = 0;
        int ri;
        D2D1_RECT_F anchor = g_menu_file_rect;
        float max_item_w = 0.0f;
        float max_short_w = 0.0f;
        float shortcut_slot = 0.0f;
        float dropdown_w = 0.0f;
        if (g_state.open_menu_id == 1) { row_n = 4; rows[0] = L"保存快照"; rows[1] = L"加载快照"; rows[2] = L"恢复自动保存"; rows[3] = L"约束调试开关"; anchor = g_menu_file_rect; }
        if (g_state.open_menu_id == 2) { row_n = 7; rows[0] = L"撤销"; rows[1] = L"重做"; rows[2] = L"复制对象"; rows[3] = L"粘贴对象"; rows[4] = L"创建圆"; rows[5] = L"创建方块"; rows[6] = L"清空日志"; anchor = g_menu_edit_rect; }
        if (g_state.open_menu_id == 3) { row_n = 3; rows[0] = L"中心点显示"; rows[1] = L"碰撞显示"; rows[2] = L"速度向量显示"; anchor = g_menu_view_rect; }
        if (g_state.open_menu_id == 4) { row_n = 0; anchor = g_menu_component_rect; }
        if (g_state.open_menu_id == 5) { row_n = 3; rows[0] = L"运行/暂停"; rows[1] = L"单步"; rows[2] = L"重置场景"; anchor = g_menu_physics_rect; }
        if (g_state.open_menu_id == 6) {
            row_n = 6;
            rows[0] = L"切换布局预设";
            rows[1] = L"显示左侧栏";
            rows[2] = L"显示右侧栏";
            rows[3] = L"显示底部栏";
            rows[4] = L"切换控制台/性能";
            rows[5] = L"切换浅色主题";
            anchor = g_menu_window_rect;
        }
        if (g_state.open_menu_id == 7) {
            row_n = 3;
            rows[0] = L"使用说明";
            rows[1] = L"物理调试指南";
            rows[2] = L"版本信息";
            anchor = g_menu_help_word_rect;
        }
        for (ri = 0; ri < row_n; ri++) {
            wchar_t measure_row[96];
            const wchar_t* sk = menu_shortcut_text(g_state.open_menu_id, ri);
            float iw;
            float sw;
            swprintf(measure_row, 96, L"? %s", rows[ri]);
            iw = measure_text_width(measure_row, g_ui.fmt_info);
            sw = measure_text_width(sk, g_ui.fmt_info);
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
            wchar_t row_text[96];
            int enabled = 1;
            int checked = 0;
            g_menu_item_rect[ri] = rr;
            if (g_state.open_menu_id == 1 && ri == 3) checked = g_state.draw_constraints;
            if (g_state.open_menu_id == 3 && ri == 0) checked = g_state.draw_centers;
            if (g_state.open_menu_id == 3 && ri == 1) checked = g_state.draw_contacts;
            if (g_state.open_menu_id == 3 && ri == 2) checked = g_state.draw_velocity;
            if (g_state.open_menu_id == 5 && ri == 0) checked = g_state.running;
            if (g_state.open_menu_id == 6 && ri == 1) checked = g_state.ui_show_left_panel;
            if (g_state.open_menu_id == 6 && ri == 2) checked = g_state.ui_show_right_panel;
            if (g_state.open_menu_id == 6 && ri == 3) checked = g_state.ui_show_bottom_panel;
            if (g_state.open_menu_id == 6 && ri == 5) checked = g_state.ui_theme_light;
            enabled = menu_item_enabled_state(g_state.open_menu_id, ri);
            g_menu_item_enabled[ri] = enabled;
            swprintf(row_text, 96, L"%s%s", checked ? L"? " : L"  ", rows[ri]);
            draw_card_round(rr, 5.0f,
                            (enabled && (point_in_rect(g_state.mouse_screen, rr) || ri == g_state.open_menu_focus_index))
                                ? rgba(0.25f, 0.33f, 0.45f, 1.0f)
                                : rgba(0.18f, 0.20f, 0.25f, 1.0f),
                            rgba(0.30f, 0.33f, 0.40f, 1.0f));
            draw_text(row_text, rc(rr.left + 4.0f, rr.top, rr.right - (shortcut_slot > 0.0f ? (shortcut_slot + 10.0f) : 6.0f), rr.bottom), g_ui.fmt_info,
                      enabled ? rgba(0.87f, 0.91f, 0.97f, 1.0f) : rgba(0.53f, 0.59f, 0.68f, 1.0f));
            {
                const wchar_t* sk = menu_shortcut_text(g_state.open_menu_id, ri);
                if (sk[0] != L'\0') {
                    draw_text(sk, rc(rr.right - (shortcut_slot + 6.0f), rr.top, rr.right - 6.0f, rr.bottom), g_ui.fmt_info,
                              enabled ? rgba(0.63f, 0.72f, 0.84f, 1.0f) : rgba(0.45f, 0.51f, 0.60f, 1.0f));
                }
            }
        }
    }

    swprintf(line, 128, L"对象:%d  约束:%d  接触:%d  回收:%d",
             count_dynamic_bodies(g_state.engine),
             g_state.engine ? physics_engine_get_constraint_count(g_state.engine) : 0,
             g_state.engine ? physics_engine_get_contact_count(g_state.engine) : 0,
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
                float cx = vm.left + 22.0f + g_state.value_input_caret * 8.0f;
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

        if (!g_state.show_value_input && g_state.show_help_modal) {
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

        if (!g_state.show_value_input && g_state.show_config_modal) {
            SceneConfig* cfg = &g_state.scenes[g_state.scene_index];
            D2D1_RECT_F row;
            D2D1_RECT_F minus_btn;
            D2D1_RECT_F plus_btn;
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
    }
    }

    hr = ID2D1HwndRenderTarget_EndDraw(g_ui.target, NULL, NULL);
    if (hr == D2DERR_RECREATE_TARGET) {
        discard_device_resources();
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
        if (g_state.running && physics_engine_get_contact_count(g_state.engine) != g_state.last_contact_count) {
            unsigned int now_ms = (unsigned int)GetTickCount();
            if ((now_ms - g_state.last_contact_log_ms) >= 500) {
                push_console_log(L"[碰撞] 接触数量: %d -> %d", g_state.last_contact_count, physics_engine_get_contact_count(g_state.engine));
                g_state.last_contact_log_ms = now_ms;
            }
            g_state.last_contact_count = physics_engine_get_contact_count(g_state.engine);
        }
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
    perf_push_sample((float)g_state.fps_display, g_state.physics_step_ms);
    InvalidateRect(hwnd, NULL, FALSE);
}

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_NCCALCSIZE:
            return 0;
        case WM_NCHITTEST: {
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
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            render(hwnd);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_SIZE: {
            if (g_ui.target != NULL) {
                D2D1_SIZE_U size = {(UINT32)LOWORD(lparam), (UINT32)HIWORD(lparam)};
                ID2D1HwndRenderTarget_Resize(g_ui.target, &size);
            }
            return 0;
        }
        case WM_TIMER:
            tick(hwnd);
            return 0;
        case WM_KEYDOWN:
            if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
                if (wparam == 'S') {
                    if (save_scene_snapshot("scene_snapshot.txt")) push_console_log(L"[快捷键] Ctrl+S 已保存 scene_snapshot.txt");
                    else push_console_log(L"[错误] 保存 scene_snapshot.txt 失败");
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
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
                    return 0;
                }
                if (wparam == 'Z') {
                    history_undo();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (wparam == 'Y') {
                    history_redo();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (wparam == 'C') {
                    if (copy_selected_body_to_clipboard()) push_console_log(L"[快捷键] Ctrl+C 已复制对象");
                    else push_console_log(L"[提示] 当前无可复制对象");
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (wparam == 'V') {
                    trace_spawn_step("key.ctrlv.begin", "");
                    history_push_snapshot();
                    if (paste_body_from_clipboard()) push_console_log(L"[快捷键] Ctrl+V 已粘贴对象");
                    else push_console_log(L"[提示] 剪贴板为空");
                    trace_spawn_step("key.ctrlv.end", "");
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
            if (g_state.show_value_input) {
                if (wparam == VK_RETURN) {
                    apply_value_input();
                    g_state.show_value_input = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (wparam == VK_ESCAPE) {
                    g_state.show_value_input = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
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
                    return 0;
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
                    return 0;
                }
                if (wparam == VK_LEFT) {
                    if (g_state.value_input_caret > 0) g_state.value_input_caret--;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (wparam == VK_RIGHT) {
                    if (g_state.value_input_caret < g_state.value_input_len) g_state.value_input_caret++;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (wparam == VK_HOME) {
                    g_state.value_input_caret = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (wparam == VK_END) {
                    g_state.value_input_caret = g_state.value_input_len;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                return 0;
            }
            if ((GetKeyState(VK_CONTROL) & 0x8000) != 0 && wparam == 'F') {
                if (g_state.ui_show_bottom_panel && !g_state.bottom_panel_collapsed && g_state.bottom_active_tab == 0) {
                    begin_value_input_for_log_search();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
            if ((GetKeyState(VK_CONTROL) & 0x8000) != 0 && wparam == 'L') {
                begin_value_input_for_hierarchy_filter();
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (g_state.open_menu_id > 0) {
                int n = menu_item_count_for_menu(g_state.open_menu_id);
                if (wparam == VK_ESCAPE) {
                    g_state.open_menu_id = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (wparam == VK_UP && n > 0) {
                    g_state.open_menu_focus_index = menu_find_enabled_from(g_state.open_menu_id, g_state.open_menu_focus_index - 1, -1);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (wparam == VK_DOWN && n > 0) {
                    g_state.open_menu_focus_index = menu_find_enabled_from(g_state.open_menu_id, g_state.open_menu_focus_index + 1, 1);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (wparam == VK_LEFT || wparam == VK_RIGHT) {
                    int next = menu_next_visible_id(g_state.open_menu_id, (wparam == VK_RIGHT) ? 1 : -1);
                    g_state.open_menu_id = next;
                    g_state.open_menu_focus_index = menu_find_enabled_from(next, 0, 1);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
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
                    return 0;
                }
            }
            if (wparam == VK_ESCAPE) {
                g_state.open_menu_id = 0;
                g_state.show_config_modal = 0;
                g_state.show_help_modal = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (g_state.show_config_modal) {
                if (wparam == VK_UP) g_state.focused_param = (g_state.focused_param + 7) % 8;
                if (wparam == VK_DOWN) g_state.focused_param = (g_state.focused_param + 1) % 8;
                if (wparam == VK_LEFT) adjust_focused_param(-1);
                if (wparam == VK_RIGHT) adjust_focused_param(1);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (g_state.show_help_modal) {
                return 0;
            }
            if (g_state.ui_show_bottom_panel && !g_state.bottom_panel_collapsed && g_state.bottom_active_tab == 0) {
                if (wparam == VK_PRIOR) {
                    g_state.log_scroll_offset += 3;
                    if (g_state.log_scroll_offset > g_log_scroll_max) g_state.log_scroll_offset = g_log_scroll_max;
                }
                if (wparam == VK_NEXT) {
                    g_state.log_scroll_offset -= 3;
                    if (g_state.log_scroll_offset < 0) g_state.log_scroll_offset = 0;
                }
            }
            if (wparam == VK_SPACE) {
                g_state.running = !g_state.running;
                push_console_log(L"[状态] 模拟:%s", g_state.running ? L"运行" : L"暂停");
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
            }
            if (wparam == 'R') apply_scene(g_state.scene_index);
            if (wparam == 'C') g_state.draw_contacts = !g_state.draw_contacts;
            if (wparam == 'V') g_state.draw_velocity = !g_state.draw_velocity;
            if (wparam == 'X') g_state.draw_centers = !g_state.draw_centers;
            if (wparam == 'M') g_state.draw_constraints = !g_state.draw_constraints;
            if (wparam == VK_F10) g_state.bottom_panel_collapsed = !g_state.bottom_panel_collapsed;
            if (wparam == VK_F11) {
                apply_layout_preset(g_state.ui_layout_preset + 1);
                save_ui_layout();
            }
            if (inspector_row_count() > 0) {
                int rcnt = inspector_row_count();
                if (wparam == VK_UP) g_state.inspector_focused_row = (g_state.inspector_focused_row + rcnt - 1) % rcnt;
                if (wparam == VK_DOWN) g_state.inspector_focused_row = (g_state.inspector_focused_row + 1) % rcnt;
                if (wparam == VK_LEFT) inspector_adjust_row(g_state.inspector_focused_row, -1);
                if (wparam == VK_RIGHT) inspector_adjust_row(g_state.inspector_focused_row, 1);
                if (wparam == VK_RETURN) begin_value_input_for_inspector_row(g_state.inspector_focused_row);
            }
            if (wparam == '1') {
                trace_spawn_step("key.1.begin", "");
                spawn_circle_at_cursor();
                trace_spawn_step("key.1.end", "");
            }
            if (wparam == '2') {
                trace_spawn_step("key.2.begin", "");
                spawn_box_at_cursor();
                trace_spawn_step("key.2.end", "");
            }
            if (wparam == 'J') try_create_constraint_from_selection(1);
            if (wparam == 'K') try_create_constraint_from_selection(2);
            if (wparam == 'L') try_create_constraint_from_selection(3);
            if (wparam == 'P') try_create_constraint_from_selection(4);
            if (wparam == 'T') update_selected_constraint_param(0.08f, 0.0f);
            if (wparam == 'G') update_selected_constraint_param(-0.08f, 0.0f);
            if (wparam == 'Y') update_selected_constraint_param(0.0f, 0.2f);
            if (wparam == 'H') update_selected_constraint_param(0.0f, -0.2f);
            if (wparam == 'U' && selected_constraint_ref() != NULL) {
                physics_engine_constraint_set_collide_connected(
                    g_state.engine,
                    g_state.selected_constraint_index,
                    !physics_engine_constraint_get_collide_connected(g_state.engine, g_state.selected_constraint_index));
            }
            if (wparam == 'B') cycle_selected_constraint_break_force();
            if (wparam == VK_DELETE && g_state.engine != NULL) {
                if (selected_constraint_ref() != NULL) {
                    remove_selected_constraint();
                } else if (g_state.selected != NULL) {
                    history_push_snapshot();
                    physics_engine_remove_body(g_state.engine, g_state.selected);
                    push_console_log(L"[删除] 已删除选中物体");
                    g_state.selected = NULL;
                    g_state.dragging = 0;
                    cleanup_constraint_selection();
                }
            }
            if (wparam == VK_F1) apply_scene(0);
            if (wparam == VK_F2) apply_scene(1);
            if (wparam == VK_F3) apply_scene(2);
            if (wparam == VK_F4) apply_scene(3);
            if (wparam == VK_F5) apply_scene(4);
            if (wparam == VK_F6) apply_scene(5);
            if (wparam == VK_F7) apply_scene(6);
            if (wparam == VK_F8) apply_scene(7);
            if (wparam == VK_F9) apply_scene(8);
            if (wparam == VK_OEM_4) apply_scene((g_state.scene_index + SCENE_COUNT - 1) % SCENE_COUNT);
            if (wparam == VK_OEM_6) apply_scene((g_state.scene_index + 1) % SCENE_COUNT);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        case WM_CHAR:
            if (g_state.show_value_input) {
                wchar_t ch = (wchar_t)wparam;
                int allow = 0;
                if (g_state.value_input_target == 2 || g_state.value_input_target == 3) {
                    allow = (ch >= 32 && ch != 127);
                } else {
                    allow = ((ch >= L'0' && ch <= L'9') || ch == L'.' || ch == L'-' || ch == L'+');
                }
                if (allow) {
                    if (g_state.value_input_len < 63) {
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
                }
                return 0;
            }
            return 0;
        case WM_SYSKEYDOWN: {
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
                return 0;
            }
            if (wparam == '2') {
                g_state.ui_show_right_panel = !g_state.ui_show_right_panel;
                save_ui_layout();
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (wparam == '3') {
                g_state.ui_show_bottom_panel = !g_state.ui_show_bottom_panel;
                save_ui_layout();
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (wparam == 'T') {
                g_state.ui_theme_light = !g_state.ui_theme_light;
                save_ui_layout();
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (mid > 0) {
                g_state.open_menu_id = mid;
                g_state.open_menu_focus_index = menu_find_enabled_from(mid, 0, 1);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            return DefWindowProcW(hwnd, msg, wparam, lparam);
        }
        case WM_MOUSEMOVE:
            g_state.mouse_screen.x = (float)GET_X_LPARAM(lparam);
            g_state.mouse_screen.y = (float)GET_Y_LPARAM(lparam);
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
                return 0;
            }
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
            }
            return 0;
        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wparam);
            POINT sp;
            sp.x = GET_X_LPARAM(lparam);
            sp.y = GET_Y_LPARAM(lparam);
            ScreenToClient(hwnd, &sp);
            g_state.mouse_screen.x = (float)sp.x;
            g_state.mouse_screen.y = (float)sp.y;
            if ((point_in_rect(g_state.mouse_screen, g_hierarchy_viewport_rect) || point_in_rect(g_state.mouse_screen, g_hierarchy_scroll_track_rect))
                && g_state.hierarchy_scroll_max > 0) {
                if (delta < 0) g_state.hierarchy_scroll_offset += 24;
                if (delta > 0) g_state.hierarchy_scroll_offset -= 24;
                if (g_state.hierarchy_scroll_offset < 0) g_state.hierarchy_scroll_offset = 0;
                if (g_state.hierarchy_scroll_offset > g_state.hierarchy_scroll_max) g_state.hierarchy_scroll_offset = g_state.hierarchy_scroll_max;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if ((point_in_rect(g_state.mouse_screen, g_project_viewport_rect) || point_in_rect(g_state.mouse_screen, g_project_scroll_track_rect))
                && g_state.project_scroll_max > 0) {
                if (delta < 0) g_state.project_scroll_offset += 24;
                if (delta > 0) g_state.project_scroll_offset -= 24;
                if (g_state.project_scroll_offset < 0) g_state.project_scroll_offset = 0;
                if (g_state.project_scroll_offset > g_state.project_scroll_max) g_state.project_scroll_offset = g_state.project_scroll_max;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if ((point_in_rect(g_state.mouse_screen, g_inspector_viewport_rect) || point_in_rect(g_state.mouse_screen, g_inspector_scroll_track_rect))
                && g_state.inspector_scroll_max > 0) {
                if (delta < 0) g_state.inspector_scroll_offset += 24;
                if (delta > 0) g_state.inspector_scroll_offset -= 24;
                if (g_state.inspector_scroll_offset < 0) g_state.inspector_scroll_offset = 0;
                if (g_state.inspector_scroll_offset > g_state.inspector_scroll_max) g_state.inspector_scroll_offset = g_state.inspector_scroll_max;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if ((point_in_rect(g_state.mouse_screen, g_debug_viewport_rect) || point_in_rect(g_state.mouse_screen, g_debug_scroll_track_rect))
                && g_state.debug_scroll_max > 0) {
                if (delta < 0) g_state.debug_scroll_offset += 24;
                if (delta > 0) g_state.debug_scroll_offset -= 24;
                if (g_state.debug_scroll_offset < 0) g_state.debug_scroll_offset = 0;
                if (g_state.debug_scroll_offset > g_state.debug_scroll_max) g_state.debug_scroll_offset = g_state.debug_scroll_max;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (g_state.ui_show_bottom_panel && !g_state.bottom_panel_collapsed && g_state.bottom_active_tab == 0) {
                if (point_in_rect(g_state.mouse_screen, g_log_viewport_rect) || point_in_rect(g_state.mouse_screen, g_log_scroll_track_rect)) {
                    if (delta < 0) g_state.log_scroll_offset += 1;
                    if (delta > 0) g_state.log_scroll_offset -= 1;
                    if (g_state.log_scroll_offset < 0) g_state.log_scroll_offset = 0;
                    if (g_state.log_scroll_offset > g_log_scroll_max) g_state.log_scroll_offset = g_log_scroll_max;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
            return 0;
        }
        case WM_SETCURSOR: {
            POINT cp;
            if (LOWORD(lparam) == HTCLIENT) {
                GetCursorPos(&cp);
                ScreenToClient(hwnd, &cp);
                g_state.mouse_screen.x = (float)cp.x;
                g_state.mouse_screen.y = (float)cp.y;
                if (point_in_rect(g_state.mouse_screen, g_splitter_left_rect) || point_in_rect(g_state.mouse_screen, g_splitter_right_rect)) {
                    SetCursor(LoadCursorW(NULL, (LPCWSTR)IDC_SIZEWE));
                    return TRUE;
                }
                if (point_in_rect(g_state.mouse_screen, g_splitter_bottom_rect)) {
                    SetCursor(LoadCursorW(NULL, (LPCWSTR)IDC_SIZENS));
                    return TRUE;
                }
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            Vec2 p;
            int r;
            int idx;
            g_state.mouse_screen.x = (float)GET_X_LPARAM(lparam);
            g_state.mouse_screen.y = (float)GET_Y_LPARAM(lparam);
            if (g_state.show_value_input || g_state.show_config_modal || g_state.show_help_modal) {
                if (g_state.show_value_input) {
                    if (!point_in_rect(g_state.mouse_screen, g_value_modal_rect)) {
                        g_state.show_value_input = 0;
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    return 0;
                }
                if (point_in_rect(g_state.mouse_screen, g_modal_close_rect)) {
                    g_state.show_config_modal = 0;
                    g_state.show_help_modal = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (!point_in_rect(g_state.mouse_screen, g_modal_rect)) {
                    g_state.show_config_modal = 0;
                    g_state.show_help_modal = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
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
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_left_fold_rect)) {
                g_state.ui_show_left_panel = !g_state.ui_show_left_panel;
                save_ui_layout();
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_right_fold_rect)) {
                g_state.ui_show_right_panel = !g_state.ui_show_right_panel;
                save_ui_layout();
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (g_state.hierarchy_scroll_max > 0 && point_in_rect(g_state.mouse_screen, g_hierarchy_scroll_thumb_rect)) {
                g_state.hierarchy_scroll_dragging = 1;
                g_state.hierarchy_scroll_drag_start_y = g_state.mouse_screen.y;
                g_state.hierarchy_scroll_offset_start = g_state.hierarchy_scroll_offset;
                SetCapture(hwnd);
                return 0;
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
                return 0;
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
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_splitter_left_rect)) {
                g_state.ui_drag_splitter = 1;
                SetCapture(hwnd);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_splitter_right_rect)) {
                g_state.ui_drag_splitter = 2;
                SetCapture(hwnd);
                return 0;
            }
            if (g_state.ui_show_bottom_panel && !g_state.bottom_panel_collapsed && point_in_rect(g_state.mouse_screen, g_splitter_bottom_rect)) {
                g_state.ui_drag_splitter = 3;
                SetCapture(hwnd);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_win_close_rect)) {
                PostMessageW(hwnd, WM_CLOSE, 0, 0);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_win_min_rect)) {
                ShowWindow(hwnd, SW_MINIMIZE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_win_max_rect)) {
                ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_top_run_rect)) {
                g_state.running = !g_state.running;
                push_console_log(L"[状态] 模拟:%s", g_state.running ? L"运行" : L"暂停");
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
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
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_top_reset_rect)) {
                apply_scene(g_state.scene_index);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
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
                        return 0;
                    }
                }
            }
            if (point_in_rect(g_state.mouse_screen, g_menu_file_rect)) {
                g_state.open_menu_id = (g_state.open_menu_id == 1) ? 0 : 1;
                g_state.open_menu_focus_index = (g_state.open_menu_id > 0) ? -1 : 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_menu_edit_rect)) {
                g_state.open_menu_id = (g_state.open_menu_id == 2) ? 0 : 2;
                g_state.open_menu_focus_index = (g_state.open_menu_id > 0) ? -1 : 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_menu_view_rect)) {
                g_state.open_menu_id = (g_state.open_menu_id == 3) ? 0 : 3;
                g_state.open_menu_focus_index = (g_state.open_menu_id > 0) ? -1 : 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_menu_physics_rect)) {
                g_state.open_menu_id = (g_state.open_menu_id == 5) ? 0 : 5;
                g_state.open_menu_focus_index = (g_state.open_menu_id > 0) ? -1 : 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_menu_window_rect)) {
                g_state.open_menu_id = (g_state.open_menu_id == 6) ? 0 : 6;
                g_state.open_menu_focus_index = (g_state.open_menu_id > 0) ? -1 : 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_menu_help_word_rect)) {
                g_state.open_menu_id = (g_state.open_menu_id == 7) ? 0 : 7;
                g_state.open_menu_focus_index = (g_state.open_menu_id > 0) ? -1 : 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (g_state.open_menu_id > 0) {
                g_state.open_menu_id = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_menu_bar_drag_rect)) {
                ReleaseCapture();
                SendMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_top_save_rect)) {
                if (save_scene_snapshot("scene_snapshot.txt")) push_console_log(L"[命令] 已保存 scene_snapshot.txt");
                else push_console_log(L"[错误] 保存 scene_snapshot.txt 失败");
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_top_undo_rect)) {
                history_undo();
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_top_redo_rect)) {
                history_redo();
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_top_grid_rect)) {
                g_state.draw_centers = !g_state.draw_centers;
                push_console_log(L"[视图] 网格/中心:%s", g_state.draw_centers ? L"开" : L"关");
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_top_collision_rect)) {
                g_state.draw_contacts = !g_state.draw_contacts;
                push_console_log(L"[视图] 碰撞点:%s", g_state.draw_contacts ? L"开" : L"关");
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_top_velocity_rect)) {
                g_state.draw_velocity = !g_state.draw_velocity;
                push_console_log(L"[视图] 速度向量:%s", g_state.draw_velocity ? L"开" : L"关");
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_top_constraint_rect)) {
                if (g_state.selected != NULL && g_state.selected->type == BODY_DYNAMIC) {
                    try_create_constraint_from_selection(1);
                } else {
                    push_console_log(L"[提示] 请先选中一个动态物体，再点击约束按钮");
                }
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_top_spring_rect)) {
                if (g_state.selected != NULL && g_state.selected->type == BODY_DYNAMIC) {
                    try_create_constraint_from_selection(2);
                } else {
                    push_console_log(L"[提示] 请先选中一个动态物体，再点击弹性约束按钮");
                }
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_top_chain_rect)) {
                if (g_state.selected != NULL && g_state.selected->type == BODY_DYNAMIC) {
                    try_create_constraint_from_selection(3);
                } else {
                    push_console_log(L"[提示] 请先选中一个动态物体，再点击链条按钮");
                }
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_top_rope_rect)) {
                if (g_state.selected != NULL && g_state.selected->type == BODY_DYNAMIC) {
                    try_create_constraint_from_selection(4);
                } else {
                    push_console_log(L"[提示] 请先选中一个动态物体，再点击橡皮绳按钮");
                }
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_bottom_fold_rect)) {
                g_state.bottom_panel_collapsed = !g_state.bottom_panel_collapsed;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_bottom_tab_console_rect)) {
                g_state.bottom_active_tab = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_bottom_tab_perf_rect)) {
                g_state.bottom_active_tab = 1;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_hierarchy_search_rect)) {
                begin_value_input_for_hierarchy_filter();
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_hierarchy_search_clear_rect)) {
                g_state.hierarchy_filter_buf[0] = L'\0';
                g_state.hierarchy_filter_len = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_tree_scene_header_rect)) {
                g_state.tree_scene_expanded = !g_state.tree_scene_expanded;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_tree_bodies_header_rect)) {
                g_state.tree_bodies_expanded = !g_state.tree_bodies_expanded;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_tree_body_circle_header_rect)) {
                g_state.tree_body_circle_expanded = !g_state.tree_body_circle_expanded;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_tree_body_polygon_header_rect)) {
                g_state.tree_body_polygon_expanded = !g_state.tree_body_polygon_expanded;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_tree_constraints_header_rect)) {
                g_state.tree_constraints_expanded = !g_state.tree_constraints_expanded;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_tree_constraint_distance_header_rect)) {
                g_state.tree_constraint_distance_expanded = !g_state.tree_constraint_distance_expanded;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_tree_constraint_spring_header_rect)) {
                g_state.tree_constraint_spring_expanded = !g_state.tree_constraint_spring_expanded;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_tree_project_root_rect)) {
                g_state.tree_project_expanded = !g_state.tree_project_expanded;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (g_state.ui_show_bottom_panel && !g_state.bottom_panel_collapsed && g_state.bottom_active_tab == 0) {
                if (point_in_rect(g_state.mouse_screen, g_log_filter_all_rect)) {
                    g_state.log_filter_mode = 0;
                    g_state.log_scroll_offset = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (point_in_rect(g_state.mouse_screen, g_log_filter_state_rect)) {
                    g_state.log_filter_mode = 1;
                    g_state.log_scroll_offset = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (point_in_rect(g_state.mouse_screen, g_log_filter_physics_rect)) {
                    g_state.log_filter_mode = 2;
                    g_state.log_scroll_offset = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (point_in_rect(g_state.mouse_screen, g_log_filter_collision_rect)) {
                    g_state.log_filter_mode = 4;
                    g_state.log_scroll_offset = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (point_in_rect(g_state.mouse_screen, g_log_filter_warn_rect)) {
                    g_state.log_filter_mode = 3;
                    g_state.log_scroll_offset = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (point_in_rect(g_state.mouse_screen, g_log_clear_rect)) {
                    g_console_log_count = 0;
                    g_console_log_head = 0;
                    g_state.log_scroll_offset = 0;
                    push_console_log(L"[日志] 已清空");
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (point_in_rect(g_state.mouse_screen, g_log_search_rect)) {
                    begin_value_input_for_log_search();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (point_in_rect(g_state.mouse_screen, g_log_search_clear_rect)) {
                    g_state.log_search_buf[0] = L'\0';
                    g_state.log_search_len = 0;
                    g_state.log_scroll_offset = 0;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
            if (!g_state.bottom_panel_collapsed && g_state.bottom_active_tab == 1) {
                if (point_in_rect(g_state.mouse_screen, g_perf_export_rect)) {
                    export_perf_report_csv();
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
            for (idx = 0; point_in_rect(g_state.mouse_screen, g_inspector_viewport_rect) && idx < g_ins_row_count && idx < INSPECTOR_MAX_ROWS; idx++) {
                if (point_in_rect(g_state.mouse_screen, g_ins_row_rect[idx])) {
                    g_state.inspector_focused_row = idx;
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (point_in_rect(g_state.mouse_screen, g_ins_minus_rect[idx])) {
                    g_state.inspector_focused_row = idx;
                    inspector_adjust_row(idx, -1);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (point_in_rect(g_state.mouse_screen, g_ins_plus_rect[idx])) {
                    g_state.inspector_focused_row = idx;
                    inspector_adjust_row(idx, 1);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
            for (idx = 0; point_in_rect(g_state.mouse_screen, g_debug_viewport_rect) && idx < 3; idx++) {
                if (point_in_rect(g_state.mouse_screen, g_dbg_minus_rect[idx])) {
                    debug_adjust_runtime_param(idx, -1);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
                if (point_in_rect(g_state.mouse_screen, g_dbg_plus_rect[idx])) {
                    debug_adjust_runtime_param(idx, 1);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
            if (point_in_rect(g_state.mouse_screen, g_dbg_collision_filter_rect)) {
                g_collision_event_filter_selected_only = !g_collision_event_filter_selected_only;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
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
                        return 0;
                    }
                }
            }
            if (point_in_rect(g_state.mouse_screen, g_hierarchy_viewport_rect) && point_in_rect(g_state.mouse_screen, g_explorer_scene_rect)) {
                apply_scene((g_state.scene_index + 1) % SCENE_COUNT);
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            for (idx = 0; idx < g_explorer_body_count && point_in_rect(g_state.mouse_screen, g_hierarchy_viewport_rect); idx++) {
                if (point_in_rect(g_state.mouse_screen, g_explorer_body_rect[idx])) {
                    g_state.selected = g_explorer_body_ptr[idx];
                    g_state.selected_constraint_index = -1;
                    g_state.dragging = 0;
                    g_state.inspector_focused_row = 0;
                    push_console_log(L"[选中] 物体已选中");
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
            for (idx = 0; idx < g_explorer_constraint_count && point_in_rect(g_state.mouse_screen, g_hierarchy_viewport_rect); idx++) {
                if (point_in_rect(g_state.mouse_screen, g_explorer_constraint_rect[idx])) {
                    g_state.selected_constraint_index = g_explorer_constraint_index[idx];
                    g_state.selected = NULL;
                    g_state.dragging = 0;
                    g_state.inspector_focused_row = 0;
                    push_console_log(L"[选中] 约束已选中");
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
            if (g_state.mouse_screen.x >= g_state.stage_left &&
                g_state.mouse_screen.x <= g_state.stage_right &&
                g_state.mouse_screen.y >= g_state.stage_top &&
                g_state.mouse_screen.y <= g_state.stage_bottom &&
                g_state.engine != NULL) {
                SetCapture(hwnd);
                p = screen_to_world(g_state.mouse_screen);
                g_state.selected = pick_dynamic_body(g_state.engine, p);
                g_state.dragging = (g_state.selected != NULL);
                if (!g_state.dragging) {
                    g_state.selected_constraint_index = pick_constraint_at_point(g_state.engine, p);
                    g_state.inspector_focused_row = 0;
                    if (g_state.selected_constraint_index >= 0) push_console_log(L"[选中] 舞台约束选中");
                } else {
                    g_state.selected_constraint_index = -1;
                    g_state.inspector_focused_row = 0;
                    push_console_log(L"[选中] 舞台物体选中");
                }
            }
            return 0;
        }
        case WM_LBUTTONUP:
            ReleaseCapture();
            if (g_state.ui_drag_splitter != 0) save_ui_layout();
            g_state.ui_drag_splitter = 0;
            g_state.hierarchy_scroll_dragging = 0;
            g_state.dragging = 0;
            return 0;
        case WM_LBUTTONDBLCLK: {
            int idx;
            g_state.mouse_screen.x = (float)GET_X_LPARAM(lparam);
            g_state.mouse_screen.y = (float)GET_Y_LPARAM(lparam);
            if (g_state.show_value_input || g_state.show_config_modal || g_state.show_help_modal) return 0;
            if (point_in_rect(g_state.mouse_screen, g_top_run_rect)) {
                g_state.running = !g_state.running;
                push_console_log(L"[状态] 模拟:%s", g_state.running ? L"运行" : L"暂停");
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (point_in_rect(g_state.mouse_screen, g_menu_bar_drag_rect)) {
                ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
                return 0;
            }
            for (idx = 0; point_in_rect(g_state.mouse_screen, g_inspector_viewport_rect) && idx < g_ins_row_count && idx < INSPECTOR_MAX_ROWS; idx++) {
                if (point_in_rect(g_state.mouse_screen, g_ins_row_rect[idx])) {
                    g_state.inspector_focused_row = idx;
                    begin_value_input_for_inspector_row(idx);
                    InvalidateRect(hwnd, NULL, FALSE);
                    return 0;
                }
            }
            return 0;
        }
        case WM_DESTROY:
            if (g_app_icon_small != NULL) {
                DestroyIcon(g_app_icon_small);
                g_app_icon_small = NULL;
            }
            if (g_app_icon_large != NULL) {
                DestroyIcon(g_app_icon_large);
                g_app_icon_large = NULL;
            }
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show) {
    WNDCLASSEXW wc;
    HWND hwnd;
    MSG msg;
    int i;
    (void)prev;
    (void)cmd;
    (void)show;

    g_status_project_path[0] = L'\0';
    g_status_user[0] = L'\0';
    GetCurrentDirectoryW(260, g_status_project_path);
    {
        DWORD uname_len = 63;
        if (!GetUserNameW(g_status_user, &uname_len)) {
            g_status_user[0] = L'\0';
        }
    }

    SetProcessDPIAware();
    if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) return 1;
    if (FAILED(init_ui())) {
        CoUninitialize();
        return 1;
    }

    for (i = 0; i < SCENE_COUNT; i++) g_state.scenes[i] = SCENE_DEFAULTS[i];
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
    clear_collision_events();
    g_collision_event_filter_selected_only = 0;
    g_clipboard_body.valid = 0;
    load_ui_layout();
    push_console_log(L"[启动] 物理沙盒已启动");
    if (file_exists_utf8_path("autosave_snapshot.txt")) {
        push_console_log(L"[启动] 检测到 autosave_snapshot.txt，可在 文件 菜单恢复");
    }
    apply_scene(0);

    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = inst;
    g_app_icon_large = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(IDI_APP_MAIN), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    g_app_icon_small = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(IDI_APP_MAIN), IMAGE_ICON,
                                         GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    wc.hIcon = g_app_icon_large ? g_app_icon_large : LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
    wc.hIconSm = g_app_icon_small ? g_app_icon_small : wc.hIcon;
    wc.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"PhysicsSandboxDWrite";
    if (!RegisterClassExW(&wc)) return 1;

    hwnd = CreateWindowExW(0, wc.lpszClassName, L"物理引擎沙盒（DirectWrite）",
                           (WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU),
                           CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, inst, NULL);
    if (hwnd == NULL) return 1;
    g_app_hwnd = hwnd;

    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);
    SetTimer(hwnd, 1, 16, NULL);

    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_state.engine) physics_engine_free(g_state.engine);
    save_ui_layout();
    shutdown_ui();
    CoUninitialize();
    return (int)msg.wParam;
}






