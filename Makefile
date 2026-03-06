CXX = g++
AR = ar
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++17 -finput-charset=UTF-8 -fexec-charset=UTF-8 -Iinclude -Isrc -Icpp -include apps/sandbox_dwrite/infrastructure/com_cpp_compat.hpp -DPHYSICS_USE_CPP_KERNEL=1 -DCOBJMACROS
LDFLAGS = -lm
DEPFLAGS = -MMD -MP
SRCDIR = src
APPDIR_DWRITE = apps/sandbox_dwrite
TESTDIR = tests
BINDIR = bin
OBJDIR = obj
LIBDIR = lib

C_SOURCES =
CPP_SOURCES = $(SRCDIR)/content/scene_schema.cpp $(SRCDIR)/content/prefab_schema.cpp $(SRCDIR)/content/asset_database.cpp $(SRCDIR)/content/asset_invalidation.cpp $(SRCDIR)/content/asset_pipeline.cpp $(SRCDIR)/content/asset_watch.cpp $(SRCDIR)/content/asset_hot_reload.cpp $(SRCDIR)/content/asset_fs_poll.cpp $(SRCDIR)/content/asset_fs_watch.cpp $(SRCDIR)/content/asset_importer.cpp $(SRCDIR)/content/subsystem_render_audio_animation.cpp $(SRCDIR)/physics2d/math.cpp $(SRCDIR)/physics2d/shape.cpp $(SRCDIR)/physics2d/body.cpp $(SRCDIR)/physics2d/constraint.cpp $(SRCDIR)/physics2d/collision.cpp $(SRCDIR)/physics2d/collision_detect.cpp $(SRCDIR)/core/physics_memory.cpp $(SRCDIR)/core/physics_ids.cpp $(SRCDIR)/core/physics_parallel.cpp $(SRCDIR)/core/physics_world.cpp $(SRCDIR)/physics2d/physics_raycast.cpp $(SRCDIR)/core/physics_snapshot.cpp $(SRCDIR)/physics2d/physics_integrate.cpp $(SRCDIR)/physics2d/physics_resolve.cpp $(SRCDIR)/core/physics_step.cpp $(SRCDIR)/physics2d/physics_broadphase.cpp $(SRCDIR)/physics2d/physics_collision_pipeline.cpp $(SRCDIR)/physics2d/physics_contact_manager.cpp $(SRCDIR)/physics2d/physics_ccd.cpp $(SRCDIR)/c_api/physics_pipeline_api.cpp $(SRCDIR)/physics2d/physics_solver.cpp $(SRCDIR)/c_api/physics_query.cpp $(SRCDIR)/c_api/physics_mutation.cpp $(SRCDIR)/c_api/physics.cpp $(SRCDIR)/c_api/physics_lifecycle.cpp $(SRCDIR)/c_api/physics_runtime_api.cpp $(SRCDIR)/c_api/physics_config.cpp
C_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(C_SOURCES))
CPP_OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(CPP_SOURCES))
OBJECTS = $(C_OBJECTS) $(CPP_OBJECTS)
DEPS = $(OBJECTS:.o=.d)

