cmake_minimum_required(VERSION 3.16)
project(PhysicsEngine2D CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_EDITOR "Build DWrite interactive editor app" ON)
option(BUILD_SANDBOX "Deprecated alias for BUILD_EDITOR" ON)
option(BUILD_TESTS "Build regression test executable" OFF)
option(BUILD_CPP_COMPAT "Build C++ compatibility smoke target" ON)
option(BUILD_CPP_WRAPPER "Build C++ RAII wrapper smoke target" ON)
option(PHYSICS_USE_CPP_KERNEL "Enable incremental C++ kernel implementation path" ON)
if(DEFINED BUILD_SANDBOX)
    set(BUILD_EDITOR ${BUILD_SANDBOX})
endif()

function(physics_add_cpp_smoke target source link_target)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE ${link_target})
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endfunction()

function(physics_force_cxx_link target)
    set_property(TARGET ${target} PROPERTY LINKER_LANGUAGE CXX)
endfunction()

function(physics_enable_win32_com_cpp_compat target)
    if(NOT WIN32)
        return()
    endif()
    target_compile_definitions(${target} PRIVATE COBJMACROS)
    if(MSVC)
        target_compile_options(${target} PRIVATE /FI${CMAKE_SOURCE_DIR}/apps/sandbox_dwrite/infrastructure/com_cpp_compat.hpp)
    else()
        target_compile_options(${target} PRIVATE -include ${CMAKE_SOURCE_DIR}/apps/sandbox_dwrite/infrastructure/com_cpp_compat.hpp)
    endif()
endfunction()

set(PHYSICS_KERNEL_SOURCES
    src/physics2d/math.cpp
    src/physics2d/shape.cpp
    src/physics2d/body.cpp
    src/physics2d/constraint.cpp
    src/physics2d/collision.cpp
    src/physics2d/collision_detect.cpp
    src/c_api/physics.cpp
    src/c_api/physics_lifecycle.cpp
    src/c_api/physics_runtime_api.cpp
    src/c_api/physics_config.cpp
    src/c_api/physics_query.cpp
    src/c_api/physics_mutation.cpp
    src/c_api/physics_pipeline_api.cpp
    src/core/physics_snapshot.cpp
    src/core/physics_world.cpp
    src/core/physics_memory.cpp
    src/core/physics_ids.cpp
    src/core/physics_parallel.cpp
    src/physics2d/physics_broadphase.cpp
    src/physics2d/physics_contact_manager.cpp
    src/physics2d/physics_ccd.cpp
    src/physics2d/physics_raycast.cpp
    src/core/physics_step.cpp
    src/physics2d/physics_collision_pipeline.cpp
    src/physics2d/physics_resolve.cpp
    src/physics2d/physics_solver.cpp
    src/physics2d/physics_integrate.cpp
)

set(PHYSICS_CONTENT_SOURCES
    src/content/scene_schema.cpp
    src/content/prefab_schema.cpp
    src/content/prefab_semantics.cpp
    src/content/project_workspace.cpp
    src/content/editor_plugin.cpp
    src/content/session_recovery.cpp
    src/content/diagnostic_bundle.cpp
    src/content/asset_database.cpp
    src/content/asset_importer.cpp
    src/content/asset_invalidation.cpp
    src/content/asset_pipeline.cpp
    src/content/asset_watch.cpp
    src/content/asset_hot_reload.cpp
    src/content/asset_fs_poll.cpp
    src/content/asset_fs_watch.cpp
    src/content/subsystem_render_audio_animation.cpp
)

set(PHYSICS_RUNTIME_SUPPORT_SOURCES
    src/runtime/snapshot_repo.cpp
)

function(physics_configure_library target)
    target_include_directories(${target} PUBLIC ${CMAKE_SOURCE_DIR}/include)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    if(PHYSICS_USE_CPP_KERNEL)
        target_compile_definitions(${target} PUBLIC PHYSICS_USE_CPP_KERNEL=1)
    else()
        target_compile_definitions(${target} PUBLIC PHYSICS_USE_CPP_KERNEL=0)
    endif()
    if(WIN32)
        target_compile_definitions(${target} PUBLIC COBJMACROS)
    endif()
    if(NOT MSVC)
        target_link_libraries(${target} PUBLIC m)
    endif()
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
endfunction()

