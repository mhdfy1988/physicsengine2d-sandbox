#include <stdio.h>

typedef int (*RegressionTestFn)(void);

typedef struct {
    const char* name;
    RegressionTestFn fn;
} RegressionTestEntry;

int regression_test_circle_circle_detection(void);
int regression_test_circle_inside_polygon(void);
int regression_test_collision_detect_null_safety(void);
int regression_test_shape_helpers_null_safety(void);
int regression_test_gravity_override(void);
int regression_test_raycast_geometry(void);
int regression_test_polygon_polygon_sat_axes(void);
int regression_test_high_speed_tunnel_regression(void);
int regression_test_high_stack_stability_regression(void);
int regression_test_broadphase_grid_filters_pairs(void);
int regression_test_add_body_duplicate_guard(void);
int regression_test_distance_constraint_stability(void);
int regression_test_constraint_break_force(void);
int regression_test_custom_pipeline_builders(void);
int regression_test_pipeline_plugin_abi(void);
int regression_test_error_channel_basics(void);
int regression_test_job_system_hook(void);
int regression_test_layered_config_snapshot(void);
int regression_test_generation_handles(void);
int regression_test_event_sink_trace(void);
int regression_test_ccd_swept_circle_contact(void);
int regression_test_island_solver_decomposition(void);
int regression_test_sleep_wakeup_system(void);
int regression_test_rope_constraint_limit(void);
int regression_test_broadphase_sap_mode(void);
int regression_test_contact_event_order_stable_after_sort(void);
int regression_test_broadphase_bvh_reports_unsupported(void);
int regression_test_snapshot_replay_determinism(void);
int regression_test_snapshot_applies_mass_inertia(void);
int regression_test_equivalence_runtime_setters(void);
int regression_test_equivalence_remove_body_paths(void);

int main(void) {
    int passed = 0;
    int total = 0;
    int i;
    static const RegressionTestEntry kTests[] = {
        {"regression_test_circle_circle_detection", regression_test_circle_circle_detection},
        {"regression_test_circle_inside_polygon", regression_test_circle_inside_polygon},
        {"regression_test_collision_detect_null_safety", regression_test_collision_detect_null_safety},
        {"regression_test_shape_helpers_null_safety", regression_test_shape_helpers_null_safety},
        {"regression_test_gravity_override", regression_test_gravity_override},
        {"regression_test_raycast_geometry", regression_test_raycast_geometry},
        {"regression_test_polygon_polygon_sat_axes", regression_test_polygon_polygon_sat_axes},
        {"regression_test_high_speed_tunnel_regression", regression_test_high_speed_tunnel_regression},
        {"regression_test_high_stack_stability_regression", regression_test_high_stack_stability_regression},
        {"regression_test_broadphase_grid_filters_pairs", regression_test_broadphase_grid_filters_pairs},
        {"regression_test_add_body_duplicate_guard", regression_test_add_body_duplicate_guard},
        {"regression_test_distance_constraint_stability", regression_test_distance_constraint_stability},
        {"regression_test_constraint_break_force", regression_test_constraint_break_force},
        {"regression_test_custom_pipeline_builders", regression_test_custom_pipeline_builders},
        {"regression_test_pipeline_plugin_abi", regression_test_pipeline_plugin_abi},
        {"regression_test_error_channel_basics", regression_test_error_channel_basics},
        {"regression_test_job_system_hook", regression_test_job_system_hook},
        {"regression_test_layered_config_snapshot", regression_test_layered_config_snapshot},
        {"regression_test_generation_handles", regression_test_generation_handles},
        {"regression_test_event_sink_trace", regression_test_event_sink_trace},
        {"regression_test_ccd_swept_circle_contact", regression_test_ccd_swept_circle_contact},
        {"regression_test_island_solver_decomposition", regression_test_island_solver_decomposition},
        {"regression_test_sleep_wakeup_system", regression_test_sleep_wakeup_system},
        {"regression_test_rope_constraint_limit", regression_test_rope_constraint_limit},
        {"regression_test_broadphase_sap_mode", regression_test_broadphase_sap_mode},
        {"regression_test_contact_event_order_stable_after_sort", regression_test_contact_event_order_stable_after_sort},
        {"regression_test_broadphase_bvh_reports_unsupported", regression_test_broadphase_bvh_reports_unsupported},
        {"regression_test_snapshot_replay_determinism", regression_test_snapshot_replay_determinism},
        {"regression_test_snapshot_applies_mass_inertia", regression_test_snapshot_applies_mass_inertia},
        {"regression_test_equivalence_runtime_setters", regression_test_equivalence_runtime_setters},
        {"regression_test_equivalence_remove_body_paths", regression_test_equivalence_remove_body_paths},
    };

    for (i = 0; i < (int)(sizeof(kTests) / sizeof(kTests[0])); ++i) {
        total++;
        if (!kTests[i].fn()) {
            printf("\nResult: FAIL (%d/%d) at %s\n", passed, total, kTests[i].name);
            return 1;
        }
        passed++;
    }

    printf("\nResult: PASS (%d/%d)\n", passed, total);
    return 0;
}
