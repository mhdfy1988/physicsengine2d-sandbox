cmake_minimum_required(VERSION 3.16)
project(PhysicsEngine2D C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
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

function(physics_add_cpp_smoke target source needs_cpp_include)
    add_executable(${target} ${source})
    if(${needs_cpp_include})
        target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/cpp)
    endif()
    target_link_libraries(${target} PRIVATE physics2d)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endfunction()

set(PHYSICS2D_SOURCES
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
    src/content/scene_schema.c
    src/content/prefab_schema.c
    src/content/asset_database.c
    src/content/asset_importer.c
    src/content/asset_invalidation.c
    src/content/asset_pipeline.c
    src/content/asset_watch.c
    src/content/asset_hot_reload.c
    src/content/asset_fs_poll.c
    src/content/subsystem_render_audio_animation.c
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

add_library(physics2d STATIC ${PHYSICS2D_SOURCES})
add_library(physics_core ALIAS physics2d)
target_include_directories(physics2d PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(physics2d PRIVATE ${CMAKE_SOURCE_DIR}/src)
if(PHYSICS_USE_CPP_KERNEL)
    target_compile_definitions(physics2d PUBLIC PHYSICS_USE_CPP_KERNEL=1)
else()
    target_compile_definitions(physics2d PUBLIC PHYSICS_USE_CPP_KERNEL=0)
endif()
if(NOT MSVC)
    target_link_libraries(physics2d PUBLIC m)
endif()

if(MSVC)
    target_compile_options(physics2d PRIVATE /W4)
else()
    target_compile_options(physics2d PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

if(BUILD_EDITOR)
    set(SANDBOX_SOURCES
        apps/sandbox_dwrite/main.c
        apps/sandbox_dwrite/infrastructure/project_tree.c
        apps/sandbox_dwrite/infrastructure/snapshot_repo.c
        apps/sandbox_dwrite/infrastructure/ui_layout_repo.c
        apps/sandbox_dwrite/presentation/render/ui_icons.c
        apps/sandbox_dwrite/presentation/render/ui_text.c
        apps/sandbox_dwrite/presentation/render/ui_primitives.c
        apps/sandbox_dwrite/presentation/render/ui_widgets.c
        apps/sandbox_dwrite/domain/app_command.c
        apps/sandbox_dwrite/infrastructure/app_event_bus.c
        apps/sandbox_dwrite/application/app_controller.c
        apps/sandbox_dwrite/application/app_runtime.c
        apps/sandbox_dwrite/application/pie_lifecycle.c
        apps/sandbox_dwrite/application/editor_command_bus.c
        apps/sandbox_dwrite/application/scene_catalog.c
        apps/sandbox_dwrite/application/scene_builder.c
        apps/sandbox_dwrite/application/history_service.c
        apps/sandbox_dwrite/application/runtime_param_service.c
        apps/sandbox_dwrite/presentation/input/input_mapping.c
        apps/sandbox_dwrite/presentation/input/menu_file_edit_actions.c
        apps/sandbox_dwrite/presentation/input/menu_view_physics_window_actions.c
        apps/sandbox_dwrite/presentation/input/menu_help_actions.c
        apps/sandbox_dwrite/presentation/input/menu_model.c
    )
    if(WIN32)
        list(APPEND SANDBOX_SOURCES apps/sandbox_dwrite/app_icon.rc)
    endif()

    add_executable(physics_sandbox WIN32 ${SANDBOX_SOURCES})
    add_executable(physics_editor ALIAS physics_sandbox)
    target_link_libraries(physics_sandbox PRIVATE physics2d d2d1 dwrite windowscodecs ole32 uuid shcore gdi32 user32)
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
    add_executable(physics_tests
        tests/regression_tests.c
        tests/regression_collision_core_tests.c
        tests/regression_event_snapshot_tests.c
        tests/regression_pipeline_error_tests.c
        tests/regression_sleep_broadphase_tests.c
        tests/regression_stress_constraint_tests.c
        tests/regression_engine_feature_tests.c
        tests/equivalence/equivalence_api_tests.c
        tests/regression_scene_schema_tests.c
        tests/regression_prefab_schema_tests.c
        tests/regression_asset_database_tests.c
        tests/regression_asset_importer_tests.c
        tests/regression_asset_pipeline_tests.c
        tests/regression_asset_hot_reload_tests.c
    )
    add_executable(physics_app_runtime_smoke
        tests/app_runtime_tick_smoke.c
        apps/sandbox_dwrite/domain/app_command.c
        apps/sandbox_dwrite/infrastructure/app_event_bus.c
        apps/sandbox_dwrite/application/app_controller.c
        apps/sandbox_dwrite/application/app_runtime.c
    )
    add_executable(physics_editor_undo_redo_smoke
        tests/editor_undo_redo_smoke.c
        apps/sandbox_dwrite/application/editor_command_bus.c
        apps/sandbox_dwrite/application/history_service.c
    )
    add_executable(physics_editor_pie_lifecycle_smoke
        tests/editor_pie_lifecycle_smoke.c
        apps/sandbox_dwrite/application/pie_lifecycle.c
    )
    add_executable(physics_subsystem_render_audio_animation_smoke
        tests/subsystem_render_audio_animation_smoke.c
    )
    physics_add_cpp_smoke(physics_cpp_script_bridge_smoke tests/cpp_script_bridge_smoke.cpp TRUE)
    add_executable(physics_bench tests/benchmark_suite.c)
    add_executable(physics_invariants tests/invariant_tests.c)
    target_link_libraries(physics_tests PRIVATE physics2d)
    target_link_libraries(physics_app_runtime_smoke PRIVATE physics2d)
    target_link_libraries(physics_editor_undo_redo_smoke PRIVATE physics2d)
    target_link_libraries(physics_editor_pie_lifecycle_smoke PRIVATE physics2d)
    target_link_libraries(physics_subsystem_render_audio_animation_smoke PRIVATE physics2d)
    target_link_libraries(physics_bench PRIVATE physics2d)
    target_link_libraries(physics_invariants PRIVATE physics2d)
    if(MSVC)
        target_compile_options(physics_tests PRIVATE /W4)
        target_compile_options(physics_app_runtime_smoke PRIVATE /W4)
        target_compile_options(physics_editor_undo_redo_smoke PRIVATE /W4)
        target_compile_options(physics_editor_pie_lifecycle_smoke PRIVATE /W4)
        target_compile_options(physics_subsystem_render_audio_animation_smoke PRIVATE /W4)
        target_compile_options(physics_bench PRIVATE /W4)
        target_compile_options(physics_invariants PRIVATE /W4)
    else()
        target_compile_options(physics_tests PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_app_runtime_smoke PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_editor_undo_redo_smoke PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_editor_pie_lifecycle_smoke PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_subsystem_render_audio_animation_smoke PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_bench PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
        target_compile_options(physics_invariants PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
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
    set_target_properties(physics_bench PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    set_target_properties(physics_invariants PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

add_executable(scene_migrate tools/scene_migrate_main.c)
target_link_libraries(scene_migrate PRIVATE physics2d)
if(MSVC)
    target_compile_options(scene_migrate PRIVATE /W4)
else()
    target_compile_options(scene_migrate PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
set_target_properties(scene_migrate PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_executable(parallel_benchmark_compare tools/parallel_benchmark_compare.c)
target_link_libraries(parallel_benchmark_compare PRIVATE physics2d)
if(MSVC)
    target_compile_options(parallel_benchmark_compare PRIVATE /W4)
else()
    target_compile_options(parallel_benchmark_compare PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
set_target_properties(parallel_benchmark_compare PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_executable(subsystem_workflow_demo tools/subsystem_workflow_demo.c)
target_link_libraries(subsystem_workflow_demo PRIVATE physics2d)
if(MSVC)
    target_compile_options(subsystem_workflow_demo PRIVATE /W4)
else()
    target_compile_options(subsystem_workflow_demo PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
set_target_properties(subsystem_workflow_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_executable(phase_d_profile_capture tools/phase_d_profile_capture.c)
target_link_libraries(phase_d_profile_capture PRIVATE physics2d)
target_include_directories(phase_d_profile_capture PRIVATE ${CMAKE_SOURCE_DIR}/src)
if(MSVC)
    target_compile_options(phase_d_profile_capture PRIVATE /W4)
else()
    target_compile_options(phase_d_profile_capture PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()
set_target_properties(phase_d_profile_capture PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(BUILD_CPP_COMPAT)
    physics_add_cpp_smoke(physics_cpp_smoke tests/cpp_api_smoke.cpp FALSE)
endif()

if(BUILD_CPP_WRAPPER)
    physics_add_cpp_smoke(physics_cpp_raii_smoke tests/cpp_raii_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_status_smoke tests/cpp_status_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_world_raii_smoke tests/cpp_world_raii_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_body_raii_smoke tests/cpp_body_raii_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_constraint_raii_smoke tests/cpp_constraint_raii_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_snapshot_smoke tests/cpp_snapshot_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_single_include_smoke tests/cpp_single_include_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_engine_view_smoke tests/cpp_engine_view_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_ecs_smoke tests/cpp_ecs_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_ecs_dirty_sync_smoke tests/cpp_ecs_dirty_sync_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_ecs_query_smoke tests/cpp_ecs_query_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_ecs_bridge_smoke tests/cpp_ecs_bridge_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_ecs_pipeline_smoke tests/cpp_ecs_pipeline_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_ecs_pipeline_config_smoke tests/cpp_ecs_pipeline_config_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_runtime_facade_smoke tests/cpp_runtime_facade_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_runtime_error_channel_smoke tests/cpp_runtime_error_channel_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_runtime_error_recovery_smoke tests/cpp_runtime_error_recovery_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_runtime_multi_error_smoke tests/cpp_runtime_multi_error_smoke.cpp TRUE)
    add_executable(physics_cpp_runtime_app_bridge_smoke
        tests/cpp_runtime_app_bridge_smoke.cpp
        apps/sandbox_dwrite/domain/app_command.c
        apps/sandbox_dwrite/infrastructure/app_event_bus.c
        apps/sandbox_dwrite/application/app_controller.c
        apps/sandbox_dwrite/application/app_runtime.c
    )
    target_include_directories(physics_cpp_runtime_app_bridge_smoke PRIVATE ${CMAKE_SOURCE_DIR}/cpp)
    target_link_libraries(physics_cpp_runtime_app_bridge_smoke PRIVATE physics2d)
    if(MSVC)
        target_compile_options(physics_cpp_runtime_app_bridge_smoke PRIVATE /W4)
    else()
        target_compile_options(physics_cpp_runtime_app_bridge_smoke PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
    set_target_properties(physics_cpp_runtime_app_bridge_smoke PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    physics_add_cpp_smoke(physics_cpp_runtime_events_smoke tests/cpp_runtime_events_smoke.cpp TRUE)
    physics_add_cpp_smoke(physics_cpp_runtime_pause_step_smoke tests/cpp_runtime_pause_step_smoke.cpp TRUE)
endif()

set_target_properties(physics2d PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