add_library(physics_kernel STATIC ${PHYSICS_KERNEL_SOURCES})
physics_configure_library(physics_kernel)

add_library(physics_content STATIC ${PHYSICS_CONTENT_SOURCES})
physics_configure_library(physics_content)
target_link_libraries(physics_content PUBLIC physics_kernel)

add_library(physics_runtime_support STATIC ${PHYSICS_RUNTIME_SUPPORT_SOURCES})
physics_configure_library(physics_runtime_support)
target_link_libraries(physics_runtime_support PUBLIC physics_kernel)

add_library(physics2d INTERFACE)
target_link_libraries(physics2d INTERFACE physics_kernel physics_content)
add_library(physics_core ALIAS physics_kernel)

if(BUILD_EDITOR)
    set(SANDBOX_SOURCES
        apps/sandbox_dwrite/main.cpp
        apps/sandbox_dwrite/infrastructure/project_tree.cpp
        apps/sandbox_dwrite/infrastructure/ui_layout_repo.cpp
        apps/sandbox_dwrite/presentation/render/ui_icons.cpp
        apps/sandbox_dwrite/presentation/render/editor_screen_renderer.cpp
        apps/sandbox_dwrite/presentation/render/ui_text.cpp
        apps/sandbox_dwrite/presentation/render/ui_primitives.cpp
        apps/sandbox_dwrite/presentation/render/ui_widgets.cpp
        apps/sandbox_dwrite/presentation/status/status_presenter.cpp
        apps/sandbox_dwrite/presentation/window/window_host.cpp
        apps/sandbox_dwrite/domain/app_command.cpp
        apps/sandbox_dwrite/infrastructure/app_event_bus.cpp
        apps/sandbox_dwrite/application/app_controller.cpp
        apps/sandbox_dwrite/application/app_runtime.cpp
        apps/sandbox_dwrite/application/editor_session_service.cpp
        apps/sandbox_dwrite/application/pie_lifecycle.cpp
        apps/sandbox_dwrite/application/editor_command_bus.cpp
        apps/sandbox_dwrite/application/scene_catalog.cpp
        apps/sandbox_dwrite/application/scene_builder.cpp
        apps/sandbox_dwrite/application/history_service.cpp
        apps/sandbox_dwrite/application/runtime_param_service.cpp
        apps/sandbox_dwrite/application/editor_extension_host_service.cpp
        apps/sandbox_dwrite/application/editor_extension_state_service.cpp
        apps/sandbox_dwrite/presentation/input/input_router.cpp
        apps/sandbox_dwrite/presentation/input/input_mapping.cpp
        apps/sandbox_dwrite/presentation/input/menu_file_edit_actions.cpp
        apps/sandbox_dwrite/presentation/input/menu_view_physics_window_actions.cpp
        apps/sandbox_dwrite/presentation/input/menu_help_actions.cpp
        apps/sandbox_dwrite/presentation/input/menu_model.cpp
    )
    if(WIN32)
        list(APPEND SANDBOX_SOURCES apps/sandbox_dwrite/app_icon.rc)
    endif()

    add_executable(physics_sandbox WIN32 ${SANDBOX_SOURCES})
    add_executable(physics_editor ALIAS physics_sandbox)
    target_link_libraries(physics_sandbox PRIVATE physics2d physics_runtime_support d2d1 dwrite windowscodecs ole32 uuid shcore gdi32 user32)
    physics_force_cxx_link(physics_sandbox)
    physics_enable_win32_com_cpp_compat(physics_sandbox)
    if(MSVC)
        target_compile_options(physics_sandbox PRIVATE /W4)
    else()
        target_compile_options(physics_sandbox PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
    set_target_properties(physics_sandbox PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

if(BUILD_TESTS)
    add_executable(physics_kernel_tests
        tests/kernel/kernel_regression_runner.cpp
        tests/kernel/regression_collision_core_tests.cpp
        tests/kernel/regression_event_snapshot_tests.cpp
        tests/kernel/regression_pipeline_error_tests.cpp
        tests/kernel/regression_sleep_broadphase_tests.cpp
        tests/kernel/regression_stress_constraint_tests.cpp
        tests/kernel/regression_engine_feature_tests.cpp
        tests/equivalence/equivalence_api_tests.cpp
    )
    add_executable(physics_content_tests
        tests/content/content_regression_runner.cpp
        tests/content/regression_scene_schema_tests.cpp
        tests/content/regression_prefab_schema_tests.cpp
        tests/content/regression_project_workspace_tests.cpp
        tests/content/regression_prefab_semantics_tests.cpp
        tests/content/regression_editor_plugin_tests.cpp
        tests/content/regression_session_recovery_tests.cpp
        tests/content/regression_asset_database_tests.cpp
        tests/content/regression_asset_importer_tests.cpp
        tests/content/regression_asset_pipeline_tests.cpp
        tests/content/regression_asset_hot_reload_tests.cpp
    )
    add_executable(physics_tests
        tests/regression_tests.cpp
        tests/kernel/regression_collision_core_tests.cpp
        tests/kernel/regression_event_snapshot_tests.cpp
        tests/kernel/regression_pipeline_error_tests.cpp
        tests/kernel/regression_sleep_broadphase_tests.cpp
        tests/kernel/regression_stress_constraint_tests.cpp
        tests/kernel/regression_engine_feature_tests.cpp
        tests/equivalence/equivalence_api_tests.cpp
        tests/content/regression_scene_schema_tests.cpp
        tests/content/regression_prefab_schema_tests.cpp
        tests/content/regression_project_workspace_tests.cpp
        tests/content/regression_prefab_semantics_tests.cpp
        tests/content/regression_editor_plugin_tests.cpp
        tests/content/regression_session_recovery_tests.cpp
        tests/content/regression_asset_database_tests.cpp
        tests/content/regression_asset_importer_tests.cpp
        tests/content/regression_asset_pipeline_tests.cpp
        tests/content/regression_asset_hot_reload_tests.cpp
    )
    add_executable(physics_app_runtime_smoke
        tests/runtime/app_runtime_tick_smoke.cpp
        apps/sandbox_dwrite/domain/app_command.cpp
        apps/sandbox_dwrite/infrastructure/app_event_bus.cpp
        apps/sandbox_dwrite/application/app_controller.cpp
        apps/sandbox_dwrite/application/app_runtime.cpp
    )
    add_executable(physics_editor_undo_redo_smoke
        tests/editor/editor_undo_redo_smoke.cpp
        apps/sandbox_dwrite/application/editor_command_bus.cpp
        apps/sandbox_dwrite/application/history_service.cpp
    )
    add_executable(physics_editor_pie_lifecycle_smoke
        tests/editor/editor_pie_lifecycle_smoke.cpp
        apps/sandbox_dwrite/application/pie_lifecycle.cpp
    )
    add_executable(physics_subsystem_render_audio_animation_smoke
        tests/smoke/subsystem_render_audio_animation_smoke.cpp
    )
    add_executable(physics_editor_extension_state_service_smoke
        tests/editor/editor_extension_state_service_smoke.cpp
        apps/sandbox_dwrite/application/editor_extension_state_service.cpp
        apps/sandbox_dwrite/application/editor_extension_host_service.cpp
    )
    add_executable(physics_core_header_smoke
        tests/smoke/physics_core_header_smoke.cpp
    )
    physics_add_cpp_smoke(physics_cpp_script_bridge_smoke tests/smoke/cpp_script_bridge_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_foundation_smoke tests/smoke/cpp_foundation_smoke.cpp physics_kernel)
    add_executable(physics_bench tests/benchmark_suite.cpp)
    add_executable(physics_invariants tests/invariant_tests.cpp)
    target_link_libraries(physics_kernel_tests PRIVATE physics_kernel)
    target_link_libraries(physics_content_tests PRIVATE physics_content)
    target_link_libraries(physics_tests PRIVATE physics2d)
    target_link_libraries(physics_app_runtime_smoke PRIVATE physics2d)
    target_link_libraries(physics_editor_undo_redo_smoke PRIVATE physics2d)
    target_link_libraries(physics_editor_pie_lifecycle_smoke PRIVATE physics2d)
    target_link_libraries(physics_subsystem_render_audio_animation_smoke PRIVATE physics2d)
    target_link_libraries(physics_editor_extension_state_service_smoke PRIVATE physics2d)
    target_link_libraries(physics_core_header_smoke PRIVATE physics_kernel physics_runtime_support)
    target_link_libraries(physics_bench PRIVATE physics2d)
    target_link_libraries(physics_invariants PRIVATE physics2d)
    physics_force_cxx_link(physics_kernel_tests)
    physics_force_cxx_link(physics_content_tests)
    physics_force_cxx_link(physics_tests)
    physics_force_cxx_link(physics_app_runtime_smoke)
    physics_force_cxx_link(physics_editor_undo_redo_smoke)
    physics_force_cxx_link(physics_editor_pie_lifecycle_smoke)
    physics_force_cxx_link(physics_subsystem_render_audio_animation_smoke)
    physics_force_cxx_link(physics_editor_extension_state_service_smoke)
    physics_force_cxx_link(physics_core_header_smoke)
    physics_force_cxx_link(physics_bench)
    physics_force_cxx_link(physics_invariants)
    physics_enable_win32_com_cpp_compat(physics_app_runtime_smoke)
    physics_enable_win32_com_cpp_compat(physics_editor_undo_redo_smoke)
    physics_enable_win32_com_cpp_compat(physics_editor_pie_lifecycle_smoke)
    if(MSVC)
        target_compile_options(physics_kernel_tests PRIVATE /W4)
        target_compile_options(physics_content_tests PRIVATE /W4)
        target_compile_options(physics_tests PRIVATE /W4)
        target_compile_options(physics_app_runtime_smoke PRIVATE /W4)
        target_compile_options(physics_editor_undo_redo_smoke PRIVATE /W4)
        target_compile_options(physics_editor_pie_lifecycle_smoke PRIVATE /W4)
        target_compile_options(physics_subsystem_render_audio_animation_smoke PRIVATE /W4)
        target_compile_options(physics_editor_extension_state_service_smoke PRIVATE /W4)
        target_compile_options(physics_core_header_smoke PRIVATE /W4)
        target_compile_options(physics_bench PRIVATE /W4)
        target_compile_options(physics_invariants PRIVATE /W4)
    else()
        target_compile_options(physics_kernel_tests PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_content_tests PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_tests PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_app_runtime_smoke PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_editor_undo_redo_smoke PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_editor_pie_lifecycle_smoke PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_subsystem_render_audio_animation_smoke PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_editor_extension_state_service_smoke PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_core_header_smoke PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_bench PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_invariants PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
    set_target_properties(physics_kernel_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    set_target_properties(physics_content_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    set_target_properties(physics_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    set_target_properties(physics_app_runtime_smoke PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    set_target_properties(physics_editor_undo_redo_smoke PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    set_target_properties(physics_editor_pie_lifecycle_smoke PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    set_target_properties(physics_subsystem_render_audio_animation_smoke PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    set_target_properties(physics_editor_extension_state_service_smoke PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    set_target_properties(physics_core_header_smoke PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    set_target_properties(physics_bench PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    set_target_properties(physics_invariants PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

add_executable(scene_migrate tools/scene_migrate_main.cpp)
target_link_libraries(scene_migrate PRIVATE physics2d)
if(MSVC)
    target_compile_options(scene_migrate PRIVATE /W4)
else()
    target_compile_options(scene_migrate PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
set_target_properties(scene_migrate PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_executable(physics_runtime_cli
    apps/runtime_cli/main.cpp
)
target_link_libraries(physics_runtime_cli PRIVATE physics_kernel physics_runtime_support)
if(MSVC)
    target_compile_options(physics_runtime_cli PRIVATE /W4)
else()
    target_compile_options(physics_runtime_cli PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
set_target_properties(physics_runtime_cli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_executable(parallel_benchmark_compare tools/parallel_benchmark_compare.cpp)
target_link_libraries(parallel_benchmark_compare PRIVATE physics2d)
physics_force_cxx_link(parallel_benchmark_compare)
if(WIN32)
    target_compile_definitions(parallel_benchmark_compare PRIVATE COBJMACROS)
endif()
if(MSVC)
    target_compile_options(parallel_benchmark_compare PRIVATE /W4)
else()
    target_compile_options(parallel_benchmark_compare PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
set_target_properties(parallel_benchmark_compare PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_executable(subsystem_workflow_demo tools/subsystem_workflow_demo.cpp)
target_link_libraries(subsystem_workflow_demo PRIVATE physics2d)
physics_force_cxx_link(subsystem_workflow_demo)
if(WIN32)
    target_compile_definitions(subsystem_workflow_demo PRIVATE COBJMACROS)
endif()
if(MSVC)
    target_compile_options(subsystem_workflow_demo PRIVATE /W4)
else()
    target_compile_options(subsystem_workflow_demo PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
set_target_properties(subsystem_workflow_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_executable(phase_d_profile_capture tools/phase_d_profile_capture.cpp)
target_link_libraries(phase_d_profile_capture PRIVATE physics2d)
target_include_directories(phase_d_profile_capture PRIVATE ${CMAKE_SOURCE_DIR}/src)
physics_force_cxx_link(phase_d_profile_capture)
if(WIN32)
    target_compile_definitions(phase_d_profile_capture PRIVATE COBJMACROS)
endif()
if(MSVC)
    target_compile_options(phase_d_profile_capture PRIVATE /W4)
else()
    target_compile_options(phase_d_profile_capture PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
set_target_properties(phase_d_profile_capture PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(BUILD_CPP_COMPAT)
    physics_add_cpp_smoke(physics_cpp_smoke tests/smoke/cpp_api_smoke.cpp physics_kernel)
endif()

if(BUILD_CPP_WRAPPER)
    physics_add_cpp_smoke(physics_cpp_raii_smoke tests/smoke/cpp_raii_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_status_smoke tests/smoke/cpp_status_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_world_raii_smoke tests/smoke/cpp_world_raii_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_body_raii_smoke tests/smoke/cpp_body_raii_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_constraint_raii_smoke tests/smoke/cpp_constraint_raii_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_snapshot_smoke tests/runtime/cpp_snapshot_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_single_include_smoke tests/smoke/cpp_single_include_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_engine_view_smoke tests/smoke/cpp_engine_view_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_ecs_smoke tests/smoke/cpp_ecs_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_ecs_dirty_sync_smoke tests/smoke/cpp_ecs_dirty_sync_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_ecs_query_smoke tests/smoke/cpp_ecs_query_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_ecs_bridge_smoke tests/smoke/cpp_ecs_bridge_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_ecs_pipeline_smoke tests/smoke/cpp_ecs_pipeline_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_ecs_pipeline_config_smoke tests/smoke/cpp_ecs_pipeline_config_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_runtime_facade_smoke tests/runtime/cpp_runtime_facade_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_runtime_error_channel_smoke tests/runtime/cpp_runtime_error_channel_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_runtime_error_recovery_smoke tests/runtime/cpp_runtime_error_recovery_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_runtime_multi_error_smoke tests/runtime/cpp_runtime_multi_error_smoke.cpp physics2d)
    add_executable(physics_cpp_runtime_app_bridge_smoke
        tests/runtime/cpp_runtime_app_bridge_smoke.cpp
        apps/sandbox_dwrite/domain/app_command.cpp
        apps/sandbox_dwrite/infrastructure/app_event_bus.cpp
        apps/sandbox_dwrite/application/app_controller.cpp
        apps/sandbox_dwrite/application/app_runtime.cpp
    )
    target_link_libraries(physics_cpp_runtime_app_bridge_smoke PRIVATE physics2d)
    physics_enable_win32_com_cpp_compat(physics_cpp_runtime_app_bridge_smoke)
    if(MSVC)
        target_compile_options(physics_cpp_runtime_app_bridge_smoke PRIVATE /W4)
    else()
        target_compile_options(physics_cpp_runtime_app_bridge_smoke PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
    set_target_properties(physics_cpp_runtime_app_bridge_smoke PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    physics_add_cpp_smoke(physics_cpp_runtime_events_smoke tests/runtime/cpp_runtime_events_smoke.cpp physics2d)
    physics_add_cpp_smoke(physics_cpp_runtime_pause_step_smoke tests/runtime/cpp_runtime_pause_step_smoke.cpp physics2d)
endif()

set_target_properties(physics_kernel PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
set_target_properties(physics_content PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
set_target_properties(physics_runtime_support PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
