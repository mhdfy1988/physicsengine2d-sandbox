#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
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
#include <string.h>
#include <wctype.h>

#include "physics_core/physics.h"
#include "infrastructure/app_path.hpp"
#include "infrastructure/project_tree.hpp"
#include "physics_runtime/runtime_snapshot_repo.hpp"
#include "infrastructure/ui_layout_repo.hpp"
#include "presentation/design_system/ui_theme.hpp"
#include "presentation/components/button/button.hpp"
#include "presentation/components/panel/panel.hpp"
#include "presentation/modal/modal_content.hpp"
#include "presentation/modal/modal_shell.hpp"
#include "presentation/overlay/status_bar.hpp"
#include "presentation/panels/console/console_perf.hpp"
#include "presentation/panels/console/console_panel_shell.hpp"
#include "presentation/panels/console/console_renderer.hpp"
#include "presentation/panels/console/console_tabs.hpp"
#include "presentation/panels/hierarchy/hierarchy_layout.hpp"
#include "presentation/panels/hierarchy/hierarchy_renderer.hpp"
#include "presentation/panels/hierarchy/hierarchy_scrollbar.hpp"
#include "presentation/panels/inspector/inspector_layout.hpp"
#include "presentation/panels/inspector/inspector_debug_renderer.hpp"
#include "presentation/panels/inspector/inspector_panel_shell.hpp"
#include "presentation/panels/inspector/inspector_renderer.hpp"
#include "presentation/panels/inspector/inspector_scrollbar.hpp"
#include "presentation/topbar/top_toolbar.hpp"
#include "presentation/render/editor_screen_renderer.hpp"
#include "presentation/render/ui_icons.hpp"
#include "presentation/render/ui_primitives.hpp"
#include "presentation/render/ui_text.hpp"
#include "presentation/render/ui_widgets.hpp"
#include "presentation/shell/side_fold_buttons.hpp"
#include "presentation/stage/stage_panel.hpp"
#include "presentation/status/status_presenter.hpp"
#include "presentation/window/window_host.hpp"
#include "domain/app_command.hpp"
#include "application/editor_session_service.hpp"
#include "application/app_runtime.hpp"
#include "application/workbench/workbench_service.hpp"
#include "application/editor_command_bus.hpp"
#include "application/scene_catalog.hpp"
#include "application/scene_builder.hpp"
#include "application/history_service.hpp"
#include "application/runtime_param_service.hpp"
#include "application/pie_lifecycle.hpp"
#include "application/editor_extension_host_service.hpp"
#include "application/editor_extension_state_service.hpp"
#include "physics_content/asset_hot_reload.hpp"
#include "physics_content/asset_fs_watch.hpp"
#include "physics_content/project_workspace.hpp"
#include "physics_content/prefab_schema.hpp"
#include "physics_content/prefab_semantics.hpp"
#include "physics_content/editor_plugin.hpp"
#include "physics_runtime/session_recovery.hpp"
#include "physics_content/diagnostic_bundle.hpp"
#include "presentation/input/input_router.hpp"
#include "presentation/input/input_mapping.hpp"
#include "presentation/input/menu_file_edit_actions.hpp"
#include "presentation/input/menu_view_physics_window_actions.hpp"
#include "presentation/input/menu_help_actions.hpp"
#include "presentation/input/menu_model.hpp"

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
    unsigned int frame_index;
    int body_count;
    int constraint_count;
    int contact_count;
    float step_ms;
} RuntimeTickHistoryEntry;

typedef struct {
    unsigned int frame_index;
    unsigned int tick_ms;
    int running;
} RuntimeStateHistoryEntry;

enum { DEBUG_HISTORY_TEXT_CAP = 160 };

typedef struct {
    unsigned int tick_ms;
    wchar_t text[DEBUG_HISTORY_TEXT_CAP];
} DebugTextHistoryEntry;

typedef struct {
    unsigned int tick_ms;
    int pie_active;
    int ready_batch_count;
    int affected_count;
    int imported_count;
    int failed_count;
    int rollback_retained;
    int imported_guid_count;
    char imported_guids[APP_HOT_RELOAD_MAX_IMPORTED][ASSET_DB_MAX_GUID];
} HotReloadHistoryEntry;

