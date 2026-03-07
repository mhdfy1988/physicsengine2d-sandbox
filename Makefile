CXX = g++
AR = ar
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++17 -finput-charset=UTF-8 -fexec-charset=UTF-8 -Iinclude -Isrc -include apps/sandbox_dwrite/infrastructure/com_cpp_compat.hpp -DPHYSICS_USE_CPP_KERNEL=1 -DCOBJMACROS
LDFLAGS = -lm
DEPFLAGS = -MMD -MP
SRCDIR = src
APPDIR_DWRITE = apps/sandbox_dwrite
TESTDIR = tests
BINDIR = bin
OBJDIR = obj
LIBDIR = lib

C_SOURCES =
PHYSICS_KERNEL_CPP_SOURCES = $(SRCDIR)/physics2d/math.cpp $(SRCDIR)/physics2d/shape.cpp $(SRCDIR)/physics2d/body.cpp $(SRCDIR)/physics2d/constraint.cpp $(SRCDIR)/physics2d/collision.cpp $(SRCDIR)/physics2d/collision_detect.cpp $(SRCDIR)/core/physics_memory.cpp $(SRCDIR)/core/physics_ids.cpp $(SRCDIR)/core/physics_parallel.cpp $(SRCDIR)/core/physics_world.cpp $(SRCDIR)/physics2d/physics_raycast.cpp $(SRCDIR)/core/physics_snapshot.cpp $(SRCDIR)/physics2d/physics_integrate.cpp $(SRCDIR)/physics2d/physics_resolve.cpp $(SRCDIR)/core/physics_step.cpp $(SRCDIR)/physics2d/physics_broadphase.cpp $(SRCDIR)/physics2d/physics_collision_pipeline.cpp $(SRCDIR)/physics2d/physics_contact_manager.cpp $(SRCDIR)/physics2d/physics_ccd.cpp $(SRCDIR)/c_api/physics_pipeline_api.cpp $(SRCDIR)/physics2d/physics_solver.cpp $(SRCDIR)/c_api/physics_query.cpp $(SRCDIR)/c_api/physics_mutation.cpp $(SRCDIR)/c_api/physics.cpp $(SRCDIR)/c_api/physics_lifecycle.cpp $(SRCDIR)/c_api/physics_runtime_api.cpp $(SRCDIR)/c_api/physics_config.cpp
PHYSICS_CONTENT_CPP_SOURCES = $(SRCDIR)/content/scene_schema.cpp $(SRCDIR)/content/prefab_schema.cpp $(SRCDIR)/content/prefab_semantics.cpp $(SRCDIR)/content/project_workspace.cpp $(SRCDIR)/content/editor_plugin.cpp $(SRCDIR)/content/session_recovery.cpp $(SRCDIR)/content/diagnostic_bundle.cpp $(SRCDIR)/content/asset_database.cpp $(SRCDIR)/content/asset_invalidation.cpp $(SRCDIR)/content/asset_pipeline.cpp $(SRCDIR)/content/asset_watch.cpp $(SRCDIR)/content/asset_hot_reload.cpp $(SRCDIR)/content/asset_fs_poll.cpp $(SRCDIR)/content/asset_fs_watch.cpp $(SRCDIR)/content/asset_importer.cpp $(SRCDIR)/content/subsystem_render_audio_animation.cpp
PHYSICS_RUNTIME_SUPPORT_CPP_SOURCES = $(SRCDIR)/runtime/snapshot_repo.cpp
C_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(C_SOURCES))
PHYSICS_KERNEL_OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(PHYSICS_KERNEL_CPP_SOURCES))
PHYSICS_CONTENT_OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(PHYSICS_CONTENT_CPP_SOURCES))
PHYSICS_RUNTIME_SUPPORT_OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(PHYSICS_RUNTIME_SUPPORT_CPP_SOURCES))
CPP_OBJECTS = $(PHYSICS_KERNEL_OBJECTS) $(PHYSICS_CONTENT_OBJECTS) $(PHYSICS_RUNTIME_SUPPORT_OBJECTS)
OBJECTS = $(C_OBJECTS) $(CPP_OBJECTS)
DEPS = $(OBJECTS:.o=.d)

