CC = gcc
CXX = g++
AR = ar
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -finput-charset=UTF-8 -fexec-charset=UTF-8 -Iinclude -Isrc
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++17 -finput-charset=UTF-8 -fexec-charset=UTF-8 -Iinclude -Isrc -DPHYSICS_USE_CPP_KERNEL=1
LDFLAGS = -lm
DEPFLAGS = -MMD -MP
SRCDIR = src
APPDIR_DWRITE = apps/sandbox_dwrite
TESTDIR = tests
BINDIR = bin
OBJDIR = obj
LIBDIR = lib

C_SOURCES =
CPP_SOURCES = $(SRCDIR)/physics2d/math.cpp $(SRCDIR)/physics2d/shape.cpp $(SRCDIR)/physics2d/body.cpp $(SRCDIR)/physics2d/constraint.cpp $(SRCDIR)/physics2d/collision.cpp $(SRCDIR)/physics2d/collision_detect.cpp $(SRCDIR)/core/physics_memory.cpp $(SRCDIR)/core/physics_ids.cpp $(SRCDIR)/core/physics_parallel.cpp $(SRCDIR)/core/physics_world.cpp $(SRCDIR)/physics2d/physics_raycast.cpp $(SRCDIR)/core/physics_snapshot.cpp $(SRCDIR)/physics2d/physics_integrate.cpp $(SRCDIR)/physics2d/physics_resolve.cpp $(SRCDIR)/core/physics_step.cpp $(SRCDIR)/physics2d/physics_broadphase.cpp $(SRCDIR)/physics2d/physics_collision_pipeline.cpp $(SRCDIR)/physics2d/physics_contact_manager.cpp $(SRCDIR)/physics2d/physics_ccd.cpp $(SRCDIR)/c_api/physics_pipeline_api.cpp $(SRCDIR)/physics2d/physics_solver.cpp $(SRCDIR)/c_api/physics_query.cpp $(SRCDIR)/c_api/physics_mutation.cpp $(SRCDIR)/c_api/physics.cpp $(SRCDIR)/c_api/physics_lifecycle.cpp $(SRCDIR)/c_api/physics_runtime_api.cpp $(SRCDIR)/c_api/physics_config.cpp
C_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(C_SOURCES))
CPP_OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(CPP_SOURCES))
OBJECTS = $(C_OBJECTS) $(CPP_OBJECTS)
DEPS = $(OBJECTS:.o=.d)

SANDBOX_SRCS = $(APPDIR_DWRITE)/main.c $(APPDIR_DWRITE)/infrastructure/project_tree.c $(APPDIR_DWRITE)/infrastructure/snapshot_repo.c $(APPDIR_DWRITE)/infrastructure/ui_layout_repo.c $(APPDIR_DWRITE)/presentation/render/ui_icons.c $(APPDIR_DWRITE)/presentation/render/ui_text.c $(APPDIR_DWRITE)/presentation/render/ui_primitives.c $(APPDIR_DWRITE)/presentation/render/ui_widgets.c $(APPDIR_DWRITE)/domain/app_command.c $(APPDIR_DWRITE)/infrastructure/app_event_bus.c $(APPDIR_DWRITE)/application/app_controller.c $(APPDIR_DWRITE)/application/app_runtime.c $(APPDIR_DWRITE)/application/scene_catalog.c $(APPDIR_DWRITE)/application/scene_builder.c $(APPDIR_DWRITE)/application/history_service.c $(APPDIR_DWRITE)/application/runtime_param_service.c $(APPDIR_DWRITE)/presentation/input/input_mapping.c $(APPDIR_DWRITE)/presentation/input/menu_file_edit_actions.c $(APPDIR_DWRITE)/presentation/input/menu_view_physics_window_actions.c $(APPDIR_DWRITE)/presentation/input/menu_help_actions.c $(APPDIR_DWRITE)/presentation/input/menu_model.c
SANDBOX_ICON_RC = $(APPDIR_DWRITE)/app_icon.rc
SANDBOX_ICON_ICO = assets/icons/physics_sandbox.ico
SANDBOX_ICON_OBJ = $(OBJDIR)/app_icon.res.o
TEST_SRC = $(TESTDIR)/regression_tests.c $(TESTDIR)/regression_collision_core_tests.c $(TESTDIR)/regression_event_snapshot_tests.c $(TESTDIR)/regression_pipeline_error_tests.c $(TESTDIR)/regression_sleep_broadphase_tests.c $(TESTDIR)/regression_stress_constraint_tests.c $(TESTDIR)/regression_engine_feature_tests.c
BENCH_SRC = $(TESTDIR)/benchmark_suite.c
INVARIANT_SRC = $(TESTDIR)/invariant_tests.c