typedef struct {
    PhysicsEngine* engine;
    SceneConfig scenes[SCENE_COUNT];
    wchar_t scene_names[SCENE_COUNT][SCENE_NAME_MAX];
    int scene_order[SCENE_COUNT];
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
    unsigned int runtime_frame_index;
    int runtime_running;
    int runtime_body_count;
    int runtime_constraint_count;
    int runtime_contact_count;
    int runtime_error_count;
    int runtime_error_code;
    int runtime_error_item_count;
    AppRuntimeErrorItem runtime_errors[APP_RUNTIME_MAX_ERRORS];
    int runtime_state_change_count;
    unsigned int runtime_event_drop_count;
    unsigned int runtime_drop_warn_ms;
    unsigned int runtime_drop_last_seen;
    unsigned int runtime_drop_last_growth_ms;
    int runtime_bus_congested;
    int hot_reload_enabled;
    int hot_reload_watch_count;
    int hot_reload_scan_change_count;
    int hot_reload_ready_batch_count;
    int hot_reload_affected_count;
    int hot_reload_imported_count;
    int hot_reload_failed_count;
    unsigned int hot_reload_total_imported;
    unsigned int hot_reload_total_failed;
    unsigned int hot_reload_last_scan_ms;
    unsigned int hot_reload_last_discover_ms;
    unsigned int hot_reload_last_event_ms;
    unsigned int hot_reload_last_warn_ms;
    RuntimeTickHistoryEntry runtime_tick_history[8];
    int runtime_tick_history_head;
    int runtime_tick_history_count;
    RuntimeStateHistoryEntry runtime_state_history[8];
    int runtime_state_history_head;
    int runtime_state_history_count;
    unsigned int fps_last_tick_ms;
    int fps_accum_frames;
    int last_contact_count;
    int last_runtime_error_code;
    unsigned int last_contact_log_ms;
    unsigned int last_runtime_error_log_ms;
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

static UiApp g_ui{};
static UiTheme g_ui_theme{};
static SandboxState g_state{};
static HWND g_app_hwnd = NULL;
static AppRuntime g_app_runtime;
static PieLifecycle g_pie_lifecycle;
static EditorCommandCallbacks g_editor_command_bus;
static AssetHotReloadService g_hot_reload_service;
static AssetFsWatchState g_asset_fs_watch;
static char g_hot_reload_root_dir[ASSET_WATCH_MAX_PATH];
static const char* PIE_EDITOR_SNAPSHOT_PATH = "pie_editor_state_snapshot.txt";
static const char* BUILD_METADATA_PATH = "artifacts/build/build_metadata.json";
static const float WORLD_SCALE = 12.0f;
static const float WORLD_ORIGIN_X = 20.0f;
static const float WORLD_ORIGIN_Y = 20.0f;
enum { EXPLORER_MAX_ITEMS = 512 };
enum { EXPLORER_SCENE_MAX_ITEMS = SCENE_COUNT };
enum { DEBUG_EVENT_ROWS_MAX = 24 };
enum { INSPECTOR_MAX_ROWS = 12 };
enum { RUNTIME_HISTORY_ROWS_MAX = 3 };
enum { DEBUG_HISTORY_CAP = 8 };
enum { DEBUG_HISTORY_ROWS_MAX = 4 };
enum { HOT_RELOAD_SCAN_INTERVAL_MS = 250 };
enum { HOT_RELOAD_DISCOVER_INTERVAL_MS = 3000 };
enum { HOT_RELOAD_DEBOUNCE_MS = 180 };
enum {
    VALUE_INPUT_TARGET_NONE = 0,
    VALUE_INPUT_TARGET_INSPECTOR = 1,
    VALUE_INPUT_TARGET_LOG_SEARCH = 2,
    VALUE_INPUT_TARGET_HIERARCHY_FILTER = 3,
    VALUE_INPUT_TARGET_DEBUG_PARAM = 4,
    VALUE_INPUT_TARGET_SCENE_NAME = 5,
    VALUE_INPUT_TARGET_SCENE_ASSET_GUID = 6
};
static const char* HOT_RELOAD_CACHE_ROOT = "Cache/imported";

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
    D2D1_RECT_F explorer_scene_rect[EXPLORER_SCENE_MAX_ITEMS];
    int explorer_scene_index[EXPLORER_SCENE_MAX_ITEMS];
    int explorer_scene_count;
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
    D2D1_RECT_F perf_diag_export_rect;
    D2D1_RECT_F dbg_row_rect[3];
    D2D1_RECT_F dbg_minus_rect[3];
    D2D1_RECT_F dbg_plus_rect[3];
    D2D1_RECT_F dbg_collision_filter_rect;
    D2D1_RECT_F dbg_collision_row_rect[DEBUG_EVENT_ROWS_MAX];
    int dbg_collision_row_event_index[DEBUG_EVENT_ROWS_MAX];
    int dbg_collision_row_count;
    D2D1_RECT_F dbg_runtime_tick_row_rect[RUNTIME_HISTORY_ROWS_MAX];
    int dbg_runtime_tick_row_entry_index[RUNTIME_HISTORY_ROWS_MAX];
    int dbg_runtime_tick_row_count;
    D2D1_RECT_F dbg_runtime_state_row_rect[RUNTIME_HISTORY_ROWS_MAX];
    int dbg_runtime_state_row_entry_index[RUNTIME_HISTORY_ROWS_MAX];
    int dbg_runtime_state_row_count;
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

static UiLayoutState g_layout{};
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
#define g_explorer_scene_index g_layout.explorer_scene_index
#define g_explorer_scene_count g_layout.explorer_scene_count
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
#define g_perf_diag_export_rect g_layout.perf_diag_export_rect
#define g_dbg_row_rect g_layout.dbg_row_rect
#define g_dbg_minus_rect g_layout.dbg_minus_rect
#define g_dbg_plus_rect g_layout.dbg_plus_rect
#define g_dbg_collision_filter_rect g_layout.dbg_collision_filter_rect
#define g_dbg_collision_row_rect g_layout.dbg_collision_row_rect
#define g_dbg_collision_row_event_index g_layout.dbg_collision_row_event_index
#define g_dbg_collision_row_count g_layout.dbg_collision_row_count
#define g_dbg_runtime_tick_row_rect g_layout.dbg_runtime_tick_row_rect
#define g_dbg_runtime_tick_row_entry_index g_layout.dbg_runtime_tick_row_entry_index
#define g_dbg_runtime_tick_row_count g_layout.dbg_runtime_tick_row_count
#define g_dbg_runtime_state_row_rect g_layout.dbg_runtime_state_row_rect
#define g_dbg_runtime_state_row_entry_index g_layout.dbg_runtime_state_row_entry_index
#define g_dbg_runtime_state_row_count g_layout.dbg_runtime_state_row_count
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
static AppRuntimeErrorItem g_tick_runtime_errors[APP_RUNTIME_MAX_ERRORS];
static int g_tick_runtime_error_count;
static DebugTextHistoryEntry g_debug_event_history[DEBUG_HISTORY_CAP];
static int g_debug_event_history_head;
static int g_debug_event_history_count;
static DebugTextHistoryEntry g_debug_error_history[DEBUG_HISTORY_CAP];
static int g_debug_error_history_head;
static int g_debug_error_history_count;
static HotReloadHistoryEntry g_hot_reload_history[DEBUG_HISTORY_CAP];
static int g_hot_reload_history_head;
static int g_hot_reload_history_count;
static EditorExtensionStartupResult g_editor_extension_startup_state{};
static int g_editor_plugin_builtin_enabled;
static BodyClipboard g_clipboard_body;
static wchar_t g_status_project_path[260];
static wchar_t g_status_user[64];
static wchar_t g_startup_status_text[96];
static int g_startup_degraded;
static HICON g_app_icon_large = NULL;
static HICON g_app_icon_small = NULL;
static const float BREAK_FORCE_PRESETS[] = {0.0f, 60.0f, 120.0f, 250.0f, 500.0f, 1000.0f};
static const int BREAK_FORCE_PRESET_COUNT = 6;
static const char* SCENE_ASSET_GUID_NONE = "asset://none";
#define g_workspace_doc g_editor_extension_startup_state.workspace_doc
#define g_project_doc g_editor_extension_startup_state.project_doc
#define g_package_doc g_editor_extension_startup_state.package_doc
#define g_project_settings_doc g_editor_extension_startup_state.project_settings_doc
#define g_session_recovery_state g_editor_extension_startup_state.session_recovery_state
#define g_editor_plugin_registry g_editor_extension_startup_state.plugin_registry
#define g_prefab_analysis g_editor_extension_startup_state.prefab_analysis
#define g_prefab_analysis_valid g_editor_extension_startup_state.prefab_analysis_valid
#define g_prefab_repair_removed_count g_editor_extension_startup_state.prefab_repair_removed_count
static const wchar_t* scene_display_name(int scene_index);
static int scene_order_index_at(int order_index);
static int scene_order_find_position(int scene_index);
static int scene_order_move_selected(int direction);
static int scene_order_reset_default(void);
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
static PieLifecycleOps pie_lifecycle_ops(void);
static int pie_enter_session(void);
static int pie_exit_session(void);
static int pie_runtime_active(void);
static int body_index_of(const PhysicsEngine* engine, const RigidBody* b);
static void clear_collision_events(void);
static void push_collision_event(int body_a_index, int body_b_index, Vec2 point, float rel_speed, float penetration);
static void capture_collision_events(void);
static void reset_tick_runtime_errors(void);
static void push_tick_runtime_error(int code, int severity, int count);
static void runtime_event_sink_collect(const PhysicsTraceEvent* event, void* user);
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
static int snapshot_append_editor_metadata(const char* path);
static int snapshot_restore_editor_metadata(const char* path);
static void trace_spawn_step(const char* stage, const char* fmt, ...);
static int hot_reload_is_supported_extension(const char* source_path);
static void hot_reload_normalize_path(char* path);
static int hot_reload_register_source_path(const char* source_path);
static void hot_reload_register_tree_recursive(const char* root_dir, int depth);
static void init_hot_reload_pipeline(void);
static void hot_reload_tick_runtime(void);
static void clear_debug_histories(void);
static void push_debug_event_history(const wchar_t* fmt, ...);
static void push_debug_error_history(const wchar_t* fmt, ...);
static void push_hot_reload_history(const AppHotReloadSnapshot* snapshot);
static void inspector_draw_card_round_adapter(D2D1_RECT_F rect, float radius, D2D1_COLOR_F fill, D2D1_COLOR_F border);
static void inspector_draw_text_adapter(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color);
static void inspector_draw_text_vcenter_adapter(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color);
static void inspector_draw_panel_header_band_adapter(D2D1_RECT_F panel_rect, float top_h, float inset);
static const wchar_t* app_command_label(AppCommandType type);
static void init_editor_command_bus_callbacks(void);
static int dispatch_editor_command(const EditorCommand* command);
static int editor_cb_scene_rename(int scene_index, const wchar_t* name, void* user);
static int editor_cb_scene_order_move(int direction, void* user);
static int editor_cb_scene_order_reset(void* user);
static int editor_cb_scene_asset_ref_set(int scene_index, const wchar_t* guid, void* user);
static int editor_cb_inspector_set_value(int row, double value, int emit_log, void* user);
static int parse_double_strict(const wchar_t* text, double* out_v);
static int scene_rename_apply(int scene_index, const wchar_t* input_name);
static int scene_asset_guid_apply(int scene_index, const wchar_t* input_guid);
static int inspector_commit_row_value(int row, double value, int emit_log);
static int inspector_adjust_focused_row(int sign);
static void begin_value_input_for_scene_name(int scene_index);
static void begin_value_input_for_scene_asset_guid(int scene_index);
static void begin_value_input_for_hierarchy_filter(void);
static const wchar_t* value_input_title(void);
static int dispatch_ui_intent(const UiIntent* intent);

static D2D1_RECT_F rc(float l, float t, float r, float b) {
    D2D1_RECT_F v = {l, t, r, b};
    return v;
}

static D2D1_COLOR_F rgba(float r, float g, float b, float a) {
    D2D1_COLOR_F v = {r, g, b, a};
    return v;
}

static void sync_ui_theme(void) {
    ui_theme_build(g_state.ui_theme_light, &g_ui_theme);
}

static D2D1_POINT_2F pt(float x, float y) {
    D2D1_POINT_2F v = {x, y};
    return v;
}

static void release_unknown(IUnknown** p) {
    if (p != NULL && *p != NULL) {
        (*p)->Release();
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

static const wchar_t* scene_display_name(int scene_index) {
    if (scene_index < 0 || scene_index >= SCENE_COUNT) {
        scene_index = 0;
    }
    if (g_state.scene_names[scene_index][0] == L'\0') {
        return scene_catalog_name(scene_index);
    }
    return g_state.scene_names[scene_index];
}

static int scene_order_index_at(int order_index) {
    int scene_index;
    if (order_index < 0 || order_index >= SCENE_COUNT) return 0;
    scene_index = g_state.scene_order[order_index];
    if (scene_index < 0 || scene_index >= SCENE_COUNT) {
        return order_index;
    }
    return scene_index;
}

static int scene_order_find_position(int scene_index) {
    int i;
    if (scene_index < 0 || scene_index >= SCENE_COUNT) return -1;
    for (i = 0; i < SCENE_COUNT; i++) {
        if (g_state.scene_order[i] == scene_index) return i;
    }
    return -1;
}

static int scene_order_reset_default(void) {
    int i;
    int changed = 0;
    for (i = 0; i < SCENE_COUNT; i++) {
        if (g_state.scene_order[i] != i) changed = 1;
        g_state.scene_order[i] = i;
    }
    return changed;
}

static int scene_order_move_selected(int direction) {
    int pos;
    int target;
    int tmp;
    if (direction == 0) return 0;
    pos = scene_order_find_position(g_state.scene_index);
    if (pos < 0) {
        scene_order_reset_default();
        pos = scene_order_find_position(g_state.scene_index);
    }
    target = pos + direction;
    if (pos < 0 || target < 0 || target >= SCENE_COUNT) return 0;
    tmp = g_state.scene_order[pos];
    g_state.scene_order[pos] = g_state.scene_order[target];
    g_state.scene_order[target] = tmp;
    return 1;
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

static void clear_debug_histories(void) {
    ZeroMemory(g_debug_event_history, sizeof(g_debug_event_history));
    ZeroMemory(g_debug_error_history, sizeof(g_debug_error_history));
    ZeroMemory(g_hot_reload_history, sizeof(g_hot_reload_history));
    g_debug_event_history_head = 0;
    g_debug_event_history_count = 0;
    g_debug_error_history_head = 0;
    g_debug_error_history_count = 0;
    g_hot_reload_history_head = 0;
    g_hot_reload_history_count = 0;
}

static void push_debug_text_history(DebugTextHistoryEntry* history, int* head, int* count, const wchar_t* fmt, va_list args) {
    wchar_t text[DEBUG_HISTORY_TEXT_CAP];
    int idx;
    if (history == NULL || head == NULL || count == NULL || fmt == NULL) return;
    vswprintf(text, DEBUG_HISTORY_TEXT_CAP, fmt, args);
    idx = (*head + *count) % DEBUG_HISTORY_CAP;
    history[idx].tick_ms = (unsigned int)GetTickCount();
    lstrcpynW(history[idx].text, text, DEBUG_HISTORY_TEXT_CAP);
    if (*count < DEBUG_HISTORY_CAP) {
        (*count)++;
    } else {
        *head = (*head + 1) % DEBUG_HISTORY_CAP;
    }
}

static void push_debug_event_history(const wchar_t* fmt, ...) {
    va_list args;
    if (fmt == NULL) return;
    va_start(args, fmt);
    push_debug_text_history(g_debug_event_history, &g_debug_event_history_head, &g_debug_event_history_count, fmt, args);
    va_end(args);
}

static void push_debug_error_history(const wchar_t* fmt, ...) {
    va_list args;
    if (fmt == NULL) return;
    va_start(args, fmt);
    push_debug_text_history(g_debug_error_history, &g_debug_error_history_head, &g_debug_error_history_count, fmt, args);
    va_end(args);
}

static void push_hot_reload_history(const AppHotReloadSnapshot* snapshot) {
    HotReloadHistoryEntry* entry;
    int idx;
    int i;
    if (snapshot == NULL || !snapshot->valid) return;
    idx = (g_hot_reload_history_head + g_hot_reload_history_count) % DEBUG_HISTORY_CAP;
    entry = &g_hot_reload_history[idx];
    ZeroMemory(entry, sizeof(*entry));
    entry->tick_ms = (unsigned int)GetTickCount();
    entry->pie_active = snapshot->pie_active;
    entry->ready_batch_count = snapshot->ready_batch_count;
    entry->affected_count = snapshot->affected_count;
    entry->imported_count = snapshot->imported_count;
    entry->failed_count = snapshot->failed_count;
    entry->rollback_retained = snapshot->rollback_retained;
    entry->imported_guid_count = snapshot->imported_guid_count;
    if (entry->imported_guid_count > APP_HOT_RELOAD_MAX_IMPORTED) {
        entry->imported_guid_count = APP_HOT_RELOAD_MAX_IMPORTED;
    }
    for (i = 0; i < entry->imported_guid_count; i++) {
        lstrcpynA(entry->imported_guids[i], snapshot->imported_guids[i], ASSET_DB_MAX_GUID);
    }
    if (g_hot_reload_history_count < DEBUG_HISTORY_CAP) {
        g_hot_reload_history_count++;
    } else {
        g_hot_reload_history_head = (g_hot_reload_history_head + 1) % DEBUG_HISTORY_CAP;
    }
}

static const wchar_t* app_command_label(AppCommandType type) {
    switch (type) {
        case APP_CMD_TOGGLE_RUN: return L"运行/暂停";
        case APP_CMD_STEP_ONCE: return L"单步";
        case APP_CMD_RESET_SCENE: return L"重置场景";
        case APP_CMD_SPAWN_CIRCLE: return L"生成圆形";
        case APP_CMD_SPAWN_BOX: return L"生成方块";
        default: return L"未知命令";
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
    {
        char log_path[MAX_PATH];
        if (!sandbox_app_path_runtime_file_utf8("spawn_trace.log", log_path, MAX_PATH)) return;
        fp = fopen(log_path, "a");
    }
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
        ScreenPt fallback_screen{
            (visible_stage_rect.left + visible_stage_rect.right) * 0.5f,
            (visible_stage_rect.top + visible_stage_rect.bottom) * 0.5f
        };
        p = screen_to_world(fallback_screen);
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
    static wchar_t path[MAX_PATH];
    if (!sandbox_app_path_runtime_file_wide(L"sandbox_ui.ini", path, MAX_PATH)) {
        lstrcpynW(path, L".\\sandbox_ui.ini", MAX_PATH);
    }
    return path;
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
    if (mode == 2) base_ok = (wcsstr(s, L"[物理]") != NULL) || (wcsstr(s, L"[碰撞]") != NULL) || (wcsstr(s, L"[运行时]") != NULL);
    if (mode == 3) base_ok = (wcsstr(s, L"[警告]") != NULL);
    if (mode == 4) return 0;
    if (!base_ok) return 0;
    if (g_state.log_search_len > 0) {
        return (wcsstr(s, g_state.log_search_buf) != NULL);
    }
    return 1;
}

static int console_collision_event_involves_selected_adapter(int event_index) {
    if (event_index < 0 || event_index >= COLLISION_EVENT_CAP) return 0;
    return collision_event_involves_selected(&g_collision_events[event_index]);
}

static int console_format_collision_event_line_adapter(int event_index, wchar_t* buffer, int buffer_count) {
    const CollisionEvent* ev;
    float tsec;
    if (buffer == NULL || buffer_count <= 0) return 0;
    if (event_index < 0 || event_index >= COLLISION_EVENT_CAP) return 0;
    ev = &g_collision_events[event_index];
    tsec = (float)ev->tick_ms / 1000.0f;
    swprintf(buffer, buffer_count, L"[碰撞] t=%.1fs #%d-#%d v=%.2f p=%.2f",
             tsec, ev->body_a_index + 1, ev->body_b_index + 1, ev->rel_speed, ev->penetration);
    return 1;
}

static const wchar_t* runtime_error_label(int code) {
    switch (code) {
        case PHYSICS_ERROR_NONE: return L"none";
        case PHYSICS_ERROR_INVALID_ARGUMENT: return L"invalid_argument";
        case PHYSICS_ERROR_API_VERSION_MISMATCH: return L"api_version_mismatch";
        case PHYSICS_ERROR_PLUGIN_INIT_FAILED: return L"plugin_init_failed";
        case PHYSICS_ERROR_CAPACITY_EXCEEDED: return L"capacity_exceeded";
        case APP_RUNTIME_ERROR_CODE_BRIDGE_MISSING_REVERSE: return L"bridge_missing_reverse";
        case APP_RUNTIME_ERROR_CODE_BRIDGE_STALE_ENTITY: return L"bridge_stale_entity";
        case APP_RUNTIME_ERROR_CODE_BRIDGE_NULL_BODY: return L"bridge_null_body";
        case APP_RUNTIME_ERROR_CODE_BRIDGE_DUPLICATE_BODY: return L"bridge_duplicate_body";
        case APP_RUNTIME_ERROR_CODE_BRIDGE_REFCOUNT_MISMATCH: return L"bridge_ref_count_mismatch";
        case APP_RUNTIME_ERROR_CODE_PIPELINE_MAPPING_ERRORS: return L"pipeline_mapping_errors";
        case APP_RUNTIME_ERROR_CODE_HOT_RELOAD_SCAN_FAILED: return L"hot_reload_scan_failed";
        case APP_RUNTIME_ERROR_CODE_HOT_RELOAD_IMPORT_FAILED: return L"hot_reload_import_failed";
        case APP_RUNTIME_ERROR_CODE_HOT_RELOAD_BATCH_FAILED: return L"hot_reload_batch_failed";
        default: return L"unknown";
    }
}

static const wchar_t* runtime_error_severity_label(int severity) {
    if (severity == APP_RUNTIME_ERROR_ERROR) return L"error";
    return L"warning";
}

static void reset_tick_runtime_errors(void) {
    g_tick_runtime_error_count = 0;
}

static void push_tick_runtime_error(int code, int severity, int count) {
    int i;
    if (code == PHYSICS_ERROR_NONE || count <= 0) return;
    for (i = 0; i < g_tick_runtime_error_count; i++) {
        if (g_tick_runtime_errors[i].code == code && g_tick_runtime_errors[i].severity == severity) {
            g_tick_runtime_errors[i].count += count;
            return;
        }
    }
    if (g_tick_runtime_error_count >= APP_RUNTIME_MAX_ERRORS) return;
    g_tick_runtime_errors[g_tick_runtime_error_count].code = code;
    g_tick_runtime_errors[g_tick_runtime_error_count].severity = severity;
    g_tick_runtime_errors[g_tick_runtime_error_count].count = count;
    g_tick_runtime_error_count++;
}

static void runtime_event_sink_collect(const PhysicsTraceEvent* event, void* user) {
    int code;
    int severity;
    (void)user;
    if (event == NULL || event->type != PHYSICS_EVENT_ERROR) return;
    code = (int)event->payload.error.code;
    severity = (code == PHYSICS_ERROR_INVALID_ARGUMENT || code == PHYSICS_ERROR_CAPACITY_EXCEEDED)
                   ? APP_RUNTIME_ERROR_ERROR
                   : APP_RUNTIME_ERROR_WARNING;
    push_tick_runtime_error(code, severity, 1);
}

static void app_runtime_collect_errors_for_tick(void) {
    AppRuntimeErrorItem items[APP_RUNTIME_MAX_ERRORS];
    int n = 0;
    int i;
    if (g_tick_runtime_error_count > 0) {
        n = g_tick_runtime_error_count;
        if (n > APP_RUNTIME_MAX_ERRORS) n = APP_RUNTIME_MAX_ERRORS;
        for (i = 0; i < n; i++) {
            items[i] = g_tick_runtime_errors[i];
        }
    } else {
        int code = (g_state.engine != NULL) ? physics_engine_get_last_error(g_state.engine) : PHYSICS_ERROR_NONE;
        if (code != PHYSICS_ERROR_NONE) {
            items[0].code = code;
            items[0].severity = (code == PHYSICS_ERROR_INVALID_ARGUMENT || code == PHYSICS_ERROR_CAPACITY_EXCEEDED)
                                    ? APP_RUNTIME_ERROR_ERROR
                                    : APP_RUNTIME_ERROR_WARNING;
            items[0].count = 1;
            n = 1;
        }
    }
    app_runtime_set_runtime_errors(&g_app_runtime, items, n);
    reset_tick_runtime_errors();
}

static void export_perf_report_csv(void) {
    char report_path[MAX_PATH];
    FILE* fp;
    int i;
    if (!sandbox_app_path_runtime_file_utf8("perf_report.csv", report_path, MAX_PATH)) return;
    fp = fopen(report_path, "w");
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
    push_console_log(L"%ls", text);
}

static int pie_cb_save_snapshot(const char* path, void* user) {
    (void)user;
    return save_scene_snapshot(path);
}

static int pie_cb_load_snapshot(const char* path, void* user) {
    (void)user;
    return load_scene_snapshot(path);
}

static void pie_cb_log_text(const wchar_t* text, void* user) {
    (void)user;
    if (text == NULL) return;
    push_console_log(L"%ls", text);
}

static PieLifecycleOps pie_lifecycle_ops(void) {
    PieLifecycleOps ops;
    ops.save_snapshot = pie_cb_save_snapshot;
    ops.load_snapshot = pie_cb_load_snapshot;
    ops.log_text = pie_cb_log_text;
    ops.user = NULL;
    return ops;
}

static int pie_runtime_active(void) {
    return g_pie_lifecycle.active != 0;
}

static int pie_enter_session(void) {
    PieLifecycleOps ops;
    if (pie_runtime_active()) return 1;
    ops = pie_lifecycle_ops();
    if (!pie_lifecycle_enter(&g_pie_lifecycle, &ops)) return 0;
    push_debug_event_history(L"PIE: enter session");
    g_state.running = 1;
    if (g_state.engine != NULL) {
        g_state.last_contact_count = physics_engine_get_contact_count(g_state.engine);
    } else {
        g_state.last_contact_count = 0;
    }
    return 1;
}

static int pie_exit_session(void) {
    PieLifecycleOps ops;
    if (!pie_runtime_active()) return 1;
    g_state.running = 0;
    ops = pie_lifecycle_ops();
    if (!pie_lifecycle_exit(&g_pie_lifecycle, &ops)) return 0;
    push_debug_event_history(L"PIE: exit session and restore editor snapshot");
    history_reset_and_capture();
    if (g_state.engine != NULL) {
        g_state.last_contact_count = physics_engine_get_contact_count(g_state.engine);
    } else {
        g_state.last_contact_count = 0;
    }
    return 1;
}

static void menu_cb_toggle_run(void* user) {
    UiIntent intent{};
    (void)user;
    intent.type = UI_INTENT_APP_COMMAND;
    intent.app_command.type = APP_CMD_TOGGLE_RUN;
    dispatch_ui_intent(&intent);
}

static void menu_cb_step_once(void* user) {
    UiIntent intent{};
    (void)user;
    intent.type = UI_INTENT_APP_COMMAND;
    intent.app_command.type = APP_CMD_STEP_ONCE;
    dispatch_ui_intent(&intent);
}

static void apply_next_layout_preset_adapter(void) {
    apply_layout_preset(g_state.ui_layout_preset + 1);
}

static int dispatch_ui_intent(const UiIntent* intent) {
    WorkbenchServiceCallbacks callbacks{};
    callbacks.app_runtime = &g_app_runtime;
    callbacks.draw_centers = &g_state.draw_centers;
    callbacks.draw_contacts = &g_state.draw_contacts;
    callbacks.draw_velocity = &g_state.draw_velocity;
    callbacks.show_bottom_panel = &g_state.ui_show_bottom_panel;
    callbacks.bottom_panel_collapsed = &g_state.bottom_panel_collapsed;
    callbacks.bottom_active_tab = &g_state.bottom_active_tab;
    callbacks.log_filter_mode = &g_state.log_filter_mode;
    callbacks.log_scroll_offset = &g_state.log_scroll_offset;
    callbacks.save_snapshot = save_scene_snapshot;
    callbacks.load_snapshot = load_scene_snapshot;
    callbacks.history_reset_and_capture = history_reset_and_capture;
    callbacks.history_undo = history_undo;
    callbacks.history_redo = history_redo;
    callbacks.history_push_snapshot = history_push_snapshot;
    callbacks.copy_selected_body_to_clipboard = copy_selected_body_to_clipboard;
    callbacks.paste_body_from_clipboard = paste_body_from_clipboard;
    callbacks.apply_layout_preset_next = apply_next_layout_preset_adapter;
    callbacks.save_ui_layout = save_ui_layout;
    callbacks.begin_hierarchy_filter_input = begin_value_input_for_hierarchy_filter;
    callbacks.push_console_log = push_console_log;
    return workbench_service_handle_intent(&callbacks, intent);
}

static void menu_cb_reset_scene(void* user) {
    UiIntent intent{};
    (void)user;
    intent.type = UI_INTENT_APP_COMMAND;
    intent.app_command.type = APP_CMD_RESET_SCENE;
    dispatch_ui_intent(&intent);
}

static void menu_cb_apply_next_layout(void* user) {
    UiIntent intent{};
    (void)user;
    intent.type = UI_INTENT_APPLY_NEXT_LAYOUT;
    dispatch_ui_intent(&intent);
}

static void menu_cb_save_layout(void* user) {
    UiIntent intent{};
    (void)user;
    intent.type = UI_INTENT_SAVE_LAYOUT;
    dispatch_ui_intent(&intent);
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
    vpw_ops.show_left = &g_state.ui_show_left_panel;
    vpw_ops.show_right = &g_state.ui_show_right_panel;
    vpw_ops.show_bottom = &g_state.ui_show_bottom_panel;
    vpw_ops.bottom_active_tab = &g_state.bottom_active_tab;
    vpw_ops.theme_light = &g_state.ui_theme_light;
    vpw_ops.ui_layout_preset = &g_state.ui_layout_preset;
    vpw_ops.toggle_run = menu_cb_toggle_run;
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
        else push_console_log(L"[物理] 创建约束成功 类型=%ls", (mode == 1) ? L"距离" : L"弹簧");
    } else if (mode == 3 || mode == 4) {
        push_console_log(L"[提示] 创建%ls失败：两物体距离过近或资源不足", (mode == 3) ? L"链条" : L"橡皮绳");
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
        CollisionInfo info{};
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
    physics_engine_set_event_sink(g_state.engine, runtime_event_sink_collect, NULL);
    reset_tick_runtime_errors();

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
    g_state.runtime_frame_index = 0;
    g_state.runtime_running = 0;
    g_state.runtime_body_count = 0;
    g_state.runtime_constraint_count = 0;
    g_state.runtime_contact_count = 0;
    g_state.runtime_error_count = 0;
    g_state.runtime_error_code = PHYSICS_ERROR_NONE;
    g_state.runtime_error_item_count = 0;
    g_state.runtime_state_change_count = 0;
    g_state.runtime_event_drop_count = 0;
    g_state.runtime_drop_warn_ms = 0;
    g_state.runtime_drop_last_seen = 0;
    g_state.runtime_drop_last_growth_ms = 0;
    g_state.runtime_bus_congested = 0;
    g_state.runtime_tick_history_head = 0;
    g_state.runtime_tick_history_count = 0;
    g_state.runtime_state_history_head = 0;
    g_state.runtime_state_history_count = 0;
    g_state.last_runtime_error_code = PHYSICS_ERROR_NONE;
    g_state.last_runtime_error_log_ms = 0;
    g_state.last_collision_capture_ms = 0;
    clear_collision_events();
    g_state.scene_needs_stage_fit = 1;
    push_console_log(L"[场景] 切换到: %ls", scene_display_name(g_state.scene_index));
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

static int snapshot_hex_encode(const unsigned char* src, int src_len, char* out, int out_cap) {
    static const char HEX[] = "0123456789ABCDEF";
    int i;
    int oi = 0;
    if (src == NULL || out == NULL || out_cap <= 0 || src_len < 0) return 0;
    if (src_len == 0) {
        if (out_cap < 2) return 0;
        out[0] = '.';
        out[1] = '\0';
        return 1;
    }
    if ((src_len * 2 + 1) > out_cap) return 0;
    for (i = 0; i < src_len; i++) {
        unsigned char b = src[i];
        out[oi++] = HEX[(b >> 4) & 0x0F];
        out[oi++] = HEX[b & 0x0F];
    }
    out[oi] = '\0';
    return 1;
}

static int snapshot_hex_decode(const char* src, unsigned char* out, int out_cap, int* out_len) {
    int i;
    int n;
    if (out_len != NULL) *out_len = 0;
    if (src == NULL || out == NULL || out_cap <= 0) return 0;
    n = (int)strlen(src);
    if (n == 1 && src[0] == '.') {
        if (out_len != NULL) *out_len = 0;
        return 1;
    }
    if ((n % 2) != 0) return 0;
    if ((n / 2) > out_cap) return 0;
    for (i = 0; i < n; i += 2) {
        int h = src[i];
        int l = src[i + 1];
        int hv;
        int lv;
        if (h >= '0' && h <= '9') hv = h - '0';
        else if (h >= 'A' && h <= 'F') hv = h - 'A' + 10;
        else if (h >= 'a' && h <= 'f') hv = h - 'a' + 10;
        else return 0;
        if (l >= '0' && l <= '9') lv = l - '0';
        else if (l >= 'A' && l <= 'F') lv = l - 'A' + 10;
        else if (l >= 'a' && l <= 'f') lv = l - 'a' + 10;
        else return 0;
        out[i / 2] = (unsigned char)((hv << 4) | lv);
    }
    if (out_len != NULL) *out_len = n / 2;
    return 1;
}

static int scene_order_is_valid(const int* order) {
    int seen[SCENE_COUNT];
    int i;
    if (order == NULL) return 0;
    for (i = 0; i < SCENE_COUNT; i++) seen[i] = 0;
    for (i = 0; i < SCENE_COUNT; i++) {
        int v = order[i];
        if (v < 0 || v >= SCENE_COUNT) return 0;
        if (seen[v]) return 0;
        seen[v] = 1;
    }
    return 1;
}

static int snapshot_append_editor_metadata(const char* path) {
    FILE* fp;
    int i;
    if (path == NULL) return 0;
    fp = fopen(path, "a");
    if (fp == NULL) return 0;
    fprintf(fp, "EDITOR_META 1\n");
    fprintf(fp, "SCENE_ACTIVE %d\n", g_state.scene_index);
    fprintf(fp, "SCENE_ORDER");
    for (i = 0; i < SCENE_COUNT; i++) {
        fprintf(fp, " %d", g_state.scene_order[i]);
    }
    fprintf(fp, "\n");
    for (i = 0; i < SCENE_COUNT; i++) {
        char utf8[SCENE_NAME_MAX * 4];
        int utf8_len;
        char hex[SCENE_NAME_MAX * 8 + 8];
        utf8_len = WideCharToMultiByte(CP_UTF8, 0, g_state.scene_names[i], -1, utf8, (int)sizeof(utf8), NULL, NULL);
        if (utf8_len <= 0) {
            utf8[0] = '\0';
        }
        if (!snapshot_hex_encode((const unsigned char*)utf8, (int)strlen(utf8), hex, (int)sizeof(hex))) {
            fclose(fp);
            return 0;
        }
        fprintf(fp, "SCENE_NAME %d %s\n", i, hex);
    }
    for (i = 0; i < SCENE_COUNT; i++) {
        const char* guid = g_state.scenes[i].asset_ref_guid;
        char guid_hex[ASSET_DB_MAX_GUID * 2 + 8];
        if (!asset_meta_is_valid_guid(guid)) guid = SCENE_ASSET_GUID_NONE;
        if (!snapshot_hex_encode((const unsigned char*)guid, (int)strlen(guid), guid_hex, (int)sizeof(guid_hex))) {
            fclose(fp);
            return 0;
        }
        fprintf(fp, "SCENE_ASSET_GUID %d %s\n", i, guid_hex);
    }
    fclose(fp);
    return 1;
}

static int snapshot_restore_editor_metadata(const char* path) {
    FILE* fp;
    char line[1024];
    int has_meta = 0;
    int got_active = 0;
    int active_scene = 0;
    int got_order = 0;
    int parsed_order[SCENE_COUNT];
    int got_name[SCENE_COUNT];
    int got_guid[SCENE_COUNT];
    wchar_t parsed_name[SCENE_COUNT][SCENE_NAME_MAX];
    char parsed_guid[SCENE_COUNT][ASSET_DB_MAX_GUID];
    int i;
    if (path == NULL) return 0;
    fp = fopen(path, "r");
    if (fp == NULL) return 0;
    for (i = 0; i < SCENE_COUNT; i++) {
        got_name[i] = 0;
        got_guid[i] = 0;
        parsed_name[i][0] = L'\0';
        lstrcpynA(parsed_guid[i], SCENE_ASSET_GUID_NONE, ASSET_DB_MAX_GUID);
        parsed_order[i] = i;
    }
    while (fgets(line, (int)sizeof(line), fp) != NULL) {
        if (strncmp(line, "EDITOR_META ", 12) == 0) {
            has_meta = 1;
            continue;
        }
        if (!has_meta) continue;
        if (strncmp(line, "SCENE_ACTIVE ", 13) == 0) {
            int v = 0;
            if (sscanf(line + 13, "%d", &v) == 1) {
                got_active = 1;
                active_scene = v;
            }
            continue;
        }
        if (strncmp(line, "SCENE_ORDER ", 12) == 0) {
            char* p = line + 12;
            int ok = 1;
            for (i = 0; i < SCENE_COUNT; i++) {
                char* endp;
                long v;
                while (*p != '\0' && isspace((unsigned char)*p)) p++;
                if (*p == '\0') {
                    ok = 0;
                    break;
                }
                v = strtol(p, &endp, 10);
                if (p == endp) {
                    ok = 0;
                    break;
                }
                parsed_order[i] = (int)v;
                p = endp;
            }
            if (ok && scene_order_is_valid(parsed_order)) {
                got_order = 1;
            }
            continue;
        }
        if (strncmp(line, "SCENE_NAME ", 11) == 0) {
            int idx = -1;
            char hex[800];
            if (sscanf(line + 11, "%d %799s", &idx, hex) == 2) {
                if (idx >= 0 && idx < SCENE_COUNT) {
                    unsigned char utf8[SCENE_NAME_MAX * 4];
                    int utf8_len = 0;
                    if (snapshot_hex_decode(hex, utf8, (int)sizeof(utf8), &utf8_len)) {
                        wchar_t wbuf[SCENE_NAME_MAX];
                        int wlen;
                        if (utf8_len < (int)sizeof(utf8)) {
                            utf8[utf8_len] = '\0';
                            wlen = MultiByteToWideChar(CP_UTF8, 0, (const char*)utf8, -1, wbuf, SCENE_NAME_MAX);
                            if (wlen > 0) {
                                lstrcpynW(parsed_name[idx], wbuf, SCENE_NAME_MAX);
                                got_name[idx] = 1;
                            }
                        }
                    }
                }
            }
            continue;
        }
        if (strncmp(line, "SCENE_ASSET_GUID ", 17) == 0) {
            int idx = -1;
            char hex[800];
            if (sscanf(line + 17, "%d %799s", &idx, hex) == 2) {
                if (idx >= 0 && idx < SCENE_COUNT) {
                    unsigned char guid_buf[ASSET_DB_MAX_GUID];
                    int guid_len = 0;
                    if (snapshot_hex_decode(hex, guid_buf, (int)sizeof(guid_buf), &guid_len)) {
                        if (guid_len >= 0 && guid_len < ASSET_DB_MAX_GUID) {
                            guid_buf[guid_len] = '\0';
                            if (asset_meta_is_valid_guid((const char*)guid_buf)) {
                                lstrcpynA(parsed_guid[idx], (const char*)guid_buf, ASSET_DB_MAX_GUID);
                                got_guid[idx] = 1;
                            }
                        }
                    }
                }
            }
            continue;
        }
    }
    fclose(fp);
    if (!has_meta) return 0;
    if (got_order) {
        for (i = 0; i < SCENE_COUNT; i++) {
            g_state.scene_order[i] = parsed_order[i];
        }
    }
    if (got_active && active_scene >= 0 && active_scene < SCENE_COUNT) {
        g_state.scene_index = active_scene;
    }
    for (i = 0; i < SCENE_COUNT; i++) {
        if (got_name[i]) {
            lstrcpynW(g_state.scene_names[i], parsed_name[i], SCENE_NAME_MAX);
        }
        if (got_guid[i]) {
            lstrcpynA(g_state.scenes[i].asset_ref_guid, parsed_guid[i], ASSET_DB_MAX_GUID);
        }
    }
    return 1;
}

static int save_scene_snapshot(const char* path) {
    char resolved_path[MAX_PATH];
    int ok;
    if (path == NULL) return 0;
    if (!sandbox_app_path_runtime_file_utf8(path, resolved_path, MAX_PATH)) return 0;
    trace_spawn_step("snapshot.save.begin", "path=%s engine=%p", resolved_path, (void*)g_state.engine);
    ok = runtime_snapshot_repo_save(g_state.engine, resolved_path);
    if (ok) {
        ok = snapshot_append_editor_metadata(resolved_path);
    }
    if (ok) trace_spawn_step("snapshot.save.end", "path=%s ok=1", resolved_path);
    return ok;
}

static int load_scene_snapshot(const char* path) {
    char resolved_path[MAX_PATH];
    if (path == NULL) return 0;
    if (!sandbox_app_path_runtime_file_utf8(path, resolved_path, MAX_PATH)) return 0;
    if (!runtime_snapshot_repo_load(g_state.engine, resolved_path)) return 0;
    snapshot_restore_editor_metadata(resolved_path);
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
    wchar_t wpath[MAX_PATH];
    char resolved_path[MAX_PATH];
    if (path == NULL) return 0;
    if (!sandbox_app_path_runtime_file_utf8(path, resolved_path, MAX_PATH)) return 0;
    MultiByteToWideChar(CP_UTF8, 0, resolved_path, -1, wpath, MAX_PATH);
    attr = GetFileAttributesW(wpath);
    if (attr == INVALID_FILE_ATTRIBUTES) return 0;
    return ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

static void utf8_to_wide_copy(const char* src, wchar_t* out, int out_cap) {
    if (out == NULL || out_cap <= 0) return;
    out[0] = L'\0';
    if (src == NULL || src[0] == '\0') return;
    MultiByteToWideChar(CP_UTF8, 0, src, -1, out, out_cap);
}

static void wide_to_utf8_copy(const wchar_t* src, char* out, int out_cap) {
    if (out == NULL || out_cap <= 0) return;
    out[0] = '\0';
    if (src == NULL || src[0] == L'\0') return;
    WideCharToMultiByte(CP_UTF8, 0, src, -1, out, out_cap, NULL, NULL);
}

static int phase_g_scene_inspector_init(void* user) {
    int* flag = (int*)user;
    if (flag != NULL) *flag = 1;
    return 1;
}

static void phase_g_scene_inspector_shutdown(void* user) {
    int* flag = (int*)user;
    if (flag != NULL) *flag = 0;
}

static int phase_g_failing_plugin_init(void* user) {
    (void)user;
    return 0;
}

static const wchar_t* startup_severity_label(EditorExtensionStartupSeverity severity) {
    if (severity == EDITOR_EXTENSION_STARTUP_SEVERITY_ERROR) return L"错误";
    if (severity == EDITOR_EXTENSION_STARTUP_SEVERITY_WARNING) return L"警告";
    return L"信息";
}

static void log_editor_extension_startup_diagnostics(void) {
    int i;
    for (i = 0; i < g_editor_extension_startup_state.diagnostic_count; ++i) {
        wchar_t wide_stage[EDITOR_EXTENSION_STARTUP_MAX_STAGE];
        wchar_t wide_path[PROJECT_WORKSPACE_MAX_PATH];
        wchar_t wide_message[EDITOR_EXTENSION_STARTUP_MAX_MESSAGE];
        const EditorExtensionStartupDiagnostic* diagnostic = &g_editor_extension_startup_state.diagnostics[i];
        utf8_to_wide_copy(diagnostic->stage, wide_stage, (int)(sizeof(wide_stage) / sizeof(wide_stage[0])));
        utf8_to_wide_copy(diagnostic->path, wide_path, (int)(sizeof(wide_path) / sizeof(wide_path[0])));
        utf8_to_wide_copy(diagnostic->message, wide_message, (int)(sizeof(wide_message) / sizeof(wide_message[0])));
        push_console_log(L"[启动][%ls] %ls | %ls | %ls",
                         startup_severity_label(diagnostic->severity),
                         wide_stage[0] ? wide_stage : L"-",
                         wide_path[0] ? wide_path : L"-",
                         wide_message[0] ? wide_message : L"-");
    }
}

static void phase_g_refresh_editor_extension_state(void) {
    EditorPluginV1 scene_inspector{};
    EditorPluginV1 failing_plugin{};

    g_editor_plugin_builtin_enabled = 0;
    g_startup_status_text[0] = L'\0';
    g_startup_degraded = 0;

    scene_inspector.init = phase_g_scene_inspector_init;
    scene_inspector.shutdown = phase_g_scene_inspector_shutdown;
    scene_inspector.user = &g_editor_plugin_builtin_enabled;
    failing_plugin.init = phase_g_failing_plugin_init;
    failing_plugin.shutdown = NULL;
    failing_plugin.user = NULL;

    editor_extension_host_refresh(&g_editor_extension_startup_state,
                                  &g_editor_plugin_builtin_enabled,
                                  g_startup_status_text,
                                  96,
                                  &g_startup_degraded,
                                  &scene_inspector,
                                  &failing_plugin);
    log_editor_extension_startup_diagnostics();
}

static void phase_g_shutdown_editor_extension_state(void) {
    editor_extension_host_shutdown(&g_editor_extension_startup_state);
}

static void session_recovery_persist(const wchar_t* recent_action) {
    char action_utf8[SESSION_RECOVERY_MAX_ACTION];
    char autosave_path[MAX_PATH];
    char scene_guid[SESSION_RECOVERY_MAX_SCENE_GUID];
    wide_to_utf8_copy(recent_action, action_utf8, SESSION_RECOVERY_MAX_ACTION);
    snprintf(scene_guid, sizeof(scene_guid), "scene://%d", g_state.scene_index + 1);
    if (!sandbox_app_path_runtime_file_utf8("autosave_snapshot.txt", autosave_path, MAX_PATH)) {
        lstrcpynA(autosave_path, "autosave_snapshot.txt", MAX_PATH);
    }
    session_recovery_mark_unclean(&g_session_recovery_state, autosave_path, autosave_path, scene_guid,
                                  action_utf8[0] != '\0' ? action_utf8 : "tick");
    session_recovery_state_save_v1(&g_session_recovery_state, editor_extension_host_default_session_recovery_path());
}

static void export_diagnostic_bundle_now(void) {
    DiagnosticBundleRequest request;
    DiagnosticBundleResult result{};
    char autosave_path[MAX_PATH];
    char scene_snapshot_path[MAX_PATH];
    int i;
    diagnostic_bundle_request_init(&request);
    if (!sandbox_app_path_runtime_file_utf8("artifacts\\diagnostics", request.output_root, DIAGNOSTIC_BUNDLE_MAX_PATH)) {
        lstrcpynA(request.output_root, "artifacts/diagnostics", DIAGNOSTIC_BUNDLE_MAX_PATH);
    }
    strcpy(request.label, "sandbox_manual_export");
    strcpy(request.build_metadata_path, BUILD_METADATA_PATH);
    request.file_count = 0;
    if (!sandbox_app_path_runtime_file_utf8("autosave_snapshot.txt", autosave_path, MAX_PATH)) lstrcpynA(autosave_path, "autosave_snapshot.txt", MAX_PATH);
    if (!sandbox_app_path_runtime_file_utf8("scene_snapshot.txt", scene_snapshot_path, MAX_PATH)) lstrcpynA(scene_snapshot_path, "scene_snapshot.txt", MAX_PATH);
    editor_extension_host_append_default_diagnostic_files(&request, autosave_path, scene_snapshot_path);
    request.log_line_count = g_console_log_count < DIAGNOSTIC_BUNDLE_MAX_LOG_LINES ? g_console_log_count : DIAGNOSTIC_BUNDLE_MAX_LOG_LINES;
    for (i = 0; i < request.log_line_count; ++i) {
        wchar_t* src = g_console_logs[(g_console_log_head + g_console_log_count - request.log_line_count + i + CONSOLE_LOG_CAP) % CONSOLE_LOG_CAP];
        wide_to_utf8_copy(src, request.log_lines[i], DIAGNOSTIC_BUNDLE_MAX_LOG_CHARS);
    }
    if (diagnostic_bundle_export_v1(&request, &result)) {
        wchar_t wide_bundle[DIAGNOSTIC_BUNDLE_MAX_PATH];
        utf8_to_wide_copy(result.bundle_dir, wide_bundle, DIAGNOSTIC_BUNDLE_MAX_PATH);
        push_console_log(L"[诊断] 已导出最小诊断包: %ls", wide_bundle);
    } else {
        push_console_log(L"[错误] 导出诊断包失败");
    }
}

static int hot_reload_is_supported_extension(const char* source_path) {
    return asset_importer_kind_from_path(source_path) != ASSET_IMPORT_KIND_UNKNOWN;
}

static void hot_reload_normalize_path(char* path) {
    size_t i;
    if (path == NULL) return;
    for (i = 0; path[i] != '\0'; i++) {
        if (path[i] == '\\') path[i] = '/';
    }
}

static int hot_reload_register_source_path(const char* source_path) {
    char guid[ASSET_DB_MAX_GUID];
    if (source_path == NULL || source_path[0] == '\0') return 0;
    if (!hot_reload_is_supported_extension(source_path)) return 0;
    if (!asset_hot_reload_register_source(&g_hot_reload_service, source_path, "sandbox:auto", guid)) return 0;
    if (!asset_fs_watch_watch_path(&g_asset_fs_watch, source_path)) return 0;
    return 1;
}

static void hot_reload_register_tree_recursive(const char* root_dir, int depth) {
    char search_pattern[ASSET_WATCH_MAX_PATH];
    char full_path[ASSET_WATCH_MAX_PATH];
    char normalized_path[ASSET_WATCH_MAX_PATH];
    WIN32_FIND_DATAA find_data;
    HANDLE find_handle;
    if (root_dir == NULL || root_dir[0] == '\0') return;
    if (depth > 16) return;
    if (snprintf(search_pattern, sizeof(search_pattern), "%s\\*", root_dir) <= 0 ||
        strlen(search_pattern) >= sizeof(search_pattern)) {
        return;
    }
    find_handle = FindFirstFileA(search_pattern, &find_data);
    if (find_handle == INVALID_HANDLE_VALUE) return;
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) continue;
        if (snprintf(full_path, sizeof(full_path), "%s\\%s", root_dir, find_data.cFileName) <= 0 ||
            strlen(full_path) >= sizeof(full_path)) {
            continue;
        }
        if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            hot_reload_register_tree_recursive(full_path, depth + 1);
        } else {
            strncpy(normalized_path, full_path, sizeof(normalized_path) - 1);
            normalized_path[sizeof(normalized_path) - 1] = '\0';
            hot_reload_normalize_path(normalized_path);
            hot_reload_register_source_path(normalized_path);
        }
    } while (FindNextFileA(find_handle, &find_data));
    FindClose(find_handle);
}

static void init_hot_reload_pipeline(void) {
    const char* roots[] = {"assets", "Assets"};
    int i;
    int root_found = 0;
    asset_hot_reload_service_init(&g_hot_reload_service, HOT_RELOAD_DEBOUNCE_MS);
    asset_fs_watch_init(&g_asset_fs_watch);
    g_hot_reload_root_dir[0] = '\0';
    g_state.hot_reload_enabled = 0;
    for (i = 0; i < (int)(sizeof(roots) / sizeof(roots[0])); i++) {
        DWORD attr = GetFileAttributesA(roots[i]);
        if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            strncpy(g_hot_reload_root_dir, roots[i], sizeof(g_hot_reload_root_dir) - 1);
            g_hot_reload_root_dir[sizeof(g_hot_reload_root_dir) - 1] = '\0';
            hot_reload_register_tree_recursive(roots[i], 0);
            asset_fs_watch_watch_directory(&g_asset_fs_watch, roots[i]);
            root_found = 1;
            break;
        }
    }
    g_state.hot_reload_watch_count = asset_fs_watch_get_watch_count(&g_asset_fs_watch);
    g_state.hot_reload_enabled = (g_state.hot_reload_watch_count > 0) ? 1 : 0;
    g_state.hot_reload_last_discover_ms = (unsigned int)GetTickCount();
    if (!root_found) {
        push_console_log(L"[热重载] 未找到 assets 目录，已跳过监听初始化");
    } else if (g_state.hot_reload_enabled) {
        if (asset_fs_watch_is_native_active(&g_asset_fs_watch)) {
            push_console_log(L"[热重载] 已监听 %d 个资源文件（原生监听触发 + 轮询增量导入）", g_state.hot_reload_watch_count);
        } else {
            push_console_log(L"[热重载] 已监听 %d 个资源文件（轮询+增量导入）", g_state.hot_reload_watch_count);
        }
    } else {
        push_console_log(L"[热重载] assets 目录无可导入资源，监听待机");
    }
}

static void hot_reload_tick_runtime(void) {
    unsigned int now_ms;
    int change_count = 0;
    AssetHotReloadTickReport report;
    now_ms = (unsigned int)GetTickCount();
    if (g_hot_reload_root_dir[0] != '\0' &&
        (g_state.hot_reload_last_discover_ms == 0 ||
         (now_ms - g_state.hot_reload_last_discover_ms) >= HOT_RELOAD_DISCOVER_INTERVAL_MS)) {
        int prev_watch_count = asset_fs_watch_get_watch_count(&g_asset_fs_watch);
        hot_reload_register_tree_recursive(g_hot_reload_root_dir, 0);
        g_state.hot_reload_watch_count = asset_fs_watch_get_watch_count(&g_asset_fs_watch);
        g_state.hot_reload_last_discover_ms = now_ms;
        if (g_state.hot_reload_watch_count > prev_watch_count) {
            push_console_log(L"[热重载] 新增监听 %d 项（总计:%d）",
                             g_state.hot_reload_watch_count - prev_watch_count,
                             g_state.hot_reload_watch_count);
        }
        if (g_state.hot_reload_watch_count > 0) {
            g_state.hot_reload_enabled = 1;
        }
    }
    if (!g_state.hot_reload_enabled) return;
    if (g_state.hot_reload_last_scan_ms > 0 &&
        (now_ms - g_state.hot_reload_last_scan_ms) < HOT_RELOAD_SCAN_INTERVAL_MS) {
        return;
    }
    g_state.hot_reload_last_scan_ms = now_ms;
    if (!asset_fs_watch_scan(&g_asset_fs_watch, &g_hot_reload_service, (long long)now_ms, &change_count)) {
        push_tick_runtime_error(APP_RUNTIME_ERROR_CODE_HOT_RELOAD_SCAN_FAILED, APP_RUNTIME_ERROR_WARNING, 1);
        if (g_state.hot_reload_last_warn_ms == 0 || (now_ms - g_state.hot_reload_last_warn_ms) >= 1000) {
            push_console_log(L"[警告] 热重载文件扫描失败");
            g_state.hot_reload_last_warn_ms = now_ms;
        }
        return;
    }
    g_state.hot_reload_scan_change_count = change_count;
    asset_hot_reload_tick_report_init(&report);
    if (!asset_hot_reload_tick(&g_hot_reload_service, (long long)now_ms, HOT_RELOAD_CACHE_ROOT, &report)) {
        if (report.pipeline_ran) {
            app_runtime_report_hot_reload(&g_app_runtime, &report, pie_runtime_active());
        } else {
            push_tick_runtime_error(APP_RUNTIME_ERROR_CODE_HOT_RELOAD_BATCH_FAILED, APP_RUNTIME_ERROR_ERROR, 1);
        }
        if (g_state.hot_reload_last_warn_ms == 0 || (now_ms - g_state.hot_reload_last_warn_ms) >= 1000) {
            push_console_log(L"[警告] 热重载导入流程失败");
            g_state.hot_reload_last_warn_ms = now_ms;
        }
        return;
    }
    if (report.pipeline_ran) {
        app_runtime_report_hot_reload(&g_app_runtime, &report, pie_runtime_active());
    }
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
    push_console_log(L"%ls", text);
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

static void init_editor_command_bus_callbacks(void) {
    editor_command_bus_init(&g_editor_command_bus);
    g_editor_command_bus.scene_rename = editor_cb_scene_rename;
    g_editor_command_bus.scene_order_move = editor_cb_scene_order_move;
    g_editor_command_bus.scene_order_reset = editor_cb_scene_order_reset;
    g_editor_command_bus.scene_asset_ref_set = editor_cb_scene_asset_ref_set;
    g_editor_command_bus.inspector_set_value = editor_cb_inspector_set_value;
    g_editor_command_bus.user = NULL;
}

static int dispatch_editor_command(const EditorCommand* command) {
    return editor_command_bus_dispatch(&g_editor_command_bus, command);
}

static int editor_cb_scene_rename(int scene_index, const wchar_t* name, void* user) {
    (void)user;
    if (!scene_rename_apply(scene_index, name)) return 0;
    history_push_snapshot();
    return 1;
}

static int editor_cb_scene_order_move(int direction, void* user) {
    (void)user;
    if (!scene_order_move_selected(direction)) return 0;
    history_push_snapshot();
    return 1;
}

static int editor_cb_scene_order_reset(void* user) {
    (void)user;
    if (!scene_order_reset_default()) return 0;
    history_push_snapshot();
    return 1;
}

static int editor_cb_scene_asset_ref_set(int scene_index, const wchar_t* guid, void* user) {
    (void)user;
    if (!scene_asset_guid_apply(scene_index, guid)) return 0;
    history_push_snapshot();
    return 1;
}

static int editor_cb_inspector_set_value(int row, double value, int emit_log, void* user) {
    (void)user;
    return inspector_commit_row_value(row, value, emit_log);
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
        L"assets/icons/physics_sandbox_32.png"
    };
    int i;

    if (g_ui.app_icon_bitmap != NULL) return S_OK;
    if (g_ui.wic_factory == NULL || g_ui.target == NULL) return E_FAIL;

    hr = E_FAIL;
    for (i = 0; i < (int)(sizeof(candidates) / sizeof(candidates[0])); i++) {
        wchar_t resolved_path[MAX_PATH];
        if (!sandbox_app_path_find_from_exe_ancestors_wide(candidates[i], 4, resolved_path, MAX_PATH)) continue;
        hr = IWICImagingFactory_CreateDecoderFromFilename(g_ui.wic_factory, resolved_path, NULL, GENERIC_READ,
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
        sync_ui_theme();
        bc = g_ui_theme.colors.text_primary;
        hr = g_ui.target->CreateSolidColorBrush(bc, &g_ui.brush);
    }
    if (SUCCEEDED(hr) && g_ui.app_icon_bitmap == NULL) {
        HRESULT icon_hr = create_app_icon_bitmap();
        if (FAILED(icon_hr)) {
            g_ui.app_icon_bitmap = NULL;
        }
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
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_ui.d2d_factory);
    if (FAILED(hr)) return hr;

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, IID_IDWriteFactory, reinterpret_cast<IUnknown**>(&g_ui.dwrite_factory));
    if (FAILED(hr)) return hr;

    hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
                          reinterpret_cast<void**>(&g_ui.wic_factory));
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

        ea = D2D1_ELLIPSE{pt(a.x, a.y), (i == g_state.selected_constraint_index) ? 4.2f : 3.2f, (i == g_state.selected_constraint_index) ? 4.2f : 3.2f};
        eb = D2D1_ELLIPSE{pt(b.x, b.y), (i == g_state.selected_constraint_index) ? 4.2f : 3.2f, (i == g_state.selected_constraint_index) ? 4.2f : 3.2f};
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

static D2D1_RECT_F runtime_bus_badge_rect(void) {
    TopToolbarLayout layout{};
    D2D1_RECT_F rect = rc(0, 0, 0, 0);
    layout.run_rect = g_top_run_rect;
    top_toolbar_runtime_badge_rect(&layout, &rect);
    return rect;
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
    g_state.value_input_target = VALUE_INPUT_TARGET_INSPECTOR;
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

static int parse_double_strict(const wchar_t* text, double* out_v) {
    const wchar_t* start;
    wchar_t* end = NULL;
    double value;
    if (text == NULL || out_v == NULL) return 0;
    start = text;
    while (*start != L'\0' && iswspace(*start)) start++;
    if (*start == L'\0') return 0;
    value = wcstod(start, &end);
    if (start == end) return 0;
    while (end != NULL && *end != L'\0' && iswspace(*end)) end++;
    if (end != NULL && *end != L'\0') return 0;
    if (!isfinite(value)) return 0;
    *out_v = value;
    return 1;
}

static const wchar_t* inspector_body_row_label(int row) {
    if (row == 0) return L"质量";
    if (row == 1) return L"位置X";
    if (row == 2) return L"位置Y";
    if (row == 3) return L"速度X";
    if (row == 4) return L"速度Y";
    if (row == 5) return L"角速度";
    if (row == 6) return L"阻尼";
    if (row == 7) return L"弹性";
    return L"字段";
}

static const wchar_t* inspector_constraint_row_label(int row) {
    if (row == 0) return L"目标长度";
    if (row == 1) return L"刚度";
    if (row == 2) return L"阻尼";
    if (row == 3) return L"断裂阈值";
    if (row == 4) return L"连体碰撞";
    if (row == 5) return L"弹性预设";
    return L"字段";
}

static int inspector_apply_body_row_value(int row, double value, int commit) {
    RigidBody* b = g_state.selected;
    if (b == NULL) return 0;
    if (!isfinite(value)) {
        if (commit) push_console_log(L"[警告] 检查器输入不是有效数值");
        return 0;
    }
    if (row == 0) {
        if (value < 0.1 || value > 1000.0) {
            if (commit) push_console_log(L"[警告] 质量范围应为 0.1 ~ 1000");
            return 0;
        }
        if (commit) {
            b->mass = (float)value;
            b->inv_mass = (b->mass > 0.0f) ? (1.0f / b->mass) : 0.0f;
            if (b->shape != NULL) {
                b->inertia = shape_get_moment_of_inertia(b->shape, b->mass);
                b->inv_inertia = (b->inertia > 0.0f) ? (1.0f / b->inertia) : 0.0f;
            }
        }
        return 1;
    }
    if (row == 1 || row == 2) {
        if (value < -5000.0 || value > 5000.0) {
            if (commit) push_console_log(L"[警告] 位置范围应为 -5000 ~ 5000");
            return 0;
        }
        if (commit) {
            if (row == 1) b->position.x = (float)value;
            else b->position.y = (float)value;
        }
        return 1;
    }
    if (row == 3 || row == 4) {
        if (value < -500.0 || value > 500.0) {
            if (commit) push_console_log(L"[警告] 速度范围应为 -500 ~ 500");
            return 0;
        }
        if (commit) {
            if (row == 3) b->velocity.x = (float)value;
            else b->velocity.y = (float)value;
        }
        return 1;
    }
    if (row == 5) {
        if (value < -200.0 || value > 200.0) {
            if (commit) push_console_log(L"[警告] 角速度范围应为 -200 ~ 200");
            return 0;
        }
        if (commit) b->angular_velocity = (float)value;
        return 1;
    }
    if (row == 6) {
        if (value < 0.0 || value > 1.0) {
            if (commit) push_console_log(L"[警告] 阻尼范围应为 0.0 ~ 1.0");
            return 0;
        }
        if (commit) b->damping = (float)value;
        return 1;
    }
    if (row == 7) {
        if (b->shape == NULL) {
            if (commit) push_console_log(L"[警告] 当前物体缺少形状，无法编辑弹性");
            return 0;
        }
        if (value < 0.0 || value > 1.0) {
            if (commit) push_console_log(L"[警告] 弹性范围应为 0.0 ~ 1.0");
            return 0;
        }
        if (commit) b->shape->restitution = (float)value;
        return 1;
    }
    if (commit) push_console_log(L"[警告] 未知检查器字段");
    return 0;
}

static int inspector_apply_constraint_row_value(int row, double value, int commit) {
    const Constraint* c = selected_constraint_ref();
    int iv;
    if (c == NULL || !c->active || g_state.engine == NULL || g_state.selected_constraint_index < 0) return 0;
    if (!isfinite(value)) {
        if (commit) push_console_log(L"[警告] 检查器输入不是有效数值");
        return 0;
    }
    if (row == 0) {
        if (value < 0.0 || value > 500.0) {
            if (commit) push_console_log(L"[警告] 目标长度范围应为 0 ~ 500");
            return 0;
        }
        if (commit) physics_engine_constraint_set_rest_length(g_state.engine, g_state.selected_constraint_index, (float)value);
        return 1;
    }
    if (row == 1) {
        if (c->type == CONSTRAINT_DISTANCE) {
            if (value < 0.08 || value > 1.0) {
                if (commit) push_console_log(L"[警告] 距离约束刚度范围应为 0.08 ~ 1.0");
                return 0;
            }
        } else {
            if (value < 0.20 || value > 40.0) {
                if (commit) push_console_log(L"[警告] 弹簧刚度范围应为 0.20 ~ 40.0");
                return 0;
            }
        }
        if (commit) physics_engine_constraint_set_stiffness(g_state.engine, g_state.selected_constraint_index, (float)value);
        return 1;
    }
    if (row == 2) {
        if (value < 0.0 || value > 20.0) {
            if (commit) push_console_log(L"[警告] 约束阻尼范围应为 0.0 ~ 20.0");
            return 0;
        }
        if (commit) physics_engine_constraint_set_damping(g_state.engine, g_state.selected_constraint_index, (float)value);
        return 1;
    }
    if (row == 3) {
        if (value < 0.0 || value > 100000.0) {
            if (commit) push_console_log(L"[警告] 断裂阈值范围应为 0 ~ 100000");
            return 0;
        }
        if (commit) physics_engine_constraint_set_break_force(g_state.engine, g_state.selected_constraint_index, (float)value);
        return 1;
    }
    if (row == 4) {
        iv = (int)value;
        if (fabs(value - (double)iv) > 1e-6 || (iv != 0 && iv != 1)) {
            if (commit) push_console_log(L"[警告] 连体碰撞只接受 0 或 1");
            return 0;
        }
        if (commit) physics_engine_constraint_set_collide_connected(g_state.engine, g_state.selected_constraint_index, iv);
        return 1;
    }
    if (row == 5) {
        if (c->type != CONSTRAINT_SPRING) {
            if (commit) push_console_log(L"[警告] 当前约束不支持弹性预设");
            return 0;
        }
        iv = (int)value;
        if (fabs(value - (double)iv) > 1e-6 || iv < 0 || iv > 2) {
            if (commit) push_console_log(L"[警告] 弹性预设仅支持 0(软)/1(中)/2(高)");
            return 0;
        }
        if (commit) apply_spring_preset_to_constraint_index(g_state.selected_constraint_index, iv);
        return 1;
    }
    if (commit) push_console_log(L"[警告] 未知检查器字段");
    return 0;
}

static int inspector_commit_row_value(int row, double value, int emit_log) {
    const Constraint* c = selected_constraint_ref();
    if (c != NULL && c->active) {
        if (!inspector_apply_constraint_row_value(row, value, 0)) return 0;
        if (!inspector_apply_constraint_row_value(row, value, 1)) return 0;
        history_push_snapshot();
        if (emit_log) {
            if (row == 4) {
                push_console_log(L"[检查器] 已更新约束.%ls = %d", inspector_constraint_row_label(row), ((int)value) != 0 ? 1 : 0);
            } else if (row == 5) {
                int preset = (int)value;
                const wchar_t* preset_name = (preset == 0) ? L"软" : ((preset == 1) ? L"中" : L"高");
                push_console_log(L"[检查器] 已更新约束.%ls = %ls", inspector_constraint_row_label(row), preset_name);
            } else {
                push_console_log(L"[检查器] 已更新约束.%ls = %.4f", inspector_constraint_row_label(row), value);
            }
        }
        return 1;
    }
    if (g_state.selected != NULL) {
        if (!inspector_apply_body_row_value(row, value, 0)) return 0;
        if (!inspector_apply_body_row_value(row, value, 1)) return 0;
        history_push_snapshot();
        if (emit_log) {
            push_console_log(L"[检查器] 已更新物体.%ls = %.4f", inspector_body_row_label(row), value);
        }
        return 1;
    }
    return 0;
}

static int inspector_adjust_focused_row(int sign) {
    const Constraint* c;
    int row;
    double next = 0.0;
    if (sign == 0) return 0;
    sign = (sign < 0) ? -1 : 1;
    row = g_state.inspector_focused_row;
    c = selected_constraint_ref();
    if (c != NULL && c->active) {
        if (row == 0) {
            next = c->rest_length + (double)sign * 0.10;
            if (next < 0.0) next = 0.0;
            if (next > 500.0) next = 500.0;
        } else if (row == 1) {
            next = c->stiffness + (double)sign * ((c->type == CONSTRAINT_DISTANCE) ? 0.05 : 0.20);
            if (c->type == CONSTRAINT_DISTANCE) {
                if (next < 0.08) next = 0.08;
                if (next > 1.0) next = 1.0;
            } else {
                if (next < 0.20) next = 0.20;
                if (next > 40.0) next = 40.0;
            }
        } else if (row == 2) {
            next = c->damping + (double)sign * 0.05;
            if (next < 0.0) next = 0.0;
            if (next > 20.0) next = 20.0;
        } else if (row == 3) {
            next = c->break_force + (double)sign * 10.0;
            if (next < 0.0) next = 0.0;
            if (next > 100000.0) next = 100000.0;
        }
        else if (row == 4) next = c->collide_connected ? 0.0 : 1.0;
        else if (row == 5 && c->type == CONSTRAINT_SPRING) {
            int preset = spring_preset_for_constraint(c);
            if (preset < 0) preset = 1;
            preset += sign;
            if (preset < 0) preset = 0;
            if (preset > 2) preset = 2;
            next = (double)preset;
        } else {
            return 0;
        }
        if (row == 0 && fabs(next - c->rest_length) < 1e-6) return 0;
        if (row == 1 && fabs(next - c->stiffness) < 1e-6) return 0;
        if (row == 2 && fabs(next - c->damping) < 1e-6) return 0;
        if (row == 3 && fabs(next - c->break_force) < 1e-6) return 0;
        if (row == 4 && ((next > 0.5) == (c->collide_connected != 0))) return 0;
        if (row == 5 && c->type == CONSTRAINT_SPRING) {
            int current_preset = spring_preset_for_constraint(c);
            int next_preset = (int)next;
            if (current_preset >= 0 && next_preset == current_preset) return 0;
        }
        {
            EditorCommand command_data;
            ZeroMemory(&command_data, sizeof(command_data));
            command_data.type = EDITOR_CMD_INSPECTOR_SET_VALUE;
            command_data.arg_i0 = row;
            command_data.arg_i1 = 0;
            command_data.arg_f0 = next;
            return dispatch_editor_command(&command_data);
        }
    }
    if (g_state.selected != NULL) {
        RigidBody* b = g_state.selected;
        if (row == 0) {
            next = b->mass + (double)sign * 0.10;
            if (next < 0.1) next = 0.1;
            if (next > 1000.0) next = 1000.0;
        } else if (row == 1) {
            next = b->position.x + (double)sign * 0.20;
            if (next < -5000.0) next = -5000.0;
            if (next > 5000.0) next = 5000.0;
        } else if (row == 2) {
            next = b->position.y + (double)sign * 0.20;
            if (next < -5000.0) next = -5000.0;
            if (next > 5000.0) next = 5000.0;
        } else if (row == 3) {
            next = b->velocity.x + (double)sign * 0.20;
            if (next < -500.0) next = -500.0;
            if (next > 500.0) next = 500.0;
        } else if (row == 4) {
            next = b->velocity.y + (double)sign * 0.20;
            if (next < -500.0) next = -500.0;
            if (next > 500.0) next = 500.0;
        } else if (row == 5) {
            next = b->angular_velocity + (double)sign * 0.10;
            if (next < -200.0) next = -200.0;
            if (next > 200.0) next = 200.0;
        } else if (row == 6) {
            next = b->damping + (double)sign * 0.02;
            if (next < 0.0) next = 0.0;
            if (next > 1.0) next = 1.0;
        } else if (row == 7) {
            next = (b->shape ? b->shape->restitution : 0.0f) + (double)sign * 0.02;
            if (next < 0.0) next = 0.0;
            if (next > 1.0) next = 1.0;
        }
        else return 0;
        if (row == 0 && fabs(next - b->mass) < 1e-6) return 0;
        if (row == 1 && fabs(next - b->position.x) < 1e-6) return 0;
        if (row == 2 && fabs(next - b->position.y) < 1e-6) return 0;
        if (row == 3 && fabs(next - b->velocity.x) < 1e-6) return 0;
        if (row == 4 && fabs(next - b->velocity.y) < 1e-6) return 0;
        if (row == 5 && fabs(next - b->angular_velocity) < 1e-6) return 0;
        if (row == 6 && fabs(next - b->damping) < 1e-6) return 0;
        if (row == 7 && b->shape != NULL && fabs(next - b->shape->restitution) < 1e-6) return 0;
        {
            EditorCommand command_data;
            ZeroMemory(&command_data, sizeof(command_data));
            command_data.type = EDITOR_CMD_INSPECTOR_SET_VALUE;
            command_data.arg_i0 = row;
            command_data.arg_i1 = 0;
            command_data.arg_f0 = next;
            return dispatch_editor_command(&command_data);
        }
    }
    return 0;
}

static int scene_rename_apply(int scene_index, const wchar_t* input_name) {
    int src_len;
    int begin = 0;
    int end;
    int n = 0;
    wchar_t name_buf[SCENE_NAME_MAX];
    if (input_name == NULL) return 0;
    if (scene_index < 0 || scene_index >= SCENE_COUNT) return 0;
    src_len = (int)lstrlenW(input_name);
    while (begin < src_len && iswspace(input_name[begin])) begin++;
    end = src_len;
    while (end > begin && iswspace(input_name[end - 1])) end--;
    if (end <= begin) {
        push_console_log(L"[警告] 场景名称不能为空");
        return 0;
    }
    while ((begin + n) < end && n < (SCENE_NAME_MAX - 1)) {
        name_buf[n] = input_name[begin + n];
        n++;
    }
    name_buf[n] = L'\0';
    lstrcpynW(g_state.scene_names[scene_index], name_buf, SCENE_NAME_MAX);
    push_console_log(L"[场景] 已重命名 #%d: %ls", scene_index + 1, scene_display_name(scene_index));
    return 1;
}

static int scene_asset_guid_apply(int scene_index, const wchar_t* input_guid) {
    int src_len;
    int begin = 0;
    int end;
    int n = 0;
    char guid[ASSET_DB_MAX_GUID];
    wchar_t wide_guid[ASSET_DB_MAX_GUID];
    int old_wlen;
    if (input_guid == NULL) return 0;
    if (scene_index < 0 || scene_index >= SCENE_COUNT) return 0;
    src_len = (int)lstrlenW(input_guid);
    while (begin < src_len && iswspace(input_guid[begin])) begin++;
    end = src_len;
    while (end > begin && iswspace(input_guid[end - 1])) end--;
    if (end <= begin) {
        push_console_log(L"[警告] 资产 GUID 不能为空");
        return 0;
    }
    while ((begin + n) < end && n < (ASSET_DB_MAX_GUID - 1)) {
        wchar_t ch = input_guid[begin + n];
        if (ch < 32 || ch > 126) {
            push_console_log(L"[警告] 资产 GUID 仅允许 ASCII 可见字符");
            return 0;
        }
        guid[n] = (char)ch;
        n++;
    }
    if ((begin + n) < end) {
        push_console_log(L"[警告] 资产 GUID 过长（上限 %d）", ASSET_DB_MAX_GUID - 1);
        return 0;
    }
    guid[n] = '\0';
    if (!asset_meta_is_valid_guid(guid)) {
        push_console_log(L"[警告] 资产 GUID 无效，需符合 asset://<safe-id> 规则");
        return 0;
    }
    if (strcmp(g_state.scenes[scene_index].asset_ref_guid, guid) == 0) {
        return 0;
    }
    lstrcpynA(g_state.scenes[scene_index].asset_ref_guid, guid, ASSET_DB_MAX_GUID);
    old_wlen = MultiByteToWideChar(CP_UTF8, 0, guid, -1, wide_guid, ASSET_DB_MAX_GUID);
    if (old_wlen <= 0) {
        lstrcpynW(wide_guid, L"(decode-failed)", ASSET_DB_MAX_GUID);
    }
    push_console_log(L"[场景] 已更新资产引用 #%d: %ls", scene_index + 1, wide_guid);
    return 1;
}

static void begin_value_input_for_hierarchy_filter(void) {
    lstrcpynW(g_state.value_input_buf, g_state.hierarchy_filter_buf, 64);
    g_state.value_input_target = VALUE_INPUT_TARGET_HIERARCHY_FILTER;
    g_state.value_input_row = 0;
    g_state.show_value_input = 1;
    g_state.value_input_len = (int)lstrlenW(g_state.value_input_buf);
    g_state.value_input_caret = g_state.value_input_len;
}

static void begin_value_input_for_scene_name(int scene_index) {
    if (scene_index < 0 || scene_index >= SCENE_COUNT) return;
    lstrcpynW(g_state.value_input_buf, scene_display_name(scene_index), 64);
    g_state.value_input_target = VALUE_INPUT_TARGET_SCENE_NAME;
    g_state.value_input_row = scene_index;
    g_state.show_value_input = 1;
    g_state.value_input_len = (int)lstrlenW(g_state.value_input_buf);
    g_state.value_input_caret = g_state.value_input_len;
}

static void begin_value_input_for_scene_asset_guid(int scene_index) {
    wchar_t wbuf[64];
    const char* guid;
    int wlen;
    if (scene_index < 0 || scene_index >= SCENE_COUNT) return;
    guid = g_state.scenes[scene_index].asset_ref_guid;
    if (guid == NULL || guid[0] == '\0') guid = SCENE_ASSET_GUID_NONE;
    wlen = MultiByteToWideChar(CP_UTF8, 0, guid, -1, wbuf, 64);
    if (wlen <= 0) {
        lstrcpynW(g_state.value_input_buf, L"asset://none", 64);
    } else {
        lstrcpynW(g_state.value_input_buf, wbuf, 64);
    }
    g_state.value_input_target = VALUE_INPUT_TARGET_SCENE_ASSET_GUID;
    g_state.value_input_row = scene_index;
    g_state.show_value_input = 1;
    g_state.value_input_len = (int)lstrlenW(g_state.value_input_buf);
    g_state.value_input_caret = g_state.value_input_len;
}

static void begin_value_input_for_debug_param(int row) {
    SceneConfig* cfg = &g_state.scenes[g_state.scene_index];
    g_state.value_input_target = VALUE_INPUT_TARGET_DEBUG_PARAM;
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
    if (g_state.value_input_target == VALUE_INPUT_TARGET_LOG_SEARCH) {
        lstrcpynW(g_state.log_search_buf, g_state.value_input_buf, 48);
        g_state.log_search_len = (int)lstrlenW(g_state.log_search_buf);
        g_state.log_scroll_offset = 0;
        push_console_log(L"[日志] 搜索关键字: %ls", g_state.log_search_len > 0 ? g_state.log_search_buf : L"(空)");
        return;
    }
    if (g_state.value_input_target == VALUE_INPUT_TARGET_HIERARCHY_FILTER) {
        lstrcpynW(g_state.hierarchy_filter_buf, g_state.value_input_buf, 32);
        g_state.hierarchy_filter_len = (int)lstrlenW(g_state.hierarchy_filter_buf);
        push_console_log(L"[层级] 过滤关键字: %ls", g_state.hierarchy_filter_len > 0 ? g_state.hierarchy_filter_buf : L"(空)");
        return;
    }
    if (g_state.value_input_target == VALUE_INPUT_TARGET_SCENE_NAME) {
        int row = g_state.value_input_row;
        EditorCommand command_data;
        if (row < 0 || row >= SCENE_COUNT) return;
        ZeroMemory(&command_data, sizeof(command_data));
        command_data.type = EDITOR_CMD_SCENE_RENAME;
        command_data.arg_i0 = row;
        lstrcpynW(command_data.text, g_state.value_input_buf, EDITOR_COMMAND_TEXT_CAP);
        if (!dispatch_editor_command(&command_data)) {
            push_console_log(L"[警告] 场景名称未更新");
        }
        return;
    }
    if (g_state.value_input_target == VALUE_INPUT_TARGET_SCENE_ASSET_GUID) {
        int row = g_state.value_input_row;
        EditorCommand command_data;
        if (row < 0 || row >= SCENE_COUNT) return;
        ZeroMemory(&command_data, sizeof(command_data));
        command_data.type = EDITOR_CMD_SCENE_ASSET_REF_SET;
        command_data.arg_i0 = row;
        lstrcpynW(command_data.text, g_state.value_input_buf, EDITOR_COMMAND_TEXT_CAP);
        if (!dispatch_editor_command(&command_data)) {
            push_console_log(L"[警告] 场景资产引用未更新");
        }
        return;
    }
    if (g_state.value_input_target == VALUE_INPUT_TARGET_DEBUG_PARAM ||
        g_state.value_input_target == VALUE_INPUT_TARGET_INSPECTOR) {
        double v = 0.0;
        if (!parse_double_strict(g_state.value_input_buf, &v)) {
            push_console_log(L"[警告] 输入无效，请输入规范数值");
            return;
        }
        if (g_state.value_input_target == VALUE_INPUT_TARGET_DEBUG_PARAM) {
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
        {
            EditorCommand command_data;
            ZeroMemory(&command_data, sizeof(command_data));
            command_data.type = EDITOR_CMD_INSPECTOR_SET_VALUE;
            command_data.arg_i0 = g_state.value_input_row;
            command_data.arg_i1 = 1;
            command_data.arg_f0 = v;
            if (dispatch_editor_command(&command_data)) return;
        }
        if (csel != NULL && csel->active) {
            push_console_log(L"[警告] 约束参数未更新，请检查输入范围");
        } else if (g_state.selected != NULL) {
            push_console_log(L"[警告] 物体参数未更新，请检查输入范围");
        } else {
            push_console_log(L"[警告] 当前无可编辑对象");
        }
    }
}

static const wchar_t* value_input_title(void) {
    switch (g_state.value_input_target) {
        case VALUE_INPUT_TARGET_LOG_SEARCH: return L"日志搜索关键字";
        case VALUE_INPUT_TARGET_HIERARCHY_FILTER: return L"层级过滤关键字";
        case VALUE_INPUT_TARGET_SCENE_NAME: return L"场景名称";
        case VALUE_INPUT_TARGET_SCENE_ASSET_GUID: return L"场景资产GUID";
        case VALUE_INPUT_TARGET_DEBUG_PARAM: return L"输入调试参数";
        case VALUE_INPUT_TARGET_INSPECTOR: return L"输入数值";
        default: return L"输入";
    }
}

static const wchar_t* inspector_row_hint_text(void) {
    static wchar_t hint[128];
    int row = g_state.inspector_focused_row;
    hint[0] = L'\0';
    if (selected_constraint_is_active()) {
        const Constraint* c = selected_constraint_ref();
        if (row == 0) lstrcpyW(hint, L"目标长度范围: 0 ~ 500 (单位: m)");
        if (row == 1) {
            if (c != NULL && c->type == CONSTRAINT_DISTANCE) lstrcpyW(hint, L"距离刚度范围: 0.08 ~ 1.0");
            else lstrcpyW(hint, L"弹簧刚度范围: 0.20 ~ 40.0 (单位: N/m)");
        }
        if (row == 2) lstrcpyW(hint, L"阻尼范围: 0.0 ~ 20.0 (单位: Ns/m)");
        if (row == 3) lstrcpyW(hint, L"断裂阈值范围: 0 ~ 100000 (单位: N)");
        if (row == 4) lstrcpyW(hint, L"连体碰撞: 0=禁止 1=允许");
        if (row == 5 && c != NULL && c->type == CONSTRAINT_SPRING) lstrcpyW(hint, L"弹性预设: 0=软 1=中 2=高");
    } else if (g_state.selected != NULL) {
        if (row == 0) lstrcpyW(hint, L"质量范围: 0.1 ~ 1000 (单位: kg)");
        if (row == 1 || row == 2) lstrcpyW(hint, L"位置范围: -5000 ~ 5000 (单位: m)");
        if (row == 3 || row == 4) lstrcpyW(hint, L"速度范围: -500 ~ 500 (单位: m/s)");
        if (row == 5) lstrcpyW(hint, L"角速度范围: -200 ~ 200 (单位: rad/s)");
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
    g_dbg_runtime_tick_row_count = 0;
    g_dbg_runtime_state_row_count = 0;
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
    for (i = 0; i < INSPECTOR_MAX_ROWS; i++) {
        g_ins_row_rect[i] = rc(0, 0, 0, 0);
        g_ins_minus_rect[i] = rc(0, 0, 0, 0);
        g_ins_plus_rect[i] = rc(0, 0, 0, 0);
    }
    for (i = 0; i < 3; i++) {
        g_dbg_row_rect[i] = rc(0, 0, 0, 0);
        g_dbg_minus_rect[i] = rc(0, 0, 0, 0);
        g_dbg_plus_rect[i] = rc(0, 0, 0, 0);
        g_dbg_runtime_tick_row_rect[i] = rc(0, 0, 0, 0);
        g_dbg_runtime_tick_row_entry_index[i] = -1;
        g_dbg_runtime_state_row_rect[i] = rc(0, 0, 0, 0);
        g_dbg_runtime_state_row_entry_index[i] = -1;
    }
}

static D2D1_RECT_F render_right_inspector_section(D2D1_RECT_F right_rect) {
    D2D1_RECT_F inspector;
    D2D1_RECT_F debug_rect;
    float inspector_content_h;
    float inspector_view_h;
    InspectorPanelLayout panel_layout{};
    InspectorRendererState renderer_state{};
    InspectorRendererContext renderer_context{};
    InspectorRendererOutput renderer_output{};

    inspector_panel_layout_build(right_rect, &panel_layout);
    inspector = panel_layout.inspector_rect;
    debug_rect = panel_layout.debug_rect;
    g_inspector_viewport_rect = panel_layout.inspector_viewport_rect;
    inspector_view_h = g_inspector_viewport_rect.bottom - g_inspector_viewport_rect.top;
    g_ins_row_count = inspector_row_count();
    if (g_state.inspector_focused_row >= g_ins_row_count) g_state.inspector_focused_row = g_ins_row_count - 1;
    if (g_state.inspector_focused_row < 0) g_state.inspector_focused_row = 0;
    inspector_content_h = (g_ins_row_count == 0) ? 28.0f : (g_ins_row_count * 32.0f + 44.0f);
    g_state.inspector_scroll_max = 0;
    if (inspector_content_h > inspector_view_h) {
        g_state.inspector_scroll_max = (int)(inspector_content_h - inspector_view_h + 0.5f);
    }
    if (g_state.inspector_scroll_offset < 0) g_state.inspector_scroll_offset = 0;
    if (g_state.inspector_scroll_offset > g_state.inspector_scroll_max) g_state.inspector_scroll_offset = g_state.inspector_scroll_max;
    renderer_state.focused_row = g_state.inspector_focused_row;
    renderer_state.scroll_offset = g_state.inspector_scroll_offset;
    renderer_state.row_count = g_ins_row_count;
    renderer_state.selected_body = g_state.selected;
    renderer_state.selected_constraint = selected_constraint_ref();
    renderer_context.target = g_ui.target;
    renderer_context.brush = g_ui.brush;
    renderer_context.fmt_ui = g_ui.fmt_ui;
    renderer_context.fmt_info = g_ui.fmt_info;
    renderer_context.inspector_rect = inspector;
    renderer_context.viewport_rect = g_inspector_viewport_rect;
    renderer_context.mouse_point = pt(g_state.mouse_screen.x, g_state.mouse_screen.y);
    renderer_context.state = &renderer_state;
    renderer_context.spring_preset_for_constraint = spring_preset_for_constraint;
    renderer_context.inspector_row_hint_text = inspector_row_hint_text;
    renderer_context.draw_panel_header_band = inspector_draw_panel_header_band_adapter;
    renderer_context.draw_card_round = inspector_draw_card_round_adapter;
    renderer_context.draw_text = inspector_draw_text_adapter;
    renderer_context.draw_text_vcenter = inspector_draw_text_vcenter_adapter;
    inspector_renderer_render(&renderer_context, &renderer_output);
    g_inspector_viewport_rect = renderer_output.viewport_rect;
    g_state.inspector_scroll_max = renderer_output.scroll_max;
    memcpy(g_ins_row_rect, renderer_output.row_rects, sizeof(renderer_output.row_rects));
    memcpy(g_ins_minus_rect, renderer_output.minus_rects, sizeof(renderer_output.minus_rects));
    memcpy(g_ins_plus_rect, renderer_output.plus_rects, sizeof(renderer_output.plus_rects));
    if (g_state.inspector_scroll_max > 0) {
        InspectorScrollbarModel scrollbar_model{};
        InspectorScrollbarResult scrollbar_result{};
        scrollbar_model.host_rect = inspector;
        scrollbar_model.viewport_rect = g_inspector_viewport_rect;
        scrollbar_model.viewport_height = inspector_view_h;
        scrollbar_model.content_height = inspector_content_h;
        scrollbar_model.scroll_max = g_state.inspector_scroll_max;
        scrollbar_model.scroll_offset = g_state.inspector_scroll_offset;
        scrollbar_model.mouse_x = g_state.mouse_screen.x;
        scrollbar_model.mouse_y = g_state.mouse_screen.y;
        inspector_scrollbar_render(g_ui.target, g_ui.brush, &g_ui_theme, &scrollbar_model, &scrollbar_result);
        g_inspector_scroll_track_rect = scrollbar_result.track_rect;
        g_inspector_scroll_thumb_rect = scrollbar_result.thumb_rect;
    } else {
        g_inspector_scroll_track_rect = rc(0, 0, 0, 0);
        g_inspector_scroll_thumb_rect = rc(0, 0, 0, 0);
    }
    return debug_rect;
}

static void render_right_debug_section(D2D1_RECT_F debug_rect) {
    float debug_view_h;
    InspectorDebugRendererModel model{};
    InspectorDebugRendererCallbacks callbacks{};
    InspectorDebugRendererOutput output{};
    int i;
    model.focused_row = g_state.debug_focused_row;
    model.scroll_offset = g_state.debug_scroll_offset;
    model.mouse_x = g_state.mouse_screen.x;
    model.mouse_y = g_state.mouse_screen.y;
    model.gravity_y = g_state.engine ? physics_engine_get_gravity(g_state.engine).y : 0.0f;
    model.time_step = g_state.engine ? physics_engine_get_time_step(g_state.engine) : 0.0f;
    model.iterations = g_state.engine ? physics_engine_get_iterations(g_state.engine) : 0;
    model.runtime_frame_index = g_state.runtime_frame_index;
    model.runtime_running = g_state.runtime_running;
    model.runtime_body_count = g_state.runtime_body_count;
    model.runtime_constraint_count = g_state.runtime_constraint_count;
    model.runtime_contact_count = g_state.runtime_contact_count;
    model.physics_step_ms = g_state.physics_step_ms;
    model.runtime_state_change_count = g_state.runtime_state_change_count;
    model.runtime_error_count = g_state.runtime_error_count;
    model.runtime_error_item_count = g_state.runtime_error_item_count;
    model.runtime_error_code = g_state.runtime_error_code;
    model.runtime_event_drop_count = g_state.runtime_event_drop_count;
    for (i = 0; i < 4; i++) {
        model.runtime_errors[i].code = g_state.runtime_errors[i].code;
        model.runtime_errors[i].severity = g_state.runtime_errors[i].severity;
        model.runtime_errors[i].count = g_state.runtime_errors[i].count;
    }
    for (i = 0; i < INSPECTOR_DEBUG_HISTORY_CAP; i++) {
        model.runtime_tick_history[i].frame_index = g_state.runtime_tick_history[i].frame_index;
        model.runtime_tick_history[i].body_count = g_state.runtime_tick_history[i].body_count;
        model.runtime_tick_history[i].constraint_count = g_state.runtime_tick_history[i].constraint_count;
        model.runtime_tick_history[i].contact_count = g_state.runtime_tick_history[i].contact_count;
        model.runtime_tick_history[i].step_ms = g_state.runtime_tick_history[i].step_ms;
        model.runtime_state_history[i].frame_index = g_state.runtime_state_history[i].frame_index;
        model.runtime_state_history[i].tick_ms = g_state.runtime_state_history[i].tick_ms;
        model.runtime_state_history[i].running = g_state.runtime_state_history[i].running;
        model.debug_event_history[i].tick_ms = g_debug_event_history[i].tick_ms;
        lstrcpynW(model.debug_event_history[i].text, g_debug_event_history[i].text, 160);
        model.debug_error_history[i].tick_ms = g_debug_error_history[i].tick_ms;
        lstrcpynW(model.debug_error_history[i].text, g_debug_error_history[i].text, 160);
        model.hot_reload_history[i].tick_ms = g_hot_reload_history[i].tick_ms;
        model.hot_reload_history[i].pie_active = g_hot_reload_history[i].pie_active;
        model.hot_reload_history[i].ready_batch_count = g_hot_reload_history[i].ready_batch_count;
        model.hot_reload_history[i].affected_count = g_hot_reload_history[i].affected_count;
        model.hot_reload_history[i].imported_count = g_hot_reload_history[i].imported_count;
        model.hot_reload_history[i].failed_count = g_hot_reload_history[i].failed_count;
        model.hot_reload_history[i].rollback_retained = g_hot_reload_history[i].rollback_retained;
        model.hot_reload_history[i].imported_guid_count = g_hot_reload_history[i].imported_guid_count;
        memcpy(model.hot_reload_history[i].imported_guids, g_hot_reload_history[i].imported_guids, sizeof(model.hot_reload_history[i].imported_guids));
    }
    model.runtime_tick_history_head = g_state.runtime_tick_history_head;
    model.runtime_tick_history_count = g_state.runtime_tick_history_count;
    model.runtime_state_history_head = g_state.runtime_state_history_head;
    model.runtime_state_history_count = g_state.runtime_state_history_count;
    model.perf_hist_count = g_state.perf_hist_count;
    model.perf_hist_head = g_state.perf_hist_head;
    model.fps_hist = g_state.fps_hist;
    model.step_hist = g_state.step_hist;
    model.pie_active = pie_runtime_active();
    model.hot_reload_total_imported = g_state.hot_reload_total_imported;
    model.hot_reload_total_failed = g_state.hot_reload_total_failed;
    model.hot_reload_watch_count = g_state.hot_reload_watch_count;
    model.hot_reload_scan_change_count = g_state.hot_reload_scan_change_count;
    model.hot_reload_ready_batch_count = g_state.hot_reload_ready_batch_count;
    model.debug_event_history_head = g_debug_event_history_head;
    model.debug_event_history_count = g_debug_event_history_count;
    model.debug_error_history_head = g_debug_error_history_head;
    model.debug_error_history_count = g_debug_error_history_count;
    model.hot_reload_history_head = g_hot_reload_history_head;
    model.hot_reload_history_count = g_hot_reload_history_count;
    callbacks.draw_card_round = draw_card_round;
    callbacks.draw_text = draw_text;
    callbacks.draw_text_vcenter = draw_text_vcenter;
    callbacks.draw_panel_header_band = draw_panel_header_band;
    callbacks.runtime_error_label = runtime_error_label;
    callbacks.runtime_error_severity_label = runtime_error_severity_label;
    inspector_debug_renderer_render(g_ui.target, g_ui.fmt_ui, g_ui.fmt_info, debug_rect, &model, &callbacks, &output);
    g_debug_viewport_rect = output.viewport_rect;
    memcpy(g_dbg_row_rect, output.param_row_rect, sizeof(output.param_row_rect));
    memcpy(g_dbg_minus_rect, output.param_minus_rect, sizeof(output.param_minus_rect));
    memcpy(g_dbg_plus_rect, output.param_plus_rect, sizeof(output.param_plus_rect));
    memcpy(g_dbg_runtime_tick_row_rect, output.runtime_tick_row_rect, sizeof(output.runtime_tick_row_rect));
    memcpy(g_dbg_runtime_tick_row_entry_index, output.runtime_tick_row_entry_index, sizeof(output.runtime_tick_row_entry_index));
    g_dbg_runtime_tick_row_count = output.runtime_tick_row_count;
    memcpy(g_dbg_runtime_state_row_rect, output.runtime_state_row_rect, sizeof(output.runtime_state_row_rect));
    memcpy(g_dbg_runtime_state_row_entry_index, output.runtime_state_row_entry_index, sizeof(output.runtime_state_row_entry_index));
    g_dbg_runtime_state_row_count = output.runtime_state_row_count;
    g_state.debug_scroll_max = output.scroll_max;
    if (g_state.debug_scroll_offset < 0) g_state.debug_scroll_offset = 0;
    if (g_state.debug_scroll_offset > g_state.debug_scroll_max) g_state.debug_scroll_offset = g_state.debug_scroll_max;
    debug_view_h = g_debug_viewport_rect.bottom - g_debug_viewport_rect.top;
    if (g_state.debug_scroll_max > 0) {
        InspectorScrollbarModel scrollbar_model{};
        InspectorScrollbarResult scrollbar_result{};
        scrollbar_model.host_rect = debug_rect;
        scrollbar_model.viewport_rect = g_debug_viewport_rect;
        scrollbar_model.viewport_height = debug_view_h;
        scrollbar_model.content_height = 1140.0f;
        scrollbar_model.scroll_max = g_state.debug_scroll_max;
        scrollbar_model.scroll_offset = g_state.debug_scroll_offset;
        scrollbar_model.mouse_x = g_state.mouse_screen.x;
        scrollbar_model.mouse_y = g_state.mouse_screen.y;
        inspector_scrollbar_render(g_ui.target, g_ui.brush, &g_ui_theme, &scrollbar_model, &scrollbar_result);
        g_debug_scroll_track_rect = scrollbar_result.track_rect;
        g_debug_scroll_thumb_rect = scrollbar_result.thumb_rect;
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
    SideFoldButtonsModel model{};
    SideFoldButtonsCallbacks callbacks{};
    SideFoldButtonsOutput output{};
    model.show_left_panel = g_state.ui_show_left_panel;
    model.show_right_panel = g_state.ui_show_right_panel;
    model.mouse_x = g_state.mouse_screen.x;
    model.mouse_y = g_state.mouse_screen.y;
    callbacks.draw_card_round = draw_card_round;
    callbacks.draw_icon_chevron_lr = ui_draw_icon_chevron_lr;
    side_fold_buttons_render(g_ui.target, g_ui.brush, top_rect, work_bottom, left_rect, center_rect, right_rect, &model, &callbacks, &output);
    g_left_fold_rect = output.left_fold_rect;
    g_right_fold_rect = output.right_fold_rect;
}

static void render_bottom_panel_tabs(D2D1_RECT_F bottom_rect) {
    ConsoleTabsModel model{};
    ConsoleTabsCallbacks callbacks{};
    ConsoleTabsOutput output{};
    if (g_state.ui_show_bottom_panel) {
        model.show_bottom_panel = g_state.ui_show_bottom_panel;
        model.bottom_panel_collapsed = g_state.bottom_panel_collapsed;
        model.bottom_active_tab = g_state.bottom_active_tab;
        model.mouse_x = g_state.mouse_screen.x;
        model.mouse_y = g_state.mouse_screen.y;
        callbacks.draw_text_hvcenter = draw_text_hvcenter;
        callbacks.draw_card_round = draw_card_round;
        callbacks.draw_icon_chevron = ui_draw_icon_chevron;
        console_tabs_render(g_ui.target, g_ui.brush, g_ui.fmt_info, bottom_rect, &model, &callbacks, &output);
        g_bottom_fold_rect = output.bottom_fold_rect;
        g_bottom_tab_console_rect = output.tab_console_rect;
        g_bottom_tab_perf_rect = output.tab_perf_rect;
    } else {
        g_bottom_fold_rect = rc(0, 0, 0, 0);
        g_bottom_tab_console_rect = rc(0, 0, 0, 0);
        g_bottom_tab_perf_rect = rc(0, 0, 0, 0);
        g_log_viewport_rect = rc(0, 0, 0, 0);
        g_log_scroll_track_rect = rc(0, 0, 0, 0);
        g_log_scroll_thumb_rect = rc(0, 0, 0, 0);
        g_dbg_collision_filter_rect = rc(0, 0, 0, 0);
        g_perf_diag_export_rect = rc(0, 0, 0, 0);
        g_dbg_collision_row_count = 0;
        g_log_scroll_max = 0;
    }
}

static void render_bottom_perf_tab(D2D1_RECT_F bottom_rect) {
    ConsolePerfModel model{};
    ConsolePerfCallbacks callbacks{};
    ConsolePerfOutput output{};
    g_log_viewport_rect = rc(0, 0, 0, 0);
    g_log_scroll_track_rect = rc(0, 0, 0, 0);
    g_log_scroll_thumb_rect = rc(0, 0, 0, 0);
    g_dbg_collision_filter_rect = rc(0, 0, 0, 0);
    g_dbg_collision_row_count = 0;
    g_log_scroll_max = 0;
    model.fps_display = g_state.fps_display;
    model.physics_step_ms = g_state.physics_step_ms;
    model.broadphase_use_grid = g_state.engine && physics_engine_get_broadphase_use_grid(g_state.engine);
    model.perf_hist_count = g_state.perf_hist_count;
    model.perf_hist_head = g_state.perf_hist_head;
    model.fps_hist = g_state.fps_hist;
    model.step_hist = g_state.step_hist;
    model.mouse_x = g_state.mouse_screen.x;
    model.mouse_y = g_state.mouse_screen.y;
    callbacks.draw_action_button = draw_action_button;
    callbacks.draw_card_round = draw_card_round;
    callbacks.draw_text = draw_text;
    callbacks.set_brush_color = set_brush_color;
    console_perf_render(g_ui.target, g_ui.brush, g_ui.fmt_info, bottom_rect, &model, &callbacks, &output);
    g_perf_diag_export_rect = output.perf_diag_export_rect;
    g_perf_export_rect = output.perf_export_rect;
}

static void render_bottom_console_tab(D2D1_RECT_F bottom_rect) {
    ConsoleRendererModel model{};
    ConsoleRendererCallbacks callbacks{};
    ConsoleRendererOutput output{};
    g_state.log_search_buf[0] = L'\0';
    g_state.log_search_len = 0;
    model.log_filter_mode = g_state.log_filter_mode;
    model.log_scroll_offset = g_state.log_scroll_offset;
    model.collision_event_filter_selected_only = g_collision_event_filter_selected_only;
    model.log_search_len = g_state.log_search_len;
    model.log_search_buf = g_state.log_search_buf;
    model.console_log_count = g_console_log_count;
    model.console_log_head = g_console_log_head;
    model.console_logs = g_console_logs;
    model.collision_event_count = g_collision_event_count;
    model.collision_event_head = g_collision_event_head;
    model.mouse_x = g_state.mouse_screen.x;
    model.mouse_y = g_state.mouse_screen.y;
    callbacks.draw_text_tab_button = draw_text_tab_button;
    callbacks.draw_action_button = draw_action_button;
    callbacks.draw_text = draw_text;
    callbacks.draw_card_round = draw_card_round;
    callbacks.log_match_filter = log_match_filter;
    callbacks.collision_event_involves_selected = console_collision_event_involves_selected_adapter;
    callbacks.format_collision_event_line = console_format_collision_event_line_adapter;
    console_renderer_render(g_ui.target, g_ui.fmt_info, bottom_rect, &model, &callbacks, &output);
    g_log_search_clear_rect = output.log_search_clear_rect;
    g_log_search_rect = output.log_search_rect;
    g_log_clear_rect = output.log_clear_rect;
    g_log_filter_warn_rect = output.log_filter_warn_rect;
    g_log_filter_collision_rect = output.log_filter_collision_rect;
    g_log_filter_physics_rect = output.log_filter_physics_rect;
    g_log_filter_state_rect = output.log_filter_state_rect;
    g_log_filter_all_rect = output.log_filter_all_rect;
    g_dbg_collision_filter_rect = output.dbg_collision_filter_rect;
    g_log_viewport_rect = output.log_viewport_rect;
    g_log_scroll_track_rect = output.log_scroll_track_rect;
    g_log_scroll_thumb_rect = output.log_scroll_thumb_rect;
    g_dbg_collision_row_count = output.dbg_collision_row_count;
    memcpy(g_dbg_collision_row_rect, output.dbg_collision_row_rect, sizeof(output.dbg_collision_row_rect));
    memcpy(g_dbg_collision_row_event_index, output.dbg_collision_row_event_index, sizeof(output.dbg_collision_row_event_index));
    g_log_scroll_max = output.log_scroll_max;
    if (g_state.log_scroll_offset < 0) g_state.log_scroll_offset = 0;
    if (g_state.log_scroll_offset > g_log_scroll_max) g_state.log_scroll_offset = g_log_scroll_max;
}

static void render_bottom_panel_content(D2D1_RECT_F bottom_rect) {
    ConsolePanelShellCallbacks callbacks{};
    callbacks.render_console_tab = render_bottom_console_tab;
    callbacks.render_perf_tab = render_bottom_perf_tab;
    console_panel_shell_render(
        bottom_rect,
        g_state.ui_show_bottom_panel,
        g_state.bottom_panel_collapsed,
        g_state.bottom_active_tab,
        &callbacks);
}

static void clear_left_panel_ui_state(void) {
    int i;
    g_explorer_body_count = 0;
    g_explorer_constraint_count = 0;
    g_explorer_scene_count = 0;
    for (i = 0; i < EXPLORER_SCENE_MAX_ITEMS; i++) {
        g_explorer_scene_rect[i] = rc(0, 0, 0, 0);
        g_explorer_scene_index[i] = -1;
    }
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
    g_explorer_scene_count = 0;
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
    int scene_rows = 0;
    int i;
    int scene_index;
    wchar_t line[128];
    for (i = 0; i < SCENE_COUNT; i++) {
        scene_index = scene_order_index_at(i);
        swprintf(line, 128, L"#%d %ls", scene_index + 1, scene_display_name(scene_index));
        if (g_state.hierarchy_filter_len > 0 && wcsstr(line, g_state.hierarchy_filter_buf) == NULL) continue;
        scene_rows++;
    }
    content_h += row_h;
    if (g_state.tree_scene_expanded) {
        if (scene_rows > 0) {
            content_h += scene_rows * (row_h + 4.0f) + 4.0f;
        } else {
            content_h += row_h + 4.0f;
        }
    }
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
    int plugin_initialized = 0;
    int plugin_disabled = 0;
    int plugin_failed = 0;
    float py;
    float project_content_h;
    float project_view_h;
    wchar_t line[128];
    wchar_t wide_name[96];
    wchar_t wide_project[96];
    D2D1_RECT_F project_viewport = rc(project_rect.left + 10.0f, project_rect.top + 44.0f, project_rect.right - 18.0f, project_rect.bottom - 112.0f);
    unsigned int now_ms = (unsigned int)GetTickCount();
    for (pi = 0; pi < g_editor_plugin_registry.plugin_count; ++pi) {
        if (g_editor_plugin_registry.plugins[pi].state == EDITOR_PLUGIN_STATE_INITIALIZED) plugin_initialized++;
        else if (g_editor_plugin_registry.plugins[pi].state == EDITOR_PLUGIN_STATE_FAILED) plugin_failed++;
        else if (g_editor_plugin_registry.plugins[pi].state == EDITOR_PLUGIN_STATE_DISABLED) plugin_disabled++;
    }
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
    utf8_to_wide_copy(g_workspace_doc.workspace_name, wide_name, 96);
    utf8_to_wide_copy(g_project_doc.project_name, wide_project, 96);
    swprintf(line, 128, L"目录:%d  文件:%d  工作区:%ls", project_tree_dir_count(), project_tree_file_count(), wide_name[0] ? wide_name : L"-");
    draw_text(line, rc(project_rect.left + 12.0f, project_rect.bottom - 96.0f, project_rect.right - 12.0f, project_rect.bottom - 76.0f),
              g_ui.fmt_info, rgba(0.66f, 0.73f, 0.83f, 1.0f));
    swprintf(line, 128, L"项目:%ls  包:%d  自动热重载:%ls", wide_project[0] ? wide_project : L"-", g_project_doc.package_count,
             g_project_settings_doc.auto_reload_assets ? L"开" : L"关");
    draw_text(line, rc(project_rect.left + 12.0f, project_rect.bottom - 74.0f, project_rect.right - 12.0f, project_rect.bottom - 54.0f),
              g_ui.fmt_info, rgba(0.70f, 0.78f, 0.90f, 1.0f));
    swprintf(line, 128, L"插件: 总%d  初始化%d  禁用%d  失败%d", g_editor_plugin_registry.plugin_count, plugin_initialized, plugin_disabled, plugin_failed);
    draw_text(line, rc(project_rect.left + 12.0f, project_rect.bottom - 52.0f, project_rect.right - 12.0f, project_rect.bottom - 32.0f),
              g_ui.fmt_info, rgba(0.78f, 0.86f, 0.96f, 1.0f));
    if (g_prefab_analysis_valid) {
        swprintf(line, 128, L"Prefab: 应用%d  冲突%d  悬挂%d  修复%d", g_prefab_analysis.applied_count, g_prefab_analysis.conflict_count,
                 g_prefab_analysis.dangling_count, g_prefab_repair_removed_count);
    } else {
        swprintf(line, 128, L"Prefab: 示例语义数据未就绪");
    }
    draw_text(line, rc(project_rect.left + 12.0f, project_rect.bottom - 30.0f, project_rect.right - 12.0f, project_rect.bottom - 8.0f),
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
            swprintf(line, 128, L"#%d %ls/%ls", s.body_visible_count + 1, body_kind_name(b), body_shape_name(b));
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
            swprintf(line, 128, L"#%d %ls", s.constraint_visible_count + 1, ctype);
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
    TopToolbarLayout layout{};
    top_toolbar_layout_build(toolbar_rect, 12.0f + measure_text_width(L"中", g_ui.fmt_ui), &layout);
    g_top_run_rect = layout.run_rect;
    g_top_step_rect = layout.step_rect;
    g_top_reset_rect = layout.reset_rect;
    g_top_save_rect = layout.save_rect;
    g_top_undo_rect = layout.undo_rect;
    g_top_redo_rect = layout.redo_rect;
    g_top_grid_rect = layout.grid_rect;
    g_top_collision_rect = layout.collision_rect;
    g_top_velocity_rect = layout.velocity_rect;
    g_top_constraint_rect = layout.constraint_rect;
    g_top_spring_rect = layout.spring_rect;
    g_top_chain_rect = layout.chain_rect;
    g_top_rope_rect = layout.rope_rect;
}

static void render_top_toolbar_buttons(void) {
    TopToolbarLayout layout{};
    TopToolbarRenderState state{};
    layout.run_rect = g_top_run_rect;
    layout.step_rect = g_top_step_rect;
    layout.reset_rect = g_top_reset_rect;
    layout.save_rect = g_top_save_rect;
    layout.undo_rect = g_top_undo_rect;
    layout.redo_rect = g_top_redo_rect;
    layout.grid_rect = g_top_grid_rect;
    layout.collision_rect = g_top_collision_rect;
    layout.velocity_rect = g_top_velocity_rect;
    layout.constraint_rect = g_top_constraint_rect;
    layout.spring_rect = g_top_spring_rect;
    layout.chain_rect = g_top_chain_rect;
    layout.rope_rect = g_top_rope_rect;
    state.running = g_state.running;
    state.runtime_bus_congested = g_state.runtime_bus_congested;
    state.draw_centers = g_state.draw_centers;
    state.draw_contacts = g_state.draw_contacts;
    state.draw_velocity = g_state.draw_velocity;
    state.constraint_create_mode = g_state.constraint_create_mode;
    state.mouse_x = g_state.mouse_screen.x;
    state.mouse_y = g_state.mouse_screen.y;
    top_toolbar_render(g_ui.target, g_ui.brush, &layout, &state);
}

static void render_hierarchy_scrollbar(D2D1_RECT_F hierarchy_rect, D2D1_RECT_F hierarchy_viewport, float viewport_h, float content_h) {
    HierarchyScrollbarModel model{};
    HierarchyScrollbarRenderResult result{};
    model.host_rect = hierarchy_rect;
    model.viewport_rect = hierarchy_viewport;
    model.viewport_height = viewport_h;
    model.content_height = content_h;
    model.scroll_max = g_state.hierarchy_scroll_max;
    model.scroll_offset = g_state.hierarchy_scroll_offset;
    model.dragging = g_state.hierarchy_scroll_dragging;
    model.mouse_x = g_state.mouse_screen.x;
    model.mouse_y = g_state.mouse_screen.y;
    hierarchy_scrollbar_render(g_ui.target, g_ui.brush, &g_ui_theme, &model, &result);
    g_hierarchy_scroll_track_rect = result.track_rect;
    g_hierarchy_scroll_thumb_rect = result.thumb_rect;
}

static void hierarchy_draw_text_adapter(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    draw_text(text, rect, fmt, color);
}

static void hierarchy_draw_tree_disclosure_icon_adapter(D2D1_RECT_F rect, int expanded, D2D1_COLOR_F color, float stroke) {
    ui_draw_tree_disclosure_icon(g_ui.target, g_ui.brush, rect, expanded, color, stroke);
}

static void inspector_draw_card_round_adapter(D2D1_RECT_F rect, float radius, D2D1_COLOR_F fill, D2D1_COLOR_F border) {
    draw_card_round(rect, radius, fill, border);
}

static void inspector_draw_text_adapter(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    draw_text(text, rect, fmt, color);
}

static void inspector_draw_text_vcenter_adapter(const wchar_t* text, D2D1_RECT_F rect, IDWriteTextFormat* fmt, D2D1_COLOR_F color) {
    draw_text_vcenter(text, rect, fmt, color);
}

static void inspector_draw_panel_header_band_adapter(D2D1_RECT_F panel_rect, float top_h, float inset) {
    draw_panel_header_band(panel_rect, top_h, inset);
}

static void render_left_hierarchy_content(D2D1_RECT_F left_rect) {
    HierarchyPanelLayout layout{};
    HierarchyRendererContext renderer_context{};
    HierarchyRendererState renderer_state{};
    HierarchyRendererStats renderer_stats{};
    HierarchyRendererOutput renderer_output{};
    float row_h;
    float y;
    D2D1_RECT_F hierarchy_rect;
    D2D1_RECT_F project_rect;
    D2D1_RECT_F hierarchy_viewport;
    float viewport_h = 0.0f;
    float content_h;
    hierarchy_panel_layout_build(left_rect, &layout);
    row_h = layout.row_height;
    hierarchy_rect = layout.hierarchy_rect;
    project_rect = layout.project_rect;
    hierarchy_viewport = layout.hierarchy_viewport_rect;
    viewport_h = hierarchy_viewport.bottom - hierarchy_viewport.top;
    y = left_rect.top + 46.0f;
    init_left_panel_headers_and_search(hierarchy_rect, project_rect, hierarchy_viewport);
    {
        HierarchyStats hs = compute_hierarchy_stats();
        renderer_stats.body_visible_count = hs.body_visible_count;
        renderer_stats.constraint_visible_count = hs.constraint_visible_count;
        renderer_stats.body_circle_visible_count = hs.body_circle_visible_count;
        renderer_stats.body_polygon_visible_count = hs.body_polygon_visible_count;
        renderer_stats.constraint_distance_visible_count = hs.constraint_distance_visible_count;
        renderer_stats.constraint_spring_visible_count = hs.constraint_spring_visible_count;
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

    g_explorer_scene_count = 0;
    g_explorer_body_count = 0;
    g_explorer_constraint_count = 0;
    renderer_state.tree_scene_expanded = g_state.tree_scene_expanded;
    renderer_state.tree_bodies_expanded = g_state.tree_bodies_expanded;
    renderer_state.tree_constraints_expanded = g_state.tree_constraints_expanded;
    renderer_state.tree_body_circle_expanded = g_state.tree_body_circle_expanded;
    renderer_state.tree_body_polygon_expanded = g_state.tree_body_polygon_expanded;
    renderer_state.tree_constraint_distance_expanded = g_state.tree_constraint_distance_expanded;
    renderer_state.tree_constraint_spring_expanded = g_state.tree_constraint_spring_expanded;
    renderer_state.hierarchy_filter_len = g_state.hierarchy_filter_len;
    renderer_state.hierarchy_filter_buf = g_state.hierarchy_filter_buf;
    renderer_state.current_scene_index = g_state.scene_index;
    renderer_state.selected_body = g_state.selected;
    renderer_state.selected_constraint_index = g_state.selected_constraint_index;
    renderer_context.target = g_ui.target;
    renderer_context.brush = g_ui.brush;
    renderer_context.fmt_mono = g_ui.fmt_mono;
    renderer_context.fmt_info = g_ui.fmt_info;
    renderer_context.viewport_rect = g_hierarchy_viewport_rect;
    renderer_context.state = &renderer_state;
    renderer_context.stats = &renderer_stats;
    renderer_context.engine = g_state.engine;
    renderer_context.scene_order_index_at = scene_order_index_at;
    renderer_context.scene_display_name = scene_display_name;
    renderer_context.body_kind_name = body_kind_name;
    renderer_context.body_shape_name = body_shape_name;
    renderer_context.draw_text = hierarchy_draw_text_adapter;
    renderer_context.draw_tree_disclosure_icon = hierarchy_draw_tree_disclosure_icon_adapter;

    y = hierarchy_renderer_render_sections(&renderer_context, hierarchy_rect, y, row_h, &renderer_output);
    g_tree_scene_header_rect = renderer_output.header_rects.scene_header_rect;
    g_tree_bodies_header_rect = renderer_output.header_rects.bodies_header_rect;
    g_tree_constraints_header_rect = renderer_output.header_rects.constraints_header_rect;
    g_tree_body_circle_header_rect = renderer_output.header_rects.body_circle_header_rect;
    g_tree_body_polygon_header_rect = renderer_output.header_rects.body_polygon_header_rect;
    g_tree_constraint_distance_header_rect = renderer_output.header_rects.constraint_distance_header_rect;
    g_tree_constraint_spring_header_rect = renderer_output.header_rects.constraint_spring_header_rect;
    g_explorer_scene_count = renderer_output.scene_count;
    if (renderer_output.scene_count > 0) {
        memcpy(g_explorer_scene_rect, renderer_output.scene_rects, sizeof(D2D1_RECT_F) * (size_t)renderer_output.scene_count);
        memcpy(g_explorer_scene_index, renderer_output.scene_indexes, sizeof(int) * (size_t)renderer_output.scene_count);
    }
    g_explorer_body_count = renderer_output.body_count;
    if (renderer_output.body_count > 0) {
        memcpy(g_explorer_body_rect, renderer_output.body_rects, sizeof(D2D1_RECT_F) * (size_t)renderer_output.body_count);
        memcpy(g_explorer_body_ptr, renderer_output.body_ptrs, sizeof(RigidBody*) * (size_t)renderer_output.body_count);
    }
    g_explorer_constraint_count = renderer_output.constraint_count;
    if (renderer_output.constraint_count > 0) {
        memcpy(g_explorer_constraint_rect, renderer_output.constraint_rects, sizeof(D2D1_RECT_F) * (size_t)renderer_output.constraint_count);
        memcpy(g_explorer_constraint_index, renderer_output.constraint_indexes, sizeof(int) * (size_t)renderer_output.constraint_count);
    }
    ID2D1HwndRenderTarget_PopAxisAlignedClip(g_ui.target);

    render_hierarchy_scrollbar(hierarchy_rect, hierarchy_viewport, viewport_h, content_h);
    render_project_tree_panel(project_rect, row_h);
}

static void render_top_bar_background(D2D1_RECT_F menu_rect, D2D1_RECT_F toolbar_rect, float width) {
    D2D1_RECT_F menu_sep = rc(0.0f, menu_rect.bottom + 1.0f, width, menu_rect.bottom + 2.5f);
    PanelStyle toolbar_panel;
    toolbar_panel.fill = g_ui_theme.components.toolbar.fill;
    toolbar_panel.border = g_ui_theme.components.toolbar.border;
    toolbar_panel.radius = g_ui_theme.components.toolbar.radius;
    draw_card_round(menu_sep, 0.0f,
                    g_ui_theme.colors.toolbar_separator,
                    g_ui_theme.colors.toolbar_separator);
    ui_panel_draw_with_shadow(g_ui.target, g_ui.brush, toolbar_rect, &toolbar_panel);
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
    PanelStyle status_panel;
    if (g_state.ui_show_left_panel) draw_outer_shadow_rr(left_rr);
    draw_outer_shadow_rr(center_rr);
    if (g_state.ui_show_right_panel) draw_outer_shadow_rr(right_rr);
    if (g_state.ui_show_bottom_panel) draw_outer_shadow_rr(bottom_rr);
    draw_outer_shadow_rr(status_rr);

    if (g_state.ui_show_left_panel) {
        ui_panel_draw(g_ui.target, g_ui.brush, left_rect, &g_ui_theme.components.shell_panel);
    }
    ui_panel_draw(g_ui.target, g_ui.brush, center_rect, &g_ui_theme.components.shell_panel);
    if (g_state.ui_show_right_panel) {
        ui_panel_draw(g_ui.target, g_ui.brush, right_rect, &g_ui_theme.components.shell_panel);
    }
    if (g_state.ui_show_bottom_panel) {
        ui_panel_draw(g_ui.target, g_ui.brush, bottom_rect, &g_ui_theme.components.shell_panel);
    }
    status_panel.fill = g_ui_theme.components.status_bar.fill;
    status_panel.border = g_ui_theme.components.status_bar.border;
    status_panel.radius = g_ui_theme.components.status_bar.radius;
    ui_panel_draw(g_ui.target, g_ui.brush, status_rect, &status_panel);
}

static void render_stage_content(D2D1_ROUNDED_RECT stage_rr, D2D1_RECT_F stage_rect) {
    StagePanelModel model{};
    StagePanelCallbacks callbacks{};
    model.engine = g_state.engine;
    model.fill = g_ui_theme.components.stage_panel.fill;
    model.border = g_ui_theme.components.stage_panel.border;
    callbacks.draw_outer_shadow_rr = draw_outer_shadow_rr;
    callbacks.set_brush_color = set_brush_color;
    callbacks.draw_body_2d = draw_body_2d;
    callbacks.draw_velocity = draw_velocity;
    callbacks.draw_constraints_debug = draw_constraints_debug;
    callbacks.draw_contacts = draw_contacts;
    callbacks.release_unknown = release_unknown;
    stage_panel_render(g_ui.d2d_factory, g_ui.target, g_ui.brush, stage_rr, stage_rect, &model, &callbacks);
}

static void render_center_header(D2D1_RECT_F center_rect) {
    StageHeaderModel model{};
    StagePanelCallbacks callbacks{};
    model.scene_name = scene_display_name(g_state.scene_index);
    model.pie_active = pie_runtime_active();
    model.running = g_state.running;
    callbacks.draw_panel_header_band = draw_panel_header_band;
    callbacks.draw_text_vcenter = draw_text_vcenter;
    callbacks.draw_text_right_vcenter = draw_text_right_vcenter;
    stage_header_render(g_ui.fmt_ui, g_ui.fmt_info, center_rect, &model, &callbacks);
}

static void render_right_panel_content(D2D1_RECT_F right_rect) {
    InspectorPanelShellCallbacks callbacks{};
    callbacks.clear_ui_state = clear_right_panel_ui_state;
    callbacks.render_inspector_section = render_right_inspector_section;
    callbacks.render_debug_section = render_right_debug_section;
    inspector_panel_shell_render(right_rect, g_state.ui_show_right_panel, &callbacks);
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
            swprintf(measure_row, 96, L"%ls", rows[ri]);
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
    StatusPresenterModel model{};
    StatusBarRenderModel render_model{};
    StatusBarRenderResult render_result{};
    const AppRuntimeSnapshot* snapshot = app_runtime_get_last_snapshot(&g_app_runtime);
    const AppHotReloadSnapshot* hot_snapshot = app_runtime_get_last_hot_reload(&g_app_runtime);
    int hot_imported = g_state.hot_reload_imported_count;
    int hot_failed = g_state.hot_reload_failed_count;
    unsigned int now_ms = (unsigned int)GetTickCount();
    int body_count = count_dynamic_bodies(g_state.engine);
    int constraint_count = (g_state.engine != NULL) ? physics_engine_get_constraint_count(g_state.engine) : 0;
    int contact_count = (g_state.engine != NULL) ? physics_engine_get_contact_count(g_state.engine) : 0;
    const wchar_t* tip = L"";
    if (snapshot != NULL && snapshot->valid) {
        body_count = snapshot->body_count;
        constraint_count = snapshot->constraint_count;
        contact_count = snapshot->contact_count;
    }
    if (hot_snapshot != NULL && hot_snapshot->valid) {
        hot_imported = hot_snapshot->imported_count;
        hot_failed = hot_snapshot->failed_count;
    }
    model.body_count = body_count;
    model.constraint_count = constraint_count;
    model.contact_count = contact_count;
    model.recycled_count = g_state.recycled_count;
    model.status_user = g_status_user;
    model.startup_status_text = g_startup_status_text;
    model.startup_degraded = g_startup_degraded;
    model.hot_imported = hot_imported;
    model.hot_failed = hot_failed;
    model.hot_reload_last_event_ms = g_state.hot_reload_last_event_ms;
    model.now_ms = now_ms;
    model.runtime_bus_congested = g_state.runtime_bus_congested;
    model.runtime_event_drop_count = g_state.runtime_event_drop_count;
    if (g_state.runtime_bus_congested && point_in_rect(g_state.mouse_screen, runtime_bus_badge_rect())) {
        tip = L"点击红点可快速打开告警日志";
    } else if (point_in_rect(g_state.mouse_screen, g_top_run_rect)) {
        if (pie_runtime_active()) {
            tip = g_state.runtime_bus_congested ? L"PIE 运行/暂停（事件总线拥塞，点击下方面板查看告警）" : L"PIE 运行/暂停（Esc 退出）";
        } else {
            tip = L"进入 PIE 并运行";
        }
    }
    else if (point_in_rect(g_state.mouse_screen, g_top_step_rect)) {
        tip = pie_runtime_active() ? L"PIE 单步执行一帧（保持暂停）" : L"进入 PIE 并单步一帧";
    }
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

    render_model.status_rect = status_rect;
    render_model.hover_tip = tip;
    render_model.presenter_model = model;
    status_bar_render(g_ui.target, g_ui.brush, g_ui.fmt_info, &g_ui_theme, &render_model, &render_result);
    g_status_meta_rect = render_result.meta_rect;
}

static void render_help_modal_content(D2D1_RECT_F modal) {
    HelpModalContentModel model{};
    ModalContentCallbacks callbacks{};
    model.help_modal_page = g_state.help_modal_page;
    callbacks.draw_text = draw_text;
    help_modal_content_render(modal, g_ui.fmt_title, g_ui.fmt_mono, &model, &callbacks);
}

static void render_config_modal_content(D2D1_RECT_F modal) {
    ConfigModalContentModel model{};
    ConfigModalContentOutput output{};
    ModalContentCallbacks callbacks{};
    model.config = &g_state.scenes[g_state.scene_index];
    model.focused_param = g_state.focused_param;
    callbacks.draw_text = draw_text;
    callbacks.draw_card_round = draw_card_round;
    callbacks.draw_outer_shadow_rr = draw_outer_shadow_rr;
    callbacks.can_adjust_param = can_adjust_param;
    config_modal_content_render(modal, g_ui.fmt_title, g_ui.fmt_mono, &model, &callbacks, &output);
    memcpy(g_cfg_row_rect, output.row_rect, sizeof(output.row_rect));
    memcpy(g_cfg_minus_rect, output.minus_rect, sizeof(output.minus_rect));
    memcpy(g_cfg_plus_rect, output.plus_rect, sizeof(output.plus_rect));
}

static void render_modals(float w, float h) {
    ModalShellModel model{};
    ModalShellCallbacks callbacks{};
    ModalShellOutput output{};
    model.show_value_input = g_state.show_value_input;
    model.show_config_modal = g_state.show_config_modal;
    model.show_help_modal = g_state.show_help_modal;
    model.value_input_title = value_input_title();
    model.value_input_buf = g_state.value_input_buf;
    model.value_input_len = g_state.value_input_len;
    model.value_input_caret = g_state.value_input_caret;
    model.width = w;
    model.height = h;
    callbacks.draw_card_round = draw_card_round;
    callbacks.draw_text = draw_text;
    callbacks.measure_text_width = measure_text_width;
    callbacks.set_brush_color = set_brush_color;
    callbacks.render_help_modal_content = render_help_modal_content;
    callbacks.render_config_modal_content = render_config_modal_content;
    modal_shell_render(g_ui.target, g_ui.brush, g_ui.fmt_ui, g_ui.fmt_mono, g_ui.fmt_info, &model, &callbacks, &output);
    g_modal_rect = output.modal_rect;
    g_modal_close_rect = output.modal_close_rect;
    g_value_modal_rect = output.value_modal_rect;
}

static void render(HWND hwnd) {
    RECT wr;
    float w;
    float h;
    sync_ui_theme();
    float menu_h = g_ui_theme.metrics.control_height_toolbar;
    float toolbar_h = 44.0f;
    float top_h = menu_h + toolbar_h + 12.0f;
    float status_h = g_ui_theme.metrics.status_bar_height;
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

    D2D1_COLOR_F clear_color = g_ui_theme.colors.clear_canvas;
    RenderLayout layout = compute_render_layout(w, h, top_h, status_h, bottom_h, left_w, right_w);
    {
        EditorScreenRenderFrame frame{};
        EditorScreenRendererCallbacks callbacks{};
        frame.hwnd = hwnd;
        frame.clear_color = clear_color;
        frame.width = w;
        frame.height = h;
        frame.menu_height = menu_h;
        frame.toolbar_height = toolbar_h;
        frame.work_bottom = layout.work_bottom;
        frame.top_rect = layout.top_rect;
        frame.status_rect = layout.status_rect;
        frame.bottom_rect = layout.bottom_rect;
        frame.left_rect = layout.left_rect;
        frame.right_rect = layout.right_rect;
        frame.center_rect = layout.center_rect;
        frame.stage_rect = layout.stage_rect;
        build_shell_round_rects(frame.left_rect, frame.center_rect, frame.right_rect, frame.bottom_rect, frame.status_rect, frame.stage_rect,
                                &frame.left_rr, &frame.center_rr, &frame.right_rr, &frame.bottom_rr, &frame.status_rr, &frame.stage_rr);
        callbacks.reset_splitter_rects = reset_splitter_rects;
        callbacks.update_stage_bounds_and_fit = update_stage_bounds_and_fit;
        callbacks.begin_render_pass = begin_render_pass;
        callbacks.render_shell_panels = render_shell_panels;
        callbacks.render_top_bar_content = render_top_bar_content;
        callbacks.render_workspace_content = render_workspace_content;
        callbacks.finalize_render_pass = finalize_render_pass;
        editor_screen_renderer_draw_frame(&frame, &callbacks);
    }
}

static void app_cmd_toggle_run(void* user) {
    (void)user;
    if (!pie_runtime_active()) {
        pie_enter_session();
        return;
    }
    g_state.running = !g_state.running;
    push_console_log(L"[PIE] %ls", g_state.running ? L"继续运行" : L"已暂停");
}

static void app_cmd_step_once(void* user) {
    (void)user;
    if (!pie_runtime_active()) {
        if (!pie_enter_session()) return;
    }
    g_state.running = 0;
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
    push_console_log(L"[PIE] 单步执行 1 帧");
}

static void app_cmd_reset_scene(void* user) {
    (void)user;
    if (pie_runtime_active()) {
        push_console_log(L"[PIE] 运行中禁止重置场景，请先按 Esc 退出");
        return;
    }
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

static void runtime_push_tick_history(const AppRuntimeSnapshot* snapshot) {
    int idx;
    if (snapshot == NULL || !snapshot->valid) return;
    if (g_state.runtime_tick_history_count < 8) {
        idx = (g_state.runtime_tick_history_head + g_state.runtime_tick_history_count) % 8;
        g_state.runtime_tick_history_count++;
    } else {
        g_state.runtime_tick_history_head = (g_state.runtime_tick_history_head + 1) % 8;
        idx = (g_state.runtime_tick_history_head + g_state.runtime_tick_history_count - 1) % 8;
    }
    g_state.runtime_tick_history[idx].frame_index = snapshot->frame_index;
    g_state.runtime_tick_history[idx].body_count = snapshot->body_count;
    g_state.runtime_tick_history[idx].constraint_count = snapshot->constraint_count;
    g_state.runtime_tick_history[idx].contact_count = snapshot->contact_count;
    g_state.runtime_tick_history[idx].step_ms = snapshot->step_ms;
}

static void runtime_push_state_history(const AppRuntimeSnapshot* snapshot) {
    int idx;
    unsigned int now_ms;
    if (snapshot == NULL || !snapshot->valid) return;
    now_ms = (unsigned int)GetTickCount();
    if (g_state.runtime_state_history_count < 8) {
        idx = (g_state.runtime_state_history_head + g_state.runtime_state_history_count) % 8;
        g_state.runtime_state_history_count++;
    } else {
        g_state.runtime_state_history_head = (g_state.runtime_state_history_head + 1) % 8;
        idx = (g_state.runtime_state_history_head + g_state.runtime_state_history_count - 1) % 8;
    }
    g_state.runtime_state_history[idx].frame_index = snapshot->frame_index;
    g_state.runtime_state_history[idx].tick_ms = now_ms;
    g_state.runtime_state_history[idx].running = snapshot->running ? 1 : 0;
}

static void apply_runtime_snapshot_to_state(const AppRuntimeSnapshot* snapshot) {
    int i;
    if (snapshot == NULL || !snapshot->valid) return;
    g_state.physics_step_ms = snapshot->step_ms;
    g_state.runtime_frame_index = snapshot->frame_index;
    g_state.runtime_running = snapshot->running ? 1 : 0;
    g_state.runtime_body_count = snapshot->body_count;
    g_state.runtime_constraint_count = snapshot->constraint_count;
    g_state.runtime_contact_count = snapshot->contact_count;
    g_state.runtime_error_count = snapshot->runtime_error_count;
    g_state.runtime_error_code = snapshot->runtime_error_code;
    g_state.runtime_error_item_count = snapshot->runtime_error_item_count;
    if (g_state.runtime_error_item_count < 0) g_state.runtime_error_item_count = 0;
    if (g_state.runtime_error_item_count > APP_RUNTIME_MAX_ERRORS) g_state.runtime_error_item_count = APP_RUNTIME_MAX_ERRORS;
    for (i = 0; i < g_state.runtime_error_item_count; i++) {
        g_state.runtime_errors[i] = snapshot->runtime_errors[i];
    }
    g_state.runtime_event_drop_count = snapshot->event_drop_count;
}

static void process_app_events(void) {
    AppEvent ev;
    while (app_runtime_pop_event(&g_app_runtime, &ev)) {
        if (ev.type == APP_EVENT_COMMAND_EXECUTED) {
            push_debug_event_history(L"命令: %ls", app_command_label(ev.command_type));
            if (ev.command_type == APP_CMD_STEP_ONCE) {
                push_console_log(L"[状态] 单步执行 1 帧");
            } else if (ev.command_type == APP_CMD_RESET_SCENE) {
                push_console_log(L"[状态] 已重置当前场景");
            } else if (ev.command_type == APP_CMD_SPAWN_CIRCLE) {
                push_console_log(L"[创建] 已生成圆形对象");
            } else if (ev.command_type == APP_CMD_SPAWN_BOX) {
                push_console_log(L"[创建] 已生成方块对象");
            }
        } else if (ev.type == APP_EVENT_RUNTIME_TICK) {
            unsigned int prev_drop = g_state.runtime_event_drop_count;
            apply_runtime_snapshot_to_state(&ev.runtime_snapshot);
            runtime_push_tick_history(&ev.runtime_snapshot);
            if (g_state.runtime_event_drop_count > prev_drop) {
                unsigned int now_ms = (unsigned int)GetTickCount();
                unsigned int delta_drop = g_state.runtime_event_drop_count - prev_drop;
                g_state.runtime_bus_congested = 1;
                g_state.runtime_drop_last_growth_ms = now_ms;
                if ((now_ms - g_state.runtime_drop_warn_ms) >= 1000 || g_state.runtime_drop_last_seen == 0) {
                    push_console_log(L"[警告] 事件总线丢弃 +%u (累计:%u)", delta_drop, g_state.runtime_event_drop_count);
                    push_debug_error_history(L"事件总线丢弃 +%u (累计:%u)", delta_drop, g_state.runtime_event_drop_count);
                    g_state.runtime_drop_warn_ms = now_ms;
                    g_state.runtime_drop_last_seen = g_state.runtime_event_drop_count;
                }
            } else if (g_state.runtime_bus_congested) {
                unsigned int now_ms = (unsigned int)GetTickCount();
                if (g_state.runtime_drop_last_growth_ms > 0 && (now_ms - g_state.runtime_drop_last_growth_ms) >= 3000) {
                    g_state.runtime_bus_congested = 0;
                    push_console_log(L"[状态] 事件总线已恢复（累计丢弃:%u）", g_state.runtime_event_drop_count);
                    push_debug_event_history(L"事件总线恢复 (累计:%u)", g_state.runtime_event_drop_count);
                }
            }
            {
                unsigned int now_ms = (unsigned int)GetTickCount();
                if (g_state.runtime_error_count > 0) {
                    if (g_state.runtime_error_code != g_state.last_runtime_error_code ||
                        (now_ms - g_state.last_runtime_error_log_ms) >= 1000) {
                        if (g_state.runtime_error_item_count > 0) {
                            const AppRuntimeErrorItem* err0 = &g_state.runtime_errors[0];
                            push_console_log((err0->severity == APP_RUNTIME_ERROR_ERROR)
                                                 ? L"[错误] 运行时错误 code=%d (%ls,%ls,x%d)"
                                                 : L"[警告] 运行时错误 code=%d (%ls,%ls,x%d)",
                                             err0->code,
                                             runtime_error_label(err0->code),
                                             runtime_error_severity_label(err0->severity),
                                             err0->count);
                            push_debug_error_history(L"运行时错误 %d (%ls,%ls,x%d)",
                                                     err0->code,
                                                     runtime_error_label(err0->code),
                                                     runtime_error_severity_label(err0->severity),
                                                     err0->count);
                        } else {
                            push_console_log(L"[警告] 运行时错误 code=%d (%ls)",
                                             g_state.runtime_error_code, runtime_error_label(g_state.runtime_error_code));
                            push_debug_error_history(L"运行时错误 %d (%ls)",
                                                     g_state.runtime_error_code,
                                                     runtime_error_label(g_state.runtime_error_code));
                        }
                        g_state.last_runtime_error_log_ms = now_ms;
                    }
                } else if (g_state.last_runtime_error_code != PHYSICS_ERROR_NONE) {
                    push_console_log(L"[状态] 运行时错误已恢复");
                    push_debug_event_history(L"运行时错误已恢复");
                    g_state.last_runtime_error_log_ms = now_ms;
                }
                g_state.last_runtime_error_code = g_state.runtime_error_code;
            }
            if (g_state.runtime_running && ev.runtime_snapshot.contact_count != g_state.last_contact_count) {
                unsigned int now_ms = (unsigned int)GetTickCount();
                if ((now_ms - g_state.last_contact_log_ms) >= 500) {
                    push_console_log(L"[碰撞] 接触数量: %d -> %d", g_state.last_contact_count, ev.runtime_snapshot.contact_count);
                    g_state.last_contact_log_ms = now_ms;
                }
                g_state.last_contact_count = ev.runtime_snapshot.contact_count;
            }
        } else if (ev.type == APP_EVENT_RUNTIME_STATE_CHANGED) {
            g_state.runtime_running = ev.runtime_snapshot.running ? 1 : 0;
            g_state.runtime_state_change_count++;
            runtime_push_state_history(&ev.runtime_snapshot);
            push_console_log(L"[状态] 模拟:%ls", ev.runtime_snapshot.running ? L"运行" : L"暂停");
            push_debug_event_history(L"PIE 状态: %ls (#%u)",
                                     ev.runtime_snapshot.running ? L"运行" : L"暂停",
                                     ev.runtime_snapshot.frame_index);
        } else if (ev.type == APP_EVENT_HOT_RELOAD_BATCH) {
            unsigned int now_ms = (unsigned int)GetTickCount();
            g_state.hot_reload_ready_batch_count = ev.hot_reload_snapshot.ready_batch_count;
            g_state.hot_reload_affected_count = ev.hot_reload_snapshot.affected_count;
            g_state.hot_reload_imported_count = ev.hot_reload_snapshot.imported_count;
            g_state.hot_reload_failed_count = ev.hot_reload_snapshot.failed_count;
            g_state.hot_reload_total_imported += (unsigned int)ev.hot_reload_snapshot.imported_count;
            g_state.hot_reload_total_failed += (unsigned int)ev.hot_reload_snapshot.failed_count;
            g_state.hot_reload_last_event_ms = now_ms;
            push_hot_reload_history(&ev.hot_reload_snapshot);
            if (ev.hot_reload_snapshot.imported_count > 0 || ev.hot_reload_snapshot.failed_count > 0) {
                push_debug_event_history(L"热重载[%ls]: 导入%d 失败%d 影响%d",
                                         ev.hot_reload_snapshot.pie_active ? L"PIE" : L"编辑",
                                         ev.hot_reload_snapshot.imported_count,
                                         ev.hot_reload_snapshot.failed_count,
                                         ev.hot_reload_snapshot.affected_count);
                if (ev.hot_reload_snapshot.failed_count > 0) {
                    push_tick_runtime_error(APP_RUNTIME_ERROR_CODE_HOT_RELOAD_IMPORT_FAILED,
                                            APP_RUNTIME_ERROR_ERROR,
                                            ev.hot_reload_snapshot.failed_count);
                    push_debug_error_history(L"热重载失败 %d 项，继续沿用上一版缓存",
                                             ev.hot_reload_snapshot.failed_count);
                    push_console_log(L"[热重载] 批次完成: 导入%d 失败%d 影响%d",
                                     ev.hot_reload_snapshot.imported_count,
                                     ev.hot_reload_snapshot.failed_count,
                                     ev.hot_reload_snapshot.affected_count);
                } else {
                    push_console_log(L"[热重载] 已重载 %d 项资源（影响:%d）",
                                     ev.hot_reload_snapshot.imported_count,
                                     ev.hot_reload_snapshot.affected_count);
                }
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
        {
            EditorSessionAutosaveArgs autosave_args{};
            autosave_args.enabled = (g_state.engine != NULL && !pie_runtime_active());
            autosave_args.now_ms = now_ms;
            autosave_args.autosave_interval_ms = 30000;
            autosave_args.last_autosave_ms = &g_state.last_autosave_ms;
            autosave_args.save_scene_snapshot = save_scene_snapshot;
            autosave_args.session_recovery_persist = session_recovery_persist;
            autosave_args.autosave_snapshot_path = "autosave_snapshot.txt";
            editor_session_tick_autosave(&autosave_args);
        }
    }
    app_runtime_collect_errors_for_tick();
    app_runtime_report_tick(&g_app_runtime, g_state.engine, g_state.running, g_state.physics_step_ms);
    hot_reload_tick_runtime();
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
    UiIntent intent{};
    if (!ctrl_down) return 0;
    if (pie_runtime_active() && wparam == 'O') {
        push_console_log(L"[PIE] 运行中禁止加载快照，请先按 Esc 退出");
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }

    if (wparam == 'S') {
        intent.type = UI_INTENT_SAVE_SNAPSHOT;
        intent.path_utf8 = "scene_snapshot.txt";
        dispatch_ui_intent(&intent);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == 'O') {
        intent.type = UI_INTENT_LOAD_SNAPSHOT;
        if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
            intent.path_utf8 = "autosave_snapshot.txt";
            intent.use_autosave = 1;
        } else {
            intent.path_utf8 = "scene_snapshot.txt";
            intent.use_autosave = 0;
        }
        dispatch_ui_intent(&intent);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == 'Z') {
        intent.type = UI_INTENT_HISTORY_UNDO;
        dispatch_ui_intent(&intent);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == 'Y') {
        intent.type = UI_INTENT_HISTORY_REDO;
        dispatch_ui_intent(&intent);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == 'C') {
        intent.type = UI_INTENT_COPY_SELECTED;
        dispatch_ui_intent(&intent);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == 'V') {
        trace_spawn_step("key.ctrlv.begin", "");
        intent.type = UI_INTENT_PASTE_SELECTED;
        dispatch_ui_intent(&intent);
        trace_spawn_step("key.ctrlv.end", "");
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == 'F') {
        return 0;
    }
    if (wparam == 'L') {
        intent.type = UI_INTENT_BEGIN_HIERARCHY_FILTER_INPUT;
        dispatch_ui_intent(&intent);
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
    UiIntent intent{};
    if (g_state.runtime_bus_congested && point_in_rect(g_state.mouse_screen, runtime_bus_badge_rect())) {
        intent.type = UI_INTENT_FOCUS_CONSOLE_LOG;
        intent.log_filter_mode = 3;
        dispatch_ui_intent(&intent);
        push_console_log(L"[状态] 已定位到事件总线告警日志");
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_run_rect)) {
        AppCommand cmd;
        if (input_try_map_toolbar_command(TOOLBAR_ACTION_TOGGLE_RUN, &cmd)) {
            intent.type = UI_INTENT_APP_COMMAND;
            intent.app_command = cmd;
            dispatch_ui_intent(&intent);
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_step_rect)) {
        intent.type = UI_INTENT_APP_COMMAND;
        intent.app_command.type = APP_CMD_STEP_ONCE;
        dispatch_ui_intent(&intent);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_reset_rect)) {
        AppCommand cmd;
        if (input_try_map_toolbar_command(TOOLBAR_ACTION_RESET_SCENE, &cmd)) {
            intent.type = UI_INTENT_APP_COMMAND;
            intent.app_command = cmd;
            dispatch_ui_intent(&intent);
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_save_rect)) {
        intent.type = UI_INTENT_SAVE_SNAPSHOT;
        intent.path_utf8 = "scene_snapshot.txt";
        dispatch_ui_intent(&intent);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_undo_rect)) {
        intent.type = UI_INTENT_HISTORY_UNDO;
        dispatch_ui_intent(&intent);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_redo_rect)) {
        intent.type = UI_INTENT_HISTORY_REDO;
        dispatch_ui_intent(&intent);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_grid_rect)) {
        intent.type = UI_INTENT_TOGGLE_DRAW_CENTERS;
        dispatch_ui_intent(&intent);
        push_console_log(L"[视图] 网格/中心:%ls", g_state.draw_centers ? L"开" : L"关");
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_collision_rect)) {
        intent.type = UI_INTENT_TOGGLE_DRAW_CONTACTS;
        dispatch_ui_intent(&intent);
        push_console_log(L"[视图] 碰撞点:%ls", g_state.draw_contacts ? L"开" : L"关");
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_top_velocity_rect)) {
        intent.type = UI_INTENT_TOGGLE_DRAW_VELOCITY;
        dispatch_ui_intent(&intent);
        push_console_log(L"[视图] 速度向量:%ls", g_state.draw_velocity ? L"开" : L"关");
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
        if (point_in_rect(g_state.mouse_screen, g_perf_diag_export_rect)) {
            export_diagnostic_bundle_now();
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
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
        if (point_in_rect(g_state.mouse_screen, g_ins_minus_rect[idx])) {
            g_state.inspector_focused_row = idx;
            g_state.keyboard_focus_area = 1;
            inspector_adjust_focused_row(-1);
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
        if (point_in_rect(g_state.mouse_screen, g_ins_plus_rect[idx])) {
            g_state.inspector_focused_row = idx;
            g_state.keyboard_focus_area = 1;
            inspector_adjust_focused_row(1);
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
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
    for (idx = 0; point_in_rect(g_state.mouse_screen, g_debug_viewport_rect) && idx < g_dbg_runtime_tick_row_count; idx++) {
        if (point_in_rect(g_state.mouse_screen, g_dbg_runtime_tick_row_rect[idx])) {
            int entry_idx = g_dbg_runtime_tick_row_entry_index[idx];
            if (entry_idx >= 0 && entry_idx < 8) {
                RuntimeTickHistoryEntry entry = g_state.runtime_tick_history[entry_idx];
                push_console_log(L"[运行时] 帧#%u 对象:%d 约束:%d 接触:%d %.2fms",
                                 entry.frame_index, entry.body_count, entry.constraint_count, entry.contact_count, entry.step_ms);
                {
                    UiIntent intent{};
                    intent.type = UI_INTENT_FOCUS_CONSOLE_LOG;
                    intent.log_filter_mode = 2;
                    dispatch_ui_intent(&intent);
                }
                InvalidateRect(hwnd, NULL, FALSE);
                return 1;
            }
        }
    }
    for (idx = 0; point_in_rect(g_state.mouse_screen, g_debug_viewport_rect) && idx < g_dbg_runtime_state_row_count; idx++) {
        if (point_in_rect(g_state.mouse_screen, g_dbg_runtime_state_row_rect[idx])) {
            int entry_idx = g_dbg_runtime_state_row_entry_index[idx];
            if (entry_idx >= 0 && entry_idx < 8) {
                RuntimeStateHistoryEntry entry = g_state.runtime_state_history[entry_idx];
                push_console_log(L"[运行时] 帧#%u 状态:%ls 时间戳:%ums",
                                 entry.frame_index, entry.running ? L"运行" : L"暂停", entry.tick_ms);
                {
                    UiIntent intent{};
                    intent.type = UI_INTENT_FOCUS_CONSOLE_LOG;
                    intent.log_filter_mode = 0;
                    dispatch_ui_intent(&intent);
                }
                InvalidateRect(hwnd, NULL, FALSE);
                return 1;
            }
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
    for (idx = 0; idx < g_explorer_scene_count && point_in_rect(g_state.mouse_screen, g_hierarchy_viewport_rect); idx++) {
        if (point_in_rect(g_state.mouse_screen, g_explorer_scene_rect[idx])) {
            if (pie_runtime_active()) {
                push_console_log(L"[PIE] 运行中禁止切换场景，请先按 Esc 退出");
                InvalidateRect(hwnd, NULL, FALSE);
                return 1;
            }
            apply_scene(g_explorer_scene_index[idx]);
            g_state.keyboard_focus_area = 1;
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
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
    if (wparam == VK_ESCAPE && pie_runtime_active()) {
        if (pie_exit_session()) {
            changed = 1;
        }
    }
    if (wparam == VK_SPACE || wparam == 'N' || wparam == 'R') {
        AppCommand cmd;
        if (input_try_map_keydown_command((unsigned int)wparam, &cmd)) {
            UiIntent intent{};
            intent.type = UI_INTENT_APP_COMMAND;
            intent.app_command = cmd;
            dispatch_ui_intent(&intent);
            changed = 1;
        }
    }
    {
        RuntimeToggleAction action = RUNTIME_TOGGLE_NONE;
        if (input_try_map_runtime_toggle((unsigned int)wparam, &action)) {
            UiIntent intent{};
            switch (action) {
                case RUNTIME_TOGGLE_DRAW_CONTACTS:
                    intent.type = UI_INTENT_TOGGLE_DRAW_CONTACTS;
                    dispatch_ui_intent(&intent);
                    changed = 1;
                    break;
                case RUNTIME_TOGGLE_DRAW_VELOCITY:
                    intent.type = UI_INTENT_TOGGLE_DRAW_VELOCITY;
                    dispatch_ui_intent(&intent);
                    changed = 1;
                    break;
                case RUNTIME_TOGGLE_DRAW_CENTERS:
                    intent.type = UI_INTENT_TOGGLE_DRAW_CENTERS;
                    dispatch_ui_intent(&intent);
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
                    intent.type = UI_INTENT_APPLY_NEXT_LAYOUT;
                    dispatch_ui_intent(&intent);
                    intent.type = UI_INTENT_SAVE_LAYOUT;
                    dispatch_ui_intent(&intent);
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
        if (wparam == VK_LEFT || wparam == VK_SUBTRACT || wparam == VK_OEM_MINUS) {
            if (inspector_adjust_focused_row(-1)) changed = 1;
        }
        if (wparam == VK_RIGHT || wparam == VK_ADD || wparam == VK_OEM_PLUS) {
            if (inspector_adjust_focused_row(1)) changed = 1;
        }
    }
    return changed;
}

static int handle_keydown_spawn_constraint_delete(WPARAM wparam) {
    int changed = 0;
    if (wparam == '1' || wparam == '2') {
        AppCommand cmd;
        if (input_try_map_keydown_command((unsigned int)wparam, &cmd)) {
            UiIntent intent{};
            intent.type = UI_INTENT_APP_COMMAND;
            intent.app_command = cmd;
            dispatch_ui_intent(&intent);
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
        if (pie_runtime_active()) {
            push_console_log(L"[PIE] 运行中禁止切换场景，请先按 Esc 退出");
            return 1;
        }
        apply_scene(scene_index);
        changed = 1;
    }
    if (input_try_map_scene_step((unsigned int)wparam, &scene_step)) {
        if (pie_runtime_active()) {
            push_console_log(L"[PIE] 运行中禁止切换场景，请先按 Esc 退出");
            return 1;
        }
        apply_scene((g_state.scene_index + scene_step + SCENE_COUNT) % SCENE_COUNT);
        changed = 1;
    }
    return changed;
}

static int handle_keydown_scene_reorder(HWND hwnd, WPARAM wparam) {
    int alt_down = (GetKeyState(VK_MENU) & 0x8000) != 0;
    int changed = 0;
    int pos = -1;
    EditorCommand command_data;
    if (!alt_down) return 0;
    if (wparam != VK_UP && wparam != VK_DOWN && wparam != VK_HOME) return 0;
    if (g_state.keyboard_focus_area != 1) return 1;
    if (pie_runtime_active()) {
        push_console_log(L"[PIE] 运行中禁止调整场景顺序，请先按 Esc 退出");
        return 1;
    }
    if (g_state.selected != NULL || selected_constraint_ref() != NULL) return 1;
    ZeroMemory(&command_data, sizeof(command_data));
    if (wparam == VK_UP) {
        command_data.type = EDITOR_CMD_SCENE_ORDER_MOVE;
        command_data.arg_i0 = -1;
        changed = dispatch_editor_command(&command_data);
    }
    if (wparam == VK_DOWN) {
        command_data.type = EDITOR_CMD_SCENE_ORDER_MOVE;
        command_data.arg_i0 = 1;
        changed = dispatch_editor_command(&command_data);
    }
    if (wparam == VK_HOME) {
        command_data.type = EDITOR_CMD_SCENE_ORDER_RESET;
        changed = dispatch_editor_command(&command_data);
    }
    if (changed) {
        if (wparam == VK_HOME) {
            push_console_log(L"[场景] 排序已恢复默认顺序");
        } else {
            pos = scene_order_find_position(g_state.scene_index);
            if (pos >= 0) {
                push_console_log(L"[场景] 顺序调整: %ls -> 第 %d 位", scene_display_name(g_state.scene_index), pos + 1);
            }
        }
        InvalidateRect(hwnd, NULL, FALSE);
    }
    return 1;
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
    if (handle_keydown_scene_reorder(hwnd, wparam)) return 1;
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
            UiIntent intent{};
            intent.type = UI_INTENT_APP_COMMAND;
            intent.app_command = cmd;
            dispatch_ui_intent(&intent);
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (point_in_rect(g_state.mouse_screen, g_menu_bar_drag_rect)) {
        ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
        return 1;
    }
    for (idx = 0; idx < g_explorer_scene_count && point_in_rect(g_state.mouse_screen, g_hierarchy_viewport_rect); idx++) {
        if (point_in_rect(g_state.mouse_screen, g_explorer_scene_rect[idx])) {
            if (pie_runtime_active()) {
                push_console_log(L"[PIE] 运行中禁止重命名场景，请先按 Esc 退出");
                InvalidateRect(hwnd, NULL, FALSE);
                return 1;
            }
            begin_value_input_for_scene_name(g_explorer_scene_index[idx]);
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
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
    if (handle_keydown_scene_reorder(hwnd, wparam)) return 1;
    if (wparam == VK_F2 && g_state.keyboard_focus_area == 1) {
        if (pie_runtime_active()) {
            push_console_log(L"[PIE] 运行中禁止重命名场景，请先按 Esc 退出");
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
        begin_value_input_for_scene_name(g_state.scene_index);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
    if (wparam == VK_F3 && g_state.keyboard_focus_area == 1) {
        if (pie_runtime_active()) {
            push_console_log(L"[PIE] 运行中禁止编辑场景资产引用，请先按 Esc 退出");
            InvalidateRect(hwnd, NULL, FALSE);
            return 1;
        }
        begin_value_input_for_scene_asset_guid(g_state.scene_index);
        InvalidateRect(hwnd, NULL, FALSE);
        return 1;
    }
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
        if (g_state.value_input_target == VALUE_INPUT_TARGET_LOG_SEARCH ||
            g_state.value_input_target == VALUE_INPUT_TARGET_HIERARCHY_FILTER ||
            g_state.value_input_target == VALUE_INPUT_TARGET_SCENE_NAME ||
            g_state.value_input_target == VALUE_INPUT_TARGET_SCENE_ASSET_GUID) {
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
    status_presenter_init_context(g_status_project_path, 260, g_status_user, 64, g_startup_status_text, 96, &g_startup_degraded);
}

static int session_recovery_should_restore_current(void) {
    return session_recovery_should_restore(&g_session_recovery_state);
}

static void reset_clipboard_body_state(void) {
    g_clipboard_body.valid = 0;
}

static int init_platform_ui(void) {
    if (!window_host_initialize_platform()) return 0;
    if (FAILED(init_ui())) {
        CoUninitialize();
        return 0;
    }
    return 1;
}

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
static void init_app_runtime_callbacks(void);
static void init_state_defaults(void);
static void phase_g_refresh_editor_extension_state(void);
static void phase_g_shutdown_editor_extension_state(void);
static void session_recovery_persist(const wchar_t* recent_action);
static void export_diagnostic_bundle_now(void);
static int session_recovery_should_restore_current(void);
static void reset_clipboard_body_state(void);

static void init_app_bootstrap_state(void) {
    EditorSessionBootstrapArgs args{};
    EditorSessionBootstrapResult result{};
    pie_lifecycle_init(&g_pie_lifecycle, PIE_EDITOR_SNAPSHOT_PATH);
    args.init_state_defaults = init_state_defaults;
    args.init_editor_command_bus_callbacks = init_editor_command_bus_callbacks;
    args.init_app_runtime_callbacks = init_app_runtime_callbacks;
    args.refresh_editor_extension_state = phase_g_refresh_editor_extension_state;
    args.init_hot_reload_pipeline = init_hot_reload_pipeline;
    args.clear_debug_histories = clear_debug_histories;
    args.clear_collision_events = clear_collision_events;
    args.reset_clipboard_body = reset_clipboard_body_state;
    args.load_ui_layout = load_ui_layout;
    args.apply_scene = apply_scene;
    args.session_recovery_should_restore = session_recovery_should_restore_current;
    args.file_exists = file_exists_utf8_path;
    args.load_scene_snapshot = load_scene_snapshot;
    args.history_reset_and_capture = history_reset_and_capture;
    args.session_recovery_persist = session_recovery_persist;
    args.autosave_snapshot_path = "autosave_snapshot.txt";
    editor_session_bootstrap(&args, &result);
    g_collision_event_filter_selected_only = 0;
    push_console_log(L"[启动] 物理沙盒已启动");
    push_console_log(L"[启动] %ls", g_startup_status_text[0] ? g_startup_status_text : L"启动:未知");
    if (result.autosave_snapshot_detected) {
        push_console_log(L"[启动] 检测到 autosave_snapshot.txt，可在 文件 菜单恢复");
    }
    if (result.recovered_session) {
        push_console_log(L"[恢复] 检测到未清理退出，已恢复最近工作会话");
    }
}

static int shutdown_and_get_exit_code(int exit_code) {
    if (g_state.engine) physics_engine_free(g_state.engine);
    phase_g_shutdown_editor_extension_state();
    asset_fs_watch_shutdown(&g_asset_fs_watch);
    session_recovery_mark_clean(&g_session_recovery_state, "clean exit");
    session_recovery_state_save_v1(&g_session_recovery_state, editor_extension_host_default_session_recovery_path());
    save_ui_layout();
    shutdown_ui();
    CoUninitialize();
    return exit_code;
}

static void init_app_runtime_callbacks(void) {
    AppCommandCallbacks callbacks;
    callbacks.toggle_run = app_cmd_toggle_run;
    callbacks.step_once = app_cmd_step_once;
    callbacks.reset_scene = app_cmd_reset_scene;
    callbacks.spawn_circle = app_cmd_spawn_circle;
    callbacks.spawn_box = app_cmd_spawn_box;
    callbacks.user = NULL;
    app_runtime_init(&g_app_runtime, callbacks);
}

static void init_state_defaults(void) {
    int i;
    scene_catalog_copy_defaults(g_state.scenes, SCENE_COUNT);
    scene_catalog_copy_default_names(g_state.scene_names, SCENE_COUNT);
    for (i = 0; i < SCENE_COUNT; i++) {
        g_state.scene_order[i] = i;
    }
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
    g_state.value_input_target = VALUE_INPUT_TARGET_NONE;
    g_state.value_input_row = 0;
    g_state.value_input_len = 0;
    g_state.value_input_caret = 0;
    g_state.log_search_buf[0] = L'\0';
    g_state.log_search_len = 0;
    g_state.hierarchy_filter_buf[0] = L'\0';
    g_state.hierarchy_filter_len = 0;
    g_state.fps_display = 0;
    g_state.physics_step_ms = 0.0f;
    g_state.runtime_frame_index = 0;
    g_state.runtime_running = 0;
    g_state.runtime_body_count = 0;
    g_state.runtime_constraint_count = 0;
    g_state.runtime_contact_count = 0;
    g_state.runtime_error_count = 0;
    g_state.runtime_error_code = PHYSICS_ERROR_NONE;
    g_state.runtime_error_item_count = 0;
    g_state.runtime_state_change_count = 0;
    g_state.runtime_event_drop_count = 0;
    g_state.runtime_drop_warn_ms = 0;
    g_state.runtime_drop_last_seen = 0;
    g_state.runtime_drop_last_growth_ms = 0;
    g_state.runtime_bus_congested = 0;
    g_state.hot_reload_enabled = 0;
    g_state.hot_reload_watch_count = 0;
    g_state.hot_reload_scan_change_count = 0;
    g_state.hot_reload_ready_batch_count = 0;
    g_state.hot_reload_affected_count = 0;
    g_state.hot_reload_imported_count = 0;
    g_state.hot_reload_failed_count = 0;
    g_state.hot_reload_total_imported = 0;
    g_state.hot_reload_total_failed = 0;
    g_state.hot_reload_last_scan_ms = 0;
    g_state.hot_reload_last_discover_ms = 0;
    g_state.hot_reload_last_event_ms = 0;
    g_state.hot_reload_last_warn_ms = 0;
    g_state.runtime_tick_history_head = 0;
    g_state.runtime_tick_history_count = 0;
    g_state.runtime_state_history_head = 0;
    g_state.runtime_state_history_count = 0;
    g_state.fps_last_tick_ms = 0;
    g_state.fps_accum_frames = 0;
    g_state.last_contact_count = 0;
    g_state.last_runtime_error_code = PHYSICS_ERROR_NONE;
    g_state.last_contact_log_ms = 0;
    g_state.last_runtime_error_log_ms = 0;
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
    InputRouterCallbacks callbacks{};
    callbacks.handle_nchittest = handle_nchittest;
    callbacks.handle_paint = handle_paint;
    callbacks.handle_size = handle_size;
    callbacks.handle_timer = handle_timer;
    callbacks.handle_keydown = handle_keydown;
    callbacks.handle_char = handle_char;
    callbacks.handle_syskeydown_message = handle_syskeydown_message;
    callbacks.handle_mousemove = handle_mousemove;
    callbacks.handle_mousewheel = handle_mousewheel;
    callbacks.handle_setcursor = handle_setcursor;
    callbacks.handle_lbuttondown = handle_lbuttondown;
    callbacks.handle_lbuttonup = handle_lbuttonup;
    callbacks.handle_lbuttondblclk_message = handle_lbuttondblclk_message;
    callbacks.handle_destroy = handle_destroy;
    callbacks.mouse_x = &g_state.mouse_screen.x;
    callbacks.mouse_y = &g_state.mouse_screen.y;
    return input_router_dispatch(&callbacks, hwnd, msg, wparam, lparam);
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

    if (!window_host_register_main_window_class(inst, wnd_proc, L"PhysicsSandboxDWrite", IDI_APP_MAIN,
                                                &g_app_icon_large, &g_app_icon_small, &wc)) {
        return shutdown_and_get_exit_code(1);
    }

    hwnd = window_host_create_main_window(inst, wc.lpszClassName, L"物理引擎沙盒（DirectWrite）", SW_SHOWMAXIMIZED);
    if (hwnd == NULL) return shutdown_and_get_exit_code(1);
    g_app_hwnd = hwnd;

    exit_code = window_host_run_message_loop();
    return shutdown_and_get_exit_code(exit_code);
}