SANDBOX_SRCS = $(APPDIR_DWRITE)/main.cpp $(APPDIR_DWRITE)/infrastructure/app_path.cpp $(APPDIR_DWRITE)/infrastructure/project_tree.cpp $(APPDIR_DWRITE)/infrastructure/ui_layout_repo.cpp $(APPDIR_DWRITE)/presentation/design_system/ui_theme.cpp $(APPDIR_DWRITE)/presentation/components/button/button.cpp $(APPDIR_DWRITE)/presentation/components/panel/panel.cpp $(APPDIR_DWRITE)/presentation/components/status_chip/status_chip.cpp $(APPDIR_DWRITE)/presentation/overlay/status_bar.cpp $(APPDIR_DWRITE)/presentation/modal/modal_shell.cpp $(APPDIR_DWRITE)/presentation/modal/modal_content.cpp $(APPDIR_DWRITE)/presentation/panels/console/console_tabs.cpp $(APPDIR_DWRITE)/presentation/panels/console/console_perf.cpp $(APPDIR_DWRITE)/presentation/panels/console/console_panel_shell.cpp $(APPDIR_DWRITE)/presentation/panels/console/console_renderer.cpp $(APPDIR_DWRITE)/presentation/panels/hierarchy/hierarchy_layout.cpp $(APPDIR_DWRITE)/presentation/panels/hierarchy/hierarchy_renderer.cpp $(APPDIR_DWRITE)/presentation/panels/hierarchy/hierarchy_scrollbar.cpp $(APPDIR_DWRITE)/presentation/panels/inspector/inspector_layout.cpp $(APPDIR_DWRITE)/presentation/panels/inspector/inspector_debug_renderer.cpp $(APPDIR_DWRITE)/presentation/panels/inspector/inspector_panel_shell.cpp $(APPDIR_DWRITE)/presentation/panels/inspector/inspector_renderer.cpp $(APPDIR_DWRITE)/presentation/panels/inspector/inspector_scrollbar.cpp $(APPDIR_DWRITE)/presentation/topbar/top_toolbar.cpp $(APPDIR_DWRITE)/presentation/render/ui_icons.cpp $(APPDIR_DWRITE)/presentation/render/editor_screen_renderer.cpp $(APPDIR_DWRITE)/presentation/render/ui_text.cpp $(APPDIR_DWRITE)/presentation/render/ui_primitives.cpp $(APPDIR_DWRITE)/presentation/render/ui_widgets.cpp $(APPDIR_DWRITE)/presentation/shell/side_fold_buttons.cpp $(APPDIR_DWRITE)/presentation/stage/stage_panel.cpp $(APPDIR_DWRITE)/presentation/status/status_presenter.cpp $(APPDIR_DWRITE)/presentation/window/window_host.cpp $(APPDIR_DWRITE)/domain/app_command.cpp $(APPDIR_DWRITE)/infrastructure/app_event_bus.cpp $(APPDIR_DWRITE)/application/app_controller.cpp $(APPDIR_DWRITE)/application/app_runtime.cpp $(APPDIR_DWRITE)/application/workbench/workbench_service.cpp $(APPDIR_DWRITE)/application/editor_session_service.cpp $(APPDIR_DWRITE)/application/pie_lifecycle.cpp $(APPDIR_DWRITE)/application/editor_command_bus.cpp $(APPDIR_DWRITE)/application/scene_catalog.cpp $(APPDIR_DWRITE)/application/scene_builder.cpp $(APPDIR_DWRITE)/application/history_service.cpp $(APPDIR_DWRITE)/application/runtime_param_service.cpp $(APPDIR_DWRITE)/application/editor_extension_host_service.cpp $(APPDIR_DWRITE)/application/editor_extension_state_service.cpp $(APPDIR_DWRITE)/presentation/input/input_router.cpp $(APPDIR_DWRITE)/presentation/input/input_mapping.cpp $(APPDIR_DWRITE)/presentation/input/menu_file_edit_actions.cpp $(APPDIR_DWRITE)/presentation/input/menu_view_physics_window_actions.cpp $(APPDIR_DWRITE)/presentation/input/menu_help_actions.cpp $(APPDIR_DWRITE)/presentation/input/menu_model.cpp
SANDBOX_ICON_RC = $(APPDIR_DWRITE)/app_icon.rc
SANDBOX_ICON_ICO = assets/icons/physics_sandbox.ico
SANDBOX_ICON_OBJ = $(OBJDIR)/app_icon.res.o
TEST_SRC = $(TESTDIR)/regression_tests.cpp $(TESTDIR)/regression_collision_core_tests.cpp $(TESTDIR)/regression_event_snapshot_tests.cpp $(TESTDIR)/regression_pipeline_error_tests.cpp $(TESTDIR)/regression_sleep_broadphase_tests.cpp $(TESTDIR)/regression_stress_constraint_tests.cpp $(TESTDIR)/regression_engine_feature_tests.cpp $(TESTDIR)/equivalence/equivalence_api_tests.cpp $(TESTDIR)/regression_scene_schema_tests.cpp $(TESTDIR)/regression_prefab_schema_tests.cpp $(TESTDIR)/regression_project_workspace_tests.cpp $(TESTDIR)/regression_prefab_semantics_tests.cpp $(TESTDIR)/regression_editor_plugin_tests.cpp $(TESTDIR)/regression_session_recovery_tests.cpp $(TESTDIR)/regression_asset_database_tests.cpp $(TESTDIR)/regression_asset_importer_tests.cpp $(TESTDIR)/regression_asset_pipeline_tests.cpp $(TESTDIR)/regression_asset_hot_reload_tests.cpp
KERNEL_TEST_SRC = $(TESTDIR)/kernel_regression_runner.cpp $(TESTDIR)/regression_collision_core_tests.cpp $(TESTDIR)/regression_event_snapshot_tests.cpp $(TESTDIR)/regression_pipeline_error_tests.cpp $(TESTDIR)/regression_sleep_broadphase_tests.cpp $(TESTDIR)/regression_stress_constraint_tests.cpp $(TESTDIR)/regression_engine_feature_tests.cpp $(TESTDIR)/equivalence/equivalence_api_tests.cpp
CONTENT_TEST_SRC = $(TESTDIR)/content_regression_runner.cpp $(TESTDIR)/regression_scene_schema_tests.cpp $(TESTDIR)/regression_prefab_schema_tests.cpp $(TESTDIR)/regression_project_workspace_tests.cpp $(TESTDIR)/regression_prefab_semantics_tests.cpp $(TESTDIR)/regression_editor_plugin_tests.cpp $(TESTDIR)/regression_session_recovery_tests.cpp $(TESTDIR)/regression_asset_database_tests.cpp $(TESTDIR)/regression_asset_importer_tests.cpp $(TESTDIR)/regression_asset_pipeline_tests.cpp $(TESTDIR)/regression_asset_hot_reload_tests.cpp
BENCH_SRC = $(TESTDIR)/benchmark_suite.cpp
INVARIANT_SRC = $(TESTDIR)/invariant_tests.cpp
APP_RUNTIME_SMOKE_SRC = $(TESTDIR)/app_runtime_tick_smoke.cpp $(APPDIR_DWRITE)/domain/app_command.cpp $(APPDIR_DWRITE)/infrastructure/app_event_bus.cpp $(APPDIR_DWRITE)/application/app_controller.cpp $(APPDIR_DWRITE)/application/app_runtime.cpp
EDITOR_UNDO_REDO_SMOKE_SRC = $(TESTDIR)/editor_undo_redo_smoke.cpp $(APPDIR_DWRITE)/application/editor_command_bus.cpp $(APPDIR_DWRITE)/application/history_service.cpp
EDITOR_PIE_LIFECYCLE_SMOKE_SRC = $(TESTDIR)/editor_pie_lifecycle_smoke.cpp $(APPDIR_DWRITE)/application/pie_lifecycle.cpp
SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_SRC = $(TESTDIR)/subsystem_render_audio_animation_smoke.cpp
EDITOR_EXTENSION_STATE_SERVICE_SMOKE_SRC = $(TESTDIR)/editor_extension_state_service_smoke.cpp $(APPDIR_DWRITE)/infrastructure/app_path.cpp $(APPDIR_DWRITE)/application/editor_extension_state_service.cpp $(APPDIR_DWRITE)/application/editor_extension_host_service.cpp
WORKBENCH_SERVICE_SMOKE_SRC = $(TESTDIR)/editor/workbench_service_smoke.cpp $(APPDIR_DWRITE)/domain/app_command.cpp $(APPDIR_DWRITE)/infrastructure/app_event_bus.cpp $(APPDIR_DWRITE)/application/app_controller.cpp $(APPDIR_DWRITE)/application/app_runtime.cpp $(APPDIR_DWRITE)/application/workbench/workbench_service.cpp
PHYSICS_CORE_HEADER_SMOKE_SRC = $(TESTDIR)/physics_core_header_smoke.cpp
CPP_SCRIPT_BRIDGE_SMOKE_SRC = $(TESTDIR)/cpp_script_bridge_smoke.cpp
CPP_FOUNDATION_SMOKE_SRC = $(TESTDIR)/cpp_foundation_smoke.cpp
PARALLEL_BENCHMARK_COMPARE_SRC = tools/parallel_benchmark_compare.cpp
SUBSYSTEM_WORKFLOW_DEMO_SRC = tools/subsystem_workflow_demo.cpp
PHASE_D_PROFILE_CAPTURE_SRC = tools/phase_d_profile_capture.cpp
SCENE_MIGRATE_SRC = tools/scene_migrate_main.cpp
RUNTIME_CLI_SRC = apps/runtime_cli/main.cpp

