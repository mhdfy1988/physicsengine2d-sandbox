#ifndef REGRESSION_TESTS_H
#define REGRESSION_TESTS_H

#define REGRESSION_TEST_LIST(X) \
    X(regression_test_circle_circle_detection) \
    X(regression_test_circle_inside_polygon) \
    X(regression_test_collision_detect_null_safety) \
    X(regression_test_shape_helpers_null_safety) \
    X(regression_test_gravity_override) \
    X(regression_test_raycast_geometry) \
    X(regression_test_polygon_polygon_sat_axes) \
    X(regression_test_high_speed_tunnel_regression) \
    X(regression_test_high_stack_stability_regression) \
    X(regression_test_broadphase_grid_filters_pairs) \
    X(regression_test_add_body_duplicate_guard) \
    X(regression_test_distance_constraint_stability) \
    X(regression_test_constraint_break_force) \
    X(regression_test_custom_pipeline_builders) \
    X(regression_test_pipeline_plugin_abi) \
    X(regression_test_error_channel_basics) \
    X(regression_test_job_system_hook) \
    X(regression_test_layered_config_snapshot) \
    X(regression_test_generation_handles) \
    X(regression_test_event_sink_trace) \
    X(regression_test_ccd_swept_circle_contact) \
    X(regression_test_island_solver_decomposition) \
    X(regression_test_sleep_wakeup_system) \
    X(regression_test_rope_constraint_limit) \
    X(regression_test_broadphase_sap_mode) \
    X(regression_test_contact_event_order_stable_after_sort) \
    X(regression_test_broadphase_bvh_reports_unsupported) \
    X(regression_test_snapshot_replay_determinism) \
    X(regression_test_snapshot_applies_mass_inertia)

#define DECLARE_REGRESSION_TEST(fn) int fn(void);
REGRESSION_TEST_LIST(DECLARE_REGRESSION_TEST)
#undef DECLARE_REGRESSION_TEST

#endif