SANDBOX_SRCS = $(APPDIR_DWRITE)/main.cpp $(APPDIR_DWRITE)/infrastructure/project_tree.cpp $(APPDIR_DWRITE)/infrastructure/snapshot_repo.cpp $(APPDIR_DWRITE)/infrastructure/ui_layout_repo.cpp $(APPDIR_DWRITE)/presentation/render/ui_icons.cpp $(APPDIR_DWRITE)/presentation/render/ui_text.cpp $(APPDIR_DWRITE)/presentation/render/ui_primitives.cpp $(APPDIR_DWRITE)/presentation/render/ui_widgets.cpp $(APPDIR_DWRITE)/domain/app_command.cpp $(APPDIR_DWRITE)/infrastructure/app_event_bus.cpp $(APPDIR_DWRITE)/application/app_controller.cpp $(APPDIR_DWRITE)/application/app_runtime.cpp $(APPDIR_DWRITE)/application/pie_lifecycle.cpp $(APPDIR_DWRITE)/application/editor_command_bus.cpp $(APPDIR_DWRITE)/application/scene_catalog.cpp $(APPDIR_DWRITE)/application/scene_builder.cpp $(APPDIR_DWRITE)/application/history_service.cpp $(APPDIR_DWRITE)/application/runtime_param_service.cpp $(APPDIR_DWRITE)/presentation/input/input_mapping.cpp $(APPDIR_DWRITE)/presentation/input/menu_file_edit_actions.cpp $(APPDIR_DWRITE)/presentation/input/menu_view_physics_window_actions.cpp $(APPDIR_DWRITE)/presentation/input/menu_help_actions.cpp $(APPDIR_DWRITE)/presentation/input/menu_model.cpp
SANDBOX_ICON_RC = $(APPDIR_DWRITE)/app_icon.rc
SANDBOX_ICON_ICO = assets/icons/physics_sandbox.ico
SANDBOX_ICON_OBJ = $(OBJDIR)/app_icon.res.o
TEST_SRC = $(TESTDIR)/regression_tests.cpp $(TESTDIR)/regression_collision_core_tests.cpp $(TESTDIR)/regression_event_snapshot_tests.cpp $(TESTDIR)/regression_pipeline_error_tests.cpp $(TESTDIR)/regression_sleep_broadphase_tests.cpp $(TESTDIR)/regression_stress_constraint_tests.cpp $(TESTDIR)/regression_engine_feature_tests.cpp $(TESTDIR)/equivalence/equivalence_api_tests.cpp $(TESTDIR)/regression_scene_schema_tests.cpp $(TESTDIR)/regression_prefab_schema_tests.cpp $(TESTDIR)/regression_asset_database_tests.cpp $(TESTDIR)/regression_asset_importer_tests.cpp $(TESTDIR)/regression_asset_pipeline_tests.cpp $(TESTDIR)/regression_asset_hot_reload_tests.cpp
BENCH_SRC = $(TESTDIR)/benchmark_suite.cpp
INVARIANT_SRC = $(TESTDIR)/invariant_tests.cpp
APP_RUNTIME_SMOKE_SRC = $(TESTDIR)/app_runtime_tick_smoke.cpp $(APPDIR_DWRITE)/domain/app_command.cpp $(APPDIR_DWRITE)/infrastructure/app_event_bus.cpp $(APPDIR_DWRITE)/application/app_controller.cpp $(APPDIR_DWRITE)/application/app_runtime.cpp
EDITOR_UNDO_REDO_SMOKE_SRC = $(TESTDIR)/editor_undo_redo_smoke.cpp $(APPDIR_DWRITE)/application/editor_command_bus.cpp $(APPDIR_DWRITE)/application/history_service.cpp
EDITOR_PIE_LIFECYCLE_SMOKE_SRC = $(TESTDIR)/editor_pie_lifecycle_smoke.cpp $(APPDIR_DWRITE)/application/pie_lifecycle.cpp
SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_SRC = $(TESTDIR)/subsystem_render_audio_animation_smoke.cpp
CPP_SCRIPT_BRIDGE_SMOKE_SRC = $(TESTDIR)/cpp_script_bridge_smoke.cpp
CPP_FOUNDATION_SMOKE_SRC = $(TESTDIR)/cpp_foundation_smoke.cpp
PARALLEL_BENCHMARK_COMPARE_SRC = tools/parallel_benchmark_compare.cpp
SUBSYSTEM_WORKFLOW_DEMO_SRC = tools/subsystem_workflow_demo.cpp
PHASE_D_PROFILE_CAPTURE_SRC = tools/phase_d_profile_capture.cpp
SCENE_MIGRATE_SRC = tools/scene_migrate_main.cpp

CORE_LIB = $(LIBDIR)/libphysics2d.a
SANDBOX_EXECUTABLE = $(BINDIR)/physics_sandbox
TEST_EXECUTABLE = $(BINDIR)/physics_tests
BENCH_EXECUTABLE = $(BINDIR)/physics_bench
INVARIANT_EXECUTABLE = $(BINDIR)/physics_invariants
APP_RUNTIME_SMOKE_EXECUTABLE = $(BINDIR)/app_runtime_smoke
EDITOR_UNDO_REDO_SMOKE_EXECUTABLE = $(BINDIR)/editor_undo_redo_smoke
EDITOR_PIE_LIFECYCLE_SMOKE_EXECUTABLE = $(BINDIR)/editor_pie_lifecycle_smoke
SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_EXECUTABLE = $(BINDIR)/subsystem_render_audio_animation_smoke
CPP_SCRIPT_BRIDGE_SMOKE_EXECUTABLE = $(BINDIR)/cpp_script_bridge_smoke
CPP_FOUNDATION_SMOKE_EXECUTABLE = $(BINDIR)/cpp_foundation_smoke
PARALLEL_BENCHMARK_COMPARE_EXECUTABLE = $(BINDIR)/parallel_benchmark_compare
SUBSYSTEM_WORKFLOW_DEMO_EXECUTABLE = $(BINDIR)/subsystem_workflow_demo
PHASE_D_PROFILE_CAPTURE_EXECUTABLE = $(BINDIR)/phase_d_profile_capture
SCENE_MIGRATE_EXECUTABLE = $(BINDIR)/scene_migrate

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