KERNEL_LIB = $(LIBDIR)/libphysics_kernel.a
CONTENT_LIB = $(LIBDIR)/libphysics_content.a
RUNTIME_SUPPORT_LIB = $(LIBDIR)/libphysics_runtime_support.a
CORE_LIBS = $(CONTENT_LIB) $(RUNTIME_SUPPORT_LIB) $(KERNEL_LIB)
SANDBOX_EXECUTABLE = $(BINDIR)/physics_sandbox
TEST_EXECUTABLE = $(BINDIR)/physics_tests
KERNEL_TEST_EXECUTABLE = $(BINDIR)/physics_kernel_tests
CONTENT_TEST_EXECUTABLE = $(BINDIR)/physics_content_tests
BENCH_EXECUTABLE = $(BINDIR)/physics_bench
INVARIANT_EXECUTABLE = $(BINDIR)/physics_invariants
APP_RUNTIME_SMOKE_EXECUTABLE = $(BINDIR)/app_runtime_smoke
EDITOR_UNDO_REDO_SMOKE_EXECUTABLE = $(BINDIR)/editor_undo_redo_smoke
EDITOR_PIE_LIFECYCLE_SMOKE_EXECUTABLE = $(BINDIR)/editor_pie_lifecycle_smoke
SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_EXECUTABLE = $(BINDIR)/subsystem_render_audio_animation_smoke
EDITOR_EXTENSION_STATE_SERVICE_SMOKE_EXECUTABLE = $(BINDIR)/editor_extension_state_service_smoke
WORKBENCH_SERVICE_SMOKE_EXECUTABLE = $(BINDIR)/workbench_service_smoke
PHYSICS_CORE_HEADER_SMOKE_EXECUTABLE = $(BINDIR)/physics_core_header_smoke
CPP_SCRIPT_BRIDGE_SMOKE_EXECUTABLE = $(BINDIR)/cpp_script_bridge_smoke
CPP_FOUNDATION_SMOKE_EXECUTABLE = $(BINDIR)/cpp_foundation_smoke
PARALLEL_BENCHMARK_COMPARE_EXECUTABLE = $(BINDIR)/parallel_benchmark_compare
SUBSYSTEM_WORKFLOW_DEMO_EXECUTABLE = $(BINDIR)/subsystem_workflow_demo
PHASE_D_PROFILE_CAPTURE_EXECUTABLE = $(BINDIR)/phase_d_profile_capture
SCENE_MIGRATE_EXECUTABLE = $(BINDIR)/scene_migrate
RUNTIME_CLI_EXECUTABLE = $(BINDIR)/physics_runtime_cli