CORE_LIB = $(LIBDIR)/libphysics2d.a
SANDBOX_EXECUTABLE = $(BINDIR)/physics_sandbox
TEST_EXECUTABLE = $(BINDIR)/physics_tests
BENCH_EXECUTABLE = $(BINDIR)/physics_bench
INVARIANT_EXECUTABLE = $(BINDIR)/physics_invariants

WIN_DWRITE_LIBS = -ld2d1 -ldwrite -lwindowscodecs -lole32 -luuid -lshcore -lgdi32 -luser32
WIN_GUI_FLAGS = -mwindows

all: sandbox

$(BINDIR):
	if not exist $(BINDIR) mkdir $(BINDIR)

$(OBJDIR):
	if not exist $(OBJDIR) mkdir $(OBJDIR)

$(LIBDIR):
	if not exist $(LIBDIR) mkdir $(LIBDIR)

$(C_OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	powershell -NoProfile -Command "New-Item -ItemType Directory -Force '$(dir $@)' | Out-Null"
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(CPP_OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	powershell -NoProfile -Command "New-Item -ItemType Directory -Force '$(dir $@)' | Out-Null"
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(CORE_LIB): $(OBJECTS) | $(LIBDIR)
	$(AR) rcs $@ $^

$(SANDBOX_ICON_OBJ): $(SANDBOX_ICON_RC) $(SANDBOX_ICON_ICO) | $(OBJDIR)
	windres $< -O coff -o $@

$(SANDBOX_EXECUTABLE): $(CORE_LIB) $(SANDBOX_SRCS) $(SANDBOX_ICON_OBJ) | $(BINDIR)
	$(CC) $(CFLAGS) $(SANDBOX_SRCS) $(SANDBOX_ICON_OBJ) $(CORE_LIB) -o $@ $(LDFLAGS) $(WIN_DWRITE_LIBS) $(WIN_GUI_FLAGS)

$(TEST_EXECUTABLE): $(CORE_LIB) $(TEST_SRC) | $(BINDIR)
	$(CC) $(CFLAGS) $(TEST_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(BENCH_EXECUTABLE): $(CORE_LIB) $(BENCH_SRC) | $(BINDIR)
	$(CC) $(CFLAGS) $(BENCH_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

$(INVARIANT_EXECUTABLE): $(CORE_LIB) $(INVARIANT_SRC) | $(BINDIR)
	$(CC) $(CFLAGS) $(INVARIANT_SRC) $(CORE_LIB) -o $@ $(LDFLAGS)

core: $(CORE_LIB)

sandbox: $(SANDBOX_EXECUTABLE)

run: $(SANDBOX_EXECUTABLE)
	./$(SANDBOX_EXECUTABLE)

test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

benchmark: $(BENCH_EXECUTABLE)
	./$(BENCH_EXECUTABLE)

test-long: $(INVARIANT_EXECUTABLE)
	./$(INVARIANT_EXECUTABLE)

check-core:
	powershell -ExecutionPolicy Bypass -File .\scripts\check_core_change.ps1

check-arch:
	powershell -ExecutionPolicy Bypass -File .\scripts\check_arch_deps.ps1

check-api:
	powershell -ExecutionPolicy Bypass -File .\scripts\check_api_surface.ps1

clean:
	if exist $(BINDIR) rmdir /S /Q $(BINDIR)
	if exist $(OBJDIR) rmdir /S /Q $(OBJDIR)
	if exist $(LIBDIR) rmdir /S /Q $(LIBDIR)

-include $(DEPS)

.PHONY: all core sandbox run test benchmark test-long check-core check-arch check-api clean
