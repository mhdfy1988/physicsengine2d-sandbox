CC = gcc
CXX = g++
AR = ar
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -finput-charset=UTF-8 -fexec-charset=UTF-8 -Iinclude
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++17 -finput-charset=UTF-8 -fexec-charset=UTF-8 -Iinclude -DPHYSICS_USE_CPP_KERNEL=1
LDFLAGS = -lm
DEPFLAGS = -MMD -MP
SRCDIR = src
APPDIR_DWRITE = apps/sandbox_dwrite
TESTDIR = tests
BINDIR = bin
OBJDIR = obj
LIBDIR = lib

C_SOURCES = $(SRCDIR)/math.c $(SRCDIR)/shape.c $(SRCDIR)/body.c $(SRCDIR)/constraint.c $(SRCDIR)/collision.c $(SRCDIR)/collision_detect.c $(SRCDIR)/physics.c $(SRCDIR)/physics_lifecycle.c $(SRCDIR)/physics_runtime_api.c $(SRCDIR)/physics_config.c $(SRCDIR)/physics_query.c $(SRCDIR)/physics_mutation.c $(SRCDIR)/physics_pipeline_api.c $(SRCDIR)/physics_broadphase.c $(SRCDIR)/physics_contact_manager.c $(SRCDIR)/physics_ccd.c $(SRCDIR)/physics_step.c $(SRCDIR)/physics_collision_pipeline.c $(SRCDIR)/physics_resolve.c $(SRCDIR)/physics_solver.c
CPP_SOURCES = $(SRCDIR)/physics_memory.cpp $(SRCDIR)/physics_ids.cpp $(SRCDIR)/physics_parallel.cpp $(SRCDIR)/physics_world.cpp $(SRCDIR)/physics_raycast.cpp $(SRCDIR)/physics_snapshot.cpp $(SRCDIR)/physics_integrate.cpp
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
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(CPP_OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
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