WIN_DWRITE_LIBS = -ld2d1 -ldwrite -lwindowscodecs -lole32 -luuid -lshcore -lgdi32 -luser32
WIN_GUI_FLAGS = -mwindows

all: sandbox

$(BINDIR):
	powershell -NoProfile -Command "New-Item -ItemType Directory -Force '$(BINDIR)' | Out-Null"

$(OBJDIR):
	powershell -NoProfile -Command "New-Item -ItemType Directory -Force '$(OBJDIR)' | Out-Null"

$(LIBDIR):
	powershell -NoProfile -Command "New-Item -ItemType Directory -Force '$(LIBDIR)' | Out-Null"

$(CPP_OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	powershell -NoProfile -Command "New-Item -ItemType Directory -Force '$(dir $@)' | Out-Null"
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(KERNEL_LIB): $(C_OBJECTS) $(PHYSICS_KERNEL_OBJECTS) | $(LIBDIR)
	$(AR) rcs $@ $^

$(CONTENT_LIB): $(PHYSICS_CONTENT_OBJECTS) | $(LIBDIR)
	$(AR) rcs $@ $^

$(RUNTIME_SUPPORT_LIB): $(PHYSICS_RUNTIME_SUPPORT_OBJECTS) | $(LIBDIR)
	$(AR) rcs $@ $^

$(SANDBOX_ICON_OBJ): $(SANDBOX_ICON_RC) $(SANDBOX_ICON_ICO) | $(OBJDIR)
	windres $< -O coff -o $@

$(SANDBOX_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(RUNTIME_SUPPORT_LIB) $(SANDBOX_SRCS) $(SANDBOX_ICON_OBJ) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SANDBOX_SRCS) $(SANDBOX_ICON_OBJ) $(CORE_LIBS) -o $@ $(LDFLAGS) $(WIN_DWRITE_LIBS) $(WIN_GUI_FLAGS)

$(TEST_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(TEST_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(TEST_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(KERNEL_TEST_EXECUTABLE): $(KERNEL_LIB) $(KERNEL_TEST_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(KERNEL_TEST_SRC) $(KERNEL_LIB) -o $@ $(LDFLAGS)

$(CONTENT_TEST_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(CONTENT_TEST_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(CONTENT_TEST_SRC) $(CONTENT_LIB) $(KERNEL_LIB) -o $@ $(LDFLAGS)

$(BENCH_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(BENCH_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(BENCH_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(INVARIANT_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(INVARIANT_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INVARIANT_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(APP_RUNTIME_SMOKE_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(APP_RUNTIME_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(APP_RUNTIME_SMOKE_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(EDITOR_UNDO_REDO_SMOKE_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(EDITOR_UNDO_REDO_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(EDITOR_UNDO_REDO_SMOKE_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(EDITOR_PIE_LIFECYCLE_SMOKE_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(EDITOR_PIE_LIFECYCLE_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(EDITOR_PIE_LIFECYCLE_SMOKE_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(EDITOR_EXTENSION_STATE_SERVICE_SMOKE_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(EDITOR_EXTENSION_STATE_SERVICE_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(EDITOR_EXTENSION_STATE_SERVICE_SMOKE_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(WORKBENCH_SERVICE_SMOKE_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(WORKBENCH_SERVICE_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(WORKBENCH_SERVICE_SMOKE_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(PHYSICS_CORE_HEADER_SMOKE_EXECUTABLE): $(KERNEL_LIB) $(RUNTIME_SUPPORT_LIB) $(PHYSICS_CORE_HEADER_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(PHYSICS_CORE_HEADER_SMOKE_SRC) $(RUNTIME_SUPPORT_LIB) $(KERNEL_LIB) -o $@ $(LDFLAGS)

$(CPP_SCRIPT_BRIDGE_SMOKE_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(CPP_SCRIPT_BRIDGE_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(CPP_SCRIPT_BRIDGE_SMOKE_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(CPP_FOUNDATION_SMOKE_EXECUTABLE): $(KERNEL_LIB) $(CPP_FOUNDATION_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(CPP_FOUNDATION_SMOKE_SRC) $(KERNEL_LIB) -o $@ $(LDFLAGS)

$(PARALLEL_BENCHMARK_COMPARE_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(PARALLEL_BENCHMARK_COMPARE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(PARALLEL_BENCHMARK_COMPARE_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(SUBSYSTEM_WORKFLOW_DEMO_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(SUBSYSTEM_WORKFLOW_DEMO_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SUBSYSTEM_WORKFLOW_DEMO_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(PHASE_D_PROFILE_CAPTURE_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(PHASE_D_PROFILE_CAPTURE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(PHASE_D_PROFILE_CAPTURE_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(SCENE_MIGRATE_EXECUTABLE): $(KERNEL_LIB) $(CONTENT_LIB) $(SCENE_MIGRATE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(SCENE_MIGRATE_SRC) $(CORE_LIBS) -o $@ $(LDFLAGS)

$(RUNTIME_CLI_EXECUTABLE): $(KERNEL_LIB) $(RUNTIME_SUPPORT_LIB) $(RUNTIME_CLI_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(RUNTIME_CLI_SRC) $(RUNTIME_SUPPORT_LIB) $(KERNEL_LIB) -o $@ $(LDFLAGS)

core: $(KERNEL_LIB) $(CONTENT_LIB) $(RUNTIME_SUPPORT_LIB)

sandbox: $(SANDBOX_EXECUTABLE)

scene-migrate: $(SCENE_MIGRATE_EXECUTABLE)

runtime-cli: $(RUNTIME_CLI_EXECUTABLE)

package-sandbox: $(SANDBOX_EXECUTABLE)
	powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\package\package.ps1 -Version 1.0.0 -Flavor editor

run: $(SANDBOX_EXECUTABLE)
	./$(SANDBOX_EXECUTABLE)

test: $(KERNEL_TEST_EXECUTABLE) $(CONTENT_TEST_EXECUTABLE) $(TEST_EXECUTABLE) $(APP_RUNTIME_SMOKE_EXECUTABLE) $(EDITOR_UNDO_REDO_SMOKE_EXECUTABLE) $(EDITOR_PIE_LIFECYCLE_SMOKE_EXECUTABLE) $(SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_EXECUTABLE) $(EDITOR_EXTENSION_STATE_SERVICE_SMOKE_EXECUTABLE) $(WORKBENCH_SERVICE_SMOKE_EXECUTABLE) $(PHYSICS_CORE_HEADER_SMOKE_EXECUTABLE) $(CPP_SCRIPT_BRIDGE_SMOKE_EXECUTABLE) $(CPP_FOUNDATION_SMOKE_EXECUTABLE)
	./$(KERNEL_TEST_EXECUTABLE)
	./$(CONTENT_TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)
	./$(APP_RUNTIME_SMOKE_EXECUTABLE)
	./$(EDITOR_UNDO_REDO_SMOKE_EXECUTABLE)
	./$(EDITOR_PIE_LIFECYCLE_SMOKE_EXECUTABLE)
	./$(SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_EXECUTABLE)
	./$(EDITOR_EXTENSION_STATE_SERVICE_SMOKE_EXECUTABLE)
	./$(WORKBENCH_SERVICE_SMOKE_EXECUTABLE)
	./$(PHYSICS_CORE_HEADER_SMOKE_EXECUTABLE)
	./$(CPP_SCRIPT_BRIDGE_SMOKE_EXECUTABLE)
	./$(CPP_FOUNDATION_SMOKE_EXECUTABLE)

benchmark: $(BENCH_EXECUTABLE)
	./$(BENCH_EXECUTABLE)

parallel-benchmark-compare: $(PARALLEL_BENCHMARK_COMPARE_EXECUTABLE)
	./$(PARALLEL_BENCHMARK_COMPARE_EXECUTABLE)

subsystem-workflow-demo: $(SUBSYSTEM_WORKFLOW_DEMO_EXECUTABLE)
	./$(SUBSYSTEM_WORKFLOW_DEMO_EXECUTABLE)

phase-d-profile: $(PHASE_D_PROFILE_CAPTURE_EXECUTABLE)
	./$(PHASE_D_PROFILE_CAPTURE_EXECUTABLE)

test-long: $(INVARIANT_EXECUTABLE)
	./$(INVARIANT_EXECUTABLE)

check-core:
	powershell -ExecutionPolicy Bypass -File .\scripts\check_core_change.ps1

check-arch:
	powershell -ExecutionPolicy Bypass -File .\scripts\check_arch_deps.ps1

check-api:
	powershell -ExecutionPolicy Bypass -File .\scripts\check_api_surface.ps1

check-cpp-convergence:
	powershell -ExecutionPolicy Bypass -File .\scripts\check_cpp_convergence.ps1

clean:
	if exist $(BINDIR) rmdir /S /Q $(BINDIR)
	if exist $(OBJDIR) rmdir /S /Q $(OBJDIR)
	if exist $(LIBDIR) rmdir /S /Q $(LIBDIR)

-include $(DEPS)

.PHONY: all core sandbox scene-migrate runtime-cli package-sandbox run test benchmark parallel-benchmark-compare subsystem-workflow-demo phase-d-profile test-long check-core check-arch check-api check-cpp-convergence clean