$(CORE_LIB): $(OBJECTS) | $(LIBDIR)
	$(AR) rcs $@ $^

$(SANDBOX_ICON_OBJ): $(SANDBOX_ICON_RC) $(SANDBOX_ICON_ICO) | $(OBJDIR)
	windres $< -O coff -o $@

$(SANDBOX_EXECUTABLE): $(CORE_LIB) $(SANDBOX_SRCS) $(SANDBOX_ICON_OBJ) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(SANDBOX_SRCS) $(SANDBOX_ICON_OBJ) $(CORE_LIB) -o $@ $(LDFLAGS) $(WIN_DWRITE_LIBS) $(WIN_GUI_FLAGS)

$(TEST_EXECUTABLE): $(CORE_LIB) $(TEST_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(TEST_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(BENCH_EXECUTABLE): $(CORE_LIB) $(BENCH_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(BENCH_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(INVARIANT_EXECUTABLE): $(CORE_LIB) $(INVARIANT_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(INVARIANT_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(APP_RUNTIME_SMOKE_EXECUTABLE): $(CORE_LIB) $(APP_RUNTIME_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(APP_RUNTIME_SMOKE_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(EDITOR_UNDO_REDO_SMOKE_EXECUTABLE): $(CORE_LIB) $(EDITOR_UNDO_REDO_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(EDITOR_UNDO_REDO_SMOKE_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(EDITOR_PIE_LIFECYCLE_SMOKE_EXECUTABLE): $(CORE_LIB) $(EDITOR_PIE_LIFECYCLE_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(EDITOR_PIE_LIFECYCLE_SMOKE_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_EXECUTABLE): $(CORE_LIB) $(SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(CPP_SCRIPT_BRIDGE_SMOKE_EXECUTABLE): $(CORE_LIB) $(CPP_SCRIPT_BRIDGE_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(CPP_SCRIPT_BRIDGE_SMOKE_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(CPP_FOUNDATION_SMOKE_EXECUTABLE): $(CORE_LIB) $(CPP_FOUNDATION_SMOKE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(CPP_FOUNDATION_SMOKE_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(PARALLEL_BENCHMARK_COMPARE_EXECUTABLE): $(CORE_LIB) $(PARALLEL_BENCHMARK_COMPARE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(PARALLEL_BENCHMARK_COMPARE_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(SUBSYSTEM_WORKFLOW_DEMO_EXECUTABLE): $(CORE_LIB) $(SUBSYSTEM_WORKFLOW_DEMO_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(SUBSYSTEM_WORKFLOW_DEMO_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(PHASE_D_PROFILE_CAPTURE_EXECUTABLE): $(CORE_LIB) $(PHASE_D_PROFILE_CAPTURE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(PHASE_D_PROFILE_CAPTURE_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(SCENE_MIGRATE_EXECUTABLE): $(CORE_LIB) $(SCENE_MIGRATE_SRC) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -Icpp $(SCENE_MIGRATE_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

core: $(CORE_LIB)

sandbox: $(SANDBOX_EXECUTABLE)

scene-migrate: $(SCENE_MIGRATE_EXECUTABLE)

run: $(SANDBOX_EXECUTABLE)
	./$(SANDBOX_EXECUTABLE)

test: $(TEST_EXECUTABLE) $(APP_RUNTIME_SMOKE_EXECUTABLE) $(EDITOR_UNDO_REDO_SMOKE_EXECUTABLE) $(EDITOR_PIE_LIFECYCLE_SMOKE_EXECUTABLE) $(SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_EXECUTABLE) $(CPP_SCRIPT_BRIDGE_SMOKE_EXECUTABLE) $(CPP_FOUNDATION_SMOKE_EXECUTABLE)
	./$(TEST_EXECUTABLE)
	./$(APP_RUNTIME_SMOKE_EXECUTABLE)
	./$(EDITOR_UNDO_REDO_SMOKE_EXECUTABLE)
	./$(EDITOR_PIE_LIFECYCLE_SMOKE_EXECUTABLE)
	./$(SUBSYSTEM_RENDER_AUDIO_ANIMATION_SMOKE_EXECUTABLE)
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

.PHONY: all core sandbox scene-migrate run test benchmark parallel-benchmark-compare subsystem-workflow-demo phase-d-profile test-long check-core check-arch check-api check-cpp-convergence clean
